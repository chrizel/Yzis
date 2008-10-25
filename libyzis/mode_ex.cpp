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
#include "mode.h"
#include "mode_pool.h"
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

#define dbg() yzDebug("YModeEx")
#define err() yzError("YModeEx")
#define ftl() yzError("YModeEx")

YExCommandArgs::YExCommandArgs( YView* _view, const QString& _input, const QString& _cmd, const QString& _arg, unsigned int _fromLine, unsigned int _toLine, bool _force )
{
    input = _input;
    cmd = _cmd;
    arg = _arg;
    view = _view;
    fromLine = _fromLine;
    toLine = _toLine;
    force = _force;
}

QString YExCommandArgs::toString() const
{
    QString s;
    s += "YExCommandArgs:\n";
    s += QString().sprintf("view=%p\n", view);
    s += QString("input=%1\n").arg(input);
    s += QString("cmd=%1\n").arg(cmd);
    s += QString("arg=%1\n").arg(arg);
    s += QString("fromLine=%1 toLine=%2\n").arg(fromLine).arg(toLine);
    s += QString("force=%1\n").arg(force);
    return s;
}

YExRange::YExRange( const QString& regexp, ExRangeMethod pm )
{
    mKeySeq = regexp;
    mPoolMethod = pm;
    mRegexp = QRegExp( "^(" + mKeySeq + ")([+\\-]\\d*)?(.*)$" );
}

YExCommand::YExCommand( const QString& input, ExPoolMethod pm, const QStringList& longName, bool word )
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

YModeEx::YModeEx() : YMode()
{
    mType = ModeEx;
    mString = _("[ Ex ]");
    mMapMode = MapCmdline;
    commands.clear();
    ranges.clear();
    mHistory = new YZHistory;
    mCompletePossibilities.clear();
    mCurrentCompletionProposal = 0;
    mIsEditMode = false;
    mIsCmdLineMode = true;
    mIsSelMode = false;
}

YModeEx::~YModeEx()
{
    foreach( const YExCommand *c, commands )
    delete c;
    foreach( const YExRange *r, ranges )
    delete r;

    delete mHistory;
}
void YModeEx::init()
{
    initPool();
}
void YModeEx::enter( YView* view )
{
    dbg() << "enter( " << view << ")" << endl;
    view->guiSetFocusCommandLine();
    view->guiSetCommandLineText( "" );
}

void YModeEx::leave( YView* view )
{
    dbg() << "leave( " << view << ")" << endl;
    view->guiSetCommandLineText( "" );
    // we need to fetch the current view because it may no longer
    // be the view on which the command was done (think :bn )
    if (view == YSession::self()->currentView()) {
        view->guiSetFocusMainWindow();
    }
    dbg() << "leave() done" << endl;
}

CmdState YModeEx::execCommand( YView* view, const YKeySequence &inputs, 
                               YKeySequence::const_iterator &parsePos )
{
    dbg() << "YModeEx::execCommand(" << view << ",..., " << parsePos << ")"
        << endl;
    YKey key = *parsePos;
    CmdState ret = CmdOk;
    if ( key != Qt::Key_Tab ) {
        //clean up the whole completion stuff
        resetCompletion();
    }
    if ( key == Qt::Key_Enter || key == Qt::Key_Return ) {
        if ( view->guiGetCommandLineText().isEmpty()) {
            view->modePool()->pop();
        } else {
            QString cmd = view->guiGetCommandLineText();
            mHistory->addEntry( cmd );
            ret = execExCommand( view, cmd );
            if ( ret != CmdQuit )
                view->modePool()->pop( ModeCommand );
        }
    } else if ( key == Qt::Key_Down ) {
        mHistory->goForwardInTime();
        view->guiSetCommandLineText( mHistory->getEntry() );
    } else if ( key == Qt::Key_Left || key == Qt::Key_Right ) {}
    else if ( key == Qt::Key_Up ) {
        mHistory->goBackInTime();
        view->guiSetCommandLineText( mHistory->getEntry() );
    } else if ( key == Qt::Key_Escape || key == YKey(Qt::Key_C, Qt::ControlModifier) ) {
        view->modePool()->pop( ModeCommand );
    } else if ( key == Qt::Key_Tab ) {
        completeCommandLine(view);
    } else if ( key == Qt::Key_Backspace ) {
        QString back = view->guiGetCommandLineText();
        if ( back.isEmpty() ) {
            view->modePool()->pop();            
        }
        else
            view->guiSetCommandLineText(back.remove(back.length() - 1, 1));
    } else if ( key == YKey(Qt::Key_W, Qt::ControlModifier) ) {
        QString cmd = view->guiGetCommandLineText();
        dbg() << "YModeEx::execCommand(): " << "deleting word from: '"
            << cmd << "'" << endl;
        QRegExp rx("\\b\\S+\\s*$");
        int pos = rx.lastIndexIn(cmd);
        if (-1 != pos) {
            dbg() << "YModeEx::execCommand(): " << "match at: " << pos << endl;
            view->guiSetCommandLineText(cmd.left(pos));
        } else {
            dbg() << "YModeEx::execCommand(): " << "didn't match" << endl;
        }
    } else if ( key == YKey(Qt::Key_U, Qt::ControlModifier) ) {
        view->guiSetCommandLineText(QString::null);
    } else {
        view->guiSetCommandLineText(view->guiGetCommandLineText() + key.toString());
    }
    // We've processed a key, advance the pointer in case the caller cares
    ++parsePos;
    return ret;
}

