/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2004 Mickael Marchand <mikmak@yzis.org>,
 *  Thomas Capricelli <orzel@freehackers.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

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
		void postEvent ( yz_event );

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
		 * Finds a buffer by a filename
		 */
		YZBuffer* findBuffer( const QString& path );

		/**
		 * Change the current view ( unassigned )
		 */
		void setCurrentView( YZView* );

		YZView* nextView();

		/**
		 * Returns a pointer to the current view
		 */
		YZView* currentView() { return mCurView; }
		
		/** 
		 * Current GUI
		 */
		Gui *mGUI;

	protected:
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
