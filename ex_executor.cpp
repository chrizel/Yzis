/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2004 Mickael Marchand <mikmak@yzis.org>
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

#include <qfileinfo.h>
#include <qdir.h>
#include "ex_executor.h"
#include "debug.h"
#include "view.h"
#include "buffer.h"
#include "swapfile.h"
#include "session.h"

YZExExecutor::YZExExecutor() {
}

YZExExecutor::~YZExExecutor() {
}

QString YZExExecutor::write( YZView *view, const QString& inputs ) {
	if ( inputs == "wall" ) {
		view->mySession()->saveAll();
		return QString::null;
	}
	if ( inputs.startsWith( "wqall" ) ) {//write all modified buffers 
		if ( view->mySession()->saveAll() ) //if it fails => dont quit
			view->mySession()->exitRequest();
		return QString::null;
	}
	QRegExp rx ( "^(.*)\\s(.*)$");
	rx.search( inputs );
	if ( rx.cap(2) != QString::null ) {
		view->myBuffer()->setPath(rx.cap(2)); //a filename was given as argument
		view->myBuffer()->getSwapFile()->setFileName( view->myBuffer()->fileName()+".ywp" );
		view->myBuffer()->getSwapFile()->init();
	}

	if ( inputs.startsWith( "wq!" ) ) { //check readonly ? XXX
		view->myBuffer()->save();
		view->mySession()->exitRequest(); //whatever happens => quit
	} else if ( inputs.startsWith( "wq" ) ) {
		if ( view->myBuffer()->save() )
			view->mySession()->exitRequest();
	} else if ( inputs.startsWith( "w" ) ) {
		view->myBuffer()->save();
	} else if ( inputs.startsWith( "w!" ) ) {
		view->myBuffer()->save();
	}
	return QString::null;
}

QString YZExExecutor::buffernext( YZView *view, const QString& ) {
	yzDebug() << "Switching buffers (actually sw views) ..." << endl;
	YZView *v = view->mySession()->nextView();
	YZASSERT( v!=view );
	if ( v )
		view->mySession()->setCurrentView(v);
	else 
		view->mySession()->popupMessage("No next buffer");
	return QString::null;
}

QString YZExExecutor::bufferprevious ( YZView *view, const QString& ) {
	yzDebug() << "Switching buffers (actually sw views) ..." << endl;
	YZView *v = view->mySession()->prevView();
	YZASSERT( v!=view );
	if ( v )
		view->mySession()->setCurrentView(v);
	else 
		view->mySession()->popupMessage("No previous buffer");

	return QString::null;
}

QString YZExExecutor::bufferdelete ( YZView *view, const QString& ) {
	yzDebug() << "Delete buffer " << view->myBuffer()->myId << endl;

	QPtrList<YZView> l = view->myBuffer()->views();
	YZView *v;
	view->myBuffer()->clearSwap();
	for ( v = l.first(); v; v=l.next() ) {
		view->mySession()->deleteView( view->myId );
	}
	
	return QString::null;
}

QString YZExExecutor::gotoCommandMode( YZView *view, const QString &) {
	view->gotoCommandMode();
	return QString::null;
}

QString YZExExecutor::gotoOpenMode( YZView *view, const QString &) {
	yzDebug() << "Switching to open mode...";
	view->gotoOpenMode();
	yzDebug() << "done." << endl;
	return QString::null;
}

QString YZExExecutor::quit ( YZView *view, const QString& inputs ) {
	yzDebug() << "View counts: "<< view->myBuffer()->views().count() << " Buffer Count : " << view->mySession()->countBuffers() << endl;
	if ( inputs == "q" || inputs == "q!" || inputs.startsWith("qu") ) {
		//close current view, if it's the last one on a buffer , check it is saved or not
		if ( view->myBuffer()->views().count() > 1 )
			view->mySession()->deleteView(view->myId);
		else if ( view->myBuffer()->views().count() == 1 && view->mySession()->countBuffers() == 1) {
			if ( !view->myBuffer()->fileIsModified() || inputs.endsWith("!") )
				view->mySession()->exitRequest();
			else view->mySession()->popupMessage( tr( "One file is modified ! Save it first ..." ) );
		} else {
			if ( !view->myBuffer()->fileIsModified() || inputs.endsWith("!") )
				view->mySession()->deleteView(view->myId);
			else view->mySession()->popupMessage( tr( "One file is modified ! Save it first ..." ) );
		}
	} else if ( inputs == "qall!" ) {//just quit
		view->mySession()->exitRequest( );
	} else if ( inputs == "qall" ) {
		if ( !view->mySession()->isOneBufferModified() )
			view->mySession()->exitRequest( );
	}
	return QString::null;
}

