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

#include "excommands.h"
#include "buffer.h"
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

YZExCommandPool::YZExCommandPool() {
	commands.clear();
	ranges.clear();
#if QT_VERSION < 0x040000
	commands.setAutoDelete( true );
	ranges.setAutoDelete( true );
#else
	for (int ab = 0 ; ab < commands.size() ; ++ab) 
		delete commands.at(ab);
	for (int ab = 0 ; ab < ranges.size() ; ++ab) 
		delete ranges.at(ab);
#endif
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
	ranges.append( new YZExRange( "'\\w", &YZExCommandPool::rangeMark ) );
	ranges.append( new YZExRange( "'[<>]", &YZExCommandPool::rangeVisual ) );
	ranges.append( new YZExRange( "/([^/]*/)?", &YZExCommandPool::rangeSearch ) );
	ranges.append( new YZExRange( "\\?([^\\?]*\\?)?", &YZExCommandPool::rangeSearch ) );

	// commands
	commands.append( new YZExCommand( "(x|wq?)(a(ll)?)?", &YZExCommandPool::write ) );
	commands.append( new YZExCommand( "w(rite)?", &YZExCommandPool::write , QStringList("write") ));
#if QT_VERSION < 0x040000
	commands.append( new YZExCommand( "q(uit|a(ll)?)?", &YZExCommandPool::quit, QStringList::split(":","quit:qall") ) );
#else
	commands.append( new YZExCommand( "q(uit|a(ll)?)?", &YZExCommandPool::quit, ( QStringList() << "quit" << "qall") ) );
#endif
	commands.append( new YZExCommand( "bn(ext)?", &YZExCommandPool::buffernext, QStringList("bnext") ) );
	commands.append( new YZExCommand( "bp(revious)?", &YZExCommandPool::bufferprevious, QStringList("bprevious") ) );
	commands.append( new YZExCommand( "bd(elete)?", &YZExCommandPool::bufferdelete, QStringList("bdelete") ) );
	commands.append( new YZExCommand( "e(dit)?", &YZExCommandPool::edit, QStringList("edit") ) );
	commands.append( new YZExCommand( "mkyzisrc", &YZExCommandPool::mkyzisrc, QStringList("mkyzisrc") ) );
	commands.append( new YZExCommand( "setlocal", &YZExCommandPool::setlocal, QStringList("setlocal") ) );
	commands.append( new YZExCommand( "set", &YZExCommandPool::set, QStringList("set") ) );
	commands.append( new YZExCommand( "s(ubstitute)?", &YZExCommandPool::substitute, QStringList("substitute") ) );
	commands.append( new YZExCommand( "hardcopy", &YZExCommandPool::hardcopy, QStringList("hardcopy") ) );
	commands.append( new YZExCommand( "open", &YZExCommandPool::gotoOpenMode, QStringList("open" )) );
	commands.append( new YZExCommand( "visual", &YZExCommandPool::gotoCommandMode, QStringList("visual") ) );
	commands.append( new YZExCommand( "preserve", &YZExCommandPool::preserve, QStringList("preserve") ) );
	commands.append( new YZExCommand( "lua", &YZExCommandPool::lua, QStringList("lua" )) );
	commands.append( new YZExCommand( "source", &YZExCommandPool::source, QStringList("source") ) );
	commands.append( new YZExCommand( "map", &YZExCommandPool::map, QStringList("map") ) );
	commands.append( new YZExCommand( "imap", &YZExCommandPool::imap, QStringList("imap") ) );
	commands.append( new YZExCommand( "[<>]", &YZExCommandPool::indent, QStringList(), false ));
	commands.append( new YZExCommand( "ene(w)?", &YZExCommandPool::enew, QStringList("enew") ));
	commands.append( new YZExCommand( "syn(tax)?", &YZExCommandPool::syntax, QStringList("syntax")));
	commands.append( new YZExCommand( "highlight", &YZExCommandPool::highlight, QStringList("highlight") ));
	commands.append( new YZExCommand( "split", &YZExCommandPool::split, QStringList("split") ));
}

