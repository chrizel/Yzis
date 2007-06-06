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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef YZ_SESSION_H
#define YZ_SESSION_H

#include "sessioniface.h" 

/* Qt */
// #include <QList>

/* yzis */
//#include "cursor.h"
#include "mode.h"  // for YZModeMap
 
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
class YZTagStack;
class YZCursor;

typedef QList<YZBuffer*> YZBufferList;
typedef QList<YZView*> YZViewList;

/**
  * Class YZSession
  *
  * @short Singleton class representing an Yzis application.
  *
  * The YZSession is managing all global attributes and method of Yzis. 
  *
  * It takes care of everything that is not local to a YZBuffer or YZView.
  *
  * Contains data referring to an instance of yzis
  * This may also be used to "transfer" a session from a GUI to another
  * A session owns the buffers
  * A buffer owns the views
  */
class YZIS_EXPORT YZSession  : public YZSessionIface 
{
	protected:
		/**
		 * Constructor. Protected, because YZSession is a singleton.
         *
         * The constructor does mostly nothing. All the hard work
         * is done in setInstance() and init()
		 */
		YZSession();

		/**
		 * Destructor. Protected, because YZSession is a singleton.
		 */
		virtual ~YZSession();

		/**
		 *  Sets the session instance object and initialise the session.
         *
		 *  It MUST be called from concrete class which inherits from YZSession.
		 *
		 *  Example from QYZisSession:
		 *  \code
		 *  void QYZisSession::createInstance() // static function
		 *  {
		 *  	static QYZisSession instance;
		 *  	setInstance(&instance);
		 *  }
		 *  \endcode
		 *
		 *  And then QYZisSession::createInstance() is called from main
		 *  before any other libyzis object gets created
		 *
		 *  @see QYZisSession::createInstance()
		 *  @see NYZSession::createInstance()
		 */
		static void setInstance(YZSession* instance);

        /** Initialise YZSession.
          *
          * This method is called in setInstance()
          */
        void init();

        /** Initialise the language stuff, for translation and display
          * in proper font and language.
          *
          * This methed is called from init()
          */
        void initLanguage();

        /** Load the init.lua scripts 
          *
          * This methed is called from init().
          */
        void initScript();

        /** Send the keys of the command line to libyzis.
          *
          * Sends the keys that were passed in the command line with -c
          * to libyzis for processing.
          *
          * This function should be called explicitly by the frontend
          * once all the frontend initialisation is complete.
          */
        void sendInitkeys();


	public:

		/** Initialise the debug engine.
          *
          * You should call this static method as the very first step
          * of your program, to ensure that all components can properly
          * use debug code.
          *
          * initDebug() should be called even before YZSession() creation.
          *
		  */
        static void initDebug(int argc, char ** argv);

		/** YZis singleton instance.
         *
		 *  @return the one and only one instance of YZSession
		 *  @see setInstance
		 */
		static YZSession * self();
		
        /** Parse the command line of the application.
          *
          * Will load the buffer, parse the options and apply them.
          *
          * If an option is not recognised, an error message is displayed
          * and the application is quitted.
          *
          */
        void parseCommandLine( int argc, char * argv[] );

        /** Call this method when the frontend is ready.
          *
          * The method will complete the initialisation process.
          * It will call sendInitkeys() to perform the commands that were
          * passed with -c
          */
        void frontendGuiReady();

        /** Return a string description of the current session:
          *
          * The description contains:
          * - the list of every view
          * - the list of every buffer
          */
        QString toString() const;

		//-------------------------------------------------------
		// ----------------- Sub-Objects
		//-------------------------------------------------------
	
    public:	
		/** Returns the mode map
		 * 
		 * @return YZModeMap
		 */
		YZModeMap getModes() const;
		
		/** Returns the ex pool
		 * 
		 * @return YZModeEx*
		 */
		YZModeEx* getExPool();
		
		/** Returns the command pool
		 * 
		 * @return YZModeCommand*
		 */
		YZModeCommand* getCommandPool();
		
		/** Returns the yzisinfo list
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
		
		/**
		 * Get the Internal Option Pool
		 */
		YZInternalOptionPool *getOptions();

		//-------------------------------------------------------
		// ----------------- Buffer Management
		//-------------------------------------------------------
		/**
		 * Creates a new buffer.
         *
         * If a buffer on the path already exists, return it. Else,
         * ask the frontend to create a buffer with guiCreateBuffer()
         * set the buffer as current and return it.
		 */
		YZBuffer *createBuffer(const QString& path=QString());
		
		/**
		 * Creates a new buffer and puts it in a new view
		 * To get the created buffer, call YZView::myBuffer()
		 */
		YZView *createBufferAndView( const QString &path = QString() );

		/**
		 * Remove a buffer from the list.
         *
         * Inform the gui frontend to delete the buffer. If this buffer was
         * the last one, calls exit.
		 */
		void rmBuffer( YZBuffer * );

		/**
		 * Returns a const reference to the buffer list
		 * Designed to be used for operations that have to occur
		 * for each buffer
		 */
		const YZBufferList & buffers() const { return mBufferList; }