QString YZExExecutor::edit ( YZView *view, const QString& inputs ) {
	int idx = inputs.find(" ");
	if ( idx == -1 ) {
		view->mySession()->popupMessage( tr( "Please specify a filename" ) );
		return QString::null;
	}
	QString path = inputs.mid( idx + 1 ); //extract the path 
	//check the file name
	QFileInfo fi ( path );
	path = fi.absFilePath();
	yzDebug() << "New buffer / view : " << path << endl;
	view->mySession()->createBuffer( path );
	return QString::null;
}

QString YZExExecutor::setlocal ( YZView *view, const QString& inputs ) {
	QRegExp rx ( "setl\\S*(\\s+)(\\w*)(\\+|-)?=(.*)" ); //option with value
	QRegExp rx2 ( "setl\\S*(\\s+)no(\\w*)" ); //deactivate a bool option
	QRegExp rx3 ( "setl\\S*(\\s+)(\\w*)" ); //activate a bool option

	YZSession::mOptions.setGroup("Global");

	if ( rx.exactMatch( inputs ) ) {
		QString option = rx.cap( 2 ).simplifyWhiteSpace();
		bool hasOperator = rx.numCaptures() == 4; // do we have a +/- in the set command ?
		QString value = hasOperator ? rx.cap( 4 ).simplifyWhiteSpace() : rx.cap( 3 ).simplifyWhiteSpace();
		YZInternalOption *opt = YZSession::mOptions.getOption(option);
		if ( !opt ) {
			view->mySession()->popupMessage(tr("Invalid option given : ") + option);
			return QString::null;
		}
		if ( hasOperator ) {
			QString oldVal;
			switch ( opt->getType() ) {
				case view_opt:
					oldVal = view->getLocalStringOption( option );
					break;
				case buffer_opt:
					oldVal = view->myBuffer()->getLocalStringOption( option );
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
					view->mySession()->popupMessage(tr("This option cannot be switched this way, this is a boolean option."));
					return QString::null;
					break;
			}
		}
		yzDebug() << "Setting option " << option << " to " << value << endl;
		switch ( opt->getType() ) {
			case global_opt :
				view->mySession()->popupMessage(tr("This option is a global option which cannot be changed with setlocal"));
				return QString::null;
			case view_opt :
				view->setLocalQStringOption( option, value );
				break;
			case buffer_opt:
				view->myBuffer()->setLocalQStringOption( option, value );
				break;
		}
	} else if ( rx2.exactMatch( inputs )) {
		YZInternalOption *opt = YZSession::mOptions.getOption(rx2.cap( 2 ).simplifyWhiteSpace());
		if ( !opt ) {
			view->mySession()->popupMessage(tr("Invalid option given"));
			return QString::null;
		}
		switch ( opt->getType() ) {
			case global_opt :
				view->mySession()->popupMessage(tr("This option is a global option which cannot be changed with setlocal"));
				return QString::null;
			case view_opt :
				view->setLocalBoolOption( rx2.cap( 2 ).simplifyWhiteSpace(), false);
				break;
			case buffer_opt:
				view->myBuffer()->setLocalBoolOption( rx2.cap( 2 ).simplifyWhiteSpace(), false);
				break;
		}
	} else if ( rx3.exactMatch( inputs ) ) {
		YZInternalOption *opt = YZSession::mOptions.getOption(rx3.cap( 2 ).simplifyWhiteSpace());
		if ( !opt ) {
			view->mySession()->popupMessage(tr("Invalid option given"));
			return QString::null;
		}
		switch ( opt->getType() ) {
			case global_opt :
				view->mySession()->popupMessage(tr("This option is a global option which cannot be changed with setlocal"));
				return QString::null;
			case view_opt :
				view->setLocalBoolOption( rx3.cap( 2 ).simplifyWhiteSpace(), true);
				break;
			case buffer_opt:
				view->myBuffer()->setLocalBoolOption( rx3.cap( 2 ).simplifyWhiteSpace(), true);
				break;
		}
	} else {
		view->mySession()->popupMessage( tr( "Invalid option given" ) );
		return QString::null;
	}
	// refresh screen
	view->reset();

	return QString::null;
}