QString YZExCommandPool::parseRange( const QString& inputs, YZView* view, int* range, bool* matched ) {
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

bool YZExCommandPool::execCommand( YZView* view, const QString& inputs ) {
	bool ret = false, matched;
	int from, to, current;
#if QT_VERSION < 0x040000
	YZView * it;
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
		return false;
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
			for ( it = view->myBuffer()->views().first(); it; it = view->myBuffer()->views().next() )
				it->setPaintAutoCommit( false );
			(this->*( commands.current()->poolMethod() )) (YZExCommandArgs( view, _input, reg.cap( 1 ), arg.stripWhiteSpace(), from, to, force ) );
			for ( it = view->myBuffer()->views().first(); it; it = view->myBuffer()->views().next() )
				it->commitPaintEvent();
#else
			for ( int bc = 0 ; bc < view->myBuffer()->views().size() ; ++bc )
				view->myBuffer()->views().at(bc)->setPaintAutoCommit( false );
			(this->*( commands.at(ab)->poolMethod() )) (YZExCommandArgs( view, _input, reg.cap( 1 ), arg.trimmed(), from, to, force ) );
			for ( int bc = 0 ; bc < view->myBuffer()->views().size() ; ++bc )
				view->myBuffer()->views().at(bc)->commitPaintEvent();
#endif
		}
	}
	if ( current != to ) {
		view->gotoxy( 0, to );
		view->moveToFirstNonBlankOfLine();
	}

	return ret;
}

/**
 * RANGES
 */

int YZExCommandPool::rangeLine( const YZExRangeArgs& args ) {
	unsigned int l = args.arg.toUInt();
	if ( l > 0 ) --l;
	return l;
}
int YZExCommandPool::rangeCurrentLine( const YZExRangeArgs& args ) {
	return args.view->getBufferCursor()->getY();
}
int YZExCommandPool::rangeLastLine( const YZExRangeArgs& args ) {
#if QT_VERSION < 0x040000
	return QMAX( (int)args.view->myBuffer()->lineCount() - 1, 0 );
#else
	return qMax( (int)args.view->myBuffer()->lineCount() - 1, 0 );
#endif
}
int YZExCommandPool::rangeMark( const YZExRangeArgs& args ) {
	bool found = false;
	YZCursorPos pos = args.view->myBuffer()->viewMarks()->get( args.arg.mid( 1 ), &found );
	if ( found )
		return pos.bPos->getY();
	return -1;
}
int YZExCommandPool::rangeVisual( const YZExRangeArgs& args ) {
	YZSelectionMap visual = args.view->visualSelection();
	if ( visual.size() ) {
		if ( args.arg.mid( 1 ) == "<" )
			return visual[ 0 ].from().getY();
		else if ( args.arg.mid( 1 ) == ">" )
			return visual[ 0 ].to().getY();
	}
	return -1;
}
int YZExCommandPool::rangeSearch( const YZExRangeArgs& args ) {
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
		return pos.getY();
	}
	return -1;
}

/**
 * COMMANDS
 */
