#ifndef YZ_SESSION_H
#define YZ_SESSION_H
/**
 * $Id$
 */

#include "events.h"
#include "gui.h"
#include "buffer.h"
#include "view.h"
#include "commands.h"
#include "motionpool.h"

class YZView;
class Gui;

/**
 * Contains data referring to an instance of yzis
 * This may also be used to "transfer" a session from a GUI to another
 * Note : I don't think this is necessary to reimplement it into a GUI but maybe ...
 * A session owns the buffers
 * A buffer owns the views
 */
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
		QString getSessionName() { return mSessionName; }

		/**
		 * gives access to the pool of commands
		 */
		YZCommandPool *getPool() { return mPool; }
    
		/**
		 * gives access to the pool of ex commands
		 */
		YZCommandPool *getExPool() { return mExPool; }

		/**
		 * Add a buffer
		 */
		void addBuffer( YZBuffer * );

		/**
		 * Save everything and get out
		 */
		QString saveBufferExit( const QString& inputsBuff = QString::null );

		/** 
		 * Get an event to handle from the core.  that's the way the core is
		 * sending messages to the gui
		 */
		yz_event fetchNextEvent(int requester=-1);

		/**
		 * Finds a view by its UID
		 */
		YZView* findView( int uid );

		/**
		 * Change the current view ( unassigned )
		 */
		void setCurrentView( YZView* );

		YZView* nextView();
		YZView* currentView() { return mCurView; }
		
		/** 
		 * Current GUI
		 */
		Gui *mGUI;

	protected:
		/*
		 * Find a buffer by QString/QRegExp ?
		 */
//		virtual		YZBuffer *buffer(int i)=0;

		//we map "filename"/buffer for buffers
		QMap<QString,YZBuffer*> mBuffers;
		QValueList<yz_event> mEvents;

	private:
		QString mSessionName;
		YZCommandPool *mPool;
		YZCommandPool *mExPool;
		YZMotionPool *mMotionPool;
		YZView* mCurView;

	public:
		static int mNbViews;
		static int mNbBuffers;

};

#endif /* YZ_SESSION_H */