QString YZExExecutor::set ( YZView *view, const QString& inputs ) {
	QRegExp rx ( "set(\\s+)(\\w*)(\\+|-)?=(.*)" ); //option with value
	QRegExp rx2 ( "set(\\s+)no(\\w*)" ); //deactivate a bool option
	QRegExp rx3 ( "set(\\s+)(\\w*)" ); //activate a bool option

	if ( rx.exactMatch( inputs ) ) {
		YZSession::mOptions.setGroup("Global");
		QString option = rx.cap( 2 ).simplifyWhiteSpace();
		bool hasOperator = rx.numCaptures() == 4; // do we have a +/- in the set command ?
		QString value = hasOperator ? rx.cap( 4 ).simplifyWhiteSpace() : rx.cap( 3 ).simplifyWhiteSpace();
		YZInternalOption *opt = YZSession::mOptions.getOption(option);
		if ( !opt ) {
			view->mySession()->popupMessage(tr("Invalid option given : ") + option);
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
					view->mySession()->popupMessage(tr("This option cannot be switched this way, this is a boolean option."));
					return QString::null;
					break;
			}
		}
		yzDebug() << "Setting option " << option << " to " << value << endl;
		YZSession::setQStringOption( option, value );
	} else if ( rx2.exactMatch( inputs )) {
		YZSession::mOptions.setGroup("Global");
		YZSession::setBoolOption( rx2.cap( 2 ).simplifyWhiteSpace(), false);
	} else if ( rx3.exactMatch( inputs ) ) {
		YZSession::mOptions.setGroup("Global");
		YZSession::setBoolOption( rx3.cap( 2 ).simplifyWhiteSpace(), true);
	} else {
		view->mySession()->popupMessage( tr( "Invalid option given" ) );
		return QString::null;
	}
	// refresh screen
	view->reset();

	return QString::null;
}

QString YZExExecutor::mkyzisrc ( YZView *, const QString& ) {
	YZSession::mOptions.saveTo( QDir::currentDirPath() + "/yzis.conf", "Global" );
	return QString::null;
}

QString YZExExecutor::substitute( YZView *view, const QString& inputs) {
	yzDebug() << "substitute" << endl;
	unsigned int idx = inputs.find("substitute");
	unsigned int len = 10;
	if (static_cast<unsigned int>(-1)==idx) {
		idx = inputs.find("s");
		len = 1;
	}
	unsigned int idxb,idxc;
	unsigned int tidx = idx+len;
	QChar c;
	while ((c = inputs.at(tidx)).isSpace())
		tidx++;
	QString range = inputs.left(idx);
	idx = inputs.find(c, tidx);
	idxb = inputs.find(c, idx+1);
	idxc = inputs.find(c, idxb+1);
	QString search = inputs.mid( idx+1, idxb-idx-1 );
	QString replace = inputs.mid( idxb+1, idxc-idxb-1 );
	QString options = inputs.mid( idxc+1 );
	view->substitute(range,search,replace,options);
	return QString::null;
}

QString YZExExecutor::hardcopy( YZView *view, const QString& inputs ) {
	int idx = inputs.find( " " );
	if ( idx == -1 ) {
		view->mySession()->popupMessage( tr( "Please specify a filename" ) );
		return QString::null;
	}
	QString path = inputs.mid( idx + 1 );
	QFileInfo fi ( path );
	path = fi.absFilePath( );
	view->printToFile( path );
	return QString::null;
}

QString YZExExecutor::preserve( YZView *view, const QString& /*inputs*/ ) {
	view->myBuffer()->getSwapFile()->flush();
	return QString::null;
}

#include "ex_executor.moc"

