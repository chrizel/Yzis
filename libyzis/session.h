/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <mikmak@yzis.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>
 *  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
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
 
#include "syntaxhighlight.h"
#include "internal_options.h"
#include "registers.h"
#include "search.h"
#include "events.h"
#include "mode.h"
#include "view.h"
#include "viewcursor.h"
#include "yzisinfo.h"
#include "yzisinfojumplistrecord.h"
#include "yzisinfostartpositionrecord.h"

class YZView;
class YZBuffer;
class YzisSchemaManager;
class YZInternalOptionPool;
class YZRegisters;
class YZSearch;
class YZEvents;
class YZMode;
class YZModeEx;
class YZModeCommand;
class YZViewCursor;
class YZYzisinfo;
class YZYzisinfoJumpListRecord;
class YZYzisinfoStartPositionRecord;

/**
 * Contains data referring to an instance of yzis
 * This may also be used to "transfer" a session from a GUI to another
 * A session owns the buffers
 * A buffer owns the views
 */

typedef QMap<QString,YZBuffer*> YZBufferMap;
#if QT_VERSION < 0x040000
typedef QValueVector<QString> StringVector;
typedef QValueVector<YZYzisinfoJumpListRecord*> TagListVector;
typedef QValueVector<YZYzisinfoJumpListRecord*> JumpListVector;
typedef QValueVector<YZYzisinfoStartPositionRecord*> StartPositionVector;
#else
typedef QVector<QString> StringVector;
typedef QVector<YZYzisinfoJumpListRecord*> TagListVector;
typedef QVector<YZYzisinfoJumpListRecord*> JumpListVector;
typedef QVector<YZYzisinfoStartPositionRecord*> StartPositionVector;
#endif
 
class YZSession {
	public:
		/**
		 * Constructor. Give a session name to identify/save/load sessions.
		 * 
		 * @param _sessionName The global session name. Default is "Yzis"
		 */
		 
		YZSession( const QString& _sessionName="Yzis" );
		
		/**
		 * Destructor
		 */
		 
		virtual ~YZSession();

		/**
		 * Returns the session name
		 * 
		 * @return QString
		 */
		 
		QString getSessionName() { return mSessionName; }

		/**
		 * Returns the mode map
		 * 
		 * @return YZModeMap
		 */
		 
		YZModeMap getModes();
		
		/**
		 * Returns the ex pool
		 * 
		 * @return YZModeEx*
		 */
		 
		YZModeEx* getExPool();
		
		/**
		 * Returns the command pool
		 * 
		 * @return YZModeCommand*
		 */
		 
		YZModeCommand* getCommandPool();
		
		/**
		 * Returns the yzisinfo list
		 * 
		 * @return YZYzisinfo*
		 */
		 
		YZYzisinfo* getYzisinfo();

		/**
		 * search
		 */
		YZSearch *search() { return mSearch; }

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
		QString saveBufferExit();

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
		 * Splits the screen vertically showing the 2 given views
		 */
//		virtual void splitHorizontallyWithViews( YZView*, YZView* ) = 0;

		/**
		 * Splits the screen vertically to show the 2 given views
		 */
//		virtual void splitVerticallyOnView( YZView*, YZView* ) = 0;
		/**
		 * Finds the first view
		 */
		YZView* firstView();
		/**
		 * Finds the last view
		 */
		YZView* lastView();

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
		 * Returns a pointer to the current buffer
		 */
		YZBuffer* currentBuffer() { return mCurBuffer; }

		/**
		 * Change the filename of a recorded buffer
		 */
		void updateBufferRecord( const QString& oldname, const QString& newname, YZBuffer *buffer );

		/**
		 * Called from GUI when the current view has been changed
		 */
		void currentViewChanged ( YZView *v ) { mCurView = v; mCurBuffer = v->myBuffer(); }

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
		virtual bool quit(int errorCode=0) = 0;

		/**
		 * Prepare the app to quit
		 * All GUIs should call this instead of kapp->quit(), exit( 0 ) etc
		 * exitRequest will call @ref quit so the real quit comes from the GUI
		 * This function is used for example to clean up swap files, prompting for unsaved files etc
		 */
		bool exitRequest(int errorCode=0);

