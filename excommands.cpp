/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Mickael Marchand <marchand@kde.org>,
 *  Thomas Capricelli <orzel@freehackers.org>,
 *  Philippe Fremy <phil@freehackers.org>,
 *  Pascal "Poizon" Maillard <poizon@gmx.at>,
 *  Loic Pauleve <panard@inzenet.org>
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

#include <qobject.h>
#include <qfileinfo.h>
#include <qdir.h>

#include "excommands.h"
#include "buffer.h"
#include "session.h"
#include "swapfile.h"
#include "ex_lua.h"

YZExCommandPool::YZExCommandPool() {
	commands.clear();
	commands.setAutoDelete( true );
	ranges.clear();
	ranges.setAutoDelete( true );
}

YZExCommandPool::~YZExCommandPool() {
	commands.clear();
	ranges.clear();
}

void YZExCommandPool::initPool() {
	// ranges
	ranges.append( new YZExRange( "\\d+", &YZExCommandPool::rangeLine ) );
	ranges.append( new YZExRange( "\\.", &YZExCommandPool::rangeCurrentLine ) );
	ranges.append( new YZExRange( "\\$", &YZExCommandPool::rangeLastLine ) );
	// commands
	commands.append( new YZExCommand( "(x|wq?)(a(ll)?)?", &YZExCommandPool::write ) );
	commands.append( new YZExCommand( "w(rite)?", &YZExCommandPool::write ) );
	commands.append( new YZExCommand( "q(uit|a(ll)?)?", &YZExCommandPool::quit ) );
	commands.append( new YZExCommand( "bn(ext)?", &YZExCommandPool::buffernext ) );
	commands.append( new YZExCommand( "bp(revious)?", &YZExCommandPool::bufferprevious ) );
	commands.append( new YZExCommand( "bd(elete)?", &YZExCommandPool::bufferdelete ) );
	commands.append( new YZExCommand( "e(dit)?", &YZExCommandPool::edit ) );
	commands.append( new YZExCommand( "mkyzisrc", &YZExCommandPool::mkyzisrc ) );
	commands.append( new YZExCommand( "setlocal", &YZExCommandPool::setlocal ) );
	commands.append( new YZExCommand( "set", &YZExCommandPool::set ) );
	commands.append( new YZExCommand( "s(ubstitute)?", &YZExCommandPool::substitute ) );
	commands.append( new YZExCommand( "hardcopy", &YZExCommandPool::hardcopy ) );
	commands.append( new YZExCommand( "open", &YZExCommandPool::gotoOpenMode ) );
	commands.append( new YZExCommand( "visual", &YZExCommandPool::gotoCommandMode ) );
	commands.append( new YZExCommand( "preserve", &YZExCommandPool::preserve ) );
	commands.append( new YZExCommand( "lua", &YZExCommandPool::lua ) );
	commands.append( new YZExCommand( "source", &YZExCommandPool::luaLoadFile ) );
}

