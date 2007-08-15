/*  This file is part of the Yzis libraries
*  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
*  Copyright (C) 2004 Thomas Capricelli <orzel@freehackers.org>,
*  Copyright (C) 2004 Philippe Fremy <phil@freehackers.org>,
*  Copyright (C) 2004 Pascal "Poizon" Maillard <poizon@gmx.at>,
*  Copyright (C) 2004-2005 Loic Pauleve <panard@inzenet.org>
*  Copyright (C) 2005 Erlend Hamberg <hamberg@stud.ntnu.no>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
**/

/* Yzis */
#include "mode_ex.h"
#include "debug.h"
#include "buffer.h"
#include "folding.h"
#include "session.h"
#include "luaengine.h"
#include "mark.h"
#include "selection.h"
#include "mapping.h"
#include "action.h"
#include "kate/schema.h"
#include "tags_interface.h"
#include "search.h"
#include "internal_options.h"
#include "resourcemgr.h"
#include "view.h"
#include "viewcursor.h"
#include "history.h"

/* Qt */
#include <QFileInfo>
#include <QDir>

using namespace yzis;

#define dbg() yzDebug("YZModeEx")
#define err() yzError("YZModeEx")

YZExCommandArgs::YZExCommandArgs( YZView* _view, const QString& _input, const QString& _cmd, const QString& _arg, unsigned int _fromLine, unsigned int _toLine, bool _force )
{
    input = _input;
    cmd = _cmd;
    arg = _arg;
    view = _view;
    fromLine = _fromLine;
    toLine = _toLine;
    force = _force;
}

QString YZExCommandArgs::toString() const
{
    QString s;
    s += "YZExCommandArgs:\n";
    s += QString().sprintf("view=%p\n", view);
    s += QString("input=%1\n").arg(input);
    s += QString("cmd=%1\n").arg(cmd);
    s += QString("arg=%1\n").arg(arg);
    s += QString("fromLine=%1 toLine=%2\n").arg(fromLine).arg(toLine);
    s += QString("force=%1\n").arg(force);
    return s;
}

YZExRange::YZExRange( const QString& regexp, ExRangeMethod pm )
{
    mKeySeq = regexp;
    mPoolMethod = pm;
    mRegexp = QRegExp( "^(" + mKeySeq + ")([+\\-]\\d*)?(.*)$" );
}

YZExCommand::YZExCommand( const QString& input, ExPoolMethod pm, const QStringList& longName, bool word )
{
    mKeySeq = input;
    mPoolMethod = pm;
    mLongName = longName;
    if ( word ) {
        mRegexp = QRegExp( "^(" + mKeySeq + ")(\\b.*)?$" );
    } else {
        mRegexp = QRegExp( "^(" + mKeySeq + ")([\\w\\s].*)?$" );
    }
}

YZModeEx::YZModeEx() : YZMode()
{
    mType = ModeEx;
    mString = _("[ Ex ]");
    mMapMode = MapCmdline;
    commands.clear();
    ranges.clear();
    mHistory = new YZHistory;
    mCompletePossibilities.clear();
    mCurrentCompletionProposal = 0;
}

YZModeEx::~YZModeEx()
{
    foreach( const YZExCommand *c, commands )
    delete c;
    foreach( const YZExRange *r, ranges )
    delete r;

    delete mHistory;
}
void YZModeEx::init()
{
    initPool();
}
void YZModeEx::enter( YZView* view )
{
    YZSession::self()->guiSetFocusCommandLine();
    view->guiSetCommandLineText( "" );
}

void YZModeEx::leave( YZView* view )
{
    view->guiSetCommandLineText( "" );
    YZSession::self()->guiSetFocusMainWindow();
}

CmdState YZModeEx::execCommand( YZView* view, const QString& key )
{
    // dbg() << "YZModeEx::execCommand " << key << endl;
    CmdState ret = CmdOk;
    if ( key != "<TAB>" ) {
        //clean up the whole completion stuff
        resetCompletion();
    }
    if ( key == "<ENTER>" ) {
        if ( view->guiGetCommandLineText().isEmpty()) {
            view->modePool()->pop();
        } else {
            QString cmd = view->guiGetCommandLineText();
            mHistory->addEntry( cmd );
            ret = execExCommand( view, cmd );
            if ( ret != CmdQuit )
                view->modePool()->pop( ModeCommand );
        }
    } else if ( key == "<DOWN>" ) {
        mHistory->goForwardInTime();
        view->guiSetCommandLineText( mHistory->getEntry() );
    } else if ( key == "<LEFT>" || key == "<RIGHT>" ) {}
    else if ( key == "<UP>" ) {
        mHistory->goBackInTime();
        view->guiSetCommandLineText( mHistory->getEntry() );
    } else if ( key == "<ESC>" || key == "<CTRL>c" ) {
        view->modePool()->pop( ModeCommand );
    } else if ( key == "<TAB>" ) {
        completeCommandLine(view);
    } else if ( key == "<BS>" ) {
        QString back = view->guiGetCommandLineText();
        if ( back.isEmpty() ) {
            view->modePool()->pop();
            return ret;
        }
        view->guiSetCommandLineText(back.remove(back.length() - 1, 1));
    } else {
        view->guiSetCommandLineText(view->guiGetCommandLineText() + key);
    }
    return ret;
}

void YZModeEx::resetCompletion()
{
    mCompletePossibilities.clear();
    mCompletionCurrentSearch = "";
    mCurrentCompletionProposal = 0;
}

const QStringList& YZModeEx::completionList()
{
    return mCompletePossibilities;
}

int YZModeEx::completionIndex()
{
    return mCurrentCompletionProposal -1;
}

const QString& YZModeEx::completionItem(int idx)
{
    return mCompletePossibilities.at(idx);
}

