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
#include "syntaxhighlight.h"
#include "schema.h"

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
		 * gives access to the pool of motions
		 */
		YZMotionPool *getMotionPool() { return mMotionPool; }

		/**
		 * Add a buffer
		 */
		void addBuffer( YZBuffer * );

		/**
		 * Remove a buffer
		 */
		void rmBuffer( YZBuffer * );

		/**
		 * Save everything and get out
		 */
		QString saveBufferExit( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

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
		virtual void deleteView ( int Id = -1 ) = 0;

		/**
		 * Deletes the given buffer
		 */
		virtual void deleteBuffer( YZBuffer *b ) = 0;

		/**
		 * Ask to quit the app
		 */
		virtual void quit(int errorCode=0) = 0;

		/**
		 * Prepare the app to quit
		 * All GUIs should call this instead of kapp->quit(), exit( 0 ) etc
		 * exitRequest will call @ref quit so the real quit comes from the GUI
		 * This function is used for example to clean up swap files, prompting for unsaved files etc
		 */
		void exitRequest(int errorCode=0);

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

		/**
		 * Focus on the command line of the current view
		 */
		virtual void setFocusCommandLine() = 0;

		/**
		 * Focus on the main window of the current view
		 */
		virtual void setFocusMainWindow() = 0;

		/**
		 * Get a pointer on the schema manager for syntax highlighting
		 */
		YzisSchemaManager *schemaManager() { return mSchemaManager; }

		//HELPERS
		/**
		 * Retrieve an int option
		 */
		static int getIntOption( const QString& option ) {
			return YZSession::mOptions.readIntEntry( option, 0 );
		}

		/**
		 * sets an int option
		 */
		static void setIntOption( const QString& key, int option ) {
			YZSession::mOptions.setIntOption( key, option );
		}

		/**
		 * Retrieve a bool option
		 */
		static bool getBoolOption( const QString& option ) {
			return YZSession::mOptions.readBoolEntry( option, false );
		}

		/**
		 * sets a bool option
		 */
		static void setBoolOption( const QString& key, bool option ) {
			YZSession::mOptions.setBoolOption( key, option );
		}


		/**
		 * Retrieve a string option
		 */
		static QString getStringOption( const QString& option ) {
			return YZSession::mOptions.readQStringEntry( option, QString("") );
		}

		/**
		 * sets a qstring option
		 */
		static void setQStringOption( const QString& key, const QString& option ) {
			YZSession::mOptions.setQStringOption( key, option );
		}

		/**
		 * Retrieve a qstringlist option
		 */
		static QStringList getStringListOption( const QString& option ) {
			return YZSession::mOptions.readQStringListEntry( option, QStringList::split(";","") );
		}

		/**
		 * sets a qstringlist option
		 */
		static void setQStringListOption( const QString& key, const QStringList& option ) {
			YZSession::mOptions.setQStringListOption( key, option );
		}

		/**
		 * Retrieve a qcolor option
		 */
		static QColor getColorOption( const QString& option ) {
			return YZSession::mOptions.readQColorEntry( option, QColor("white") );
		}

		/**
		 * sets a qcolor option
		 */
		static void setQColorOption( const QString& key, const QColor& option ) {
			YZSession::mOptions.setQColorOption( key, option );
		}

	protected:
		//we map "filename"/buffer for buffers
		QMap<QString,YZBuffer*> mBuffers;

	private:
		QString mSessionName;
		YZCommandPool *mPool;
		YZCommandPool *mExPool;
		YZMotionPool *mMotionPool;
		YZView* mCurView;
		YzisSchemaManager *mSchemaManager;
		
	public:
		static int mNbViews;
		static int mNbBuffers;
		static YZOption mOptions;
		static YZRegisters mRegisters;
		static YZSession *me;
};

#endif /* YZ_SESSION_H */