QString YZExCommandPool::write( const YZExCommandArgs& args ) {
	bool quit = args.cmd.contains( 'q') || args.cmd.contains( 'x' );
	bool all = args.cmd.contains( 'a' );
	bool force = args.force;
	if ( ! quit && all ) {
		args.view->mySession()->saveAll();
		return QString::null;
	}
	yzDebug() << args.arg << "," << args.cmd << " " << quit << " " << force << endl;
	if ( quit && all ) {//write all modified buffers
		if ( args.view->mySession()->saveAll() ) //if it fails => dont quit
			args.view->mySession()->exitRequest();
		return QString::null;
	}
	if ( args.arg.length() ) {
		args.view->myBuffer()->setPath( args.arg ); //a filename was given as argument
		args.view->myBuffer()->getSwapFile()->setFileName( args.view->myBuffer()->fileName() );
	}
	if ( quit && force ) {//check readonly ? XXX
		args.view->myBuffer()->save();
		args.view->mySession()->exitRequest(); //whatever happens => quit
	} else if ( quit ) {
		if ( args.view->myBuffer()->save() )
			args.view->mySession()->exitRequest();
	} else if ( ! force ) {
		args.view->myBuffer()->save();
	} else if ( force ) {
		args.view->myBuffer()->save();
	}
	return QString::null;
}
QString YZExCommandPool::quit( const YZExCommandArgs& args ) {
	bool force = args.force;
	yzDebug() << "View counts: "<< args.view->myBuffer()->views().count() 
		<< " Buffer Count : " << args.view->mySession()->countBuffers() << endl;
	if ( args.cmd.startsWith( "qa" ) ) {
		if ( force || ! args.view->mySession()->isOneBufferModified() ) {
			args.view->mySession()->exitRequest( );
		} else args.view->mySession()->popupMessage( QObject::tr( "One file is modified ! Save it first ..." ) );
	} else {
		//close current view, if it's the last one on a buffer , check it is saved or not
		if ( args.view->myBuffer()->views().count() > 1 )
			args.view->mySession()->deleteView( args.view->myId );
		else if ( args.view->myBuffer()->views().count() == 1 && args.view->mySession()->countBuffers() == 1) {
			if ( force || !args.view->myBuffer()->fileIsModified() )
				args.view->mySession()->exitRequest();
			else args.view->mySession()->popupMessage( QObject::tr( "One file is modified ! Save it first ..." ) );
		} else {
			if ( force || !args.view->myBuffer()->fileIsModified() )
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
	if ( path.length() >= 1 && path[ 0 ] == '~' ) // expand ~
#if QT_VERSION < 0x040000
		path = QDir::homeDirPath() + path.mid( 1 );
	QFileInfo fi ( path );
	path = fi.absFilePath();
#else
		path = QDir::homePath() + path.mid( 1 );
	QFileInfo fi ( path );
	path = fi.absoluteFilePath();
#endif
	yzDebug() << "New buffer / view : " << path << endl;
	args.view->mySession()->createBuffer( path );
	return QString::null;
}

QString YZExCommandPool::setlocal ( const YZExCommandArgs& args ) {
	QRegExp rx ( "(\\w*)(\\+|-)?=(.*)" ); //option with value
	QRegExp rx2 ( "no(\\w*)" ); //deactivate a bool option
	QRegExp rx3 ( "(\\w*)" ); //activate a bool option

	YZView *view = args.view;
	YZBuffer *buffer = NULL;
	if ( !args.view ) { //hmm no view, that might be kdevelop that is starting , creating first the buffer then and then a view
		buffer = YZSession::me->currentBuffer();
	} else
		buffer = args.view->myBuffer();
	
	YZSession::mOptions.setGroup("Global");

	if ( rx.exactMatch( args.arg ) ) {
#if QT_VERSION < 0x040000
		QString option = rx.cap( 1 ).simplifyWhiteSpace();
		bool hasOperator = rx.numCaptures() == 3; // do we have a +/- in the set command ?
		QString value = hasOperator ? rx.cap( 3 ).simplifyWhiteSpace() : rx.cap( 2 ).simplifyWhiteSpace();
#else
		QString option = rx.cap( 1 ).trimmed();
		bool hasOperator = rx.numCaptures() == 3; // do we have a +/- in the set command ?
		QString value = hasOperator ? rx.cap( 3 ).trimmed() : rx.cap( 2 ).trimmed();
#endif
		YZInternalOption *opt = YZSession::mOptions.getOption(option);
		if ( !opt ) {
			YZSession::me->popupMessage( QObject::tr("Invalid option given : ") + option);
			return QString::null;
		}
		if ( hasOperator ) {
			QString oldVal;
			switch ( opt->getType() ) {
				case view_opt:
					if ( view ) oldVal = view->getLocalStringOption( option );
					break;
				case buffer_opt:
					if ( buffer ) oldVal = buffer->getLocalStringOption( option );
				case global_opt:
					break;
			}
			switch ( opt->getValueType() ) {
				case stringlist_t :
				case string_t :
					if ( rx.cap( 2 ) == "+" ) value = oldVal + value;
					else if ( rx.cap( 2 ) == "-" ) value = oldVal.remove( value );
					break;
				case int_t :
					if ( rx.cap( 2 ) == "+" ) value = QString::number( oldVal.toInt() + value.toInt() );
					else if ( rx.cap( 2 ) == "-" ) value = QString::number( oldVal.toInt() - value.toInt() );
					break;
				case bool_t :
					YZSession::me->popupMessage(QObject::tr("This option cannot be switched this way, this is a boolean option."));
					return QString::null;
					break;
				case color_t :
					break;
			}
		}
		yzDebug() << "Setting option " << option << " to " << value << endl;
		switch ( opt->getType() ) {
			case global_opt :
				YZSession::me->popupMessage(QObject::tr("This option is a global option which cannot be changed with setlocal"));
				return QString::null;
			case view_opt :
				if ( view ) view->setLocalQStringOption( option, value );
				break;
			case buffer_opt:
				if ( buffer ) buffer->setLocalQStringOption( option, value );
				break;
		}
	} else if ( rx2.exactMatch( args.arg )) {
#if QT_VERSION < 0x040000
		YZInternalOption *opt = YZSession::mOptions.getOption(rx2.cap( 1 ).simplifyWhiteSpace());
#else
		YZInternalOption *opt = YZSession::mOptions.getOption(rx2.cap( 1 ).trimmed());
#endif
		if ( !opt ) {
			YZSession::me->popupMessage(QObject::tr("Invalid option given"));
			return QString::null;
		}
		switch ( opt->getType() ) {
			case global_opt :
				YZSession::me->popupMessage(QObject::tr("This option is a global option which cannot be changed with setlocal"));
				return QString::null;
			case view_opt :
				if ( view ) 
#if QT_VERSION < 0x040000
					view->setLocalBoolOption( rx2.cap( 1 ).simplifyWhiteSpace(), false);
#else
					view->setLocalBoolOption( rx2.cap( 1 ).trimmed(), false);
#endif
				break;
			case buffer_opt:
				if ( buffer ) 
#if QT_VERSION < 0x040000
					buffer->setLocalBoolOption( rx2.cap( 1 ).simplifyWhiteSpace(), false);
#else
					buffer->setLocalBoolOption( rx2.cap( 1 ).trimmed(), false);
#endif
				break;
		}
	} else if ( rx3.exactMatch( args.arg ) ) {
#if QT_VERSION < 0x040000
		YZInternalOption *opt = YZSession::mOptions.getOption(rx3.cap( 1 ).simplifyWhiteSpace());
#else
		YZInternalOption *opt = YZSession::mOptions.getOption(rx3.cap( 1 ).trimmed());
#endif
		if ( !opt ) {
			YZSession::me->popupMessage(QObject::tr("Invalid option given"));
			return QString::null;
		}
		switch ( opt->getType() ) {
			case global_opt :
				YZSession::me->popupMessage(QObject::tr("This option is a global option which cannot be changed with setlocal"));
				return QString::null;
			case view_opt :
				if ( view ) 
#if QT_VERSION < 0x040000
					view->setLocalBoolOption( rx3.cap( 1 ).simplifyWhiteSpace(), true);
#else
					view->setLocalBoolOption( rx3.cap( 1 ).trimmed(), true);
#endif
				break;
			case buffer_opt:
				if ( buffer ) 
#if QT_VERSION < 0x040000
					buffer->setLocalBoolOption( rx3.cap( 1 ).simplifyWhiteSpace(), true);
#else
					buffer->setLocalBoolOption( rx3.cap( 1 ).trimmed(), true);
#endif
				break;
		}
	} else {
		YZSession::me->popupMessage( QObject::tr( "Invalid option given" ) );
		return QString::null;
	}
	// refresh screen
//	if ( args.view )
//		args.view->recalcScreen();

	return QString::null;
}

QString YZExCommandPool::set ( const YZExCommandArgs& args ) {
	QRegExp rx ( "(\\w*)(\\+|-)?=(.*)" ); //option with value
	QRegExp rx2 ( "no(\\w*)" ); //deactivate a bool option
	QRegExp rx3 ( "(\\w*)" ); //activate a bool option

	if ( rx.exactMatch( args.arg ) ) {
		YZSession::mOptions.setGroup("Global");
		bool hasOperator = rx.numCaptures() == 3; // do we have a +/- in the set command ?
#if QT_VERSION < 0x040000
		QString option = rx.cap( 1 ).simplifyWhiteSpace();
		QString value = hasOperator ? rx.cap( 3 ).simplifyWhiteSpace() : rx.cap( 2 ).simplifyWhiteSpace();
#else
		QString option = rx.cap( 1 ).trimmed();
		QString value = hasOperator ? rx.cap( 3 ).trimmed() : rx.cap( 2 ).trimmed();
#endif
		YZInternalOption *opt = YZSession::mOptions.getOption(option);
		if ( !opt ) {
			YZSession::me->popupMessage(QObject::tr("Invalid option given : ") + option);
			return QString::null;
		}
		if ( hasOperator ) {
			switch ( opt->getValueType() ) {
				case stringlist_t :
				case string_t :
					if ( rx.cap( 2 ) == "+" ) value = YZSession::mOptions.readQStringEntry( option ) + value;
					else if ( rx.cap( 2 ) == "-" ) value = QString( YZSession::mOptions.readQStringEntry( option ) ).remove( value );
					break;
				case int_t :
					if ( rx.cap( 2 ) == "+" ) value = QString::number( YZSession::mOptions.readQStringEntry( option ).toInt() + value.toInt() );
					else if ( rx.cap( 2 ) == "-" ) value = QString::number( YZSession::mOptions.readQStringEntry( option ).toInt() - value.toInt() );
					break;
				case bool_t :
					YZSession::me->popupMessage(QObject::tr("This option cannot be switched this way, this is a boolean option."));
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
#if QT_VERSION < 0x040000
		YZSession::setBoolOption( rx2.cap( 1 ).simplifyWhiteSpace(), false);
#else
		YZSession::setBoolOption( rx2.cap( 1 ).trimmed(), false);
#endif
	} else if ( rx3.exactMatch( args.arg ) ) {
		YZSession::mOptions.setGroup("Global");
#if QT_VERSION < 0x040000
		YZSession::setBoolOption( rx3.cap( 1 ).simplifyWhiteSpace(), true);
#else
		YZSession::setBoolOption( rx3.cap( 1 ).trimmed(), true);
#endif
	} else {
		YZSession::me->popupMessage( QObject::tr( "Invalid option given" ) );
		return QString::null;
	}
	// refresh screen
//	if ( args.view )
//		args.view->recalcScreen();

	return QString::null;
}

QString YZExCommandPool::mkyzisrc ( const YZExCommandArgs& ) {
#if QT_VERSION < 0x040000
	YZSession::mOptions.saveTo( QDir::currentDirPath() + "/yzis.conf", "", "HL Cache" );
#else
	YZSession::mOptions.saveTo( QDir::currentPath() + "/yzis.conf", "", "HL Cache" );
#endif
	return QString::null;
}

QString YZExCommandPool::substitute( const YZExCommandArgs& args ) {
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

	return QString::null;
}

QString YZExCommandPool::hardcopy( const YZExCommandArgs& args ) {
	if ( args.arg.length() == 0 ) {
		args.view->mySession()->popupMessage( QObject::tr( "Please specify a filename" ) );
		return QString::null;
	}
	QString path = args.arg;
	QFileInfo fi ( path );
#if QT_VERSION < 0x040000
	path = fi.absFilePath( );
#else
	path = fi.absoluteFilePath( );
#endif
	args.view->printToFile( path );
	return QString::null;
}

QString YZExCommandPool::preserve( const YZExCommandArgs& args  ) {
	args.view->myBuffer()->getSwapFile()->flush();
	return QString::null;
}

QString YZExCommandPool::lua( const YZExCommandArgs& args ) {
	return YZExLua::instance()->lua( args.view, args.arg );
}

QString YZExCommandPool::source( const YZExCommandArgs& args ) {
	return YZExLua::instance()->source( args.view, args.arg );
}

QString YZExCommandPool::map( const YZExCommandArgs& args ) {
	QRegExp rx("(\\S+)\\s+(.+)");
	if ( rx.exactMatch(args.arg) ) {
		yzDebug() << "Adding global mapping : " << rx.cap(1) << " to " << rx.cap(2) << endl;
		YZMapping::self()->addGlobalMapping(rx.cap(1), rx.cap(2));
	}
	return QString::null;
}

QString YZExCommandPool::imap( const YZExCommandArgs& args ) {
	QRegExp rx("(\\S+)\\s+(.+)");
	if ( rx.exactMatch(args.arg) ) {
		yzDebug() << "Adding insert mapping : " << rx.cap(1) << " to " << rx.cap(2) << endl;
		YZMapping::self()->addInsertMapping(rx.cap(1), rx.cap(2));
	}
	return QString::null;
}

QString YZExCommandPool::indent( const YZExCommandArgs& args ) {
	int count = 1;
	if ( args.arg.length() > 0 ) count = args.arg.toUInt();
	if ( args.cmd[ 0 ] == '<' ) count *= -1;
	for ( unsigned int i = args.fromLine; i <= args.toLine; i++ ) {
		args.view->myBuffer()->action()->indentLine( args.view, i, count );
	}
	args.view->commitNextUndo();
	return QString::null;
}

QString YZExCommandPool::enew( const YZExCommandArgs& ) {
	YZSession::me->createBuffer();
	return QString::null;
}

QString YZExCommandPool::syntax( const YZExCommandArgs& args ) {
	if ( args.arg == "on" ) {
		args.view->myBuffer()->detectHighLight();
	} else if ( args.arg == "off" ) {
		args.view->myBuffer()->setHighLight(0);
	}
	return QString::null;
}

QString YZExCommandPool::highlight( const YZExCommandArgs& args ) {
// :highlight Defaults Comment fg= selfg= bg= selbg= italic nobold underline strikeout
#if QT_VERSION < 0x040000
	QStringList list = QStringList::split(" ", args.arg);
#else
	QStringList list = args.arg.split(" ");
#endif
	QStringList::Iterator it = list.begin(), end = list.end();
	yzDebug() << list << endl;
	if (list.count() < 3) return QString::null; //at least 3 parameters...
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
	YZSession::mOptions.setGroup(style);
	QStringList option = YZSession::mOptions.readQStringListEntry(type);
	yzDebug() << "HIGHLIGHT : Current " << type << " : " << option << endl;
	if (option.count() < 7) return QString::null; //just make sure it's fine ;)

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
	YZSession::mOptions.setQStringListOption(type, option);
	
	YZSession::mOptions.setGroup("Global");
	if ( args.view && args.view->myBuffer() ) {
		YzisHighlighting *yzis = args.view->myBuffer()->highlight();
		if (yzis) {
			args.view->myBuffer()->makeAttribs();
			args.view->sendRefreshEvent();
		}
	}

	return QString::null;
}

QString YZExCommandPool::split( const YZExCommandArgs& args ) {
	YZSession::me->splitHorizontally(args.view);
	return QString::null;
}