void YZModeEx::completeCommandLine(YZView *view)
{
    QString current = view->guiGetCommandLineText();
    QStringList words = current.split(" ", QString::SkipEmptyParts);

    if ( mCompletePossibilities.isEmpty() ) { //no list set up yet, init the completion stuff before proceding
        //XXX we probably need something to say that a command can get a filename parameter, the following check is very lame for now...
        if ( (current.endsWith(" ") && words.count() == 1) || words.count() == 2 ) { //file name completion : ":command /somefile<TAB>" or ":command <TAB>"
            dbg() << "Init filename completion";
            QDir cDir; //current dir
            QStringList filters; //filename filtering
            if ( words.count() == 2 ) { //we have a prefix to look for
                dbg() << "init filter for completion";
                mCompletionCurrentSearch = words.last();
                dbg() << "Searched item : " << mCompletionCurrentSearch;
                current.chop(mCompletionCurrentSearch.length());
                if (!mCompletionCurrentSearch.endsWith("/")) {
                    QFileInfo fi (mCompletionCurrentSearch);
                    filters << fi.fileName() + "*"; //add the filename part to the filters, this will be used to list only these matches just below
                    dbg() << "Adding filter : " << fi.fileName() + "*";
                    cDir = QDir(fi.path()); //get the parent directory to switch over the entries in this directory
                } else {
                    cDir.cd(mCompletionCurrentSearch);
                }
                //list of files in that directory
                if (cDir.isAbsolute()) {
                    dbg() << "Using absolute paths from dir " << cDir.absolutePath();
                    QFileInfoList ifl = cDir.entryInfoList(filters, QDir::AllEntries | QDir::Dirs | QDir::NoDotAndDotDot);
                    for (int i = 0; i < ifl.size(); ++i) {
                        mCompletePossibilities << ifl.at(i).absoluteFilePath();
                    }
                } else {
                    dbg() << "Using relative paths from dir " << cDir.path();
                    mCompletePossibilities = cDir.entryList(filters, QDir::AllEntries | QDir::Dirs | QDir::NoDotAndDotDot);
                    //prepend the relative path to the entries, so we keep a "full" but still relative path
                    QRegExp rx("^(.*)$");
                    mCompletePossibilities.replaceInStrings(rx, cDir.path() + QDir::separator() + "\\1");
                }
                mCompletePossibilities.sort();
                mCompletePossibilities << mCompletionCurrentSearch; //XXX dups ?
                dbg() << "complete with pfx : " << filters << " | " << mCompletePossibilities;
            } else {
                dbg() << "no filter given for completion";
                mCompletionCurrentSearch = "";
                //list of files in that directory
                mCompletePossibilities = cDir.entryList(QStringList(), QDir::AllEntries | QDir::Dirs | QDir::NoDotAndDotDot);
                mCompletePossibilities.sort();
                mCompletePossibilities << mCompletionCurrentSearch; //XXX dups ?
                dbg() << "complete no pfx : " << mCompletePossibilities;
            }
        } else if ( words.count() == 1 ) { //command completion
            dbg() << "Init command completion";
            //set the searched item to the last word part of the command line
            if ( mCompletionCurrentSearch.isEmpty() && words.count() > 0 ) {
                mCompletionCurrentSearch = words.last();
                current.chop(mCompletionCurrentSearch.length());
            }
            QStringList l = extractCommandNames();
            foreach ( const QString &s, l ) {
                if ( s.startsWith(mCompletionCurrentSearch) ) {
                    mCompletePossibilities << s;
                }
            }
            mCompletePossibilities.sort();
            mCompletePossibilities << mCompletionCurrentSearch; //add the searched item, so we can loop over XXX check dups
        } else { //command's option completion ?
            //TODO
            return ;
        }
    }
    //so now we should have a list with proposals, fire them in order after a TAB key press, until the user presses something else (and stops completion)
    if ( mCompletePossibilities.count() > 1 ) {
        //remove previous proposal
        if (mCurrentCompletionProposal > 0) {
            current.chop(mCompletePossibilities.at(completionIndex()).length());
        }
        //we looped all over possibilities, reset to beginning
        if (mCurrentCompletionProposal >= mCompletePossibilities.count()) {
            mCurrentCompletionProposal = 0;
        }
        //add new proposal to the command line
        dbg() << "next complete test : " << mCompletePossibilities.at(mCurrentCompletionProposal);
        current += mCompletePossibilities.at(mCurrentCompletionProposal++);
    } else { //nothing matched, reset search
        dbg() << "no completion match, resetting";
        current = view->guiGetCommandLineText();
        resetCompletion();
    }
    view->guiSetCommandLineText(current);
}

const QStringList YZModeEx::extractCommandNames()
{
    QStringList list;
    foreach( const YZExCommand *c, commands ) {
        list << c->longName();
    }
    return list;
}

