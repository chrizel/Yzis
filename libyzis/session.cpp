/*
 * $Id$
 */

#include "session.h"

YZSession::YZSession( const QString& _sessionName ) {
  pool = new YZCommandPool();
	pool->initPool();
  expool = new YZCommandPool();
	expool->initExPool();
  motionpool = new YZMotionPool();
	motionpool->initPool();
	sessionName = _sessionName;
	curView = 0;
}

YZSession::~YZSession() {
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

#if 0
YZBuffer *YZSession::createBuffer(const QString& path) {
	YZBuffer *b = new YZBuffer(this, path );
	addBuffer( b );
	return b;
}
#endif

#if 0
YZView *YZSession::createView(YZBuffer *buffer) {
	YZView *view = new YZView(buffer, this);
	return view;
}
#endif

void YZSession::addBuffer( YZBuffer *b ) {
	buffers.insert(b->fileName(), b);
}

QString YZSession::saveBufferExit( const QString& /* inputsBuff */ ) {
	QMap<QString,YZBuffer*>::Iterator it;
	for ( it = buffers.begin(); it!=buffers.end(); it++ )
		it.data()->save();
	gui_manager->quit( true );	
	//should not be reached
	return QString::null;
}

YZView* YZSession::findView( int uid ) {
	QMap<QString,YZBuffer*>::Iterator it;
	for ( it = buffers.begin(); it!=buffers.end(); it++ ) {
		YZView *v = ( it.data() )->findView( uid );
		if ( v ) return v;
	}
	return NULL;
}

void YZSession::setCurrentView( YZView* view ) {
	curView = view;
	//update the GUIs now
	gui_manager->setCurrentView( view );
}

YZView* YZSession::nextView() {
	QMap<QString,YZBuffer*>::Iterator it;
	YZView *next = NULL;
	bool found = false;

	for ( it = buffers.begin(); !found && it!=buffers.end(); it++ ) {
		YZBuffer* b = ( *it );
		for ( QValueList<YZView*>::iterator vit = b->views().begin(); 
				!found && vit != b->views().end(); ++vit ) {
			YZView *idxv = ( *vit );
			if ( next != NULL ) {
				next = idxv;
				found = true;
				break;
			}
			if ( idxv == curView ) next = idxv;//fake,just make it change at next turn
		}
	}

	return next;
}