void YModeEx::resetCompletion()
{
    mCompletePossibilities.clear();
    mCompletionCurrentSearch = "";
    mCurrentCompletionProposal = 0;
}

const QStringList& YModeEx::completionList()
{
    return mCompletePossibilities;
}

int YModeEx::completionIndex()
{
    return mCurrentCompletionProposal -1;
}

const QString& YModeEx::completionItem(int idx)
{
    return mCompletePossibilities.at(idx);
}

void YModeEx::completeCommandLine(YView *view)
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

const QStringList YModeEx::extractCommandNames()
{
    QStringList list;
    foreach( const YExCommand *c, commands ) {
        list << c->longName();
    }
    return list;
}

void YModeEx::initPool()
{
    // ranges
    ranges.push_back( new YExRange( "\\d+", &YModeEx::rangeLine ) );
    ranges.push_back( new YExRange( "\\.", &YModeEx::rangeCurrentLine ) );
    ranges.push_back( new YExRange( "\\$", &YModeEx::rangeLastLine ) );
    ranges.push_back( new YExRange( "'\\w", &YModeEx::rangeMark ) );
    ranges.push_back( new YExRange( "'[<>]", &YModeEx::rangeVisual ) );
    ranges.push_back( new YExRange( "/([^/]*/)?", &YModeEx::rangeSearch ) );
    ranges.push_back( new YExRange( "\\?([^\\?]*\\?)?", &YModeEx::rangeSearch ) );

    // commands
    commands.push_back( new YExCommand( "(x|wq?)(a(ll)?)?", &YModeEx::write ) );
    commands.push_back( new YExCommand( "w(rite)?", &YModeEx::write , QStringList("write") ));
    commands.push_back( new YExCommand( "q(uit|a(ll)?)?", &YModeEx::quit, QString("quit:qall").split(":") ) );
    commands.push_back( new YExCommand( "bf(irst)?", &YModeEx::bufferfirst, QStringList("bfirst") ) );
    commands.push_back( new YExCommand( "bl(ast)?", &YModeEx::bufferlast, QStringList("blast") ) );
    commands.push_back( new YExCommand( "bn(ext)?", &YModeEx::buffernext, QStringList("bnext") ) );
    commands.push_back( new YExCommand( "bp(revious)?", &YModeEx::bufferprevious, QStringList("bprevious") ) );
    commands.push_back( new YExCommand( "bd(elete)?", &YModeEx::bufferdelete, QStringList("bdelete") ) );
    commands.push_back( new YExCommand( "e(dit)?", &YModeEx::edit, QStringList("edit") ) );
    commands.push_back( new YExCommand( "mkyzisrc", &YModeEx::mkyzisrc, QStringList("mkyzisrc") ) );
    commands.push_back( new YExCommand( "se(t)?", &YModeEx::set, QStringList("set") ) );
    commands.push_back( new YExCommand( "setl(ocal)?", &YModeEx::set, QStringList("setlocal") ) );
    commands.push_back( new YExCommand( "setg(lobal)?", &YModeEx::set, QStringList("setglobal") ) );
    commands.push_back( new YExCommand( "s(ubstitute)?", &YModeEx::substitute, QStringList("substitute") ) );
    commands.push_back( new YExCommand( "hardcopy", &YModeEx::hardcopy, QStringList("hardcopy") ) );
    commands.push_back( new YExCommand( "visual", &YModeEx::gotoCommandMode, QStringList("visual") ) );
    commands.push_back( new YExCommand( "preserve", &YModeEx::preserve, QStringList("preserve") ) );
    commands.push_back( new YExCommand( "lua", &YModeEx::lua, QStringList("lua" )) );
    commands.push_back( new YExCommand( "source", &YModeEx::source, QStringList("source") ) );
    commands.push_back( new YExCommand( "map", &YModeEx::map, QStringList("map") ) );
    commands.push_back( new YExCommand( "unmap", &YModeEx::unmap, QStringList("unmap") ) );
    commands.push_back( new YExCommand( "imap", &YModeEx::imap, QStringList("imap") ) );
    commands.push_back( new YExCommand( "iunmap", &YModeEx::iunmap, QStringList("iunmap") ) );
    commands.push_back( new YExCommand( "vmap", &YModeEx::vmap, QStringList("vmap") ) );
    commands.push_back( new YExCommand( "vunmap", &YModeEx::vunmap, QStringList("vunmap") ) );
    commands.push_back( new YExCommand( "omap", &YModeEx::omap, QStringList("omap") ) );
    commands.push_back( new YExCommand( "ounmap", &YModeEx::ounmap, QStringList("ounmap") ) );
    commands.push_back( new YExCommand( "nmap", &YModeEx::nmap, QStringList("nmap") ) );
    commands.push_back( new YExCommand( "nunmap", &YModeEx::nunmap, QStringList("nunmap") ) );
    commands.push_back( new YExCommand( "cmap", &YModeEx::cmap, QStringList("cmap") ) );
    commands.push_back( new YExCommand( "cunmap", &YModeEx::cunmap, QStringList("cunmap") ) );
    commands.push_back( new YExCommand( "noremap", &YModeEx::noremap, QStringList("noremap") ) );
    commands.push_back( new YExCommand( "nnoremap", &YModeEx::nnoremap, QStringList("nnoremap") ) );
    commands.push_back( new YExCommand( "vnoremap", &YModeEx::vnoremap, QStringList("vnoremap") ) );
    commands.push_back( new YExCommand( "inoremap", &YModeEx::inoremap, QStringList("inoremap") ) );
    commands.push_back( new YExCommand( "cnoremap", &YModeEx::cnoremap, QStringList("cnoremap") ) );
    commands.push_back( new YExCommand( "onoremap", &YModeEx::onoremap, QStringList("onoremap") ) );
    commands.push_back( new YExCommand( "[<>]", &YModeEx::indent, QStringList(), false ));
    commands.push_back( new YExCommand( "ene(w)?", &YModeEx::enew, QStringList("enew") ));
    commands.push_back( new YExCommand( "syn(tax)?", &YModeEx::syntax, QStringList("syntax")));
    commands.push_back( new YExCommand( "highlight", &YModeEx::highlight, QStringList("highlight") ));
    commands.push_back( new YExCommand( "reg(isters)", &YModeEx::registers, QStringList("registers" ) ));
    commands.push_back( new YExCommand( "split", &YModeEx::split, QStringList("split") ));
    commands.push_back( new YExCommand( "cd", &YModeEx::cd, QStringList("cd") ));
    commands.push_back( new YExCommand( "pwd", &YModeEx::pwd, QStringList("pwd") ));
    commands.push_back( new YExCommand( "tag", &YModeEx::tag, QStringList("tag") ));
    commands.push_back( new YExCommand( "po(p)?", &YModeEx::pop, QStringList("pop") ));
    commands.push_back( new YExCommand( "tn(ext)?", &YModeEx::tagnext, QStringList("tnext") ));
    commands.push_back( new YExCommand( "tp(revious)?", &YModeEx::tagprevious, QStringList("tprevious") ));
    commands.push_back( new YExCommand( "ret(ab)?", &YModeEx::retab, QStringList("retab") ));

    // folding
    commands.push_back( new YExCommand( "fo(ld)?", &YModeEx::foldCreate, QStringList("fold") ));
}

