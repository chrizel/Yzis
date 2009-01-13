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
#include <QPoint>

/* yzis */
//#include "cursor.h"
#include "mode.h"  // for YModeMap

class YzisSchemaManager;
class YInternalOptionPool;
class YRegisters;
class YSearch;
class YEvents;
class YMode;
class YModeEx;
class YModeCommand;
class YViewCursor;
class YInfo;
class YTagStack;
class YCursor;
class YResourceMgr;

typedef QList<YBuffer*> YBufferList;
typedef QList<YView*> YViewList;

/**
  * Class YSession
  *
  * @short Singleton class representing an Yzis application.
  *
  * The YSession is managing all global attributes and method of Yzis. 
  *
  * It takes care of everything that is not local to a YBuffer or YView.
  *
  * Contains data referring to an instance of yzis
  * This may also be used to "transfer" a session from a GUI to another
  * A session owns the buffers
  * A buffer owns the views
  */
class YZIS_EXPORT YSession : public YSessionIface
{
protected:
    /**
     * Constructor. Protected, because YSession is a singleton.
           *
           * The constructor does mostly nothing. All the hard work
           * is done in setInstance() and init()
     */
    YSession();

    /**
     * Destructor. Protected, because YSession is a singleton.
     */
    virtual ~YSession();

    /**
     *  Sets the session instance object and initialise the session.
           *
     *  It MUST be called from concrete class which inherits from YSession.
     *
     *  Example from QYZisSession:
     *  \code
     *  void QYZisSession::createInstance() // static function
     *  {
     *   static QYZisSession instance;
     *   setInstance(&instance);
     *  }
     *  \endcode
     *
     *  And then QYZisSession::createInstance() is called from main
     *  before any other libyzis object gets created
     *
     *  @see QYZisSession::createInstance()
     *  @see NYSession::createInstance()
     */
    static void setInstance(YSession* instance);

    /** Initialise YSession.
      *
      * This method is called in setInstance()
      */
    void init();

    /** Initialise the language stuff, for translation and display
      * in proper font and language.
      *
      * This method is called from init()
      */
    void initLanguage();

    /** Creates the Yzis Resource Manager and the ~/.yzis directory if
      * needed.
      *
      * This methed is called from init().
      */
    void initResource();

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

    /** Run a Lua Script specified on the commandline
      *
      * run the Lua script that was passed on the command line with -s
      * to libyzis for processing.
      *
      * This function should be called explicitly by the frontend
      * once all the frontend initialisation is complete.
      */
    void runLuaScript();


public:

    /** Initialise the debug engine.
            *
            * You should call this static method as the very first step
            * of your program, to ensure that all components can properly
            * use debug code.
            *
            * initDebug() should be called even before YSession() creation.
            *
      */
    static void initDebug(int argc, char ** argv);

    /** YZis singleton instance.
           *
     *  @return the one and only one instance of YSession
     *  @see setInstance
     */
    static YSession * self();

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
     * @return YModeMap
     */
    YModeMap getModes() const;

    /** Returns the ex pool
     * 
     * @return YModeEx*
     */
    YModeEx* getExPool();

    /** Returns the command pool
     * 
     * @return YModeCommand*
     */
    YModeCommand* getCommandPool();

    /** Returns the yzisinfo list
     * 
     * @return YInfo*
     */
    YInfo* getYzisinfo();

    /**
     * search
     */
    YSearch *search()
    {
        return mSearch;
    }

    YTagStack &getTagStack();
    const YTagStack &getTagStack() const;

    /**
     * Get a pointer on the schema manager for syntax highlighting
     */
    YzisSchemaManager *schemaManager()
    {
        return mSchemaManager;
    }

    /**
     * Get the Internal Option Pool
     */
    YInternalOptionPool *getOptions();

    //-------------------------------------------------------
    // ----------------- Buffer Management
    //-------------------------------------------------------

    /**
     * Creates a new buffer and puts it in a new view
     * To get the created buffer, call YView::myBuffer()
     */
    YView *createBufferAndView( const QString &path = QString() );

    /**
     * Remove a buffer.
           *
     * This buffer is no longer used by Yzis. Yzis calls deleteView() on all
     * the views of the buffer. The last view calls deleteBuffer().
     */
    void removeBuffer( YBuffer * b );

    /**
     * Final step of buffer deletion.
     *
     * Only the YView destructor should call this function, because buffer
     * destruction is managed by the views.
     *
     * For starting the buffer removal, please call removeBuffer()
     *
     * The gui is informed of the buffer deletion
     * with a call to guiDeleteBuffer().
     */
    void deleteBuffer( YBuffer * b );

