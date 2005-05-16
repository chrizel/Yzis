/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2004 Thomas Capricelli <orzel@freehackers.org>,
 *  Copyright (C) 2004 Philippe Fremy <phil@freehackers.org>,
 *  Copyright (C) 2004 Pascal "Poizon" Maillard <poizon@gmx.at>,
 *  Copyright (C) 2004-2005 Loic Pauleve <panard@inzenet.org>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id$
 */

#include "portability.h"
#include "mode_ex.h"
#include "buffer.h"
#include "registers.h"
#include "session.h"
#include "swapfile.h"
#include "ex_lua.h"
#include "mark.h"
#include "selection.h"
#include "mapping.h"
#include "action.h"
#include "schema.h"
#if QT_VERSION < 0x040000
#include <qobject.h>
#include <qfileinfo.h>
#include <qdir.h>
#else
#include <QDir>
#endif


YZExRange::YZExRange( const QString& regexp, ExRangeMethod pm ) {
	mKeySeq = regexp;
	mPoolMethod = pm;
	mRegexp = QRegExp( "^(" + mKeySeq + ")([+\\-]\\d*)?(.*)$" );
}

YZExCommand::YZExCommand( const QString& input, ExPoolMethod pm, const QStringList& longName, bool word ) {
	mKeySeq = input;
	mPoolMethod = pm;
	mLongName = longName;
	if ( word ) {
		mRegexp = QRegExp( "^(" + mKeySeq + ")(\\b.*)?$" );
	} else {
		mRegexp = QRegExp( "^(" + mKeySeq + ")([\\w\\s].*)?$" );
	}
}

YZModeEx::YZModeEx() : YZMode() {
	mType = MODE_EX;
	mString = _("[ Ex ]");
	mMapMode = cmdline;
	commands.clear();
	ranges.clear();
#if QT_VERSION < 0x040000
	commands.setAutoDelete( true );
	ranges.setAutoDelete( true );
#endif
}

YZModeEx::~YZModeEx() {
#if QT_VERSION >= 0x040000
	for (int ab = 0 ; ab < commands.size() ; ++ab) 
		delete commands.at(ab);
	for (int ab = 0 ; ab < ranges.size() ; ++ab) 
		delete ranges.at(ab);
#endif
	commands.clear();
	ranges.clear();
}
void YZModeEx::init() {
	initPool();
}
void YZModeEx::enter( YZView* mView ) {
	YZSession::me->setFocusCommandLine();
	mView->setCommandLineText( "" );
}
void YZModeEx::leave( YZView* mView ) {
	mView->setCommandLineText( "" );
	YZSession::me->setFocusMainWindow();
}
cmd_state YZModeEx::execCommand( YZView* mView, const QString& key ) {
	yzDebug() << "YZModeEx::execCommand " << key << endl;
	cmd_state ret = CMD_OK;
	if ( key == "<ENTER>" ) {
		if( mView->getCommandLineText().isEmpty()) {
			mView->modePool()->pop();
		} else {
			QString cmd = mView->mExHistory[mView->mCurrentExItem] = mView->getCommandLineText();
			mView->mCurrentExItem++;
			ret = execExCommand( mView, cmd );
			if ( ret != CMD_QUIT ) 
				mView->modePool()->pop( MODE_COMMAND );
		}
	} else if ( key == "<DOWN>" ) {
		if(mView->mExHistory[mView->mCurrentExItem].isEmpty())
			return ret;

		mView->mCurrentExItem++;
		mView->setCommandLineText( mView->mExHistory[mView->mCurrentExItem] );
	} else if ( key == "<LEFT>" || key == "<RIGHT>" ) {
	} else if ( key == "<UP>" ) {
		if(mView->mCurrentExItem == 0)
			return ret;

		mView->mCurrentExItem--;
		mView->setCommandLineText( mView->mExHistory[mView->mCurrentExItem] );
	} else if ( key == "<ESC>" || key == "<CTRL>c" ) {
		mView->modePool()->pop( MODE_COMMAND );
	} else if ( key == "<TAB>" ) {
		//ignore for now
	} else if ( key == "<BS>" ) {
		QString back = mView->getCommandLineText();
		if ( back.isEmpty() ) {
			mView->modePool()->pop();
			return ret;
		}
		mView->setCommandLineText(back.remove(back.length() - 1, 1));
	} else {
		mView->setCommandLineText( mView->getCommandLineText() + key );
	}
	return ret;
}