void YZModeEx::initPool()
{
    // ranges
    ranges.push_back( new YZExRange( "\\d+", &YZModeEx::rangeLine ) );
    ranges.push_back( new YZExRange( "\\.", &YZModeEx::rangeCurrentLine ) );
    ranges.push_back( new YZExRange( "\\$", &YZModeEx::rangeLastLine ) );
    ranges.push_back( new YZExRange( "'\\w", &YZModeEx::rangeMark ) );
    ranges.push_back( new YZExRange( "'[<>]", &YZModeEx::rangeVisual ) );
    ranges.push_back( new YZExRange( "/([^/]*/)?", &YZModeEx::rangeSearch ) );
    ranges.push_back( new YZExRange( "\\?([^\\?]*\\?)?", &YZModeEx::rangeSearch ) );

    // commands
    commands.push_back( new YZExCommand( "(x|wq?)(a(ll)?)?", &YZModeEx::write ) );
    commands.push_back( new YZExCommand( "w(rite)?", &YZModeEx::write , QStringList("write") ));
    commands.push_back( new YZExCommand( "q(uit|a(ll)?)?", &YZModeEx::quit, QString("quit:qall").split(":") ) );
    commands.push_back( new YZExCommand( "bf(irst)?", &YZModeEx::bufferfirst, QStringList("bfirst") ) );
    commands.push_back( new YZExCommand( "bl(ast)?", &YZModeEx::bufferlast, QStringList("blast") ) );
    commands.push_back( new YZExCommand( "bn(ext)?", &YZModeEx::buffernext, QStringList("bnext") ) );
    commands.push_back( new YZExCommand( "bp(revious)?", &YZModeEx::bufferprevious, QStringList("bprevious") ) );
    commands.push_back( new YZExCommand( "bd(elete)?", &YZModeEx::bufferdelete, QStringList("bdelete") ) );
    commands.push_back( new YZExCommand( "e(dit)?", &YZModeEx::edit, QStringList("edit") ) );
    commands.push_back( new YZExCommand( "mkyzisrc", &YZModeEx::mkyzisrc, QStringList("mkyzisrc") ) );
    commands.push_back( new YZExCommand( "se(t)?", &YZModeEx::set, QStringList("set") ) );
    commands.push_back( new YZExCommand( "setl(ocal)?", &YZModeEx::set, QStringList("setlocal") ) );
    commands.push_back( new YZExCommand( "setg(lobal)?", &YZModeEx::set, QStringList("setglobal") ) );
    commands.push_back( new YZExCommand( "s(ubstitute)?", &YZModeEx::substitute, QStringList("substitute") ) );
    commands.push_back( new YZExCommand( "hardcopy", &YZModeEx::hardcopy, QStringList("hardcopy") ) );
    commands.push_back( new YZExCommand( "visual", &YZModeEx::gotoCommandMode, QStringList("visual") ) );
    commands.push_back( new YZExCommand( "preserve", &YZModeEx::preserve, QStringList("preserve") ) );
    commands.push_back( new YZExCommand( "lua", &YZModeEx::lua, QStringList("lua" )) );
    commands.push_back( new YZExCommand( "source", &YZModeEx::source, QStringList("source") ) );
    commands.push_back( new YZExCommand( "map", &YZModeEx::map, QStringList("map") ) );
    commands.push_back( new YZExCommand( "unmap", &YZModeEx::unmap, QStringList("unmap") ) );
    commands.push_back( new YZExCommand( "imap", &YZModeEx::imap, QStringList("imap") ) );
    commands.push_back( new YZExCommand( "iunmap", &YZModeEx::iunmap, QStringList("iunmap") ) );
    commands.push_back( new YZExCommand( "vmap", &YZModeEx::vmap, QStringList("vmap") ) );
    commands.push_back( new YZExCommand( "vunmap", &YZModeEx::vunmap, QStringList("vunmap") ) );
    commands.push_back( new YZExCommand( "omap", &YZModeEx::omap, QStringList("omap") ) );
    commands.push_back( new YZExCommand( "ounmap", &YZModeEx::ounmap, QStringList("ounmap") ) );
    commands.push_back( new YZExCommand( "nmap", &YZModeEx::nmap, QStringList("nmap") ) );
    commands.push_back( new YZExCommand( "nunmap", &YZModeEx::nunmap, QStringList("nunmap") ) );
    commands.push_back( new YZExCommand( "cmap", &YZModeEx::cmap, QStringList("cmap") ) );
    commands.push_back( new YZExCommand( "cunmap", &YZModeEx::cunmap, QStringList("cunmap") ) );
    commands.push_back( new YZExCommand( "noremap", &YZModeEx::noremap, QStringList("noremap") ) );
    commands.push_back( new YZExCommand( "nnoremap", &YZModeEx::nnoremap, QStringList("nnoremap") ) );
    commands.push_back( new YZExCommand( "vnoremap", &YZModeEx::vnoremap, QStringList("vnoremap") ) );
    commands.push_back( new YZExCommand( "inoremap", &YZModeEx::inoremap, QStringList("inoremap") ) );
    commands.push_back( new YZExCommand( "cnoremap", &YZModeEx::cnoremap, QStringList("cnoremap") ) );
    commands.push_back( new YZExCommand( "onoremap", &YZModeEx::onoremap, QStringList("onoremap") ) );
    commands.push_back( new YZExCommand( "[<>]", &YZModeEx::indent, QStringList(), false ));
    commands.push_back( new YZExCommand( "ene(w)?", &YZModeEx::enew, QStringList("enew") ));
    commands.push_back( new YZExCommand( "syn(tax)?", &YZModeEx::syntax, QStringList("syntax")));
    commands.push_back( new YZExCommand( "highlight", &YZModeEx::highlight, QStringList("highlight") ));
    commands.push_back( new YZExCommand( "reg(isters)", &YZModeEx::registers, QStringList("registers" ) ));
    commands.push_back( new YZExCommand( "split", &YZModeEx::split, QStringList("split") ));
    commands.push_back( new YZExCommand( "cd", &YZModeEx::cd, QStringList("cd") ));
    commands.push_back( new YZExCommand( "pwd", &YZModeEx::pwd, QStringList("pwd") ));
    commands.push_back( new YZExCommand( "tag", &YZModeEx::tag, QStringList("tag") ));
    commands.push_back( new YZExCommand( "po(p)?", &YZModeEx::pop, QStringList("pop") ));
    commands.push_back( new YZExCommand( "tn(ext)?", &YZModeEx::tagnext, QStringList("tnext") ));
    commands.push_back( new YZExCommand( "tp(revious)?", &YZModeEx::tagprevious, QStringList("tprevious") ));
    commands.push_back( new YZExCommand( "ret(ab)?", &YZModeEx::retab, QStringList("retab") ));

    // folding
    commands.push_back( new YZExCommand( "fo(ld)?", &YZModeEx::foldCreate, QStringList("fold") ));
}

QString YZModeEx::parseRange( const QString& inputs, YZView* view, int* range, bool* matched )
{
    QString _input = inputs;
    *matched = false;
    foreach( const YZExRange *currentRange, ranges ) {
        QRegExp reg( currentRange->regexp() );
        *matched = reg.exactMatch( _input );
        if ( *matched ) {
            unsigned int nc = reg.numCaptures();
            *range = (this->*( currentRange->poolMethod() )) (YZExRangeArgs( currentRange, view, reg.cap( 1 ) ));
            QString s_add = reg.cap( nc - 1 );
            dbg() << "matched " << currentRange->keySeq() << ": " << *range << " and " << s_add << endl;
            if ( s_add.length() > 0 ) { // a range can be followed by +/-nb
                int add = 1;
                if ( s_add.length() > 1 ) add = s_add.mid( 1 ).toUInt();
                if ( s_add[ 0 ] == '-' ) add = -add;
                *range += add;
            }
            _input = reg.cap( nc );
        }
    }
    return _input;
}