		/**
		 * Display the specified error/information message
		 */
		virtual void popupMessage( const QString& message ) = 0;

		/**
		 * Prompt a Yes/No question for the user
		 */
		virtual bool promptYesNo(const QString& title, const QString& message) = 0;

		/**
		 * Prompt a Yes/No/Cancel question for the user
		 * Returns 0,1,2 in this order
		 */
		virtual int promptYesNoCancel(const QString& title, const QString& message) = 0;

		/**
		 * Creates a new buffer
		 */
		virtual	YZBuffer *createBuffer(const QString& path=QString::null) = 0;

		/**
		 * Create a new view
		 */
		virtual YZView* createView ( YZBuffer* ) = 0;

		/**
		 * Splits horizontally the mainwindow area to create a new view on the current buffer
		 */
		 virtual void splitHorizontally ( YZView* ) = 0;

		/**
		 * Saves all buffers with a filename set
		 * @return whether all buffers were saved correctly
		 */
		bool saveAll();

		/**
		 * Count the number of buffers
		 */
		int countBuffers() { return mBuffers.count(); }

		YZBufferMap buffers() const { return mBuffers; }

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

		/**
		 * Send multiple key sequence to yzis.
		 * This method is preferred to the one used in YZView since it will handle
		 * view changes caused by commands like :bd :bn etc
		 */
		void sendMultipleKeys ( const QString& text);

		/**
		 * To be called by the GUI once it has been initialised
		 */
		void guiStarted();
		
		//HELPERS
		/**
		 * Retrieve an int option
		 */
		static int getIntegerOption( const QString& option ) {
			return YZSession::mOptions->readIntegerOption( option );
		}

		/**
		 * Retrieve a bool option
		 */
		static bool getBooleanOption( const QString& option ) {
			return YZSession::mOptions->readBooleanOption( option );
		}

		/**
		 * Retrieve a string option
		 */
		static QString getStringOption( const QString& option ) {
			return YZSession::mOptions->readStringOption( option );
		}
		/**
		 * Retrieve a qstringlist option
		 */
		static QStringList getListOption( const QString& option ) {
			return YZSession::mOptions->readListOption( option );
		}
		void registerModifier ( const QString& mod );
		void unregisterModifier ( const QString& mod );

		void saveJumpPosition();
		YZCursor * previousJumpPosition();
		
		void saveTagPosition();
		YZCursor * previousTagPosition();

	protected:
		//we map "filename"/buffer for buffers
		YZBufferMap mBuffers;

	private:
		QString mSessionName;
		YZView* mCurView;
		YZBuffer* mCurBuffer;
		YzisSchemaManager *mSchemaManager;
		YZSearch *mSearch;
		YZModeMap mModes;
		void initModes();
		void endModes();

	public:
		static int mNbViews;
		static int mNbBuffers;
		static YZInternalOptionPool *mOptions;
		static YZRegisters *mRegisters;
		static YZSession *me;
		static YZEvents *events;
		static YZYzisinfo* mYzisinfo;
		
      /**
       * yzisinfo initialized
       */
     
       static bool mYzisinfoInitialized;
     
	    /**
	     * command history
	     */
	     
	    static StringVector mExHistory;
	     
	    /**
	     * current command history item
	     */
	     
	    static unsigned int mCurrentExItem;
	
	   /**
	    * search history
	    */
	    
	    static StringVector mSearchHistory;
     
	    /**
	     * current search history item
	     */
	     
	    static unsigned int mCurrentSearchItem;
	    
	    /**
	     * start position history
	     */
	     
	    static StartPositionVector mStartPosition;
	    
	    /**
	     * jump list history
	     */
	     
	    static JumpListVector mJumpList;
	    
	    /**
	     * current jump list item
	     */
	     
	    static unsigned int mCurrentJumpListItem;
	    
	    /**
	     * tag list stack
	     */
	     
		static TagListVector mTagList;
};

#endif /* YZ_SESSION_H */
