/*
 * $Id$
 */

#include "yz_session.h"

YZSession::YZSession( const QString& _sessionName ) {
  pool = new YZCommandPool();
	pool->initPool();
  expool = new YZCommandPool();
	expool->initExPool();
  motionpool = new YZMotionPool();
	motionpool->initPool();
	sessionName = _sessionName;
}

YZSession::~YZSession() {
/*	delete pool;
	delete expool;
	delete motionpool;*/
}

void YZSession::registerManager ( Gui *mgr ) {
	gui_manager = mgr;
}

void YZSession::postEvent (yz_event e) {
	events.push_back( e ); //append to the FIFO
	if ( gui_manager )
		gui_manager->postEvent( e );
}

//requester is a pointer on the View requesting an event which is send to
//itself
yz_event YZSession::fetchNextEvent(int requester) {
	if ( !events.empty() ) {
		if ( requester == -1) {
			yz_event e = events.first();
			events.pop_front(); //remove it
			return e;
		} else {
			QValueList<yz_event>::iterator it;
			for ( it = events.begin(); it != events.end(); ++it ) {
				if ( ( *it ).view == requester ) {
					yz_event e = *it;
					events.remove( it );
					return e;
				}
			}
		}
	}
	return YZEvent::mkEventNoop();
}

YZBuffer *YZSession::createBuffer(const QString& path) {
	YZBuffer *b = new YZBuffer(this, path );
	return b;
}

void YZSession::addBuffer( YZBuffer *b ) {
	buffers.insert(b->fileName(), b);
}

QString YZSession::saveBufferExit( const QString& /* inputsBuff */ ) {
	QMap<QString,YZBuffer*>::Iterator it;
	for ( it = buffers.begin(); it!=buffers.end(); it++ )
		it.data()->save();
	//XXX QUIT !
	return QString::null;
}

YZView* YZSession::findView( int uid ) {
	QMap<QString,YZBuffer*>::Iterator it;
	for ( it = buffers.begin(); it!=buffers.end(); it++ ) {
		YZView *v = ( *it )->findView( uid );
		if ( v ) return v;
	}
	return NULL;
}