QString YModeEx::parseRange( const QString& inputs, YView* view, int* range, bool* matched )
{
    QString _input = inputs;
    *matched = false;
    foreach( const YExRange *currentRange, ranges ) {
        QRegExp reg( currentRange->regexp() );
        *matched = reg.exactMatch( _input );
        if ( *matched ) {
            unsigned int nc = reg.numCaptures();
            *range = (this->*( currentRange->poolMethod() )) (YExRangeArgs( currentRange, view, reg.cap( 1 ) ));
            QString s_add = reg.cap( nc - 1 );
            dbg() << "matched " << currentRange->keySeq() << ": " << *range << " and " << s_add << endl;
            if ( s_add.length() > 0 ) { // a range can be followed by +/-nb
                int add = 1;
                if ( s_add.length() > 1 ) add = s_add.mid( 1 ).toUInt();
                if ( s_add[ 0 ] == '-' ) add = -add;
                *range += add;
            }
            return reg.cap( nc );
        }
    }
    return _input;
}


CmdState YModeEx::execExCommand( YView* view, const QString& inputs )
{
    CmdState ret = CmdError;
    bool matched;
    bool commandIsValid = false;
    int from, to, current;
    QString _input = inputs.trimmed();
    dbg() << "ExCommand: " << _input << endl;
    _input = _input.replace( QRegExp( "^%" ), "1,$" );
    // range
    current = from = to = rangeCurrentLine( YExRangeArgs( NULL, view, "." ) );

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
    foreach( const YExCommand *curCommand, commands ) {
        QRegExp reg(curCommand->regexp());
        matched = reg.exactMatch( _input );
        if ( matched ) {
            unsigned int nc = reg.numCaptures();
            dbg() << "matched " << curCommand->keySeq() << " " << reg.cap( 1 ) << "," << reg.cap( nc ) << endl;
            QString arg = reg.cap( nc );
            bool force = arg[ 0 ] == '!';
            if ( force ) arg = arg.mid( 1 );
            ret = (this->*( curCommand->poolMethod() )) (YExCommandArgs( view, _input, reg.cap( 1 ), arg.trimmed(), from, to, force ) );
            commandIsValid = true;
        }
    }
    if ( _input.length() == 0 ) {
		view->gotoViewCursor(view->viewCursorFromLinePosition(view->buffer()->firstNonBlankChar(to), to));
    } else if ( !commandIsValid ) {
        YSession::self()->guiPopupMessage( _("Not an editor command: ") + _input);
    }

    return ret;
}