CmdState YZModeEx::execExCommand( YZView* view, const QString& inputs )
{
    CmdState ret = CmdError;
    bool matched;
    bool commandIsValid = false;
    int from, to, current;
    QString _input = inputs.trimmed();
    dbg() << "ExCommand: " << _input << endl;
    _input = _input.replace( QRegExp( "^%" ), "1,$" );
    // range
    current = from = to = rangeCurrentLine( YZExRangeArgs( NULL, view, "." ) );

    _input = parseRange( _input, view, &from, &matched );
    if ( matched ) to = from;
    if ( matched && _input[ 0 ] == ',' ) {
        _input = _input.mid( 1 );
        dbg() << "ExCommand : still " << _input << endl;
        _input = parseRange( _input, view, &to, &matched );
    }
    if ( from > to ) {
        int tmp = to;
        to = from;
        from = tmp;
    }
    dbg() << "ExCommand : naked command: " << _input << "; range " << from << "," << to << endl;
    if ( from < 0 || to < 0 ) {
        dbg() << "ExCommand : ERROR! < 0 range" << endl;
        return ret;
    }

    matched = false;
    foreach( const YZExCommand *curCommand, commands ) {
        QRegExp reg(curCommand->regexp());
        matched = reg.exactMatch( _input );
        if ( matched ) {
            unsigned int nc = reg.numCaptures();
            dbg() << "matched " << curCommand->keySeq() << " " << reg.cap( 1 ) << "," << reg.cap( nc ) << endl;
            QString arg = reg.cap( nc );
            bool force = arg[ 0 ] == '!';
            if ( force ) arg = arg.mid( 1 );
            ret = (this->*( curCommand->poolMethod() )) (YZExCommandArgs( view, _input, reg.cap( 1 ), arg.trimmed(), from, to, force ) );
            commandIsValid = true;
        }
    }
    if ( _input.length() == 0 ) {
        view->gotoxy( 0, to );
        view->moveToFirstNonBlankOfLine();
    } else if ( !commandIsValid ) {
        YZSession::self()->guiPopupMessage( _("Not an editor command: ") + _input);
    }

    return ret;
}

/**
 * RANGES
 */

int YZModeEx::rangeLine( const YZExRangeArgs& args )
{
    unsigned int l = args.arg.toUInt();
    if ( l > 0 ) --l;
    return l;
}
int YZModeEx::rangeCurrentLine( const YZExRangeArgs& args )
{
    return args.view->getBufferCursor().y();
}
int YZModeEx::rangeLastLine( const YZExRangeArgs& args )
{
    return qMax( (int)args.view->myBuffer()->lineCount() - 1, 0 );
}
int YZModeEx::rangeMark( const YZExRangeArgs& args )
{
    YZViewMarker *mark = args.view->myBuffer()->viewMarks();
    if ( mark->contains(args.arg.mid(1)))
        return mark->value(args.arg.mid(1)).mBuffer.y();
    return -1;
}
int YZModeEx::rangeVisual( const YZExRangeArgs& args )
{
    YZSelectionMap visual = args.view->visualSelection();
    if ( visual.size() ) {
        if ( args.arg.mid( 1 ) == "<" )
            return visual[ 0 ].fromPos().y();
        else if ( args.arg.mid( 1 ) == ">" )
            return visual[ 0 ].toPos().y();
    }
    return -1;
}
int YZModeEx::rangeSearch( const YZExRangeArgs& args )
{
    bool reverse = args.arg[ 0 ] == QChar('?');

    bool found;
    YZCursor pos;
    if ( args.arg.length() == 1 ) {
        dbg() << "rangeSearch : replay" << endl;
        if ( reverse ) {
            pos = YZSession::self()->search()->replayBackward( args.view->myBuffer(), &found, args.view->myBuffer()->end(), true );
        } else {
            pos = YZSession::self()->search()->replayForward( args.view->myBuffer(), &found, args.view->myBuffer()->begin(), true );
        }
    } else {
        QString pat = args.arg.mid( 1, args.arg.length() - 2 );
        if ( reverse )
            pat.replace( "\\?", "?" );
        else
            pat.replace( "\\/", "/" );
        dbg() << "rangeSearch: " << pat << endl;
        pos = YZSession::self()->search()->forward( args.view->myBuffer(), pat, &found, args.view->getBufferCursor() );
    }

    if ( found ) {
        return pos.y();
    }
    return -1;
}

/**
 * COMMANDS
 */
CmdState YZModeEx::write( const YZExCommandArgs& args )
{
    CmdState ret = CmdOk;
    bool quit = args.cmd.contains( 'q') || args.cmd.contains( 'x' );
    bool all = args.cmd.contains( 'a' );
    bool force = args.force;
    if ( ! quit && all ) {
        YZSession::self()->saveAll();
        return ret;
    }
    dbg() << args.arg << "," << args.cmd << " " << quit << " " << force << endl;
    if ( quit && all ) { //write all modified buffers
        if ( YZSession::self()->saveAll() ) { //if it fails => don't quit
            YZSession::self()->exitRequest();
            ret = CmdQuit;
        }
        return ret;
    }
    if ( args.arg.length() ) {
        args.view->myBuffer()->setPath( args.arg ); //a filename was given as argument
    }
    if ( quit && force ) { //check readonly ? XXX
        args.view->myBuffer()->save();
        YZSession::self()->deleteView( args.view );
        ret = CmdQuit;
    } else if ( quit ) {
        if ( args.view->myBuffer()->save() ) {
            YZSession::self()->deleteView( args.view );
            ret = CmdQuit;
        }
    } else if ( ! force ) {
        args.view->myBuffer()->save();
    } else if ( force ) {
        args.view->myBuffer()->save();
    }
    return ret;
}

CmdState YZModeEx::quit( const YZExCommandArgs& args )
{
    dbg() << "quit( )" << endl;
    CmdState ret = CmdOk;
    bool force = args.force;

    dbg() << YZSession::self()->toString() << endl;

    if ( args.cmd.startsWith( "qa" ) ) {
        if ( force || ! YZSession::self()->isOneBufferModified() ) {
            ret = CmdQuit;
            YZSession::self()->exitRequest( );
        } else {
            YZSession::self()->guiPopupMessage( _( "One file is modified! Save it first..." ) );
        }
    } else {
        //close current view, if it's the last one on a buffer , check it is saved or not
        if ( args.view->myBuffer()->views().count() > 1 ) {
            ret = CmdQuit;
            YZSession::self()->deleteView( args.view );
        } else if ( args.view->myBuffer()->views().count() == 1 && YZSession::self()->buffers().count() == 1) {
            if ( force || !args.view->myBuffer()->fileIsModified() ) {
                if ( YZSession::self()->exitRequest() )
                    ret = CmdQuit;
                else {
                    ret = CmdOk;
                }
            } else YZSession::self()->guiPopupMessage( _( "One file is modified! Save it first..." ) );
        } else {
            if ( force || !args.view->myBuffer()->fileIsModified() ) {
                ret = CmdQuit;
                YZSession::self()->deleteView(args.view);
            } else YZSession::self()->guiPopupMessage( _( "One file is modified! Save it first..." ) );
        }
    }
    return ret;
}