void YZModeEx::initPool() {
	// ranges
	ranges.append( new YZExRange( "\\d+", &YZModeEx::rangeLine ) );
	ranges.append( new YZExRange( "\\.", &YZModeEx::rangeCurrentLine ) );
	ranges.append( new YZExRange( "\\$", &YZModeEx::rangeLastLine ) );
	ranges.append( new YZExRange( "'\\w", &YZModeEx::rangeMark ) );
	ranges.append( new YZExRange( "'[<>]", &YZModeEx::rangeVisual ) );
	ranges.append( new YZExRange( "/([^/]*/)?", &YZModeEx::rangeSearch ) );
	ranges.append( new YZExRange( "\\?([^\\?]*\\?)?", &YZModeEx::rangeSearch ) );

	// commands
	commands.append( new YZExCommand( "(x|wq?)(a(ll)?)?", &YZModeEx::write ) );
	commands.append( new YZExCommand( "w(rite)?", &YZModeEx::write , QStringList("write") ));
#if QT_VERSION < 0x040000
	commands.append( new YZExCommand( "q(uit|a(ll)?)?", &YZModeEx::quit, QStringList::split(":","quit:qall") ) );
#else
	commands.append( new YZExCommand( "q(uit|a(ll)?)?", &YZModeEx::quit, ( QStringList() << "quit" << "qall") ) );
#endif
        commands.append( new YZExCommand( "bf(irst)?", &YZModeEx::bufferfirst, QStringList("bfirst") ) );
        commands.append( new YZExCommand( "bl(ast)?", &YZModeEx::bufferlast, QStringList("blast") ) );
	commands.append( new YZExCommand( "bn(ext)?", &YZModeEx::buffernext, QStringList("bnext") ) );
	commands.append( new YZExCommand( "bp(revious)?", &YZModeEx::bufferprevious, QStringList("bprevious") ) );
	commands.append( new YZExCommand( "bd(elete)?", &YZModeEx::bufferdelete, QStringList("bdelete") ) );
	commands.append( new YZExCommand( "e(dit)?", &YZModeEx::edit, QStringList("edit") ) );
	commands.append( new YZExCommand( "mkyzisrc", &YZModeEx::mkyzisrc, QStringList("mkyzisrc") ) );
	commands.append( new YZExCommand( "se(t)?", &YZModeEx::set, QStringList("set") ) );
	commands.append( new YZExCommand( "setl(ocal)?", &YZModeEx::set, QStringList("setlocal") ) );
	commands.append( new YZExCommand( "setg(lobal)?", &YZModeEx::set, QStringList("setglobal") ) );
	commands.append( new YZExCommand( "s(ubstitute)?", &YZModeEx::substitute, QStringList("substitute") ) );
	commands.append( new YZExCommand( "hardcopy", &YZModeEx::hardcopy, QStringList("hardcopy") ) );
	commands.append( new YZExCommand( "visual", &YZModeEx::gotoCommandMode, QStringList("visual") ) );
	commands.append( new YZExCommand( "preserve", &YZModeEx::preserve, QStringList("preserve") ) );
	commands.append( new YZExCommand( "lua", &YZModeEx::lua, QStringList("lua" )) );
	commands.append( new YZExCommand( "source", &YZModeEx::source, QStringList("source") ) );
	commands.append( new YZExCommand( "map", &YZModeEx::map, QStringList("map") ) );
	commands.append( new YZExCommand( "unmap", &YZModeEx::unmap, QStringList("unmap") ) );
	commands.append( new YZExCommand( "imap", &YZModeEx::imap, QStringList("imap") ) );
	commands.append( new YZExCommand( "iunmap", &YZModeEx::iunmap, QStringList("iunmap") ) );
	commands.append( new YZExCommand( "vmap", &YZModeEx::vmap, QStringList("vmap") ) );
	commands.append( new YZExCommand( "vunmap", &YZModeEx::vunmap, QStringList("vunmap") ) );
	commands.append( new YZExCommand( "omap", &YZModeEx::omap, QStringList("omap") ) );
	commands.append( new YZExCommand( "ounmap", &YZModeEx::ounmap, QStringList("ounmap") ) );
	commands.append( new YZExCommand( "nmap", &YZModeEx::nmap, QStringList("nmap") ) );
	commands.append( new YZExCommand( "nunmap", &YZModeEx::nunmap, QStringList("nunmap") ) );
	commands.append( new YZExCommand( "cmap", &YZModeEx::cmap, QStringList("cmap") ) );
	commands.append( new YZExCommand( "cunmap", &YZModeEx::cunmap, QStringList("cunmap") ) );
	commands.append( new YZExCommand( "[<>]", &YZModeEx::indent, QStringList(), false ));
	commands.append( new YZExCommand( "ene(w)?", &YZModeEx::enew, QStringList("enew") ));
	commands.append( new YZExCommand( "syn(tax)?", &YZModeEx::syntax, QStringList("syntax")));
	commands.append( new YZExCommand( "highlight", &YZModeEx::highlight, QStringList("highlight") ));
	commands.append( new YZExCommand( "reg(isters)", &YZModeEx::registers, QStringList("registers" ) ));
	commands.append( new YZExCommand( "split", &YZModeEx::split, QStringList("split") ));
}