/**
 * RANGES
 */

int YModeEx::rangeLine( const YExRangeArgs& args )
{
    unsigned int l = args.arg.toUInt();
    if ( l > 0 ) --l;
    return l;
}
int YModeEx::rangeCurrentLine( const YExRangeArgs& args )
{
    return args.view->getLinePositionCursor().y();
}
int YModeEx::rangeLastLine( const YExRangeArgs& args )
{
    return qMax( (int)args.view->buffer()->lineCount() - 1, 0 );
}
int YModeEx::rangeMark( const YExRangeArgs& args )
{
    YViewMarker *mark = args.view->buffer()->viewMarks();
    if ( mark->contains(args.arg.mid(1)))
        return mark->value(args.arg.mid(1)).line();
    return -1;
}
int YModeEx::rangeVisual( const YExRangeArgs& args )
{
	//TODO
#if 0
    YSelectionMap visual = args.view->visualSelection();
    if ( visual.size() ) {
        if ( args.arg.mid( 1 ) == "<" )
            return visual[ 0 ].fromPos().y();
        else if ( args.arg.mid( 1 ) == ">" )
            return visual[ 0 ].toPos().y();
    }
#endif
    return -1;
}
int YModeEx::rangeSearch( const YExRangeArgs& args )
{
    bool reverse = args.arg[ 0 ] == QChar('?');

    bool found;
    YCursor pos;
    if ( args.arg.length() == 1 ) {
        dbg() << "rangeSearch : replay" << endl;
        if ( reverse ) {
            pos = YSession::self()->search()->replayBackward( args.view->buffer(), &found, args.view->buffer()->end(), true );
        } else {
            pos = YSession::self()->search()->replayForward( args.view->buffer(), &found, args.view->buffer()->begin(), true );
        }
    } else {
        QString pat = args.arg.mid( 1, args.arg.length() - 2 );
        if ( reverse )
            pat.replace( "\\?", "?" );
        else
            pat.replace( "\\/", "/" );
        dbg() << "rangeSearch: " << pat << endl;
        pos = YSession::self()->search()->forward( args.view->buffer(), pat, &found, args.view->getLinePositionCursor() );
    }

    if ( found ) {
        return pos.y();
    }
    return -1;
}

/**
 * COMMANDS
 */
