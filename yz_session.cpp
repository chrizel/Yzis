/*
 * $Id$
 */

#include "yz_session.h"

YZSession::YZSession( const QString& _sessionName ) {
  pool = new YZCommandPool();
	sessionName = _sessionName;
}

YZSession::~YZSession() {
}

YZBuffer *YZSession::createBuffer(const QString& path) {
	YZBuffer *b = new YZBuffer( path );
	addBuffer( b );
	return b;
}

void YZSession::addBuffer( YZBuffer *b ) {
	buffers.insert(b->fileName(), b);
}

QString YZSession::saveBufferExit( const QString& /* inputsBuff */ ) {
	QMap<QString,YZBuffer*>::Iterator it;
	for ( it = buffers.begin(); it!=buffers.end(); it++ )
		it.data()->save();
	return QString::null;
}