CmdState YZModeEx::bufferfirst( const YZExCommandArgs& )
{
    dbg() << "Switching buffers (actually sw views) ..." << endl;
    YZView *v = YZSession::self()->firstView();
    if ( v )
        YZSession::self()->setCurrentView(v);
    // else
    //   ??? Info message?
    return CmdOk;
}

CmdState YZModeEx::bufferlast( const YZExCommandArgs& )
{
    dbg() << "Switching buffers (actually sw views) ..." << endl;
    YZView *v = YZSession::self()->lastView();
    if ( v )
        YZSession::self()->setCurrentView(v);
    // else
    //   ??? Info message?
    return CmdOk;
}

CmdState YZModeEx::buffernext( const YZExCommandArgs& args )
{
    dbg() << "Switching buffers (actually sw views) ..." << endl;

    YZView *v = YZSession::self()->nextView();
    YZASSERT( v != args.view );

    if ( v ) {
        YZSession::self()->setCurrentView(v);
    } else {
        bufferfirst( args ); // goto first buffer
    }

    return CmdOk;
}

CmdState YZModeEx::bufferprevious ( const YZExCommandArgs& args )
{
    dbg() << "Switching buffers (actually sw views) ..." << endl;

    YZView *v = YZSession::self()->prevView();
    YZASSERT( v != args.view );

    if ( v ) {
        YZSession::self()->setCurrentView(v);
    } else {
        bufferlast( args ); // goto lastbuffer
    }

    return CmdOk;
}

CmdState YZModeEx::bufferdelete ( const YZExCommandArgs& args )
{
    dbg() << "Delete buffer " << (long int)args.view->myBuffer() << endl;

    QList<YZView*> l = args.view->myBuffer()->views();
    foreach ( YZView *v, l )
    YZSession::self()->deleteView( v );

    return CmdQuit;
}

CmdState YZModeEx::gotoCommandMode( const YZExCommandArgs& args )
{
    args.view->modePool()->pop();
    return CmdOk;
}

CmdState YZModeEx::gotoOpenMode( const YZExCommandArgs& /*args*/ )
{
    dbg() << "Switching to open mode...";
    // args.view->gotoOpenMode();
    dbg() << "done." << endl;
    return CmdOk;
}

CmdState YZModeEx::edit ( const YZExCommandArgs& args )
{
    bool force = args.force;
    QString filename;

    // check if the file needs to be saved
    if ( !force && args.view->myBuffer()->fileIsModified() ) {
        YZSession::self()->guiPopupMessage( _( "No write since last change (add ! to override)" ) );
        return CmdError;
    }

    filename = args.arg;

    // Guardian for no arguments
    // in this case Vim reloads the current buffer
    if ( filename.isEmpty() ) {
        YZBuffer *buff = args.view->myBuffer();
        buff->saveYzisInfo( args.view );
        filename = buff->fileName();

        buff->clearText();
        buff->load( filename );

        args.view->applyStartPosition( YZBuffer::getStartPosition(filename, false) );

        return CmdOk;
    }

    filename = YZBuffer::parseFilename(filename);
    YZBuffer *b = YZSession::self()->findBuffer(filename);
    YZView *v = YZSession::self()->findViewByBuffer(b);
    if ( b && v ) {
        // buffer and view already exist
        YZSession::self()->setCurrentView( v );
    } else if ( b ) {
        v = YZSession::self()->createView( b );
        YZSession::self()->setCurrentView( v );
    } else {
        dbg() << "New buffer / view: " << filename << endl;
        v = YZSession::self()->createBufferAndView( args.arg );
        YZSession::self()->setCurrentView( v );
    }
    v->applyStartPosition( YZBuffer::getStartPosition(args.arg) );

    return CmdOk;
}

CmdState YZModeEx::set ( const YZExCommandArgs& args )
{
    CmdState ret = CmdOk;

    OptScope user_scope = ScopeDefault;
    if ( args.cmd.startsWith("setg") )
        user_scope = ScopeGlobal;
    else if ( args.cmd.startsWith("setl") )
        user_scope = ScopeLocal;
    YZBuffer* buff = NULL;
    if ( args.view ) buff = args.view->myBuffer();
    bool matched;
    bool success = YZSession::self()->getOptions()->setOptionFromString( &matched,
                   args.arg.simplified()
                   , user_scope, buff, args.view );

    if ( ! matched ) {
        ret = CmdError;
        YZSession::self()->guiPopupMessage( QString(_("Invalid option name : %1")).arg(args.arg.simplified()) );
    } else if ( ! success ) {
        ret = CmdError;
        YZSession::self()->guiPopupMessage( _("Bad value for option given") );
    }
    return ret;
}

CmdState YZModeEx::mkyzisrc ( const YZExCommandArgs& args )
{
    YZSession::self()->getOptions()->saveTo(
        resourceMgr()->findResource( WritableConfigResource, "yzis.conf" ),
        "", "HL Cache", args.force );
    return CmdOk;
}