CmdState YModeEx::write( const YExCommandArgs& args )
{
    CmdState ret = CmdOk;
    bool quit = args.cmd.contains( 'q') || args.cmd.contains( 'x' );
    bool all = args.cmd.contains( 'a' );
    bool force = args.force;
    if ( ! quit && all ) {
        YSession::self()->saveAll();
        return ret;
    }
    dbg() << args.arg << "," << args.cmd << " " << quit << " " << force << endl;
    if ( quit && all ) { //write all modified buffers
        if ( YSession::self()->saveAll() ) { //if it fails => don't quit
            YSession::self()->exitRequest();
            ret = CmdQuit;
        }
        return ret;
    }
    if ( args.arg.length() ) {
        args.view->buffer()->setPath( args.arg ); //a filename was given as argument
    }
    if ( quit && force ) { //check readonly ? XXX
        args.view->buffer()->save();
        YSession::self()->deleteView( args.view );
        ret = CmdQuit;
    } else if ( quit ) {
        if ( args.view->buffer()->save() ) {
            YSession::self()->deleteView( args.view );
            ret = CmdQuit;
        }
    } else if ( ! force ) {
        args.view->buffer()->save();
    } else if ( force ) {
        args.view->buffer()->save();
    }
    return ret;
}

CmdState YModeEx::quit( const YExCommandArgs& args )
{
    dbg() << "quit( )" << endl;
    CmdState ret = CmdOk;
    bool force = args.force;

    dbg() << YSession::self()->toString() << endl;

    if ( args.cmd.startsWith( "qa" ) ) {
        if ( force || ! YSession::self()->isOneBufferModified() ) {
            ret = CmdQuit;
            YSession::self()->exitRequest( );
        } else {
            YSession::self()->guiPopupMessage( _( "One file is modified! Save it first..." ) );
        }
    } else {
        //close current view, if it's the last one on a buffer , check it is saved or not
        if ( args.view->buffer()->views().count() > 1 ) {
            ret = CmdQuit;
            YSession::self()->deleteView( args.view );
        } else if ( args.view->buffer()->views().count() == 1 && YSession::self()->buffers().count() == 1) {
            if ( force || !args.view->buffer()->fileIsModified() ) {
                if ( YSession::self()->exitRequest() )
                    ret = CmdQuit;
                else {
                    ret = CmdOk;
                }
            } else YSession::self()->guiPopupMessage( _( "One file is modified! Save it first..." ) );
        } else {
            if ( force || !args.view->buffer()->fileIsModified() ) {
                ret = CmdQuit;
                YSession::self()->deleteView(args.view);
            } else YSession::self()->guiPopupMessage( _( "One file is modified! Save it first..." ) );
        }
    }
    return ret;
}

CmdState YModeEx::bufferfirst( const YExCommandArgs& )
{
    dbg() << "Switching buffers (actually sw views) ..." << endl;
    YView *v = YSession::self()->firstView();
    if ( v )
        YSession::self()->setCurrentView(v);
    // else
    //   ??? Info message?
    return CmdOk;
}

CmdState YModeEx::bufferlast( const YExCommandArgs& )
{
    dbg() << "Switching buffers (actually sw views) ..." << endl;
    YView *v = YSession::self()->lastView();
    if ( v )
        YSession::self()->setCurrentView(v);
    // else
    //   ??? Info message?
    return CmdOk;
}

CmdState YModeEx::buffernext( const YExCommandArgs& args )
{
    dbg() << "Switching buffers (actually sw views) ..." << endl;

    YView *v = YSession::self()->nextView();
    YASSERT( v != args.view );

    if ( v ) {
        YSession::self()->setCurrentView(v);
    } else {
        bufferfirst( args ); // goto first buffer
    }

    return CmdOk;
}

CmdState YModeEx::bufferprevious ( const YExCommandArgs& args )
{
    dbg() << "Switching buffers (actually sw views) ..." << endl;

    YView *v = YSession::self()->prevView();
    YASSERT( v != args.view );

    if ( v ) {
        YSession::self()->setCurrentView(v);
    } else {
        bufferlast( args ); // goto lastbuffer
    }

    return CmdOk;
}

CmdState YModeEx::bufferdelete ( const YExCommandArgs& args )
{
    dbg() << "bufferdelete( " << args.toString() << " ) " << endl;

    YSession::self()->removeBuffer( args.view->buffer() );

    return CmdQuit;
}

CmdState YModeEx::gotoCommandMode( const YExCommandArgs& args )
{
    args.view->modePool()->pop();
    return CmdOk;
}