QString YZExCommandPool::parseRange( const QString& inputs, YZView* view, int* range, bool* matched ) {
	QString _input = inputs;
	*matched = false;
	for ( ranges.first(); ! *matched && ranges.current(); ranges.next() ) {
		QRegExp reg( "^(" + ranges.current()->keySeq() + ")([+\\-]\\d*)?(.*)$" );
		*matched = reg.exactMatch( _input );
		if ( *matched ) {
			unsigned int nc = reg.numCaptures();
			*range = (this->*( ranges.current()->poolMethod() )) (YZExRangeArgs( ranges.current(), view, reg.cap( 1 ) ));
			QString s_add = reg.cap( nc - 1 );
//			yzDebug() << "matched " << ranges.current()->keySeq() << " : " << *range << " and " << s_add << endl;
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

bool YZExCommandPool::execCommand( YZView* view, const QString& inputs ) {
	bool ret = false, matched;
	int from, to;
	QString _input = inputs.stripWhiteSpace();
	yzDebug() << "ExCommand : " << _input << endl;
	_input = _input.replace( QRegExp( "^%" ), "1,$" );
	// range
	from = to = rangeCurrentLine( YZExRangeArgs( NULL, view, "." ) );

	_input = parseRange( _input, view, &from, &matched );
	if ( matched ) to = from;
	if ( matched && _input[ 0 ] == ',' ) {
		_input = _input.mid( 1 );
		yzDebug() << "ExCommand : still " << _input << endl;
		_input = parseRange( _input, view, &to, &matched );
	}
	if ( from > to ) {
		unsigned int tmp = to;
		to = from;
		from = tmp;
	}
	yzDebug() << "ExCommand : naked command : " << _input << "; range " << from << "," << to << endl;

	matched = false;
	for ( commands.first(); ! matched && commands.current(); commands.next() ) {
		QRegExp reg( "^(" + commands.current()->keySeq() + ")(!)?(.*)$" );
		matched = reg.exactMatch( _input );
		if ( matched ) {
			unsigned int nc = reg.numCaptures();
			yzDebug() << "matched " << commands.current()->keySeq() << " " << reg.cap( 1 ) 
				<< "," << reg.cap( nc - 1 ) << "," << reg.cap( nc ) << endl;
			(this->*( commands.current()->poolMethod() )) (YZExCommandArgs( view, _input, reg.cap( 1 ), reg.cap( nc ).stripWhiteSpace(), 
					from, to, reg.cap( nc - 1 ).length() > 0 ));
		}
	}
	if ( ! matched ) view->gotoStickyCol( to );

	return ret;
}

/**
 * RANGES
 */

int YZExCommandPool::rangeLine( const YZExRangeArgs& args ) {
	return args.arg.toUInt() - 1;
}
int YZExCommandPool::rangeCurrentLine( const YZExRangeArgs& args ) {
	return args.view->getBufferCursor()->getY();
}
int YZExCommandPool::rangeLastLine( const YZExRangeArgs& args ) {
	return QMAX( args.view->myBuffer()->lineCount() - 1, 0 );
}

/**
 * COMMANDS
 */
QString YZExCommandPool::write( const YZExCommandArgs& args ) {
	bool quit = args.cmd.contains( 'q') || args.cmd.contains( 'x' );
	bool all = args.cmd.contains( 'a' );
	if ( ! quit && all ) {
		args.view->mySession()->saveAll();
		return QString::null;
	}
	if ( quit && all ) {//write all modified buffers
		if ( args.view->mySession()->saveAll() ) //if it fails => dont quit
			args.view->mySession()->exitRequest();
		return QString::null;
	}
	if ( args.arg.length() ) {
		args.view->myBuffer()->setPath( args.arg ); //a filename was given as argument
		args.view->myBuffer()->getSwapFile()->setFileName( args.view->myBuffer()->fileName()+".ywp" );
		args.view->myBuffer()->getSwapFile()->init();
	}
	if ( quit && args.force ) {//check readonly ? XXX
		args.view->myBuffer()->save();
		args.view->mySession()->exitRequest(); //whatever happens => quit
	} else if ( quit ) {
		if ( args.view->myBuffer()->save() )
			args.view->mySession()->exitRequest();
	} else if ( ! args.force ) {
		args.view->myBuffer()->save();
	} else if ( args.force ) {
		args.view->myBuffer()->save();
	}
	return QString::null;
}
QString YZExCommandPool::quit( const YZExCommandArgs& args ) {
	yzDebug() << "View counts: "<< args.view->myBuffer()->views().count() 
		<< " Buffer Count : " << args.view->mySession()->countBuffers() << endl;
	if ( args.cmd.startsWith( "qa" ) ) {
		if ( args.force || ! args.view->mySession()->isOneBufferModified() ) {
			args.view->mySession()->exitRequest( );
		} else args.view->mySession()->popupMessage( QObject::tr( "One file is modified ! Save it first ..." ) );
	} else {
		//close current view, if it's the last one on a buffer , check it is saved or not
		if ( args.view->myBuffer()->views().count() > 1 )
			args.view->mySession()->deleteView( args.view->myId );
		else if ( args.view->myBuffer()->views().count() == 1 && args.view->mySession()->countBuffers() == 1) {
			if ( args.force || !args.view->myBuffer()->fileIsModified() )
				args.view->mySession()->exitRequest();
			else args.view->mySession()->popupMessage( QObject::tr( "One file is modified ! Save it first ..." ) );
		} else {
			if ( args.force || !args.view->myBuffer()->fileIsModified() )
				args.view->mySession()->deleteView(args.view->myId);
			else args.view->mySession()->popupMessage( QObject::tr( "One file is modified ! Save it first ..." ) );
		}
	}
	return QString::null;
}

QString YZExCommandPool::buffernext( const YZExCommandArgs& args ) {
	yzDebug() << "Switching buffers (actually sw views) ..." << endl;
	YZView *v = args.view->mySession()->nextView();
	YZASSERT( v!=args.view );
	if ( v )
		args.view->mySession()->setCurrentView(v);
	else
		args.view->mySession()->popupMessage( QObject::tr( "No next buffer" ) );
	return QString::null;
}

QString YZExCommandPool::bufferprevious ( const YZExCommandArgs& args ) {
	yzDebug() << "Switching buffers (actually sw views) ..." << endl;
	YZView *v = args.view->mySession()->prevView();
	YZASSERT( v!=args.view );
	if ( v )
		args.view->mySession()->setCurrentView(v);
	else
		args.view->mySession()->popupMessage( QObject::tr( "No previous buffer" ) );

	return QString::null;
}

QString YZExCommandPool::bufferdelete ( const YZExCommandArgs& args ) {
	yzDebug() << "Delete buffer " << args.view->myBuffer()->myId << endl;

	QPtrList<YZView> l = args.view->myBuffer()->views();
	YZView *v;
	args.view->myBuffer()->clearSwap();
	for ( v = l.first(); v; v=l.next() ) {
		args.view->mySession()->deleteView( args.view->myId );
	}

	return QString::null;
}

QString YZExCommandPool::gotoCommandMode( const YZExCommandArgs& args ) {
	args.view->gotoCommandMode();
	return QString::null;
}

QString YZExCommandPool::gotoOpenMode( const YZExCommandArgs& args ) {
	yzDebug() << "Switching to open mode...";
	args.view->gotoOpenMode();
	yzDebug() << "done." << endl;
	return QString::null;
}

QString YZExCommandPool::edit ( const YZExCommandArgs& args ) {
	QString path = args.arg; //extract the path
	if ( path.length() == 0 ) {
		args.view->mySession()->popupMessage( QObject::tr( "Please specify a filename" ) );
		return QString::null;
	}
	//check the file name
	QFileInfo fi ( path );
	path = fi.absFilePath();
	yzDebug() << "New buffer / view : " << path << endl;
	args.view->mySession()->createBuffer( path );
	return QString::null;
}

QString YZExCommandPool::setlocal ( const YZExCommandArgs& args ) {
	QRegExp rx ( "(\\w*)(\\+|-)?=(.*)" ); //option with value
	QRegExp rx2 ( "no(\\w*)" ); //deactivate a bool option
	QRegExp rx3 ( "(\\w*)" ); //activate a bool option

	YZSession::mOptions.setGroup("Global");

	if ( rx.exactMatch( args.arg ) ) {
		QString option = rx.cap( 2 ).simplifyWhiteSpace();
		bool hasOperator = rx.numCaptures() == 4; // do we have a +/- in the set command ?
		QString value = hasOperator ? rx.cap( 4 ).simplifyWhiteSpace() : rx.cap( 3 ).simplifyWhiteSpace();
		YZInternalOption *opt = YZSession::mOptions.getOption(option);
		if ( !opt ) {
			args.view->mySession()->popupMessage( QObject::tr("Invalid option given : ") + option);
			return QString::null;
		}
		if ( hasOperator ) {
			QString oldVal;
			switch ( opt->getType() ) {
				case view_opt:
					oldVal = args.view->getLocalStringOption( option );
					break;
				case buffer_opt:
					oldVal = args.view->myBuffer()->getLocalStringOption( option );
				case global_opt:
					break;
			}
			switch ( opt->getValueType() ) {
				case string_t :
					if ( rx.cap( 3 ) == "+" ) value = oldVal + value;
					else if ( rx.cap( 3 ) == "-" ) value = oldVal.remove( value );
					break;
				case int_t :
					if ( rx.cap( 3 ) == "+" ) value = QString::number( oldVal.toInt() + value.toInt() );
					else if ( rx.cap( 3 ) == "-" ) value = QString::number( oldVal.toInt() - value.toInt() );
					break;
				case bool_t :
					args.view->mySession()->popupMessage(QObject::tr("This option cannot be switched this way, this is a boolean option."));
					return QString::null;
					break;
				case color_t :
					break;
			}
		}
		yzDebug() << "Setting option " << option << " to " << value << endl;
		switch ( opt->getType() ) {
			case global_opt :
				args.view->mySession()->popupMessage(QObject::tr("This option is a global option which cannot be changed with setlocal"));
				return QString::null;
			case view_opt :
				args.view->setLocalQStringOption( option, value );
				break;
			case buffer_opt:
				args.view->myBuffer()->setLocalQStringOption( option, value );
				break;
		}
	} else if ( rx2.exactMatch( args.arg )) {
		YZInternalOption *opt = YZSession::mOptions.getOption(rx2.cap( 2 ).simplifyWhiteSpace());
		if ( !opt ) {
			args.view->mySession()->popupMessage(QObject::tr("Invalid option given"));
			return QString::null;
		}
		switch ( opt->getType() ) {
			case global_opt :
				args.view->mySession()->popupMessage(QObject::tr("This option is a global option which cannot be changed with setlocal"));
				return QString::null;
			case view_opt :
				args.view->setLocalBoolOption( rx2.cap( 2 ).simplifyWhiteSpace(), false);
				break;
			case buffer_opt:
				args.view->myBuffer()->setLocalBoolOption( rx2.cap( 2 ).simplifyWhiteSpace(), false);
				break;
		}
	} else if ( rx3.exactMatch( args.arg ) ) {
		YZInternalOption *opt = YZSession::mOptions.getOption(rx3.cap( 2 ).simplifyWhiteSpace());
		if ( !opt ) {
			args.view->mySession()->popupMessage(QObject::tr("Invalid option given"));
			return QString::null;
		}
		switch ( opt->getType() ) {
			case global_opt :
				args.view->mySession()->popupMessage(QObject::tr("This option is a global option which cannot be changed with setlocal"));
				return QString::null;
			case view_opt :
				args.view->setLocalBoolOption( rx3.cap( 2 ).simplifyWhiteSpace(), true);
				break;
			case buffer_opt:
				args.view->myBuffer()->setLocalBoolOption( rx3.cap( 2 ).simplifyWhiteSpace(), true);
				break;
		}
	} else {
		args.view->mySession()->popupMessage( QObject::tr( "Invalid option given" ) );
		return QString::null;
	}
	// refresh screen
	args.view->reset();

	return QString::null;
}

QString YZExCommandPool::set ( const YZExCommandArgs& args ) {
	QRegExp rx ( "(\\w*)(\\+|-)?=(.*)" ); //option with value
	QRegExp rx2 ( "no(\\w*)" ); //deactivate a bool option
	QRegExp rx3 ( "(\\w*)" ); //activate a bool option

	if ( rx.exactMatch( args.arg ) ) {
		YZSession::mOptions.setGroup("Global");
		QString option = rx.cap( 2 ).simplifyWhiteSpace();
		bool hasOperator = rx.numCaptures() == 4; // do we have a +/- in the set command ?
		QString value = hasOperator ? rx.cap( 4 ).simplifyWhiteSpace() : rx.cap( 3 ).simplifyWhiteSpace();
		YZInternalOption *opt = YZSession::mOptions.getOption(option);
		if ( !opt ) {
			args.view->mySession()->popupMessage(QObject::tr("Invalid option given : ") + option);
			return QString::null;
		}
		if ( hasOperator ) {
			switch ( opt->getValueType() ) {
				case string_t :
					if ( rx.cap( 3 ) == "+" ) value = YZSession::mOptions.readQStringEntry( option ) + value;
					else if ( rx.cap( 3 ) == "-" ) value = QString( YZSession::mOptions.readQStringEntry( option ) ).remove( value );
					break;
				case int_t :
					if ( rx.cap( 3 ) == "+" ) value = QString::number( YZSession::mOptions.readQStringEntry( option ).toInt() + value.toInt() );
					else if ( rx.cap( 3 ) == "-" ) value = QString::number( YZSession::mOptions.readQStringEntry( option ).toInt() - value.toInt() );
					break;
				case bool_t :
					args.view->mySession()->popupMessage(QObject::tr("This option cannot be switched this way, this is a boolean option."));
					return QString::null;
					break;
				case color_t :
					break;
			}
		}
		yzDebug() << "Setting option " << option << " to " << value << endl;
		YZSession::setQStringOption( option, value );
	} else if ( rx2.exactMatch( args.arg )) {
		YZSession::mOptions.setGroup("Global");
		YZSession::setBoolOption( rx2.cap( 2 ).simplifyWhiteSpace(), false);
	} else if ( rx3.exactMatch( args.arg ) ) {
		YZSession::mOptions.setGroup("Global");
		YZSession::setBoolOption( rx3.cap( 2 ).simplifyWhiteSpace(), true);
	} else {
		args.view->mySession()->popupMessage( QObject::tr( "Invalid option given" ) );
		return QString::null;
	}
	// refresh screen
	args.view->reset();

	return QString::null;
}

QString YZExCommandPool::mkyzisrc ( const YZExCommandArgs& ) {
	YZSession::mOptions.saveTo( QDir::currentDirPath() + "/yzis.conf", "Global" );
	return QString::null;
}

QString YZExCommandPool::substitute( const YZExCommandArgs& args ) {
	yzDebug() << "substitute" << endl;
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
	for( unsigned int i = args.fromLine; i <= args.toLine; i++ ) {
		if ( args.view->myBuffer()->substitute( search, replace, options.contains( "g" ), i ) )
			needsUpdate = true;
	}
	if ( needsUpdate ) {
		args.view->myBuffer()->updateAllViews();
	}

	return QString::null;
}

QString YZExCommandPool::hardcopy( const YZExCommandArgs& args ) {
	if ( args.arg.length() == 0 ) {
		args.view->mySession()->popupMessage( QObject::tr( "Please specify a filename" ) );
		return QString::null;
	}
	QString path = args.arg;
	QFileInfo fi ( path );
	path = fi.absFilePath( );
	args.view->printToFile( path );
	return QString::null;
}

QString YZExCommandPool::preserve( const YZExCommandArgs& args  ) {
	args.view->myBuffer()->getSwapFile()->flush();
	return QString::null;
}

QString YZExCommandPool::lua( const YZExCommandArgs& args ) {
	return YZSession::me->luaExecutor()->lua( args.view );
}

QString YZExCommandPool::luaLoadFile( const YZExCommandArgs& args ) {
	return YZSession::me->luaExecutor()->loadFile( args.view, args.arg );
}