		/**
		 * Finds a buffer by a filename
		 */
		YZBuffer* findBuffer( const QString& path );

		/**
		 * Check if one buffer is modified and not saved
		 * @returns whether a buffer has been modified and not saved since
		 */
		bool isOneBufferModified() const;

		//-------------------------------------------------------
		// ----------------- View Management
		//-------------------------------------------------------
		/**
		 * Create a new view
		 */
		YZView* createView ( YZBuffer* buffer );

		/**
		 * Delete the current view
		 */
		void deleteView ( YZView* v );

		/**
		 * Returns a pointer to the current view
		 */
		YZView* currentView() { return mCurView; }

		/**
		 * Change the current view ( unassigned )
		 */
		void setCurrentView( YZView* );

		/**
		 * Gets a list of all YZViews active in the system
		 */
		const YZViewList getAllViews() const;

		/** \brief Find a view containing the buffer.
         *
         * Can be called with a NULL argument.
		 */
		YZView *findViewByBuffer( const YZBuffer *buffer );

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
		// ----------------- Application Termination
		//-------------------------------------------------------
		/**
		 * Prepare the app to quit
         *
		 * All GUIs should call this instead of kapp->quit(), exit( 0 ) etc
         * exitRequest will call guiQuit() so the real quit comes from the GUI
         * This function is used for example to clean up swap files, prompting
         * for unsaved files etc
         *
         * @return XXX dunno
		 */
		bool exitRequest(int errorCode=0);

		/**
		 * Saves all buffers with a filename set.
         *
         * May popup to ask fo the name of the file to save (from
         * YZBuffer::save() ).
         *
		 * @return whether all buffers were saved correctly
		 */
		bool saveAll();

		/** Save everything and get out.
         *
         * Calls saveAll() and exitRequest()
		 */
		void saveBufferExit();

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
		// ----------------- Event plugins
		//-------------------------------------------------------
		/**
		 * Connect an event to a lua function
		 */
		void eventConnect( const QString& event, const QString& function );
		
		/**
		 * call a lua event
		 */
		QStringList eventCall(const QString& event, YZView *view=NULL);
		
		//-------------------------------------------------------
		// ----------------- Registers
		//-------------------------------------------------------
		/**
		 * Fills the register @param r with the @param value
		 */
		void setRegister( QChar r, const QStringList& value );

		/**
		 * Gets the value of register @param r
		 * Returns a QString containing the register content
		 */
		QStringList& getRegister ( QChar r );

		/**
		 * Gets the list of registers
		 */
		QList<QChar> getRegisters() const;

		//-------------------------------------------------------
		// ----------------- Command line
		//-------------------------------------------------------
        /** Show help text for -h and --help option */
        void showCmdLineHelp( const QString & progName );

        /** Show version text for -v and --version option */
        void showCmdLineVersion();

        /** Show error message for unknown option */
        void showCmdLineUnknowOption( const QString & opt );

        /** Return a version string for Yzis.
          *
          * The string is multi-line and is displayed when yzis is started
          * with --version and -v.
          *
          * Gui frontend are expected to complement the version information
          * of libyzis with their own version information.
          */
        virtual QString version();
    	
        //-------------------------------------------------------
		// ----------------- Miscellaneous
		//-------------------------------------------------------

		/**
		 * Send multiple key sequence to yzis.
		 * 
         * The key sequence is automatically sent to the right view,
         * even if the view is switched in the middle.
		 */
		virtual void scriptSendMultipleKeys ( const QString& text );

        /** Copied from view */
		virtual void sendMultipleKeys( YZView * view, const QString& keys );

		//-------------------------------------------------------
		// ----------------- Send events to GUI
		//-------------------------------------------------------
		/**
		 * transfer key events from GUI to core
		 */
		virtual void sendKey( YZView * view, const QString& key, const QString& modifiers="");

		void registerModifier ( const QString& mod );
		void unregisterModifier ( const QString& mod );

		void saveJumpPosition();
		void saveJumpPosition( const int x, const int y );
		void saveJumpPosition( const YZCursor cursor );
		const YZCursor previousJumpPosition();

        /** Because of windows, we need to have new defined in the
          * shared library. */
        void * operator new( size_t tSize );

        /** Because of windows, we need to have delete defined in the
          * shared library. */
        void  operator delete( void* p );

		
	protected:
		
		void initModes();
		void endModes();

	private:
		/**
		 *  Copy constructor. Disable copy by declaring it as private
		 */
		YZSession(const YZSession&);
		/**
		 *  Copy operator. Disable copy by declaring it as private
		 */
		YZSession& operator=(const YZSession&);
		/**
		 *  Single instance of YZSession
		 */
		static YZSession* mInstance;

        QString mInitkeys;
		YZView* mCurView;
		YZBuffer* mCurBuffer;
		YzisSchemaManager *mSchemaManager;
		YZSearch *mSearch;
		YZModeMap mModes;
		YZBufferList mBufferList;
		YZViewList mViewList;
		YZEvents *events;
		YZInternalOptionPool *mOptions;
		YZRegisters *mRegisters;
		YZYzisinfo* mYzisinfo;
		YZTagStack *mTagStack;

};

#endif /* YZ_SESSION_H */