CmdState YModeEx::gotoOpenMode( const YExCommandArgs& /*args*/ )
{
    dbg() << "Switching to open mode...";
    // args.view->gotoOpenMode();
    dbg() << "done." << endl;
    return CmdOk;
}

CmdState YModeEx::edit ( const YExCommandArgs& args )
{
    bool force = args.force;
    QString filename;

    // check if the file needs to be saved
    if ( !force && args.view->buffer()->fileIsModified() ) {
        YSession::self()->guiPopupMessage( _( "No write since last change (add ! to override)" ) );
        return CmdError;
    }

    filename = args.arg;

    // Guardian for no arguments
    // in this case Vim reloads the current buffer
    if ( filename.isEmpty() /* XXX or if the filename is the name of the current buffer */ ) {
        YBuffer *buff = args.view->buffer();
        buff->saveYzisInfo( args.view );
        filename = buff->fileName();

        buff->clearText();
        buff->load( filename );

        args.view->applyStartPosition( YBuffer::getStartPosition(filename, false) );

        return CmdOk;
    }

    filename = YBuffer::parseFilename(filename);
    YBuffer *b = YSession::self()->findBuffer(filename);
    YView *v = YSession::self()->findViewByBuffer(b);
    if ( b && v ) {
        // buffer and view already exist
        dbg() << "edit(): using existing view for " << filename << endl;
        YSession::self()->setCurrentView( v );
    } else if ( b ) {
        err() << HERE() << endl;
        ftl() << "edit(): the buffer containing " << filename << " was found without a view on it. That should never happen!" << endl;
    } else {
        dbg() << "edit(): New buffer / view: " << filename << endl;
        v = YSession::self()->createBufferAndView( args.arg );
        YSession::self()->setCurrentView( v );
    }
    v->applyStartPosition( YBuffer::getStartPosition(args.arg) );

    return CmdOk;
}

CmdState YModeEx::set ( const YExCommandArgs& args )
{
    CmdState ret = CmdOk;

    OptScope user_scope = ScopeDefault;
    if ( args.cmd.startsWith("setg") )
        user_scope = ScopeGlobal;
    else if ( args.cmd.startsWith("setl") )
        user_scope = ScopeLocal;
    YBuffer* buff = NULL;
    if ( args.view ) buff = args.view->buffer();
    bool matched;
    bool success = YSession::self()->getOptions()->setOptionFromString( &matched,
                   args.arg.simplified()
                   , user_scope, buff, args.view );

    if ( ! matched ) {
        ret = CmdError;
        YSession::self()->guiPopupMessage( QString(_("Invalid option name : %1")).arg(args.arg.simplified()) );
    } else if ( ! success ) {
        ret = CmdError;
        YSession::self()->guiPopupMessage( _("Bad value for option given") );
    }
    return ret;
}

CmdState YModeEx::mkyzisrc ( const YExCommandArgs& args )
{
    YSession::self()->getOptions()->saveTo(
        resourceMgr()->findResource( WritableConfigResource, "yzis.conf" ),
        "", "HL Cache", args.force );
    return CmdOk;
}

CmdState YModeEx::substitute( const YExCommandArgs& args )
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
    int lastLine = 0;
    YCursor start( 0, args.fromLine );
    YSession::self()->search()->forward( args.view->buffer(), search, &found, start );
    if ( found ) {
        for ( int i = args.fromLine; i <= args.toLine; i++ ) {
            if ( args.view->buffer()->substitute( search, replace, options.contains( "g" ), i ) ) {
                needsUpdate = true;
                lastLine = i;
            }
        }
        if ( needsUpdate ) {
            args.view->commitNextUndo();
			args.view->gotoLinePosition(lastLine, args.view->buffer()->firstNonBlankChar(lastLine));
        }
    }

    return CmdOk;
}

CmdState YModeEx::hardcopy( const YExCommandArgs& args )
{
    if ( args.arg.length() == 0 ) {
        YSession::self()->guiPopupMessage( _( "Please specify a filename" ) );
        return CmdError;
    }
    QString path = args.arg;
    QFileInfo fi ( path );
    path = fi.absoluteFilePath( );
    args.view->printToFile( path );
    return CmdOk;
}

CmdState YModeEx::preserve( const YExCommandArgs& args )
{
    args.view->buffer()->preserve();
    return CmdOk;
}

CmdState YModeEx::lua( const YExCommandArgs& args )
{
    YLuaEngine::self()->lua( args.view, args.arg );
    return CmdOk;
}

