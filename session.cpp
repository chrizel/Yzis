/*
 * $Id$
 */

#include "session.h"
#include "debug.h"

int YZSession::mNbViews = 0;
int YZSession::mNbBuffers = 0;

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

void YZSession::registerManager ( Gui *mgr ) {
	mGUI = mgr;
}

void YZSession::postEvent (yz_event e) {
	mEvents.push_back( e ); //append to the FIFO
	if ( mGUI )
		mGUI->postEvent( e );
}

//requester is a pointer on the View requesting an event which is send to
//itself
yz_event YZSession::fetchNextEvent(int requester) {
	if ( !mEvents.empty() ) {
		if ( requester == -1) {
			yz_event e = mEvents.first();
			mEvents.pop_front(); //remove it
			return e;
		} else {
			QValueList<yz_event>::iterator it;
			for ( it = mEvents.begin(); it != mEvents.end(); ++it ) {
				if ( ( *it ).view == requester ) {
					yz_event e = *it;
					mEvents.remove( it );
					return e;
				}
			}
		}
	}
	return YZEvent::mkEventNoop();
}

void YZSession::addBuffer( YZBuffer *b ) {
	yzDebug() << "Session : addBuffer" << endl;
	mBuffers.insert(b->fileName(), b);
}

QString YZSession::saveBufferExit( const QString& /* inputsBuff */ ) {
	QMap<QString,YZBuffer*>::Iterator it;
	for ( it = mBuffers.begin(); it!=mBuffers.end(); it++ )
		it.data()->save();
	mGUI->quit( true );	
	//should not be reached
	return QString::null;
}

YZView* YZSession::findView( int uid ) {
	yzDebug() << "Session: looking for view " << uid << endl;
	QMap<QString,YZBuffer*>::Iterator it;
	for ( it = mBuffers.begin(); it!=mBuffers.end(); it++ ) {
		YZBuffer *b = ( it.data() );
		yzDebug() << "Session : findView, checking buffer " << b->fileName() << endl;
		YZView *v = b->findView( uid );
		if ( v ) return v;
	}
	yzDebug() << "Session: View " << uid << " not found !" << endl;
	return NULL;
}

void YZSession::setCurrentView( YZView* view ) {
	mCurView = view;
	//update the GUIs now
	mGUI->setCurrentView( view );
}

YZView* YZSession::nextView() {
	QMap<QString,YZBuffer*>::Iterator it;
	YZView *next = NULL;
	bool found = false;

	for ( it = mBuffers.begin(); !found && it!=mBuffers.end(); it++ ) {
		YZBuffer* b = ( *it );
		for ( QValueList<YZView*>::iterator vit = b->views().begin(); 
				!found && vit != b->views().end(); ++vit ) {
			YZView *idxv = ( *vit );
			if ( next != NULL ) {
				next = idxv;
				found = true;
				break;
			}
			if ( idxv == mCurView ) next = idxv;//fake,just make it change at next turn
		}
	}

	return next;
}