    /**
     * Returns a const reference to the buffer list
     * Designed to be used for operations that have to occur
     * for each buffer
     */
    const YBufferList & buffers() const
    {
        return mBufferList;
    }

    /**
     * Finds a buffer by a filename
     */
    YBuffer* findBuffer( const QString& path );

    /**
     * Check if one buffer is modified and not saved
     * @returns whether a buffer has been modified and not saved since
     */
    bool isOneBufferModified() const;

/*private: XXX why is this thing private?? */
    /**
     * Creates a new buffer.
     *
     * If a buffer on the path already exists, return it. Else,
     * ask the frontend to create a buffer with guiCreateBuffer()
     * set the buffer as current and return it.
     */
    YBuffer *createBuffer(const QString& path = QString());

public:
    //-------------------------------------------------------
    // ----------------- View Management
    //-------------------------------------------------------
    /**
     * Create a new view on a @p buffer.
     */
    YView* createView ( YBuffer* buffer );

    /**
     * Delete the view @p v.
     *
     * The view is removed from the view list and guiDeleteView() is called to
     * let the GUI delete the view instance. 
     *
     * Deleting a view triggers a buffer deletion if the view is the last one
     * on the buffer.
     *
     * When the view is the last view of the application, exitRequest() is
     * called and methods returns immediately, without calling
     * guiDeleteView().
     */
    void deleteView ( YView* v );

    /**
     * Returns a pointer to the current view
     */
    YView* currentView() { return mCurView; }

    /**
     * Change the current view ( unassigned )
     */
    void setCurrentView( YView* );

    /**
     * Gets a list of all YViews active in the system
     */
    const YViewList getAllViews() const;

    /** \brief Find a view containing the buffer.
           *
           * Can be called with a NULL argument.
     */
    YView *findViewByBuffer( const YBuffer *buffer );

    /**
     * Finds the first view
     */
    YView* firstView();

    /**
     * Finds the last view
     */
    YView* lastView();

    /**
     * Finds the next view relative to the current one
     */
    YView* nextView();

    /**
     * Finds the previous view relative to the current one
     */
    YView* prevView();


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
    bool exitRequest(int errorCode = 0);

    /**
     * Saves all buffers with a filename set.
           *
           * May popup to ask fo the name of the file to save (from
           * YBuffer::save() ).
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
    QStringList eventCall(const QString& event, YView *view = NULL);

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


    /** Get an instance of the resource manager */
    virtual YResourceMgr * resourceMgr()
    {
        return mResourceMgr;
    }

    //-------------------------------------------------------
    // ----------------- Miscellaneous
    //-------------------------------------------------------

    /** Copied from view */
    CmdState sendMultipleKeys( YView * view, YKeySequence &keys);

    //-------------------------------------------------------
    // ----------------- Send events to GUI
    //-------------------------------------------------------
    /**
     * transfer key events from GUI to core
     */
    virtual CmdState sendKey( YView * view, YKey _key );

    void registerModifier ( const QString& mod );
    void unregisterModifier ( const QString& mod );

    void saveJumpPosition();
    void saveJumpPosition( const QPoint cursor );
    const YCursor previousJumpPosition();

    /** Because of windows, we need to have new defined in the
      * shared library. */
    void * operator new( size_t tSize );

    /** Because of windows, we need to have delete defined in the
      * shared library. */
    void operator delete( void* p );


protected:
    /**
     * Send multiple key sequence to yzis.
     * 
           * The key sequence is automatically sent to the right view,
           * even if the view is switched in the middle.
     */
    void scriptSendMultipleKeys ( const QString& text );


    void initModes();
    void endModes();

private:
    /**
     *  Copy constructor. Disable copy by declaring it as private
     */
    YSession(const YSession&);
    /**
     *  Copy operator. Disable copy by declaring it as private
     */
    YSession& operator=(const YSession&);
    /**
     *  Single instance of YSession
     */
    static YSession* mInstance;

    QString mInitkeys;
    QString mLuaScript;
    YView* mCurView;
    YBuffer* mCurBuffer;
    YzisSchemaManager *mSchemaManager;
    YSearch *mSearch;
    YModeMap mModes;
    YBufferList mBufferList;
    YViewList mViewList;
    YEvents *events;
    YInternalOptionPool *mOptions;
    YRegisters *mRegisters;
    YInfo* mYzisinfo;
    YTagStack *mTagStack;
    YResourceMgr * mResourceMgr;

};

#endif /* YZ_SESSION_H */