CmdState YModeEx::source( const YExCommandArgs& args )
{
    dbg() << "source( " << args.toString() << " ) " << endl;
    QString filename = args.arg.left( args.arg.indexOf( " " ));
    dbg().SPrintf( "source() filename=%s", qp(filename) );
    if (YLuaEngine::self()->source( filename ) != 0)
        YSession::self()->guiPopupMessage(_("The file %1 could not be found" ).arg( filename ));
    dbg() << "source() done" << endl;
    return CmdOk;
}

CmdState YModeEx::genericMap ( const YExCommandArgs& args, int type)
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
        if (rx.cap(1).startsWith("<CTRL>") || rx.cap(1).startsWith("<SHIFT>")) {
            mModifierKeys << rx.cap(1);
            YViewList views = YSession::self()->getAllViews();
            foreach ( YView *v, views )
            v->registerModifierKeys( rx.cap( 1 ) );
        }
    }
    return CmdOk;
}

CmdState YModeEx::genericUnmap ( const YExCommandArgs& args, int type)
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
        YViewList views = YSession::self()->getAllViews();
        for ( YViewList::const_iterator itr = views.begin(); itr != views.end(); ++itr ) {
            YView *v = *itr;
            v->unregisterModifierKeys( args.arg );
        }
    }
    return CmdOk;
}

CmdState YModeEx::genericNoremap ( const YExCommandArgs& args, int type)
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
            YViewList views = YSession::self()->getAllViews();
            for ( YViewList::const_iterator itr = views.begin(); itr != views.end(); ++itr ) {
                YView *v = *itr;
                v->registerModifierKeys( rx.cap( 1 ) );
            }
        }
    }
    return CmdOk;
}

CmdState YModeEx::map( const YExCommandArgs& args )
{
    return genericMap(args, 0);
}

CmdState YModeEx::unmap( const YExCommandArgs& args )
{
    return genericUnmap(args, 0);
}

CmdState YModeEx::imap( const YExCommandArgs& args )
{
    return genericMap(args, 1);
}

CmdState YModeEx::iunmap( const YExCommandArgs& args )
{
    return genericUnmap(args, 1);
}

CmdState YModeEx::omap( const YExCommandArgs& args )
{
    return genericMap(args, 2);
}

CmdState YModeEx::ounmap( const YExCommandArgs& args )
{
    return genericUnmap(args, 2);
}

CmdState YModeEx::vmap( const YExCommandArgs& args )
{
    return genericMap(args, 3);
}

CmdState YModeEx::vunmap( const YExCommandArgs& args )
{
    return genericUnmap(args, 3);
}

CmdState YModeEx::nmap( const YExCommandArgs& args )
{
    return genericMap(args, 4);
}

CmdState YModeEx::nunmap( const YExCommandArgs& args )
{
    return genericUnmap(args, 4);
}

CmdState YModeEx::cmap( const YExCommandArgs& args )
{
    return genericMap(args, 5);
}

CmdState YModeEx::cunmap( const YExCommandArgs& args )
{
    return genericUnmap(args, 5);
}

CmdState YModeEx::noremap( const YExCommandArgs& args )
{
    return genericNoremap(args, 0);
}

CmdState YModeEx::inoremap( const YExCommandArgs& args )
{
    return genericNoremap(args, 1);
}

CmdState YModeEx::onoremap( const YExCommandArgs& args )
{
    return genericNoremap(args, 2);
}

CmdState YModeEx::vnoremap( const YExCommandArgs& args )
{
    return genericNoremap(args, 3);
}

CmdState YModeEx::nnoremap( const YExCommandArgs& args )
{
    return genericNoremap(args, 4);
}

CmdState YModeEx::cnoremap( const YExCommandArgs& args )
{
    return genericNoremap(args, 5);
}

CmdState YModeEx::indent( const YExCommandArgs& args )
{
    int count = 1;
    if ( args.arg.length() > 0 ) count = args.arg.toUInt();
    if ( args.cmd[ 0 ] == '<' ) count *= -1;
    for ( unsigned int i = args.fromLine; i <= args.toLine; i++ ) {
        args.view->buffer()->action()->indentLine( args.view, i, count );
    }
    args.view->commitNextUndo();
	args.view->gotoViewCursor(args.view->viewCursorFromLinePosition(args.view->buffer()->firstNonBlankChar(args.toLine), args.toLine));
    return CmdOk;
}

CmdState YModeEx::enew( const YExCommandArgs& )
{
    YSession::self()->createBufferAndView();
    return CmdOk;
}

