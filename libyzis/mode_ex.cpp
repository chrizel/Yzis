/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2004 Thomas Capricelli <orzel@freehackers.org>,
 *  Copyright (C) 2004 Philippe Fremy <phil@freehackers.org>,
 *  Copyright (C) 2004 Pascal "Poizon" Maillard <poizon@gmx.at>,
 *  Copyright (C) 2004-2005 Loic Pauleve <panard@inzenet.org>
 *  Copyright (C) 2005 Erlend Hamberg <ehamberg@online.no>
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
#include "tags_interface.h"
#include "search.h"
#include "internal_options.h"
#include "viewcursor.h"
#include "history.h"

#include <qobject.h>
#include <qfileinfo.h>
#include <qdir.h>


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
	mHistory = new YZHistory;
}

YZModeEx::~YZModeEx() {
	for ( YZList<const YZExCommand*>::Iterator itr = commands.begin(); itr != commands.end(); ++itr ) {
		delete *itr;
	}
	for ( YZList<const YZExRange*>::Iterator itr = ranges.begin(); itr != ranges.end(); ++itr ) {
		delete *itr;
	}
	
	delete mHistory;
}
void YZModeEx::init() {
	initPool();
}
void YZModeEx::enter( YZView* view ) {
	YZSession::me->setFocusCommandLine();
	view->setCommandLineText( "" );
}
void YZModeEx::leave( YZView* view ) {
	view->setCommandLineText( "" );
	YZSession::me->setFocusMainWindow();
}
cmd_state YZModeEx::execCommand( YZView* view, const QString& key ) {
	yzDebug() << "YZModeEx::execCommand " << key << endl;
	cmd_state ret = CMD_OK;
	if ( key == "<ENTER>" ) {
		if( view->getCommandLineText().isEmpty()) {
			view->modePool()->pop();
		} else {
			QString cmd = view->getCommandLineText();
			mHistory->addEntry( cmd );
			ret = execExCommand( view, cmd );
			if ( ret != CMD_QUIT ) 
				view->modePool()->pop( MODE_COMMAND );
		}
	} else if ( key == "<DOWN>" ) {
		mHistory->goForwardInTime();
		view->setCommandLineText( mHistory->getEntry() );
	} else if ( key == "<LEFT>" || key == "<RIGHT>" ) {
	} else if ( key == "<UP>" ) {
		mHistory->goBackInTime();
		view->setCommandLineText( mHistory->getEntry() );
	} else if ( key == "<ESC>" || key == "<CTRL>c" ) {
		view->modePool()->pop( MODE_COMMAND );
	} else if ( key == "<TAB>" ) {
		//ignore for now
	} else if ( key == "<BS>" ) {
		QString back = view->getCommandLineText();
		if ( back.isEmpty() ) {
			view->modePool()->pop();
			return ret;
		}
		view->setCommandLineText(back.remove(back.length() - 1, 1));
	} else {
		view->setCommandLineText( view->getCommandLineText() + key );
	}
	return ret;
}

void YZModeEx::initPool() {
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
	commands.push_back( new YZExCommand( "q(uit|a(ll)?)?", &YZModeEx::quit, QStringList::split(":","quit:qall") ) );
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
}

