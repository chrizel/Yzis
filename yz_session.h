#ifndef YZ_SESSION_H
#define YZ_SESSION_H
/**
 * YZSession - Contains data referring to an instance of yzis
 * This may also be used to "transfer" a session from a GUI to another
 */

#include "yz_buffer.h"

/**
 * C++ API
 */

class YZView;

#ifdef __cplusplus
class YZSession {
	public:
		/**
		 * Constructor. Give a session name to identify/save/load sessions.
		 */
		YZSession( const char *_session_name );

		const char *get_session_name(void) { return session_name; }

	protected:
		virtual		YZBuffer *buffer(int i)=0;
		virtual		YZView *view(int i)=0;

		int		buffers_nb;
		int		views_nb;

	private:
		const char *session_name;

		
		// shall we create views and buffers from there ?
		// makes sense i think

};
#endif

/**
 * C API
 */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void yzsession_new_session (const char *_session_name);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // YZ_SESSION_H
