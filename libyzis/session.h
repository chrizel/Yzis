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

#include "buffer.h"
#include "commands.h"
#include "options.h"
#include "registers.h"
#include "motionpool.h"

class YZView;
class YZMotionPool;

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
		 * Used by the buffer to post events
		 */
//		void postEvent ( yz_event );

		/** 
		 * Send events to the GUI
		 */
//		virtual void receiveEvent ( yz_event ) = 0;

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
		QString saveBufferExit( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/** 
		 * Get an event to handle from the core.  that's the way the core is
		 * sending messages to the gui
		 */
//		yz_event fetchNextEvent(int requester=-1);

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

		/**
		 * Notify the change of current view
		 */
		virtual void changeCurrentView( YZView* ) = 0;

		/**
		 * Finds the next view relative to the current one
		 */
		YZView* nextView();

		/**
		 * Finds the previous view relative to the current one
		 */
		YZView* prevView();

		/**
		 * Returns a pointer to the current view
		 */
		YZView* currentView() { return mCurView; }

		/**
		 * Change the filename of a recorded buffer
		 */
		void updateBufferRecord( const QString& oldname, const QString& newname, YZBuffer *buffer );

		/**
		 * Called from GUI when the current view has been changed
		 */
		void currentViewChanged ( YZView *v ) { mCurView = v; }

		/**
		 * Delete the current view
		 */
		virtual void deleteView ( ) = 0;

		/**
		 * Ask to quit the app
		 */
		virtual void quit(bool savePopup=true) = 0;

		/**
		 * Display the specified error/information message
		 */
		virtual void popupMessage( const QString& message ) = 0;

		/**
		 * Creates a new buffer
		 */
		virtual	YZBuffer *createBuffer(const QString& path=QString::null) = 0;

		/**
		 * Create a new view
		 */
		virtual YZView* createView ( YZBuffer* ) = 0;

		/**
		 * Saves all buffers with a filename set
		 * @return whether all buffers were saved correctly
		 */
		bool saveAll();

		/**
		 * Count the number of buffers
		 */
		int countBuffers() { return mBuffers.count(); }

		/**
		 * Check if one buffer is modified and not saved
		 * @returns whether a buffer has been modified and not saved since
		 */
		bool isOneBufferModified();

	protected:
		//we map "filename"/buffer for buffers
		QMap<QString,YZBuffer*> mBuffers;

	private:
		QString mSessionName;
		YZCommandPool *mPool;
		YZCommandPool *mExPool;
		YZMotionPool *mMotionPool;
		YZView* mCurView;

	public:
		static int mNbViews;
		static int mNbBuffers;
		static YZOption mOptions;
		static YZRegisters mRegisters;
};

#endif /* YZ_SESSION_H */
