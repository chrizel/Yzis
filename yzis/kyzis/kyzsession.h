#ifndef KYZ_SESSION_H
#define KYZ_SESSION_H

#include <yz_session.h>
#include <qptrlist.h>

class KYZSession : public YZSession {
	public:
		KYZSession(int argc, char **argv, const char *_session_name = "default_session");
		
	protected:
		
	private:
};

#endif 
