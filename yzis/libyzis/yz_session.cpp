/*
 * $id$
 */

#include "yz_session.h"

YZSession::YZSession( QString _sessionName ) {
  pool = new YZCommandPool();
	sessionName = _sessionName;
	buffers_nb = 0;
	views_nb = 0;
}

YZSession::~YZSession() {
}

