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

#include "ex_executor.h"
#include "debug.h"
#include <qfileinfo.h>
#include <qdir.h>

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
			view->mySession()->quit();
		return QString::null;
	}
	QRegExp rx ( "^(.*)\\s(.*)$");
	rx.search( inputs );
	if ( rx.cap(2) != QString::null ) 
		view->myBuffer()->setPath(rx.cap(2)); //a filename was given as argument

	if ( inputs.startsWith( "wq!" ) ) { //check readonly ? XXX
		view->myBuffer()->save();
		view->mySession()->quit(); //whatever happens => quit
	} else if ( inputs.startsWith( "wq" ) ) {
		if ( view->myBuffer()->save() )
			view->mySession()->quit();
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

///XXX wrong this one should delete all views on a buffer
QString YZExExecutor::bufferdelete ( YZView *view, const QString& ) {
	yzDebug() << "Delete view " << view->myId << endl;
	view->mySession()->deleteView();
	return QString::null;
}

QString YZExExecutor::quit ( YZView *view, const QString& inputs ) {
	yzDebug() << "View counts: "<< view->myBuffer()->views().count() << " Buffer Count : " << view->mySession()->countBuffers() << endl;
	if ( inputs == "q" || inputs == "q!" ) {
		//close current view, if it's the last one on a buffer , check it is saved or not
		if ( view->myBuffer()->views().count() > 1 )
			view->mySession()->deleteView();
		else if ( view->myBuffer()->views().count() == 1 && view->mySession()->countBuffers() == 1) {
			if ( !view->myBuffer()->fileIsModified() || inputs.endsWith("!") )
				view->mySession()->quit();
			else view->mySession()->popupMessage( tr( "One file is modified ! Save it first ..." ) );
		} else {
			if ( !view->myBuffer()->fileIsModified() || inputs.endsWith("!") )
				view->mySession()->deleteView();
			else view->mySession()->popupMessage( tr( "One file is modified ! Save it first ..." ) );
		}
	} else if ( inputs == "qall!" ) {//just quit
		view->mySession()->quit( );
	} else if ( inputs == "qall" ) {
		if ( !view->mySession()->isOneBufferModified() )
			view->mySession()->quit( );
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

QString YZExExecutor::set ( YZView *view, const QString& inputs ) {
	QRegExp rx ( "set(\\s*)(.*)=(.*)" ); //option with value
	QRegExp rx2 ( "set(\\s*)no(.*)" ); //deactivate a bool option
	QRegExp rx3 ( "set(\\s*)(.*)" ); //activate a bool option
	if ( rx.exactMatch( inputs ) ) {
		YZSession::mOptions.setGroup("General");
		YZSession::setQStringOption( rx.cap( 2 ).simplifyWhiteSpace(), rx.cap( 3 ).simplifyWhiteSpace());
	} else if ( rx2.exactMatch( inputs )) {
		YZSession::mOptions.setGroup("General");
		YZSession::setBoolOption( rx2.cap( 2 ).simplifyWhiteSpace(), false);
	} else if ( rx3.exactMatch( inputs ) ) {
		YZSession::mOptions.setGroup("General");
		YZSession::setBoolOption( rx3.cap( 2 ).simplifyWhiteSpace(), true);
	} else {
		view->mySession()->popupMessage( tr( "Invalid option given" ) );
	}

	return QString::null;
}

QString YZExExecutor::mkyzisrc ( YZView *, const QString& ) {
	YZSession::mOptions.saveTo( QDir::currentDirPath() + "/yzis.conf" );
	return QString::null;
}

#include "ex_executor.moc"