QString YZModeEx::parseRange( const QString& inputs, YZView* view, int* range, bool* matched ) {
	QString _input = inputs;
	*matched = false;
	for ( YZList<const YZExRange*>::Iterator itr = ranges.begin(); !*matched && itr != ranges.end(); ++itr ) {
		const YZExRange *currentRange = *itr;
		QRegExp reg( currentRange->regexp() );
		*matched = reg.exactMatch( _input );
		if ( *matched ) {
			unsigned int nc = reg.numCaptures();
			*range = (this->*( currentRange->poolMethod() )) (YZExRangeArgs( currentRange, view, reg.cap( 1 ) ));
			QString s_add = reg.cap( nc - 1 );
			yzDebug() << "matched " << currentRange->keySeq() << " : " << *range << " and " << s_add << endl;
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
	QString _input = inputs.stripWhiteSpace();
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
	for ( YZList<const YZExCommand*>::Iterator itr = commands.begin(); !matched && itr != commands.end(); ++itr ) {
		const YZExCommand *curCommand = *itr;
		QRegExp reg(curCommand->regexp());
		matched = reg.exactMatch( _input );
		if ( matched ) {
			unsigned int nc = reg.numCaptures();
			yzDebug() << "matched " << curCommand->keySeq() << " " << reg.cap( 1 ) << "," << reg.cap( nc ) << endl;
			QString arg = reg.cap( nc );
			bool force = arg[ 0 ] == '!';
			if ( force ) arg = arg.mid( 1 );
			ret = (this->*( curCommand->poolMethod() )) (YZExCommandArgs( view, _input, reg.cap( 1 ), arg.stripWhiteSpace(), from, to, force ) );
			commandIsValid = true;
		}
	}
	if ( _input.length() == 0 ) {
		view->gotoxy( 0, to );
		view->moveToFirstNonBlankOfLine();
	} else if ( !commandIsValid ) {
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
			pos = YZSession::me->search()->replayBackward( args.view->myBuffer(), &found, NULL, true );
		} else {
			pos = YZSession::me->search()->replayForward( args.view->myBuffer(), &found, NULL, true );
		}
	} else {
		QString pat = args.arg.mid( 1, args.arg.length() - 2 );
		if ( reverse ) 
			pat.replace( "\\?", "?" );
		else
			pat.replace( "\\/", "/" );
		yzDebug() << "rangeSearch : " << pat << endl;
		pos = YZSession::me->search()->forward( args.view->myBuffer(), pat, &found, args.view->getBufferCursor() );
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
	}
	if ( quit && force ) {//check readonly ? XXX
		args.view->myBuffer()->save();
		args.view->mySession()->deleteView( args.view->getId() );
		ret = CMD_QUIT;
	} else if ( quit ) {
		if ( args.view->myBuffer()->save() ) {
			args.view->mySession()->deleteView( args.view->getId() );
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
			args.view->mySession()->deleteView( args.view->getId() );
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
				args.view->mySession()->deleteView(args.view->getId());
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
	
	YZView *v = YZSession::me->nextView();
	YZASSERT( v!=args.view );
	
	if ( v ) {
		YZSession::me->setCurrentView(v);
	} else {
		bufferfirst( args ); // goto first buffer
	}
	
	return CMD_OK;
}

cmd_state YZModeEx::bufferprevious ( const YZExCommandArgs& args ) {
	yzDebug() << "Switching buffers (actually sw views) ..." << endl;
	
	YZView *v = YZSession::me->prevView();
	YZASSERT( v!=args.view );
	
	if ( v ) {
		YZSession::me->setCurrentView(v);
	} else {
		bufferlast( args ); // goto lastbuffer
	}

	return CMD_OK;
}

cmd_state YZModeEx::bufferdelete ( const YZExCommandArgs& args ) {
	yzDebug() << "Delete buffer " << args.view->myBuffer()->getId() << endl;

	YZList<YZView*> l = args.view->myBuffer()->views();
	for ( YZList<YZView*>::Iterator itr = l.begin(); itr != l.end(); ++itr ) {
		args.view->mySession()->deleteView( (*itr)->getId() );
	}

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
	path = fi.absFilePath();
	YZBuffer *b =args.view->mySession()->findBuffer(path);
	if (b) {
		yzDebug() << "Buffer already loaded" << endl;
		args.view->mySession()->setCurrentView(b->firstView());
		return CMD_OK;
	}
	yzDebug() << "New buffer / view : " << path << endl;
	args.view->mySession()->createBuffer( path );
	YZBuffer *bu = args.view->mySession()->findBuffer( path );
	YZASSERT_MSG( bu != NULL, QString("Created buffer %1 was not found!").arg(path) );
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
	bool success = YZSession::me->getOptions()->setOptionFromString( &matched, 
	args.arg.simplifyWhiteSpace()
	, user_scope, buff, args.view );
	
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
	YZSession::me->getOptions()->saveTo( QDir::currentDirPath() + "/yzis.conf", "", "HL Cache", args.force );
	return CMD_OK;
}

cmd_state YZModeEx::substitute( const YZExCommandArgs& args ) {
	unsigned int idx = args.input.find("substitute");
	unsigned int len = 10;
	if (static_cast<unsigned int>(-1)==idx) {
		idx = args.input.find("s");
		len = 1;
	}
	unsigned int idxb,idxc;
	unsigned int tidx = idx+len;
	QChar c;
	while ((c = args.input.at(tidx)).isSpace())
		tidx++;
	idx = args.input.find(c, tidx);
	idxb = args.input.find(c, idx+1);
	idxc = args.input.find(c, idxb+1);
	QString search = args.input.mid( idx+1, idxb-idx-1 );
	QString replace = args.input.mid( idxb+1, idxc-idxb-1 );
	QString options = args.input.mid( idxc+1 );

	bool needsUpdate = false;
	bool found;
	if ( options.contains( "i" ) && !search.endsWith( "\\c" ) ) {
		search.append("\\c");
	}
	unsigned int lastLine;
	YZCursor start( 0, args.fromLine );
	YZSession::me->search()->forward( args.view->myBuffer(), search, &found, &start );
	if ( found ) {
		for( unsigned int i = args.fromLine; i <= args.toLine; i++ ) {
			if ( args.view->myBuffer()->substitute( search, replace, options.contains( "g" ), i ) ) {
				needsUpdate = true;
				lastLine = i;
			}
		}
	}
	if ( needsUpdate ) {
		args.view->myBuffer()->updateAllViews();
		args.view->gotoxy( 0, lastLine );
		args.view->moveToFirstNonBlankOfLine();
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
	path = fi.absFilePath( );
	args.view->printToFile( path );
	return CMD_OK;
}

cmd_state YZModeEx::preserve( const YZExCommandArgs& args  ) {
	args.view->myBuffer()->preserve();
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
			YZViewList views = YZSession::me->getAllViews();
			for ( YZViewList::const_iterator itr = views.begin(); itr != views.end(); ++itr ) {
				YZView *v = *itr;
				v->registerModifierKeys( rx.cap( 1 ) );
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
		mModifierKeys.remove(args.arg);
		YZViewList views = YZSession::me->getAllViews();
		for ( YZViewList::const_iterator itr = views.begin(); itr != views.end(); ++itr ) {
			YZView *v = *itr;
			v->unregisterModifierKeys( args.arg );
		}
	}
	return CMD_OK;	
}

cmd_state YZModeEx::genericNoremap ( const YZExCommandArgs& args, int type) {
	QRegExp rx("(\\S+)\\s+(.+)");
	if ( rx.exactMatch(args.arg) ) {
		// yzDebug() << "Adding noremapping : " << rx.cap(1) << " to " << rx.cap(2) << endl;
		switch (type) {
			case 0://global
				YZMapping::self()->addGlobalNoreMapping(rx.cap(1), rx.cap(2));
				break;
			case 1://insert
				YZMapping::self()->addInsertNoreMapping(rx.cap(1), rx.cap(2));
				break;
			case 2://operator
				YZMapping::self()->addPendingOpNoreMapping(rx.cap(1), rx.cap(2));
				break;
			case 3://visual
				YZMapping::self()->addVisualNoreMapping(rx.cap(1), rx.cap(2));
				break;
			case 4://normal
				YZMapping::self()->addNormalNoreMapping(rx.cap(1), rx.cap(2));
				break;
			case 5://cmdline
				YZMapping::self()->addCmdLineNoreMapping(rx.cap(1), rx.cap(2));
				break;
		}
		if (rx.cap(1).startsWith("<CTRL>")) {
			mModifierKeys << rx.cap(1);
			YZViewList views = YZSession::me->getAllViews();
			for ( YZViewList::const_iterator itr = views.begin(); itr != views.end(); ++itr ) {
				YZView *v = *itr;
				v->registerModifierKeys( rx.cap( 1 ) );
			}
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

cmd_state YZModeEx::noremap( const YZExCommandArgs& args ) {
	return genericNoremap(args,0);
}

cmd_state YZModeEx::inoremap( const YZExCommandArgs& args ) {
	return genericNoremap(args,1);
}

cmd_state YZModeEx::onoremap( const YZExCommandArgs& args ) {
	return genericNoremap(args,2);
}

cmd_state YZModeEx::vnoremap( const YZExCommandArgs& args ) {
	return genericNoremap(args,3);
}

cmd_state YZModeEx::nnoremap( const YZExCommandArgs& args ) {
	return genericNoremap(args,4);
}

cmd_state YZModeEx::cnoremap( const YZExCommandArgs& args ) {
	return genericNoremap(args,5);
}

cmd_state YZModeEx::indent( const YZExCommandArgs& args ) {
	int count = 1;
	if ( args.arg.length() > 0 ) count = args.arg.toUInt();
	if ( args.cmd[ 0 ] == '<' ) count *= -1;
	for ( unsigned int i = args.fromLine; i <= args.toLine; i++ ) {
		args.view->myBuffer()->action()->indentLine( args.view, i, count );
	}
	args.view->commitNextUndo();
	args.view->gotoxy( 0, args.toLine );
	args.view->moveToFirstNonBlankOfLine();
	return CMD_OK;
}

cmd_state YZModeEx::enew( const YZExCommandArgs& ) {
	YZSession::me->createBuffer();
	return CMD_OK;
}

cmd_state YZModeEx::registers( const YZExCommandArgs& ) {
	QString infoMessage(_("Registers:\n")); // will contain register-value table
	QValueList<QChar> keys = YZSession::me->getRegisters();
	QValueList<QChar>::ConstIterator it = keys.begin(), end = keys.end();
	QString regContents;
	for( ; it != end ; ++it )
	{
		infoMessage += QString("\"") + (*it) + "  ";
		// why I use space as separator? I don't know :)
		// if you know what must be used here, fix it ;)
		regContents = YZSession::me->getRegister( *it ).join(" ");
		// FIXME dimsuz: maybe replace an abstract 27 with some predefined value?
		if( regContents.length() >= 27 )
		{
			// if register contents is too large, truncate it a little
			regContents.truncate( 27 );
			regContents += "...";
		}
		infoMessage += regContents + "\n";
	}
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
	QStringList list = QStringList::split(" ", args.arg);
	QStringList::Iterator it = list.begin(), end = list.end();
	yzDebug() << list << endl;
	if (list.count() < 3) return CMD_ERROR; //at least 3 parameters...
	QString style = list[0];
	QString type = list[1];
	list.remove(it++); list.remove(it++);
	if (!list[0].contains("=") && !list[0].endsWith("bold") && !list[0].endsWith("italic") && !list[0].endsWith("underline") && !list[0].endsWith("strikeout")) {
		type += " " + list[0];
		list.remove(it++);
	}

	//get the current settings for this option
	int idx = 0;
	if ( style == "Defaults" || style == "Default" ) 
		style = "Default Item Styles - Schema ";
	else { 
		style = "Highlighting " + style.simplifyWhiteSpace() + " - Schema ";
		idx++;
	}
	style += YZSession::me->schemaManager()->name(0); //XXX make it use the 'current' schema
	YZSession::me->getOptions()->setGroup(style);
	QStringList option = YZSession::me->getOptions()->readListOption(type);
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
	YZSession::me->getOptions()->getOption( type )->setList( option );
	YZSession::me->getOptions()->setGroup("Global");

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

cmd_state YZModeEx::cd( const YZExCommandArgs& args ) {
        QString targetDir = YZBuffer::tildeExpand(args.arg);
	if ( QDir::setCurrent(targetDir) ) {
		// we could be using a new tag file, so reset tags
		tagReset();
		return CMD_OK;
	} else {
		args.view->mySession()->popupMessage( _( "Cannot change to specified directory" ) );
		return CMD_ERROR;
	}
}

cmd_state YZModeEx::pwd( const YZExCommandArgs& args ) {
	args.view->mySession()->popupMessage( _( QDir::current().absPath() ) );
	return CMD_OK;
}

cmd_state YZModeEx::tag( const YZExCommandArgs& args ) {
	tagJumpTo(args.arg);

	return CMD_OK;
}

cmd_state YZModeEx::pop( const YZExCommandArgs& /*args*/ ) {
	tagPop();

	return CMD_OK;
}

cmd_state YZModeEx::tagnext( const YZExCommandArgs& /*args*/ ) {
	tagNext();

	return CMD_OK;
}

cmd_state YZModeEx::tagprevious( const YZExCommandArgs& /*args*/ ) {
	tagPrev();

	return CMD_OK;
}

cmd_state YZModeEx::retab( const YZExCommandArgs& args ) {
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
			YZSession::me->getOptions()->setOptionFromString( args.arg.simplifyWhiteSpace().insert(0, "tabstop="),
					local_scope, args.view->myBuffer(), args.view );
			tabstop = args.arg.toInt();
		}
		else {
			// Value must be > 0 FIXME: The user should get an error message
			return CMD_ERROR;
		}
	}

	for (unsigned int lnum = 0; lnum < buffer->lineCount(); lnum++) {
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
				}
				else {
					gotTab = true;
				}
			}
			else {
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
						newLine.insert(startCol + len, oldLine.mid(col, oldLen - col +1));

						for (col = 0; col < len; col++)
							newLine[col+startCol] = (col < numTabs) ? '\t' : ' ';
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
			if (oldLine[col] == QChar::null)
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

	return CMD_OK;
}

YZHistory *YZModeEx::getHistory()
{
	return mHistory;
}
