/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <mikmak@yzis.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>
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

/*
 * $Id$
 */

#include "session.h"
#include "debug.h"
#include "schema.h"
#include "buffer.h"
#include "internal_options.h"
#include "registers.h"
#include "view.h"
#include "swapfile.h"
#include "ex_lua.h"
#if QT_VERSION < 0x040000
#include <qapplication.h>
#include <qdir.h>
#else
#include <QApplication>
#include <QDir>
#endif

int YZSession::mNbViews = 0;
int YZSession::mNbBuffers = 0;
YZInternalOptionPool *YZSession::mOptions = 0;
YZRegisters *YZSession::mRegisters = 0;
YZSession *YZSession::me = 0;
YZEvents *YZSession::events = 0;

YZSession::YZSession( const QString& _sessionName ) {
	yzDebug() << "If you see me twice in the debug , then immediately call the police because it means yzis is damn borked ..." << endl;
	//if ( me != 0 ) int t = 5/( me - me );
	//FIXME
	mPool = new YZCommandPool();
	mPool->initPool();
	mExPool = new YZExCommandPool();
	mExPool->initPool();
	mSearch = new YZSearch();
	mSessionName = _sessionName;
	mCurView = 0;
	mCurBuffer = 0;
	me = this;
	events = new YZEvents();
	mSchemaManager = new YzisSchemaManager();
	mOptions = new YZInternalOptionPool();
	mRegisters = new YZRegisters();
}

YZSession::~YZSession() {
	delete mSchemaManager;
	delete mExPool;
	delete mPool;
	delete mSearch;
	delete events;
	delete mRegisters;
	delete mOptions;
	delete YzisHlManager::self();
	delete YZExLua::instance();
}

void YZSession::guiStarted() {
	//read init files
#if QT_VERSION < 0x040000
	if (QFile::exists(QDir::rootDirPath() + "/etc/yzis/init.lua"))
		YZExLua::instance()->source( NULL, QDir::rootDirPath() + "/etc/yzis/init.lua" );
	if (QFile::exists(QDir::homeDirPath() + "/.yzis/init.lua"))
		YZExLua::instance()->source( NULL, QDir::homeDirPath() + "/.yzis/init.lua" );
#else
	if (QFile::exists(QDir::rootPath() + "/etc/yzis/init.lua"))
		YZExLua::instance()->source( NULL, QDir::rootPath() + "/etc/yzis/init.lua" );
	if (QFile::exists(QDir::homePath() + "/.yzis/init.lua"))
		YZExLua::instance()->source( NULL, QDir::homePath() + "/.yzis/init.lua" );
#endif
}

void YZSession::addBuffer( YZBuffer *b ) {
	yzDebug() << "Session : addBuffer " << b->fileName() << endl;
	mBuffers.insert(b->fileName(), b);
	mCurBuffer = b;
}

void YZSession::rmBuffer( YZBuffer *b ) {
	yzDebug() << "Session : rmBuffer " << b->fileName() << endl;
	if ( mBuffers.contains( b->fileName() ) ) {
			mBuffers.remove( b->fileName() );
			deleteBuffer( b );
	}
//	delete b; // kinda hot,no?
}

QString YZSession::saveBufferExit() {
	if ( saveAll() )
		quit();
	return QString::null;
}

YZView* YZSession::findView( int uid ) {
//	yzDebug() << " ========= " << endl;
//	yzDebug() << "Session::findView " << uid << endl;
	YZBufferMap::Iterator it = mBuffers.begin(), end = mBuffers.end();
	if ( uid<0 ) return NULL;
	for ( ; it!=end; ++it ) {
#if QT_VERSION < 0x040000
		YZBuffer *b = ( it.data() );
#else
		YZBuffer *b = ( it.value() );
#endif
//		yzDebug() << "Session::findView, checking buffer " << b->fileName() << endl;
		YZView *v = b->findView( uid );
		if ( v ) return v;
	}
//	yzDebug() << "Session::findView " << uid << " not found !" << endl;
	return NULL;
}

void YZSession::setCurrentView( YZView* view ) {
	yzDebug() << "Session : setCurrentView" << endl;
	mCurView = view;
	mCurBuffer = view->myBuffer();
	changeCurrentView( view );
}

YZView* YZSession::prevView() {
	if ( mCurView == 0 ) {
		yzDebug() << "WOW, mCurview is NULL !" << endl;
		return NULL;
	}
//	yzDebug() << "Current view is " << mCurView->myId << endl;
	
	YZView * v = NULL;
	int i = 1;
	while (!v && i <= mNbViews  ) {
		v = findView( mCurView->myId - i );
		i++;
	}
	return v;
}

YZView* YZSession::nextView() {
	if ( mCurView == 0 ) {
		yzDebug() << "WOW, mCurview is NULL !" << endl;
		return NULL;
	}
//	yzDebug() << "Current view is " << mCurView->myId << endl;
	YZView * v = NULL;
	int i = 1;
	while (!v && i <= mNbViews) {
		v = findView( mCurView->myId + i );
		i++;
	}
	return v;
}

YZBuffer* YZSession::findBuffer( const QString& path ) {
	QMap<QString,YZBuffer*>::Iterator it = mBuffers.begin(), end = mBuffers.end();
	for ( ; it!=end; ++it ) {
		YZBuffer* b = ( *it );
		if ( b->fileName() == path ) return b;
	}
	return NULL; //not found
}

void YZSession::updateBufferRecord( const QString& oldname, const QString& newname, YZBuffer *buffer ) {
	mBuffers.remove( oldname );
	mBuffers.insert( newname, buffer );
}

bool YZSession::saveAll() {
	QMap<QString,YZBuffer*>::Iterator it = mBuffers.begin(), end = mBuffers.end();
	bool savedAll=true;
	for ( ; it!=end; ++it ) {
		YZBuffer* b = ( *it );
		if ( !b->fileIsNew() ) {
			if ( !b->save() ) savedAll=false;
		}
	}
	return savedAll;
}

bool YZSession::isOneBufferModified() {
	QMap<QString,YZBuffer*>::Iterator it = mBuffers.begin(), end = mBuffers.end();
	for ( ; it!=end; ++it ) {
		YZBuffer* b = ( *it );
		if ( b->fileIsNew() ) return true;
	}
	return false;
}

void YZSession::exitRequest( int errorCode ) {
	yzDebug() << "Preparing for final exit with code " << errorCode << endl;
	//prompt unsaved files XXX
	QMap<QString,YZBuffer*>::Iterator it = mBuffers.begin(), end = mBuffers.end();
	for ( ; it!=end; ++it ) {
		YZBuffer* b = ( *it );
		delete b;
	}
	quit( errorCode );
}

void YZSession::sendMultipleKeys ( const QString& text) {
//	QStringList list = QStringList::split("<ENTER>", text);
/*	QStringList::Iterator it = list.begin(), end = list.end();
	for (; it != end; ++it) {*/
		YZView* cView = YZSession::me->currentView();
		cView->sendMultipleKey(/* *it + "<ENTER>" */ text);
		qApp->processEvents();
/*	}*/
}