CmdState YZModeEx::substitute( const YZExCommandArgs& args )
{
    unsigned int idx = args.input.indexOf("substitute");
    unsigned int len = 10;
    if (static_cast<unsigned int>( -1) == idx) {
        idx = args.input.indexOf("s");
        len = 1;
    }
    unsigned int idxb, idxc;
    unsigned int tidx = idx + len;
    QChar c;
    while ((c = args.input.at(tidx)).isSpace())
        tidx++;
    idx = args.input.indexOf(c, tidx);
    idxb = args.input.indexOf(c, idx + 1);
    idxc = args.input.indexOf(c, idxb + 1);
    QString search = args.input.mid( idx + 1, idxb - idx - 1 );
    QString replace = args.input.mid( idxb + 1, idxc - idxb - 1 );
    QString options = args.input.mid( idxc + 1 );

    bool needsUpdate = false;
    bool found;
    if ( options.contains( "i" ) && !search.endsWith( "\\c" ) ) {
        search.append("\\c");
    }
    unsigned int lastLine = 0;
    YZCursor start( 0, args.fromLine );
    YZSession::self()->search()->forward( args.view->myBuffer(), search, &found, start );
    if ( found ) {
        for ( unsigned int i = args.fromLine; i <= args.toLine; i++ ) {
            if ( args.view->myBuffer()->substitute( search, replace, options.contains( "g" ), i ) ) {
                needsUpdate = true;
                lastLine = i;
            }
        }
        if ( needsUpdate ) {
            args.view->myBuffer()->updateAllViews();
            args.view->gotoxy( 0, lastLine );
            args.view->moveToFirstNonBlankOfLine();
        }
    }

    return CmdOk;
}

CmdState YZModeEx::hardcopy( const YZExCommandArgs& args )
{
    if ( args.arg.length() == 0 ) {
        YZSession::self()->guiPopupMessage( _( "Please specify a filename" ) );
        return CmdError;
    }
    QString path = args.arg;
    QFileInfo fi ( path );
    path = fi.absoluteFilePath( );
    args.view->printToFile( path );
    return CmdOk;
}

CmdState YZModeEx::preserve( const YZExCommandArgs& args )
{
    args.view->myBuffer()->preserve();
    return CmdOk;
}

CmdState YZModeEx::lua( const YZExCommandArgs& args )
{
    YZLuaEngine::self()->lua( args.view, args.arg );
    return CmdOk;
}

CmdState YZModeEx::source( const YZExCommandArgs& args )
{
    dbg() << "source( " << args.toString() << " ) " << endl;
    QString filename = args.arg.left( args.arg.indexOf( " " ));
    dbg().sprintf( "source() filename=%s", qp(filename) );
    if (YZLuaEngine::self()->source( filename ) != 0)
        YZSession::self()->guiPopupMessage(_("The file %1 could not be found" ).arg( filename ));
    dbg() << "source() done" << endl;
    return CmdOk;
}

CmdState YZModeEx::genericMap ( const YZExCommandArgs& args, int type)
{
    QRegExp rx("(\\S+)\\s+(.+)");
    if ( rx.exactMatch(args.arg) ) {
        dbg() << "Adding mapping: " << rx.cap(1) << " to " << rx.cap(2) << endl;
        switch (type) {
        case 0: //global
            YZMapping::self()->addGlobalMapping(rx.cap(1), rx.cap(2));
            break;
        case 1: //insert
            YZMapping::self()->addInsertMapping(rx.cap(1), rx.cap(2));
            break;
        case 2: //operator
            YZMapping::self()->addPendingOpMapping(rx.cap(1), rx.cap(2));
            break;
        case 3: //visual
            YZMapping::self()->addVisualMapping(rx.cap(1), rx.cap(2));
            break;
        case 4: //normal
            YZMapping::self()->addNormalMapping(rx.cap(1), rx.cap(2));
            break;
        case 5: //cmdline
            YZMapping::self()->addCmdLineMapping(rx.cap(1), rx.cap(2));
            break;
        }
        if (rx.cap(1).startsWith("<CTRL>")) {
            mModifierKeys << rx.cap(1);
            YZViewList views = YZSession::self()->getAllViews();
            foreach ( YZView *v, views )
            v->registerModifierKeys( rx.cap( 1 ) );
        }
    }
    return CmdOk;
}

CmdState YZModeEx::genericUnmap ( const YZExCommandArgs& args, int type)
{
    dbg() << "Removing mapping: " << args.arg << endl;
    switch (type) {
    case 0: //global
        YZMapping::self()->deleteGlobalMapping(args.arg);
        break;
    case 1: //insert
        YZMapping::self()->deleteInsertMapping(args.arg);
        break;
    case 2: //operator
        YZMapping::self()->deletePendingOpMapping(args.arg);
        break;
    case 3: //visual
        YZMapping::self()->deleteVisualMapping(args.arg);
        break;
    case 4: //normal
        YZMapping::self()->deleteNormalMapping(args.arg);
        break;
    case 5: //cmdline
        YZMapping::self()->deleteCmdLineMapping(args.arg);
        break;
    }
    if (args.arg.startsWith("<CTRL>")) {
        mModifierKeys.removeAll(args.arg);
        YZViewList views = YZSession::self()->getAllViews();
        for ( YZViewList::const_iterator itr = views.begin(); itr != views.end(); ++itr ) {
            YZView *v = *itr;
            v->unregisterModifierKeys( args.arg );
        }
    }
    return CmdOk;
}

CmdState YZModeEx::genericNoremap ( const YZExCommandArgs& args, int type)
{
    QRegExp rx("(\\S+)\\s+(.+)");
    if ( rx.exactMatch(args.arg) ) {
        // dbg() << "Adding noremapping: " << rx.cap(1) << " to " << rx.cap(2) << endl;
        switch (type) {
        case 0: //global
            YZMapping::self()->addGlobalNoreMapping(rx.cap(1), rx.cap(2));
            break;
        case 1: //insert
            YZMapping::self()->addInsertNoreMapping(rx.cap(1), rx.cap(2));
            break;
        case 2: //operator
            YZMapping::self()->addPendingOpNoreMapping(rx.cap(1), rx.cap(2));
            break;
        case 3: //visual
            YZMapping::self()->addVisualNoreMapping(rx.cap(1), rx.cap(2));
            break;
        case 4: //normal
            YZMapping::self()->addNormalNoreMapping(rx.cap(1), rx.cap(2));
            break;
        case 5: //cmdline
            YZMapping::self()->addCmdLineNoreMapping(rx.cap(1), rx.cap(2));
            break;
        }
        if (rx.cap(1).startsWith("<CTRL>")) {
            mModifierKeys << rx.cap(1);
            YZViewList views = YZSession::self()->getAllViews();
            for ( YZViewList::const_iterator itr = views.begin(); itr != views.end(); ++itr ) {
                YZView *v = *itr;
                v->registerModifierKeys( rx.cap( 1 ) );
            }
        }
    }
    return CmdOk;
}