CmdState YModeEx::registers( const YExCommandArgs& )
{
    QString infoMessage(_("Registers:\n")); // will contain register-value table
    QList<QChar> keys = YSession::self()->getRegisters();
    QString regContents;
    foreach ( QChar c, keys ) {
        infoMessage += QString("\"") + c + "  ";
        // why I use space as separator? I don't know :)
        // if you know what must be used here, fix it ;)
        regContents = YSession::self()->getRegister( c ).join(" ");
        // FIXME dimsuz: maybe replace an abstract 27 with some predefined value?
        if ( regContents.length() >= 27 ) {
            // if register contents is too large, truncate it a little
            regContents.truncate( 27 );
            regContents += "...";
        }
        infoMessage += regContents + '\n';
    }
    YSession::self()->guiPopupMessage( infoMessage );
    return CmdOk;
}

CmdState YModeEx::syntax( const YExCommandArgs& args )
{
    if ( args.arg == "on" ) {
        args.view->buffer()->detectHighLight();
    } else if ( args.arg == "off" ) {
        args.view->buffer()->setHighLight(0);
    }
    return CmdOk;
}

CmdState YModeEx::highlight( const YExCommandArgs& args )
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
    style += YSession::self()->schemaManager()->name(0); //XXX make it use the 'current' schema
    YSession::self()->getOptions()->setGroup(style);
    QStringList option = YSession::self()->getOptions()->readListOption(type);
    dbg() << "HIGHLIGHT : Current " << type << ": " << option << endl;
    if (option.count() < 7) return CmdError; //just make sure it's fine ;)

    end = list.end();
    //and update it with parameters passed from user
    QRegExp rx("(\\S*)=(\\S*)");
    for (it = list.begin();it != end; ++it) {
        dbg() << "Testing " << *it << endl;
        if ( rx.exactMatch(*it) ) { // fg=, selfg= ...
            YColor col (rx.cap(2)); //can be a name or rgb
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
    YSession::self()->getOptions()->getOption( type )->setList( option );
    YSession::self()->getOptions()->setGroup("Global");

    if ( args.view && args.view->buffer() ) {
        YzisHighlighting *yzis = args.view->buffer()->highlight();
        if (yzis) {
            args.view->buffer()->makeAttribs();
            args.view->sendRefreshEvent();
        }
    }

    return CmdOk;
}

CmdState YModeEx::split( const YExCommandArgs& args )
{
    YSession::self()->guiSplitHorizontally(args.view);
    return CmdOk;
}

CmdState YModeEx::foldCreate( const YExCommandArgs& args )
{
    args.view->folds()->create( args.fromLine, args.toLine );
    return CmdOk;
}

CmdState YModeEx::cd( const YExCommandArgs& args )
{
    QString targetDir = YBuffer::tildeExpand(args.arg);
    if ( QDir::setCurrent(targetDir) ) {
        // we could be using a new tag file, so reset tags
        tagReset();
        return CmdOk;
    } else {
        YSession::self()->guiPopupMessage( _( "Cannot change to specified directory" ) );
        return CmdError;
    }
}

CmdState YModeEx::pwd( const YExCommandArgs& )
{
    YSession::self()->guiPopupMessage( QDir::current().absolutePath().toUtf8().data() );
    return CmdOk;
}

CmdState YModeEx::tag( const YExCommandArgs& args )
{
    tagJumpTo(args.arg);

    return CmdOk;
}

CmdState YModeEx::pop( const YExCommandArgs& /*args*/ )
{
    tagPop();

    return CmdOk;
}

CmdState YModeEx::tagnext( const YExCommandArgs& /*args*/ )
{
    tagNext();

    return CmdOk;
}

CmdState YModeEx::tagprevious( const YExCommandArgs& /*args*/ )
{
    tagPrev();

    return CmdOk;
}

CmdState YModeEx::retab( const YExCommandArgs& args )
{
    YBuffer *buffer = args.view->buffer();

    // save the cursor's position on screen so it can be restored
	YCursor cursor = args.view->viewCursor().buffer();

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
            YSession::self()->getOptions()->setOptionFromString( args.arg.trimmed().insert(0, "tabstop="),
                    ScopeLocal, args.view->buffer(), args.view );
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
    args.view->gotoViewCursor(args.view->viewCursorFromLinePosition(cursor));

    args.view->recalcScreen();

    return CmdOk;
}

YZHistory *YModeEx::getHistory()
{
    return mHistory;
}
