/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2004 Mickael Marchand <mikmak@yzis.org>,
 *  Thomas Capricelli <orzel@freehackers.org>
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
#include "ex_executor.h"
#include "ex_lua.h"
#include "schema.h"
#include "buffer.h"
#include "internal_options.h"
#include "registers.h"
#include "view.h"
#include "swapfile.h"

int YZSession::mNbViews = 0;
int YZSession::mNbBuffers = 0;
YZInternalOptionPool YZSession::mOptions = YZInternalOptionPool();
YZRegisters YZSession::mRegisters = YZRegisters();
YZSession *YZSession::me = 0;

YZSession::YZSession( const QString& _sessionName ) {
	yzDebug() << "If you see me twice in the debug , then immediately call the police because it means yzis is damn borked ..." << endl;
	//if ( me != 0 ) int t = 5/( me - me );
	//FIXME
	mPool = new YZCommandPool();
	mPool->initPool();
	mExPool = new YZCommandPool();
	mExPool->initExPool();
	mSessionName = _sessionName;
	mCurView = 0;
	me = this;
	mSchemaManager = new YzisSchemaManager();
	lua_executor = new YZExLua();
	executor = new YZExExecutor();
}

YZSession::~YZSession() {
	delete executor;
	delete lua_executor;
	delete mSchemaManager;
	delete mExPool;
	delete mPool;
}

void YZSession::addBuffer( YZBuffer *b ) {
	yzDebug() << "Session : addBuffer " << b->fileName() << endl;
	mBuffers.insert(b->fileName(), b);
}

void YZSession::rmBuffer( YZBuffer *b ) {
	yzDebug() << "Session : rmBuffer " << b->fileName() << endl;
	mBuffers.remove( b->fileName() );
	deleteBuffer( b );
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
	QMap<QString,YZBuffer*>::Iterator it;
	if ( uid<0 ) return NULL;
	for ( it = mBuffers.begin(); it!=mBuffers.end(); it++ ) {
		YZBuffer *b = ( it.data() );
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
	changeCurrentView( view );
}

YZView* YZSession::prevView() {
	if ( mCurView == 0 ) {
		yzDebug() << "WOW, mCurview is NULL !" << endl;
		return NULL;
	}
//	yzDebug() << "Current view is " << mCurView->myId << endl;
	return findView( mCurView->myId - 1 );
}

YZView* YZSession::nextView() {
	if ( mCurView == 0 ) {
		yzDebug() << "WOW, mCurview is NULL !" << endl;
		return NULL;
	}
//	yzDebug() << "Current view is " << mCurView->myId << endl;
	return findView( mCurView->myId + 1 );
}

YZBuffer* YZSession::findBuffer( const QString& path ) {
	QMap<QString,YZBuffer*>::Iterator it;
	for ( it = mBuffers.begin(); it!=mBuffers.end(); it++ ) {
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
	QMap<QString,YZBuffer*>::Iterator it;
	bool savedAll=true;
	for ( it = mBuffers.begin(); it!=mBuffers.end(); it++ ) {
		YZBuffer* b = ( *it );
		if ( !b->fileIsNew() ) {
			if ( !b->save() ) savedAll=false;
		}
	}
	return savedAll;
}

bool YZSession::isOneBufferModified() {
	QMap<QString,YZBuffer*>::Iterator it;
	for ( it = mBuffers.begin(); it!=mBuffers.end(); it++ ) {
		YZBuffer* b = ( *it );
		if ( b->fileIsNew() ) return true;
	}
	return false;
}

void YZSession::exitRequest( int errorCode ) {
	yzDebug() << "Preparing for final exit with code " << errorCode << endl;
	//prompt unsaved files XXX
	QMap<QString,YZBuffer*>::Iterator it;
	for ( it = mBuffers.begin(); it!=mBuffers.end(); it++ ) {
		YZBuffer* b = ( *it );
		b->getSwapFile()->unlink();
		delete b;
	}
	quit( errorCode );
}