CmdState YZModeEx::map( const YZExCommandArgs& args )
{
    return genericMap(args, 0);
}

CmdState YZModeEx::unmap( const YZExCommandArgs& args )
{
    return genericUnmap(args, 0);
}

CmdState YZModeEx::imap( const YZExCommandArgs& args )
{
    return genericMap(args, 1);
}

CmdState YZModeEx::iunmap( const YZExCommandArgs& args )
{
    return genericUnmap(args, 1);
}

CmdState YZModeEx::omap( const YZExCommandArgs& args )
{
    return genericMap(args, 2);
}

CmdState YZModeEx::ounmap( const YZExCommandArgs& args )
{
    return genericUnmap(args, 2);
}

CmdState YZModeEx::vmap( const YZExCommandArgs& args )
{
    return genericMap(args, 3);
}

CmdState YZModeEx::vunmap( const YZExCommandArgs& args )
{
    return genericUnmap(args, 3);
}

CmdState YZModeEx::nmap( const YZExCommandArgs& args )
{
    return genericMap(args, 4);
}

CmdState YZModeEx::nunmap( const YZExCommandArgs& args )
{
    return genericUnmap(args, 4);
}

CmdState YZModeEx::cmap( const YZExCommandArgs& args )
{
    return genericMap(args, 5);
}

CmdState YZModeEx::cunmap( const YZExCommandArgs& args )
{
    return genericUnmap(args, 5);
}

CmdState YZModeEx::noremap( const YZExCommandArgs& args )
{
    return genericNoremap(args, 0);
}

CmdState YZModeEx::inoremap( const YZExCommandArgs& args )
{
    return genericNoremap(args, 1);
}

CmdState YZModeEx::onoremap( const YZExCommandArgs& args )
{
    return genericNoremap(args, 2);
}

CmdState YZModeEx::vnoremap( const YZExCommandArgs& args )
{
    return genericNoremap(args, 3);
}

CmdState YZModeEx::nnoremap( const YZExCommandArgs& args )
{
    return genericNoremap(args, 4);
}

CmdState YZModeEx::cnoremap( const YZExCommandArgs& args )
{
    return genericNoremap(args, 5);
}

CmdState YZModeEx::indent( const YZExCommandArgs& args )
{
    int count = 1;
    if ( args.arg.length() > 0 ) count = args.arg.toUInt();
    if ( args.cmd[ 0 ] == '<' ) count *= -1;
    for ( unsigned int i = args.fromLine; i <= args.toLine; i++ ) {
        args.view->myBuffer()->action()->indentLine( args.view, i, count );
    }
    args.view->commitNextUndo();
    args.view->gotoxy( 0, args.toLine );
    args.view->moveToFirstNonBlankOfLine();
    return CmdOk;
}

CmdState YZModeEx::enew( const YZExCommandArgs& )
{
    YZSession::self()->createBufferAndView();
    return CmdOk;
}

CmdState YZModeEx::registers( const YZExCommandArgs& )
{
    QString infoMessage(_("Registers:\n")); // will contain register-value table
    QList<QChar> keys = YZSession::self()->getRegisters();
    QString regContents;
    foreach ( QChar c, keys ) {
        infoMessage += QString("\"") + c + "  ";
        // why I use space as separator? I don't know :)
        // if you know what must be used here, fix it ;)
        regContents = YZSession::self()->getRegister( c ).join(" ");
        // FIXME dimsuz: maybe replace an abstract 27 with some predefined value?
        if ( regContents.length() >= 27 ) {
            // if register contents is too large, truncate it a little
            regContents.truncate( 27 );
            regContents += "...";
        }
        infoMessage += regContents + '\n';
    }
    YZSession::self()->guiPopupMessage( infoMessage );
    return CmdOk;
}

CmdState YZModeEx::syntax( const YZExCommandArgs& args )
{
    if ( args.arg == "on" ) {
        args.view->myBuffer()->detectHighLight();
    } else if ( args.arg == "off" ) {
        args.view->myBuffer()->setHighLight(0);
    }
    return CmdOk;
}

CmdState YZModeEx::highlight( const YZExCommandArgs& args )
{
    // :highlight Defaults Comment fg= selfg= bg= selbg= italic nobold underline strikeout
    QStringList list = args.arg.split(" ");
    QStringList::Iterator it = list.begin(), end = list.end();
    dbg() << list << endl;
    if (list.count() < 3) return CmdError; //at least 3 parameters...
    QString style = list[0];
    QString type = list[1];
    list.erase(it++); list.erase(it++);
    if (!list[0].contains("=") && !list[0].endsWith("bold") && !list[0].endsWith("italic") && !list[0].endsWith("underline") && !list[0].endsWith("strikeout")) {
        type += ' ' + list[0];
        list.erase(it++);
    }

    //get the current settings for this option
    int idx = 0;
    if ( style == "Defaults" || style == "Default" )
        style = "Default Item Styles - Schema ";
    else {
        style = "Highlighting " + style.trimmed() + " - Schema ";
        idx++;
    }
    style += YZSession::self()->schemaManager()->name(0); //XXX make it use the 'current' schema
    YZSession::self()->getOptions()->setGroup(style);
    QStringList option = YZSession::self()->getOptions()->readListOption(type);
    dbg() << "HIGHLIGHT : Current " << type << ": " << option << endl;
    if (option.count() < 7) return CmdError; //just make sure it's fine ;)

    end = list.end();
    //and update it with parameters passed from user
    QRegExp rx("(\\S*)=(\\S*)");
    for (it = list.begin();it != end; ++it) {
        dbg() << "Testing " << *it << endl;
        if ( rx.exactMatch(*it) ) { // fg=, selfg= ...
            YZColor col (rx.cap(2)); //can be a name or rgb
            if ( rx.cap(1) == "fg" ) {
                option[idx] = QString::number(col.rgb(), 16);
            } else if ( rx.cap(1) == "bg" ) {
                option[6 + idx] = QString::number(col.rgb(), 16);
            } else if ( rx.cap(1) == "selfg" ) {
                option[1 + idx] = QString::number(col.rgb(), 16);
            } else if ( rx.cap(1) == "selbg" ) {
                option[7 + idx] = QString::number(col.rgb(), 16);
            }
        } else { // bold, noitalic ...
            if ( *it == "bold" )
                option[2 + idx] = "1";
            if ( *it == "nobold" )
                option[2 + idx] = "0";
            if ( *it == "italic" )
                option[3 + idx] = "1";
            if ( *it == "noitalic" )
                option[3 + idx] = "0";
            if ( *it == "strikeout" )
                option[4 + idx] = "1";
            if ( *it == "nostrikeout" )
                option[4 + idx] = "0";
            if ( *it == "underline" )
                option[5 + idx] = "1";
            if ( *it == "nounderline" )
                option[5 + idx] = "0";
        }
    }
    dbg() << "HIGHLIGHT : Setting new " << option << endl;
    YZSession::self()->getOptions()->getOption( type )->setList( option );
    YZSession::self()->getOptions()->setGroup("Global");

    if ( args.view && args.view->myBuffer() ) {
        YzisHighlighting *yzis = args.view->myBuffer()->highlight();
        if (yzis) {
            args.view->myBuffer()->makeAttribs();
            args.view->sendRefreshEvent();
        }
    }

    return CmdOk;
}

