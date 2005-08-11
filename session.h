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
 
#include "yzis.h"

#include "mode.h"  // for YZModeMap
 
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
class YZTagStack;
class YZViewId;

/**
 * Contains data referring to an instance of yzis
 * This may also be used to "transfer" a session from a GUI to another
 * A session owns the buffers
 * A buffer owns the views
 */

typedef YZList<YZBuffer*> YZBufferList;
typedef YZList<YZView*> YZViewList;
typedef YZVector<YZYzisinfoJumpListRecord*> JumpListVector;
typedef YZVector<YZYzisinfoStartPositionRecord*> StartPositionVector;
 
class YZSession {
	public:
		//-------------------------------------------------------
		// ----------------- Constructor/Destructor and Name
		//-------------------------------------------------------
	
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

		//-------------------------------------------------------
		// ----------------- Sub-Objects
		//-------------------------------------------------------
		
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

		YZTagStack &getTagStack();
		const YZTagStack &getTagStack() const;
		
		/**
		 * Get a pointer on the schema manager for syntax highlighting
		 */
		YzisSchemaManager *schemaManager() { return mSchemaManager; }

		//-------------------------------------------------------
		// ----------------- Buffer Creation/Destruction
		//-------------------------------------------------------
		/**
		 * Creates a new buffer
		 */
		virtual	YZBuffer *createBuffer(const QString& path=QString::null) = 0;

		/**
		 * Deletes the given buffer
		 */
		virtual void deleteBuffer( YZBuffer *b ) = 0;

		//-------------------------------------------------------
		// ----------------- Buffer List Management
		//-------------------------------------------------------
		/**
		 * Add a buffer
		 */
		void addBuffer( YZBuffer * );

		/**
		 * Remove a buffer
		 */
		void rmBuffer( YZBuffer * );

		/**
		 * Count the number of buffers
		 */
		unsigned int countBuffers() { return mBufferList.count(); }

		/**
		 * Returns a const reference to the buffer list
		 * Designed to be used for operations that have to occur
		 * for each buffer
		 */
		const YZBufferList &buffers() const { return mBufferList; }

		//-------------------------------------------------------
		// ----------------- Buffer Navigation
		//-------------------------------------------------------
		/**
		 * Finds a buffer by a filename
		 */
		YZBuffer* findBuffer( const QString& path );

		//-------------------------------------------------------
		// ----------------- Buffer Misc. Operations
		//-------------------------------------------------------
		/**
		 * Check if one buffer is modified and not saved
		 * @returns whether a buffer has been modified and not saved since
		 */
		bool isOneBufferModified();

		//-------------------------------------------------------
		// ----------------- View Creation/Destruction
		//-------------------------------------------------------
		/**
		 * Create a new view
		 */
		YZView* createView ( YZBuffer* buffer );

		/**
		 * Delete the current view
		 */
		void deleteView ( const YZViewId &Id = YZViewId::invalid );

		//-------------------------------------------------------
		// ----------------- Current View
		//-------------------------------------------------------
		/**
		 * Returns a pointer to the current view
		 */
		YZView* currentView() { return mCurView; }

		/**
		 * Called from GUI when the current view has been changed
		 */
		void currentViewChanged ( YZView *v );

		/**
		 * Change the current view ( unassigned )
		 */
		void setCurrentView( YZView* );

		/**
		 * Notify the change of current view
		 */
		virtual void changeCurrentView( YZView* ) = 0;

		//-------------------------------------------------------
		// ----------------- View Navigation
		//-------------------------------------------------------
		/**
		 * Finds a view by its UID
		 */
		YZView* findView( const YZViewId &id );

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

		//-------------------------------------------------------
		// ----------------- View Modification
		//-------------------------------------------------------
		/**
		 * Splits horizontally the mainwindow area to create a new view on the current buffer
		 */
		virtual void splitHorizontally ( YZView* ) = 0;

		/**
		 * Splits the screen vertically showing the 2 given views
		 */
//		virtual void splitHorizontallyWithViews( YZView*, YZView* ) = 0;

		/**
		 * Splits the screen vertically to show the 2 given views
		 */
//		virtual void splitVerticallyOnView( YZView*, YZView* ) = 0;
		
		//-------------------------------------------------------
		// ----------------- Application Termination
		//-------------------------------------------------------
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
		 * Save everything and get out
		 */
		QString saveBufferExit();

		//-------------------------------------------------------
		// ----------------- GUI Prompts
		//-------------------------------------------------------
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
		 * Saves all buffers with a filename set
		 * @return whether all buffers were saved correctly
		 */
		bool saveAll();

		//-------------------------------------------------------
		// ----------------- Focus
		//-------------------------------------------------------
		/**
		 * Focus on the command line of the current view
		 */
		virtual void setFocusCommandLine() = 0;

		/**
		 * Focus on the main window of the current view
		 */
		virtual void setFocusMainWindow() = 0;

		//-------------------------------------------------------
		// ----------------- Options
		//-------------------------------------------------------
		/**
		 * Retrieve an int option
		 */
		static int getIntegerOption( const QString& option );

		/**
		 * Retrieve a bool option
		 */
		static bool getBooleanOption( const QString& option );

		/**
		 * Retrieve a string option
		 */
		static QString getStringOption( const QString& option );
		
		/**
		 * Retrieve a qstringlist option
		 */
		static QStringList getListOption( const QString& option );
		
		//-------------------------------------------------------
		// ----------------- Miscellaneous
		//-------------------------------------------------------
		/**
		 * Send multiple key sequence to yzis.
		 * This method is preferred to the one used in YZView since it will handle
		 * view changes caused by commands like :bd :bn etc
		 */
		void sendMultipleKeys ( const QString& text );

		/**
		 * To be called by the GUI once it has been initialised
		 */
		void guiStarted();
		
		void registerModifier ( const QString& mod );
		void unregisterModifier ( const QString& mod );

		void saveJumpPosition();
		void saveJumpPosition( const int x, const int y );
		void saveJumpPosition( const YZCursor * cursor );
		const YZCursor * previousJumpPosition();
		
	protected:
		virtual YZView *doCreateView( YZBuffer *buffer ) = 0;
		virtual void doDeleteView ( YZView *view ) = 0;
		
		//-------------------------------------------------------
		// ----------------- View List Management
		//-------------------------------------------------------
		/**
		 * Add a view to the view list
		 */
		void addView( YZView *view );
		
		/**
		 * Remove a view from the view list
		 */
		void removeView( YZView *view );

	private:
		QString mSessionName;
		YZView* mCurView;
		YZBuffer* mCurBuffer;
		YzisSchemaManager *mSchemaManager;
		YZSearch *mSearch;
		YZModeMap mModes;
		
		YZBufferList mBufferList;
		YZViewList mViewList;
		
		void initModes();
		void endModes();

	public:
		static int mNbViews;
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
	     
	    static YZVector<QString> mExHistory;
	     
	    /**
	     * current command history item
	     */
	     
	    static unsigned int mCurrentExItem;
	
	   /**
	    * search history
	    */
	    
	    static YZVector<QString> mSearchHistory;
     
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
	     
		static YZTagStack mTagStack;
};

#endif /* YZ_SESSION_H */
