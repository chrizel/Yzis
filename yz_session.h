#ifndef YZ_SESSION_H
#define YZ_SESSION_H
/**
 * YZSession - Contains data referring to an instance of yzis
 * This may also be used to "transfer" a session from a GUI to another
 */

#include "yz_buffer.h"
#include "yz_commands.h"

class YZView;

class YZSession {
	public:
		/**
		 * Constructor. Give a session name to identify/save/load sessions.
		 */
		YZSession( QString _sessionName );
    virtual ~YZSession();

		QString getSessionName(void) { return sessionName; }

    YZCommandPool *getPool() { return pool; }

	protected:
		virtual		YZBuffer *buffer(int i)=0;
		virtual		YZView *view(int i)=0;

		int		buffers_nb;
		int		views_nb;

	private:
		QString sessionName;
    YZCommandPool *pool;

		// shall we create views and buffers from there ?
		// makes sense i think

};

#endif /* YZ_SESSION_H */
