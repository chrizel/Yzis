#ifndef YZ_SESSION_H
#define YZ_SESSION_H
/**
 * $Id$
 * YZSession - Contains data referring to an instance of yzis
 * This may also be used to "transfer" a session from a GUI to another
 * Note : I don't think this is necessary to reimplement it into a GUI but maybe ...
 * A session owns the buffers
 * A buffer owns the views
 */

#include "yz_events.h"
#include "gui.h"
#include "yz_buffer.h"
#include "yz_view.h"
#include "yz_commands.h"
#include "yz_motionpool.h"

class YZView;
class Gui;

class YZSession {
	public:
		/**
		 * Constructor. Give a session name to identify/save/load sessions.
		 */
		YZSession( const QString& _sessionName="Yzis" );
    virtual ~YZSession();

		/**
		 * Register a GUI event manager
		 */
		void registerManager ( Gui *mgr );

		/** 
		 * Used by the buffer to post events
		 */
		void	postEvent ( yz_event );

		/**
		 * return the session name
		 */
		QString getSessionName() { return sessionName; }

		/**
		 * gives access to the pool of commands
		 */
    YZCommandPool *getPool() { return pool; }
    
		/**
		 * gives access to the pool of ex commands
		 */
		YZCommandPool *getExPool() { return expool; }

		/**
		 * Count the current buffers in this session
		 */
		int nbBuffers() { return buffers.count(); }

		/**
		 * Create a new buffer
		 */
		YZBuffer *createBuffer(const QString& path=QString::null);

		/**
		 * Add a buffer
		 */
		void addBuffer( YZBuffer * );

		/**
		 * Save everything and get out
		 */
		QString saveBufferExit( const QString& inputsBuff = QString::null );

		/** get a event to handle from the core.  that's the way the core is
		 * sending messages to the gui
		 */
		/* for the qt/kde gui, we should create QEvents from that? */
		yz_event fetchNextEvent(int requester=-1);

		/**
		 * Finds a view by its UID
		 */
		YZView* findView( int uid );

		/** 
		 * Current GUI
		 */
		Gui *gui_manager;

	protected:
		/*
		 * Find a buffer by QString/QRegExp ?
		 */
//		virtual		YZBuffer *buffer(int i)=0;

		//we map "filename"/buffer for buffers
		QMap<QString,YZBuffer*> buffers;
		QValueList<yz_event> events;

	private:
		QString sessionName;
    YZCommandPool *pool;
    YZCommandPool *expool;
    YZMotionPool *motionpool;

};

#endif /* YZ_SESSION_H */
