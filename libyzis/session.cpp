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

int YZSession::mNbViews = 0;
int YZSession::mNbBuffers = 0;
YZOption YZSession::mOptions = YZOption();
YZRegisters YZSession::mRegisters = YZRegisters();

YZSession::YZSession( const QString& _sessionName ) {
	mPool = new YZCommandPool();
	mPool->initPool();
	mExPool = new YZCommandPool();
	mExPool->initExPool();
	mMotionPool = new YZMotionPool();
	mMotionPool->initPool();
	mSessionName = _sessionName;
	mCurView = 0;
}

YZSession::~YZSession() {
}

void YZSession::addBuffer( YZBuffer *b ) {
	yzDebug() << "Session : addBuffer " << b->fileName() << endl;
	mBuffers.insert(b->fileName(), b);
}

QString YZSession::saveBufferExit( const QString& /* inputsBuff */, YZCommandArgs /* args */ ) {
	QMap<QString,YZBuffer*>::Iterator it;
	for ( it = mBuffers.begin(); it!=mBuffers.end(); it++ )
		it.data()->save();
	quit( true );	
	YZASSERT_MSG( 0, "YZSession::saveBufferExit() - this point should never be reached!" );
	//should not be reached
	return QString::null;
}

YZView* YZSession::findView( int uid ) {
//	yzDebug() << " ========= " << endl;
//	yzDebug() << "Session: looking for view " << uid << endl;
	QMap<QString,YZBuffer*>::Iterator it;
	for ( it = mBuffers.begin(); it!=mBuffers.end(); it++ ) {
		YZBuffer *b = ( it.data() );
//		yzDebug() << "Session : findView, checking buffer " << b->fileName() << endl;
		YZView *v = b->findView( uid );
		if ( v ) return v;
	}
//	yzDebug() << "Session: View " << uid << " not found !" << endl;
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

	yzDebug() << "Current view is " << mCurView->myId << endl;

	YZView*	nv = findView( ( mCurView->myId >= 1 ) ? mCurView->myId - 1 : mCurView->myId );

	return nv;
}

YZView* YZSession::nextView() {
	if ( mCurView == 0 ) {
		yzDebug() << "WOW, mCurview is NULL !" << endl;
		return NULL;
	}

	yzDebug() << "Current view is " << mCurView->myId << endl;

	YZView *nv = findView( mCurView->myId + 1 );
	if ( !nv ) 
		nv = findView( mCurView->myId );

	return nv;
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

void YZSession::saveAll() {
	QMap<QString,YZBuffer*>::Iterator it;
	for ( it = mBuffers.begin(); it!=mBuffers.end(); it++ ) {
		YZBuffer* b = ( *it );
		if ( !b->fileIsNew() ) b->save();
	}
}