QString YZModeEx::parseRange( const QString& inputs, YZView* view, int* range, bool* matched ) {
	QString _input = inputs;
	*matched = false;
#if QT_VERSION < 0x040000
	for ( ranges.first(); ! *matched && ranges.current(); ranges.next() ) {
		QRegExp reg( ranges.current()->regexp() );
#else
	for ( int ab = 0 ; ab < ranges.size() ; ++ab ) {
		QRegExp reg( ranges.at(ab)->regexp() );
#endif
		*matched = reg.exactMatch( _input );
		if ( *matched ) {
			unsigned int nc = reg.numCaptures();
#if QT_VERSION < 0x040000
			*range = (this->*( ranges.current()->poolMethod() )) (YZExRangeArgs( ranges.current(), view, reg.cap( 1 ) ));
#else
			*range = (this->*( ranges.at(ab)->poolMethod() )) (YZExRangeArgs( ranges.at(ab), view, reg.cap( 1 ) ));
#endif
			QString s_add = reg.cap( nc - 1 );
#if QT_VERSION < 0x040000
			yzDebug() << "matched " << ranges.current()->keySeq() << " : " << *range << " and " << s_add << endl;
#else
			yzDebug() << "matched " << ranges.at(ab)->keySeq() << " : " << *range << " and " << s_add << endl;
#endif
			if ( s_add.length() > 0 ) { // a range can be followed by +/-nb
				int add = 1;
				if ( s_add.length() > 1 ) add = s_add.mid( 1 ).toUInt();
				if ( s_add[ 0 ] == '-' ) add = add * -1;
				*range += add;
			}
			_input = reg.cap( nc );
		}
	}
	return _input;
}


cmd_state YZModeEx::execExCommand( YZView* view, const QString& inputs ) {
	cmd_state ret = CMD_ERROR;
	bool matched;
	bool commandIsValid = false;
	int from, to, current;
#if QT_VERSION < 0x040000
	QString _input = inputs.stripWhiteSpace();
#else
	QString _input = inputs.trimmed();
#endif
	yzDebug() << "ExCommand : " << _input << endl;
	_input = _input.replace( QRegExp( "^%" ), "1,$" );
	// range
	current = from = to = rangeCurrentLine( YZExRangeArgs( NULL, view, "." ) );

	_input = parseRange( _input, view, &from, &matched );
	if ( matched ) to = from;
	if ( matched && _input[ 0 ] == ',' ) {
		_input = _input.mid( 1 );
		yzDebug() << "ExCommand : still " << _input << endl;
		_input = parseRange( _input, view, &to, &matched );
	}
	if ( from > to ) {
		int tmp = to;
		to = from;
		from = tmp;
	}
	yzDebug() << "ExCommand : naked command : " << _input << "; range " << from << "," << to << endl;
	if ( from < 0 || to < 0 ) {
		yzDebug() << "ExCommand : ERROR! < 0 range" << endl;
		return ret;
	}

	matched = false;
#if QT_VERSION < 0x040000
	for ( commands.first(); ! matched && commands.current(); commands.next() ) {
		QRegExp reg(commands.current()->regexp());
#else
	for ( int ab = 0 ; ab < commands.size() ; ++ab ) {
		QRegExp reg(commands.at(ab)->regexp());
#endif
		matched = reg.exactMatch( _input );
		if ( matched ) {
			unsigned int nc = reg.numCaptures();
#if QT_VERSION < 0x040000
			yzDebug() << "matched " << commands.current()->keySeq() << " " << reg.cap( 1 ) << "," << reg.cap( nc ) << endl;
#else
			yzDebug() << "matched " << commands.at(ab)->keySeq() << " " << reg.cap( 1 ) << "," << reg.cap( nc ) << endl;
#endif
			QString arg = reg.cap( nc );
			bool force = arg[ 0 ] == '!';
			if ( force ) arg = arg.mid( 1 );
#if QT_VERSION < 0x040000
			ret = (this->*( commands.current()->poolMethod() )) (YZExCommandArgs( view, _input, reg.cap( 1 ), arg.stripWhiteSpace(), from, to, force ) );
#else
			ret = (this->*( commands.at(ab)->poolMethod() )) (YZExCommandArgs( view, _input, reg.cap( 1 ), arg.trimmed(), from, to, force ) );
#endif
			commandIsValid = true;
		}
	}
	if ( current != to ) {
		view->gotoxy( 0, to );
		view->moveToFirstNonBlankOfLine();
	}


	if (!commandIsValid && _input.length() > 0) {
		YZSession::me->popupMessage( _("Not an editor command: ") + _input);
	}

	return ret;
}

/**
 * RANGES
 */

int YZModeEx::rangeLine( const YZExRangeArgs& args ) {
	unsigned int l = args.arg.toUInt();
	if ( l > 0 ) --l;
	return l;
}
int YZModeEx::rangeCurrentLine( const YZExRangeArgs& args ) {
	return args.view->getBufferCursor()->y();
}
int YZModeEx::rangeLastLine( const YZExRangeArgs& args ) {
	return qMax( (int)args.view->myBuffer()->lineCount() - 1, 0 );
}
int YZModeEx::rangeMark( const YZExRangeArgs& args ) {
	bool found = false;
	YZCursorPos pos = args.view->myBuffer()->viewMarks()->get( args.arg.mid( 1 ), &found );
	if ( found )
		return pos.bPos->y();
	return -1;
}
int YZModeEx::rangeVisual( const YZExRangeArgs& args ) {
	YZSelectionMap visual = args.view->visualSelection();
	if ( visual.size() ) {
		if ( args.arg.mid( 1 ) == "<" )
			return visual[ 0 ].fromPos().y();
		else if ( args.arg.mid( 1 ) == ">" )
			return visual[ 0 ].toPos().y();
	}
	return -1;
}
int YZModeEx::rangeSearch( const YZExRangeArgs& args ) {
	bool reverse = args.arg[ 0 ] == QChar('?');

	bool found;
	YZCursor pos;
	if ( args.arg.length() == 1 ) {
		yzDebug() << "rangeSearch : replay" << endl;
		if ( reverse ) {
			pos = YZSession::me->search()->replayBackward( args.view, &found, NULL, true );
		} else {
			pos = YZSession::me->search()->replayForward( args.view, &found, NULL, true );
		}
	} else {
		QString pat = args.arg.mid( 1, args.arg.length() - 2 );
		if ( reverse ) 
			pat.replace( "\\?", "?" );
		else
			pat.replace( "\\/", "/" );
		yzDebug() << "rangeSearch : " << pat << endl;
		pos = YZSession::me->search()->forward( args.view, pat, &found );
	}

	if ( found ) {
		return pos.y();
	}
	return -1;
}

/**
 * COMMANDS
 */
cmd_state YZModeEx::write( const YZExCommandArgs& args ) {
	cmd_state ret = CMD_OK;
	bool quit = args.cmd.contains( 'q') || args.cmd.contains( 'x' );
	bool all = args.cmd.contains( 'a' );
	bool force = args.force;
	if ( ! quit && all ) {
		args.view->mySession()->saveAll();
		return ret;
	}
	yzDebug() << args.arg << "," << args.cmd << " " << quit << " " << force << endl;
	if ( quit && all ) {//write all modified buffers
		if ( args.view->mySession()->saveAll() ) {//if it fails => dont quit
			args.view->mySession()->exitRequest();
			ret = CMD_QUIT;
		}
		return ret;
	}
	if ( args.arg.length() ) {
		args.view->myBuffer()->setPath( args.arg ); //a filename was given as argument
		args.view->myBuffer()->getSwapFile()->setFileName( args.view->myBuffer()->fileName() );
	}
	if ( quit && force ) {//check readonly ? XXX
		args.view->myBuffer()->save();
		args.view->mySession()->deleteView( args.view->myId );
		ret = CMD_QUIT;
	} else if ( quit ) {
		if ( args.view->myBuffer()->save() ) {
			args.view->mySession()->deleteView( args.view->myId );
			ret = CMD_QUIT;
		}
	} else if ( ! force ) {
		args.view->myBuffer()->save();
	} else if ( force ) {
		args.view->myBuffer()->save();
	}
	return ret;
}
cmd_state YZModeEx::quit( const YZExCommandArgs& args ) {
	cmd_state ret = CMD_OK;
	bool force = args.force;
	yzDebug() << "View counts: "<< args.view->myBuffer()->views().count() 
		<< " Buffer Count : " << args.view->mySession()->countBuffers() << endl;
	if ( args.cmd.startsWith( "qa" ) ) {
		if ( force || ! args.view->mySession()->isOneBufferModified() ) {
			ret = CMD_QUIT;
			args.view->mySession()->exitRequest( );
		} else args.view->mySession()->popupMessage( _( "One file is modified! Save it first..." ) );
	} else {
		//close current view, if it's the last one on a buffer , check it is saved or not
		if ( args.view->myBuffer()->views().count() > 1 ) {
			ret = CMD_QUIT;
			args.view->mySession()->deleteView( args.view->myId );
		} else if ( args.view->myBuffer()->views().count() == 1 && args.view->mySession()->countBuffers() == 1) {
			if ( force || !args.view->myBuffer()->fileIsModified() ) {
				if ( args.view->mySession()->exitRequest() )
					ret = CMD_QUIT;
				else {
					ret = CMD_OK;
				}
			}
			else args.view->mySession()->popupMessage( _( "One file is modified! Save it first..." ) );
		} else {
			if ( force || !args.view->myBuffer()->fileIsModified() ) {
				ret = CMD_QUIT;
				args.view->mySession()->deleteView(args.view->myId);
			}
			else args.view->mySession()->popupMessage( _( "One file is modified! Save it first..." ) );
		}
	}
	return ret;
}

cmd_state YZModeEx::bufferfirst( const YZExCommandArgs& args ) {
	yzDebug() << "Switching buffers (actually sw views) ..." << endl;
        YZView *v = args.view->mySession()->firstView();
        if ( v )
                args.view->mySession()->setCurrentView(v);
        // else
        //   ??? Info message?
        return CMD_OK;
}

cmd_state YZModeEx::bufferlast( const YZExCommandArgs& args ) {
	yzDebug() << "Switching buffers (actually sw views) ..." << endl;
        YZView *v = args.view->mySession()->lastView();
        if ( v )
                args.view->mySession()->setCurrentView(v);
        // else
        //   ??? Info message?
        return CMD_OK;
}

cmd_state YZModeEx::buffernext( const YZExCommandArgs& args ) {
	yzDebug() << "Switching buffers (actually sw views) ..." << endl;
	YZView *v = args.view->mySession()->nextView();
	YZASSERT( v!=args.view );
	if ( v )
		args.view->mySession()->setCurrentView(v);
	else
                bufferfirst( args ); // goto first buffer
	return CMD_OK;
}

cmd_state YZModeEx::bufferprevious ( const YZExCommandArgs& args ) {
	yzDebug() << "Switching buffers (actually sw views) ..." << endl;
	YZView *v = args.view->mySession()->prevView();
	YZASSERT( v!=args.view );
	if ( v )
		args.view->mySession()->setCurrentView(v);
	else
                bufferlast( args ); // goto lastbuffer

	return CMD_OK;
}

cmd_state YZModeEx::bufferdelete ( const YZExCommandArgs& args ) {
	yzDebug() << "Delete buffer " << args.view->myBuffer()->myId << endl;

	args.view->myBuffer()->clearSwap();
#if QT_VERSION < 0x040000
	QPtrList<YZView> l = args.view->myBuffer()->views();
	YZView *v;
	for ( v = l.first(); v; v=l.next() )
		args.view->mySession()->deleteView( args.view->myId );
#else
	int nbV = args.view->myBuffer()->views().size();
	for ( int ab = 0 ; ab < nbV ; ++ab )
		args.view->mySession()->deleteView( args.view->myId );
#endif

	return CMD_QUIT;
}

cmd_state YZModeEx::gotoCommandMode( const YZExCommandArgs& args ) {
	args.view->modePool()->pop();
	return CMD_OK;
}

cmd_state YZModeEx::gotoOpenMode( const YZExCommandArgs& /*args*/ ) {
	yzDebug() << "Switching to open mode...";
//	args.view->gotoOpenMode();
	yzDebug() << "done." << endl;
	return CMD_OK;
}

cmd_state YZModeEx::edit ( const YZExCommandArgs& args ) {
	QString path = args.arg; //extract the path
	if ( path.length() == 0 ) {
		args.view->mySession()->popupMessage( _( "Please specify a filename" ) );
		return CMD_ERROR;
	}
	path = YZBuffer::tildeExpand( path );
	QFileInfo fi ( path );
#if QT_VERSION < 0x040000
	path = fi.absFilePath();
#else
	path = fi.absoluteFilePath();
#endif
	YZBuffer *b =args.view->mySession()->findBuffer(path);
	if (b) {
		yzDebug() << "Buffer already loaded" << endl;
		args.view->mySession()->setCurrentView(b->firstView());
		return CMD_OK;
	}
	yzDebug() << "New buffer / view : " << path << endl;
	args.view->mySession()->createBuffer( path );
	YZBuffer *bu = args.view->mySession()->findBuffer( path );
	YZSession::me->setCurrentView( bu->firstView() );
	return CMD_OK;
}

cmd_state YZModeEx::set ( const YZExCommandArgs& args ) {
	cmd_state ret = CMD_OK;
	
	scope_t user_scope = default_scope;
	if ( args.cmd.startsWith("setg") )
		user_scope = global_scope;
	else if ( args.cmd.startsWith("setl") )
		user_scope = local_scope;
	YZBuffer* buff = NULL;
	if ( args.view ) buff = args.view->myBuffer();
	bool matched;
	bool success = YZSession::mOptions->setOptionFromString( &matched, args.arg.simplifyWhiteSpace(), user_scope, buff, args.view );
	
	if ( ! matched ) {
		ret = CMD_ERROR;
		YZSession::me->popupMessage( QString(_("Invalid option name : %1")).arg(args.arg.simplifyWhiteSpace()) );
	} else if ( ! success ) {
		ret = CMD_ERROR;
		YZSession::me->popupMessage( _("Bad value for option given") );
	}
	return ret;
}

cmd_state YZModeEx::mkyzisrc ( const YZExCommandArgs& args ) {
#if QT_VERSION < 0x040000
	YZSession::mOptions->saveTo( QDir::currentDirPath() + "/yzis.conf", "", "HL Cache", args.force );
#else
	YZSession::mOptions->saveTo( QDir::currentPath() + "/yzis.conf", "", "HL Cache", args.force );
#endif
	return CMD_OK;
}

cmd_state YZModeEx::substitute( const YZExCommandArgs& args ) {
#if QT_VERSION < 0x040000
	unsigned int idx = args.input.find("substitute");
#else
	unsigned int idx = args.input.indexOf("substitute");
#endif
	unsigned int len = 10;
	if (static_cast<unsigned int>(-1)==idx) {
#if QT_VERSION < 0x040000
		idx = args.input.find("s");
#else
		idx = args.input.indexOf("s");
#endif
		len = 1;
	}
	unsigned int idxb,idxc;
	unsigned int tidx = idx+len;
	QChar c;
	while ((c = args.input.at(tidx)).isSpace())
		tidx++;
#if QT_VERSION < 0x040000
	idx = args.input.find(c, tidx);
	idxb = args.input.find(c, idx+1);
	idxc = args.input.find(c, idxb+1);
#else
	idx = args.input.indexOf(c, tidx);
	idxb = args.input.indexOf(c, idx+1);
	idxc = args.input.indexOf(c, idxb+1);
#endif
	QString search = args.input.mid( idx+1, idxb-idx-1 );
	QString replace = args.input.mid( idxb+1, idxc-idxb-1 );
	QString options = args.input.mid( idxc+1 );

	bool needsUpdate = false;
	args.view->gotoxy( 0, args.fromLine );
	args.view->moveToFirstNonBlankOfLine();
	bool found;
	if ( options.contains( "i" ) && !search.endsWith( "\\c" ) ) {
		search.append("\\c");
	}
	YZSession::me->search()->forward( args.view, search, &found );
	if ( found ) {
		for( unsigned int i = args.fromLine; i <= args.toLine; i++ ) {
			if ( args.view->myBuffer()->substitute( search, replace, options.contains( "g" ), i ) )
				needsUpdate = true;
		}
	}
	if ( needsUpdate ) {
		args.view->myBuffer()->updateAllViews();
	}

	return CMD_OK;
}

cmd_state YZModeEx::hardcopy( const YZExCommandArgs& args ) {
	if ( args.arg.length() == 0 ) {
		args.view->mySession()->popupMessage( _( "Please specify a filename" ) );
		return CMD_ERROR;
	}
	QString path = args.arg;
	QFileInfo fi ( path );
#if QT_VERSION < 0x040000
	path = fi.absFilePath( );
#else
	path = fi.absoluteFilePath( );
#endif
	args.view->printToFile( path );
	return CMD_OK;
}

cmd_state YZModeEx::preserve( const YZExCommandArgs& args  ) {
	args.view->myBuffer()->getSwapFile()->flush();
	return CMD_OK;
}

cmd_state YZModeEx::lua( const YZExCommandArgs& args ) {
	YZExLua::instance()->lua( args.view, args.arg );
	return CMD_OK;
}

cmd_state YZModeEx::source( const YZExCommandArgs& args ) {
	YZExLua::instance()->source( args.view, args.arg );
	return CMD_OK;
}

cmd_state YZModeEx::genericMap ( const YZExCommandArgs& args, int type) {
	QRegExp rx("(\\S+)\\s+(.+)");
	if ( rx.exactMatch(args.arg) ) {
		yzDebug() << "Adding mapping : " << rx.cap(1) << " to " << rx.cap(2) << endl;
		switch (type) {
			case 0://global
				YZMapping::self()->addGlobalMapping(rx.cap(1), rx.cap(2));
				break;
			case 1://insert
				YZMapping::self()->addInsertMapping(rx.cap(1), rx.cap(2));
				break;
			case 2://operator
				YZMapping::self()->addPendingOpMapping(rx.cap(1), rx.cap(2));
				break;
			case 3://visual
				YZMapping::self()->addVisualMapping(rx.cap(1), rx.cap(2));
				break;
			case 4://normal
				YZMapping::self()->addNormalMapping(rx.cap(1), rx.cap(2));
				break;
			case 5://cmdline
				YZMapping::self()->addCmdLineMapping(rx.cap(1), rx.cap(2));
				break;
		}
		if (rx.cap(1).startsWith("<CTRL>")) {
			mModifierKeys << rx.cap(1);
			for (int i = 0 ; i <= YZSession::mNbViews; i++) {
				YZView *v = YZSession::me->findView(i);
				if (v)
					v->registerModifierKeys(rx.cap(1));
			}
		}
	}
	return CMD_OK;	
}

cmd_state YZModeEx::genericUnmap ( const YZExCommandArgs& args, int type) {
	yzDebug() << "Removing mapping : " << args.arg << endl;
	switch (type) {
		case 0://global
			YZMapping::self()->deleteGlobalMapping(args.arg);
			break;
		case 1://insert
			YZMapping::self()->deleteInsertMapping(args.arg);
			break;
		case 2://operator
			YZMapping::self()->deletePendingOpMapping(args.arg);
			break;
		case 3://visual
			YZMapping::self()->deleteVisualMapping(args.arg);
			break;
		case 4://normal
			YZMapping::self()->deleteNormalMapping(args.arg);
			break;
		case 5://cmdline
			YZMapping::self()->deleteCmdLineMapping(args.arg);
			break;
	}
	if (args.arg.startsWith("<CTRL>")) {
#if QT_VERSION < 0x040000
		mModifierKeys.remove(args.arg);
#else
		mModifierKeys.removeAll(args.arg);
#endif
		for (int i = 0 ; i <= YZSession::mNbViews; i++) {
			YZView *v = YZSession::me->findView(i);
			if (v)
				v->unregisterModifierKeys(args.arg);
		}
	}
	return CMD_OK;	
}

cmd_state YZModeEx::map( const YZExCommandArgs& args ) {
	return genericMap(args,0);
}

cmd_state YZModeEx::unmap( const YZExCommandArgs& args ) {
	return genericUnmap(args,0);
}

cmd_state YZModeEx::imap( const YZExCommandArgs& args ) {
	return genericMap(args,1);
}

cmd_state YZModeEx::iunmap( const YZExCommandArgs& args ) {
	return genericUnmap(args,1);
}

cmd_state YZModeEx::omap( const YZExCommandArgs& args ) {
	return genericMap(args,2);
}

cmd_state YZModeEx::ounmap( const YZExCommandArgs& args ) {
	return genericUnmap(args,2);
}

cmd_state YZModeEx::vmap( const YZExCommandArgs& args ) {
	return genericMap(args,3);
}

cmd_state YZModeEx::vunmap( const YZExCommandArgs& args ) {
	return genericUnmap(args,3);
}

cmd_state YZModeEx::nmap( const YZExCommandArgs& args ) {
	return genericMap(args,4);
}

cmd_state YZModeEx::nunmap( const YZExCommandArgs& args ) {
	return genericUnmap(args,4);
}

cmd_state YZModeEx::cmap( const YZExCommandArgs& args ) {
	return genericMap(args,5);
}

cmd_state YZModeEx::cunmap( const YZExCommandArgs& args ) {
	return genericUnmap(args,5);
}

cmd_state YZModeEx::indent( const YZExCommandArgs& args ) {
	int count = 1;
	if ( args.arg.length() > 0 ) count = args.arg.toUInt();
	if ( args.cmd[ 0 ] == '<' ) count *= -1;
	for ( unsigned int i = args.fromLine; i <= args.toLine; i++ ) {
		args.view->myBuffer()->action()->indentLine( args.view, i, count );
	}
	args.view->commitNextUndo();
	return CMD_OK;
}

cmd_state YZModeEx::enew( const YZExCommandArgs& ) {
	YZSession::me->createBuffer();
	return CMD_OK;
}

cmd_state YZModeEx::registers( const YZExCommandArgs& ) {
	YZRegisters* regs = YZSession::me->mRegisters;
	QString infoMessage(_("<qt><h3>Register Table</h3>")); // will contain register-value table
	infoMessage += _("<table border=1><tr><th>Name</th><th>Contents</tr>");
#if QT_VERSION < 0x040000
	QValueList<QChar> keys = regs->keys();
	QValueList<QChar>::ConstIterator it = keys.begin(), end = keys.end();
	QString regContents;
	for( ; it != end ; ++it )
	{
		infoMessage += QString("<tr><td>%1</td>").arg(*it);
		// why I use space as separator? I don't know :)
		// if you know what must be used here, fix it ;)
		regContents = regs->getRegister( *it ).join(" ");
		// FIXME dimsuz: maybe replace an abstract 27 with some predefined value?
		if( regContents.length() >= 27 )
		{
			// if register contents is too large, truncate it a little
			regContents.truncate( 27 );
			regContents += "...";
		}
		infoMessage += QString("<td>%1</td></tr>").arg(regContents);
	}
#else
	QList<QChar> keys = regs->keys();
	QString regContents;
	for ( int c = 0; c < keys.size(); ++c )
	{
		infoMessage += QString("<tr><td>%1</td>").arg(keys.at(c));
		// why I use space as separator? I don't know :)
		// if you know what must be used here, fix it ;)
		regContents = regs->getRegister( keys.at(c) ).join(" ");
		// FIXME dimsuz: maybe replace an abstract 27 with some predefined value?
		if( regContents.length() >= 27 )
		{
			// if register contents is too large, truncate it a little
			regContents.truncate( 27 );
			regContents += "...";
		}
		infoMessage += QString("<td>%1</td></tr>").arg(regContents);
	}
#endif
	infoMessage += "</table></qt>";
	YZSession::me->popupMessage( infoMessage );
	return CMD_OK;
}

cmd_state YZModeEx::syntax( const YZExCommandArgs& args ) {
	if ( args.arg == "on" ) {
		args.view->myBuffer()->detectHighLight();
	} else if ( args.arg == "off" ) {
		args.view->myBuffer()->setHighLight(0);
	}
	return CMD_OK;
}

cmd_state YZModeEx::highlight( const YZExCommandArgs& args ) {
// :highlight Defaults Comment fg= selfg= bg= selbg= italic nobold underline strikeout
#if QT_VERSION < 0x040000
	QStringList list = QStringList::split(" ", args.arg);
#else
	QStringList list = args.arg.split(" ");
#endif
	QStringList::Iterator it = list.begin(), end = list.end();
	yzDebug() << list << endl;
	if (list.count() < 3) return CMD_ERROR; //at least 3 parameters...
	QString style = list[0];
	QString type = list[1];
#if QT_VERSION < 0x040000
	list.remove(it++); list.remove(it++);
#else
	list.erase(it++); list.erase(it++);
#endif
	if (!list[0].contains("=") && !list[0].endsWith("bold") && !list[0].endsWith("italic") && !list[0].endsWith("underline") && !list[0].endsWith("strikeout")) {
		type += " " + list[0];
#if QT_VERSION < 0x040000
		list.remove(it++);
#else
		list.erase(it++);
#endif
	}

	//get the current settings for this option
	int idx = 0;
	if ( style == "Defaults" || style == "Default" ) 
		style = "Default Item Styles - Schema ";
	else { 
#if QT_VERSION < 0x040000
		style = "Highlighting " + style.simplifyWhiteSpace() + " - Schema ";
#else
		style = "Highlighting " + style.trimmed() + " - Schema ";
#endif
		idx++;
	}
	style += YZSession::me->schemaManager()->name(0); //XXX make it use the 'current' schema
	YZSession::mOptions->setGroup(style);
	QStringList option = YZSession::mOptions->readListOption(type);
	yzDebug() << "HIGHLIGHT : Current " << type << " : " << option << endl;
	if (option.count() < 7) return CMD_ERROR; //just make sure it's fine ;)

	end = list.end();
	//and update it with parameters passed from user
	QRegExp rx("(\\S*)=(\\S*)");
	for (it=list.begin();it!=end; ++it) {
		yzDebug() << "Testing " << *it << endl;
		if ( rx.exactMatch(*it) ) { // fg=, selfg= ...
			QColor col (rx.cap(2)); //can be a name or rgb
			if ( rx.cap(1) == "fg" ) {
				option[idx] = QString::number(col.rgb(),16);
			} else if ( rx.cap(1) == "bg" ) {
				option[6+idx] = QString::number(col.rgb(),16);
			} else if ( rx.cap(1) == "selfg" ) {
				option[1+idx] = QString::number(col.rgb(),16);
			} else if ( rx.cap(1) == "selbg" ) {
				option[7+idx] = QString::number(col.rgb(),16);
			}
		} else { // bold, noitalic ...
			if ( *it=="bold" )
				option[2+idx] = "1";
			if ( *it=="nobold" )
				option[2+idx] = "0";
			if ( *it=="italic" )
				option[3+idx] = "1";
			if ( *it=="noitalic" )
				option[3+idx] = "0";
			if ( *it=="strikeout" )
				option[4+idx] = "1";
			if ( *it=="nostrikeout" )
				option[4+idx] = "0";
			if ( *it=="underline" )
				option[5+idx] = "1";
			if ( *it=="nounderline" )
				option[5+idx] = "0";
		}
	}
	yzDebug() << "HIGHLIGHT : Setting new " << option << endl;
	YZSession::mOptions->getOption( type )->setList( option );
	YZSession::mOptions->setGroup("Global");

	if ( args.view && args.view->myBuffer() ) {
		YzisHighlighting *yzis = args.view->myBuffer()->highlight();
		if (yzis) {
			args.view->myBuffer()->makeAttribs();
			args.view->sendRefreshEvent();
		}
	}

	return CMD_OK;
}

cmd_state YZModeEx::split( const YZExCommandArgs& args ) {
	YZSession::me->splitHorizontally(args.view);
	return CMD_OK;
}

