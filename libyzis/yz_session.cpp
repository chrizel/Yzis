/*
 * $Id: yz_session.cpp,v 1.5 2003/04/25 12:45:30 mikmak Exp $
 */

#include "yz_session.h"

YZSession::YZSession( QString _sessionName ) {
  pool = new YZCommandPool();
	sessionName = _sessionName;
}

YZSession::~YZSession() {
}

YZBuffer *YZSession::createBuffer(QString path) {
	YZBuffer *b = new YZBuffer( path );
	addBuffer( b );
	return b;
}

void YZSession::addBuffer( YZBuffer *b ) {
	buffers.insert(b->fileName(), b);
}

QString YZSession::saveBufferExit( QString inputsBuff ) {
	QMap<QString,YZBuffer*>::Iterator it;
	for ( it = buffers.begin(); it!=buffers.end(); it++ )
		it.data()->save();
	return QString::null;
}
