/*
 * $id$
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