CmdState YZModeEx::split( const YZExCommandArgs& args )
{
    YZSession::self()->guiSplitHorizontally(args.view);
    return CmdOk;
}

CmdState YZModeEx::foldCreate( const YZExCommandArgs& args )
{
    args.view->folds()->create( args.fromLine, args.toLine );
    return CmdOk;
}

CmdState YZModeEx::cd( const YZExCommandArgs& args )
{
    QString targetDir = YZBuffer::tildeExpand(args.arg);
    if ( QDir::setCurrent(targetDir) ) {
        // we could be using a new tag file, so reset tags
        tagReset();
        return CmdOk;
    } else {
        YZSession::self()->guiPopupMessage( _( "Cannot change to specified directory" ) );
        return CmdError;
    }
}

CmdState YZModeEx::pwd( const YZExCommandArgs& )
{
    YZSession::self()->guiPopupMessage( QDir::current().absolutePath().toUtf8().data() );
    return CmdOk;
}

CmdState YZModeEx::tag( const YZExCommandArgs& args )
{
    tagJumpTo(args.arg);

    return CmdOk;
}

CmdState YZModeEx::pop( const YZExCommandArgs& /*args*/ )
{
    tagPop();

    return CmdOk;
}

CmdState YZModeEx::tagnext( const YZExCommandArgs& /*args*/ )
{
    tagNext();

    return CmdOk;
}

CmdState YZModeEx::tagprevious( const YZExCommandArgs& /*args*/ )
{
    tagPrev();

    return CmdOk;
}

CmdState YZModeEx::retab( const YZExCommandArgs& args )
{
    YZBuffer *buffer = args.view->myBuffer();

    // save the cursor's position on screen so it can be restored
    int cursordx = args.view->viewCursor().screenX();
    int cursordy = args.view->viewCursor().screenY();

    int tabstop = args.view->getLocalIntegerOption("tabstop");
    bool changed = false;
    int numSpaces = 0;
    int numTabs = 0;
    bool gotTab = false;
    int startCol = 0;
    int startVcol = 0;
    int len = 0;
    int oldLen = 0;
    QString oldLine;
    QString newLine;

    if (args.arg.length() > 0) { // we got an argument
        if (args.arg.toInt() > 0) {
            // set the value of 'tabstop' to the argument given
            YZSession::self()->getOptions()->setOptionFromString( args.arg.trimmed().insert(0, "tabstop="),
                    ScopeLocal, args.view->myBuffer(), args.view );
            tabstop = args.arg.toInt();
        } else {
            // Value must be > 0 FIXME: The user should get an error message
            return CmdError;
        }
    }

    for (int lnum = 0; lnum < buffer->lineCount(); lnum++) {
        oldLine = buffer->textline(lnum);
        newLine = "";
        int col = 0;
        int vcol = 0;

        for (;;) {
            if (oldLine[col].isSpace()) {
                if (!gotTab && numSpaces == 0) {
                    // First consecutive white-space
                    startVcol = vcol;
                    startCol = col;
                }

                if (oldLine[col] == ' ') {
                    numSpaces++;
                } else {
                    gotTab = true;
                }
            } else {
                if (gotTab || (args.force && numSpaces > 1)) {
                    // Retabulate this string of white-space

                    len = numSpaces = vcol - startVcol;
                    numTabs = 0;

                    if (!args.view->getLocalBooleanOption("expandtab")) {
                        if (numSpaces >= (tabstop - (startVcol % tabstop))) {
                            numSpaces -= (tabstop - (startVcol % tabstop));
                            numTabs++;
                        }
                        numTabs += numSpaces / tabstop;
                        numSpaces -= (numSpaces / tabstop) * tabstop;
                    }
                    if (args.view->getLocalBooleanOption("expandtab") || gotTab || (numSpaces + numTabs < len)) {
                        // len is actual number of white characters used
                        len = numSpaces + numTabs;
                        oldLen = oldLine.length();

                        if (startCol > 0)
                            newLine = oldLine.mid(0, startCol);
                        newLine.insert(startCol + len, oldLine.mid(col, oldLen - col + 1));

                        for (col = 0; col < len; col++)
                            newLine[col + startCol] = (col < numTabs) ? '\t' : ' ';
                        if (newLine != oldLine) {
                            // replace the line and set changed to true
                            buffer->action()->replaceLine( args.view, lnum, newLine );
                            changed = true;
                        }
                        oldLine = newLine;
                        col = startCol + len;
                    }
                }
                gotTab = false;
                numSpaces = 0;
            }
            if (oldLine[col].isNull())
                break;

            if (oldLine[col] == '\t')
                vcol += tabstop - (vcol % tabstop); // number of columns the tab fills
            else
                vcol++;

            ++col;
        }
    }

    if (changed)
        args.view->commitNextUndo();

    // move the cursor to the same *screen* position it was at
    args.view->gotodxdy(cursordx, cursordy);

    args.view->recalcScreen();

    return CmdOk;
}

YZHistory *YZModeEx::getHistory()
{
    return mHistory;
}
