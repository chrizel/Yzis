/*
 * yz_session.cpp
 */

#include "yz_session.h"


YZSession::YZSession( const char *_session_name )
{
	session_name = _session_name;
	buffers_nb = 0;
	views_nb = 0;
}


