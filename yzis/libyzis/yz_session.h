#ifndef YZ_SESSION_H
#define YZ_SESSION_H
/**
 * YZSession - Contains data referring to an instance of yzis
 * This may also be used to "transfer" a session from a GUI to another
 * Note : I don't think this is necessary to reimplement it into a GUI but maybe ...
 * A session owns the buffers
 * A buffer owns the views
 */

#include "yz_buffer.h"
#include "yz_view.h"
#include "yz_commands.h"

class YZView;

class YZSession {
	public:
		/**
		 * Constructor. Give a session name to identify/save/load sessions.
		 */
		YZSession( QString _sessionName="Yzis" );
    virtual ~YZSession();

		/**
		 * return the session name
		 */
		QString getSessionName() { return sessionName; }

		/**
		 * gives access to the pool of commands
		 */
    YZCommandPool *getPool() { return pool; }

		/**
		 * Count the current buffers in this session
		 */
		int nbBuffers() { return buffers.count(); }

		/**
		 * Create a new buffer
		 */
		YZBuffer *createBuffer(QString path=QString::null);

		/**
		 * Add a buffer
		 */
		void addBuffer( YZBuffer * );

		/**
		 * Save everything and get out
		 */
		QString saveBufferExit( QString inputsBuff = QString::null );

	protected:
		/*
		 * Find a buffer by QString/QRegExp ?
		 */
//		virtual		YZBuffer *buffer(int i)=0;

		//we map "filename"/buffer for buffers
		QMap<QString,YZBuffer*> buffers;

	private:
		QString sessionName;
    YZCommandPool *pool;
};

#endif /* YZ_SESSION_H */
