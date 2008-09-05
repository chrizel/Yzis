/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>
 *  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
 *  Copyright (C) 2008 Loic Pauleve <panard@inzenet.org>
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

#ifndef YZ_VIEW_H
#define YZ_VIEW_H

#include <QString>

/* yzis */
#include "viewiface.h"
#include "viewcursor.h"
#include "mode_pool.h"
#include "drawbuffer.h"
#include "buffer.h"

class YViewCursor;
class YColor;
class YCursor;
class YSession;
class YzisAttribute;
class YLine;
class YLineSearch;
class YMode;
class YModeCompletion;
class YOptionValue;

class YZFoldPool;

/**
 * MUST be reimplemented in the GUI. 
 * It's the basis to display the content of a buffer.
 * One view is the display of some part of a buffer, it is used to receive inputs and displays
 * corresponding outputs.
 * Each @ref YBuffer can have multiple views.
 * @ref YBuffer will take care of synchronizing every views so updates are propagated to all views.
 * @short Abstract object for a view.
 * 
 */
class YZIS_EXPORT YView : public YViewIface
{

	public:
		//-------------------------------------------------------
		// ----------------- Constructor/Destructor and ID
		//-------------------------------------------------------
		/**
		 * Each view is bound to a buffer, @arg lines is the initial
		 * number of columns and @arg lines the initial
		 * number of lines that this view can display
		 */
		YView(YBuffer *_b, YSession *sess, int cols, int lines);

		/**
		 * The destructor
		 */
		virtual ~YView();

		/**
		 * Accessor to the list of foldings
		 */
		inline YZFoldPool* folds() const
		{
			return mFoldPool;
		}

		/**
		 * A global UID for this view
		 *   only used for local options (<filename>-view-<id>)
		 **/
		inline const int getId() const { return id; }

		/** Return a string description of the view.
		 *
		 * The string description contains:
		 * - the id
		 * - the buffer filename
		 * - the this pointer
		 *
		 **/
		QString toString() const;

		//-------------------------------------------------------
		// ----------------- Visible Areas
		//-------------------------------------------------------
		/**
		 * Updates the number of visible @arg c columns and @arg l lines
		 * 	it will cause a screen recalculation.
		 * @arg c is the number of columns
		 * @arg l is the number of lines
		 */
		void setVisibleArea( int c, int l );

		//-------------------------------------------------------
		// ----------------- Line Visibility
		//-------------------------------------------------------
		/**
		 * Returns the index of the first line displayed on the view
		 */
		int getCurrentTop() const;

		/**
		 * returns the number of line this view can display
		 */
		int getLinesVisible() const;

		/**
		 * returns the number of lines this view can display
		 * @return the number of visible lines
		 */
		int getColumnsVisible() const;

		/**
		 * Returns true if the line @arg l is visible. False otherwise.
		 */
		bool isLineVisible(int l) const;

		/**
		 * Returns true if the column @arg c is visible for @arg line ( expanding TABs ). False otherwise
		 */
		bool isColumnVisible(int column, int line) const;

		//-------------------------------------------------------
		// ----------------- Associated Objects
		//-------------------------------------------------------
		/**
		 * Return my current buffer
		 */
		YBuffer *myBuffer() const
		{
			return mBuffer;
		}
		//  const YBuffer *myBuffer() const { return mBuffer; }

		/**
		 * Return my current line search
		 */
		YLineSearch* myLineSearch()
		{
			return mLineSearch;
		}

		/**
     * Accessor to the list of recorded registers
     * @return a QList of @ref YRegisters
     */
    const QList<QChar> registersRecorded() const
    {
        return mRegs;
    }

    //-------------------------------------------------------
    // ----------------- Scrolling
    //-------------------------------------------------------
    /**
     * Adjust view vertically to show @arg line on bottom
     */
    void bottomViewVertically( int line );

    /**
     * Center view vertically on the given @arg line
     * if line == -1, then center on the current line
     */
    void centerViewVertically( int line = -1 );

    /**
     * Center view horizontally on the given @arg column
     */
    void centerViewHorizontally( int column );

    /**
     * align view vertically on the given buffer @arg line
     */
    void alignViewBufferVertically(int line);

    /**
     * align view vertically on the given screen @arg line
     */
    void alignViewVertically(int line);

    //-------------------------------------------------------
    // ----------------- Command Input Buffer
    //-------------------------------------------------------
    /**
     * Clean out the current buffer of inputs
     * Typically used after a command is recognized or when ESC is pressed
     */
    void purgeInputBuffer()
    {
        mPreviousChars.clear();
    }
    void appendInputBuffer( const YKey & k )
    {
        mPreviousChars.append( k );
    }
    void saveInputBuffer();
    const YKeySequence &getInputBuffer() const
    {
        return mPreviousChars;
    }
    const YKeySequence &getLastInputBuffer() const
    {
        return mLastPreviousChars;
    }

    //-------------------------------------------------------
    // ----------------- Cursor Motion
    //-------------------------------------------------------
    // Return value is whether motion stopped by constraints of buffer
    /**
     * moves the cursor of the current view down
     */
    bool moveDown( int nb_lines = 1, bool applyCursor = true );
    bool moveDown( YViewCursor* viewCursor, int nb_lines = 1, bool applyCursor = true );

    /**
     * moves the cursor of the current view up
     */
    bool moveUp( int nb_lines = 1, bool applyCursor = true );
    bool moveUp( YViewCursor* viewCursor, int nb_lines = 1, bool applyCursor = true );

    /**
     * moves the cursor of the current view to the left
     */
    bool moveLeft(int nb_cols = 1, bool wrap = false, bool applyCursor = true);
    bool moveLeft( YViewCursor* viewCursor, int nb_cols = 1, bool wrap = false, bool applyCursor = true);

    /**
     * moves the cursor of the current view to the right
     */
    bool moveRight(int nb_cols = 1, bool wrap = false, bool applyCursor = true);
    bool moveRight( YViewCursor* viewCursor, int nb_cols = 1, bool wrap = false, bool applyCursor = true);

    /**
     * moves the cursor of the current view to the first non-blank character
     * of the current line
     */
    QString moveToFirstNonBlankOfLine();
    QString moveToFirstNonBlankOfLine( YViewCursor* viewCursor, bool applyCursor = true );

    /**
     * moves the cursor of the current view to the start of the current line
     */
    QString moveToStartOfLine();
    QString moveToStartOfLine( YViewCursor* viewCursor, bool applyCursor = true );

    /**
     * moves the cursor of the current view to the end of the current line
     */
    QString moveToEndOfLine();
    QString moveToEndOfLine( YViewCursor* viewCursor, bool applyCursor = true );

    /**
     * Moves the draw cursor to @arg nextx, @arg nexty
     */
    void gotodxdy(QPoint nextpos, bool applyCursor = true );
    // TODO : remoev
    void gotodxdy(int nextx, int nexty, bool applyCursor = true )
    {
        gotodxdy(QPoint(nextx, nexty), applyCursor);
    }
    void gotodxdy( YViewCursor* viewCursor, QPoint nextpos, bool applyCursor = true );
    void gotodxdy( YViewCursor* viewCursor, const int x, const int y, bool applyCursor = true )
    {
        gotodxdy(viewCursor, QPoint(x, y), applyCursor);
    }

    /**
     * Moves the cursor to @arg draw nextx, @arg buffer nexty
     */
    void gotodxy(int nextx, int nexty, bool applyCursor = true );
    void gotodxy( YViewCursor* viewCursor, int nextx, int nexty, bool applyCursor = true );

    /**
     * Moves the buffer cursor to @arg nextx, @arg nexty
     */
    void gotoxy(const QPoint nextpos, bool applyCursor = true );
    void gotoxy( YViewCursor* viewCursor, const QPoint nextpos, bool applyCursor = true );
    void gotoxy(int nextx, int nexty, bool applyCursor = true )
    {
        gotoxy(QPoint(nextx, nexty), applyCursor);
    }
    void gotoxy( YViewCursor* viewCursor, int nextx, int nexty, bool applyCursor = true )
    {
        gotoxy(viewCursor, QPoint(nextx, nexty), applyCursor);
    }

    /**
     * Moves the buffer cursor to @arg cursor and stick the column
     */
    void gotoxyAndStick( const QPoint cursor );
    void gotoxyAndStick( const int x, const int y)
    {
        gotoxyAndStick(QPoint(x, y));
    }
    void gotodxdyAndStick( const QPoint cursor );
    void gotodxdyAndStick( const int x, const int y )
    {
        gotodxdyAndStick(QPoint(x, y));
    }

    /**

     */
    void gotoLine( int line );
    void gotoLine( YViewCursor* viewCursor, int line, bool applyCursor = true );

    /**
     * Go to last line of the file
     */
    void gotoLastLine();
    void gotoLastLine( YViewCursor* viewCursor, bool applyCursor = true );

    /**
     * move the cursor to the sticky column
     */
    void gotoStickyCol( int Y );
    void gotoStickyCol( YViewCursor* viewCursor, int Y, bool applyCursor = true );

    void applyStartPosition( const YCursor pos );

	//-------------------------------------------------------
    // ----------------- Selection
    //-------------------------------------------------------

	/*TODO: docstring */
	YRawData setSelection( yzis::SelectionType type, const YInterval& bufferInterval );


    //-------------------------------------------------------
    // ----------------- Drawing
    //-------------------------------------------------------

    /**
     * Character color at column line
     */
    const YColor& drawColor ( int col, int line ) const;

    //-------------------------------------------------------
    // ----------------- Undo
    //-------------------------------------------------------
    /**
     * Commit undo item
     */
    void commitUndoItem();

    /**
     * Undo last action
     */
    void undo ( int count = 1 );

    /**
     * Redo last undoed action
     */
    void redo ( int count = 1 );

    /**
     * Prepares to record next undo item
     */
    void commitNextUndo();

    //-------------------------------------------------------
    // ----------------- Macros
    //-------------------------------------------------------
    /**
     * Start recording a macro into @param regs
     */
    void recordMacro( const QList<QChar> &regs );

    /**
     * Stop recording macros
     */
    void stopRecordMacro();

    /**
     * Are macros being recorded
     */
    bool isRecording() const
    {
        return mRegs.count() > 0;
    }

    //-------------------------------------------------------
    // ----------------- Buffer Modification
    //-------------------------------------------------------
    /**
     * Append after current character
     */
    QString append ( );

    /**
     * Reindent given line ( cindent )
     */
    void reindent(const QPoint pos);

    /**
     * Create new indented line
     */
    void indent();

    /**
     * Prepend enough spaces to string so line is "centered" 
     */
    QString centerLine( const QString& );

    //-------------------------------------------------------
    // ----------------- Options
    //-------------------------------------------------------
    QString getLocalOptionKey() const;

    YOptionValue* getLocalOption( const QString& option ) const;

    /**
     * Retrieve an int option
     */
    int getLocalIntegerOption( const QString& option ) const;

    /**
     * Retrieve a bool option
     */
    bool getLocalBooleanOption( const QString& option ) const;

    /**
     * Retrieve a string option
     */
    QString getLocalStringOption( const QString& option ) const;

    /**
     * Retrieve a qstringlist option
     */
    QStringList getLocalListOption( const QString& option ) const;

    /**
     * Retrieve a map option
     */
    MapOption getLocalMapOption( const QString& option ) const;

    //-------------------------------------------------------
    // ----------------- Paint Events
    //-------------------------------------------------------
    virtual void guiPaintEvent( const YSelection& drawMap );

	void sendPaintEvent( const YInterval& i );
    void sendPaintEvent( const YCursor from, const YCursor to ) YZIS_DEPRECATED;
    void sendPaintEvent( int curx, int cury, int curw, int curh ) YZIS_DEPRECATED;
    void sendPaintEvent( YSelectionMap map, bool isBufferMap = true ) YZIS_DEPRECATED;

    /**
     * Ask for refresh screen
     */
    void sendRefreshEvent();

    void removePaintEvent( const YCursor from, const YCursor to ) YZIS_DEPRECATED;

    /**
     * @arg enable is true, future paint events will be directly applied
     * @arg enable is false, paint events will wait a commit to be applied
     */
    void setPaintAutoCommit( bool enable = true );

    /**
     * drop all pending paint events and returns into autocommit mode
     */
    void abortPaintEvent();

    /**
     * If the number of calls of commitPaintEvent is equals to the number of 
     * calls of setPaintAutoCommit(false), pending events are applied, and returns
     * in autocommit mode.
     */
    void commitPaintEvent();

    /**
     * Asks a redraw of the whole view, use sendRefreshEvent instead
     */
    virtual void refreshScreen() YZIS_DEPRECATED;

    /**
     * recalcScreen refresh the screen and recalculate cursor position
     */
    void recalcScreen();

    //-------------------------------------------------------
    // ----------------- Cursors
    //-------------------------------------------------------
    void sendCursor( YViewCursor cursor );

    /**
     * Get the view cursor
     * @return a constant ref to the view cursor ( YViewCursor )
     */
    const YViewCursor &viewCursor() const
    {
        return mainCursor;
    }

    /**
     * Get the current cursor information
     * @return a reference on the current cursor
     */
    const YCursor getCursor() const;

    /**
     * Get the current buffer cursor information
     * @return a reference on the current buffer cursor
     */
    const YCursor getBufferCursor() const;

    /** Update the GUI when the cursor position has changed.
     * Updates the status bar and notificates the GUI through
     * guiUpdateCursorPosition, which should update the editor cursor position.
     *
     * Can't be called until the GUI has been initialized.
     */
    void updateCursor();

    /** Update the GUI when current mode has changed.
     * Updates the status bar and informs the GUI through 
     * guiUpdateMode, which should update the cursor shape.
     *
     * Can't be called until the GUI has been initialized.
     */
    void updateMode();

    /** Update the GUI when current file name has changed.
     * Updates the status bar and notificates the GUI
     * through guiUpdateFileName.
     *
     * Can't be called until the GUI has been initialized.
     */
    void updateFileName();

    /** Update the GUI when file status has changed.
     * Updates the status bar and notificates the GUI
     * through guiUpdateFileInfo.
     *
     * Can't be called until the GUI has been initialized.
     */
    void updateFileInfo();

    /** Display an informational message to the user.
     * The message is shown in the status bar and passed
     * to the GUI through guiDisplayInfo.
     *
     * Can't be called until the GUI has been initialized.
     */
    void displayInfo(const QString&);

    //-------------------------------------------------------
    // ----------------- Sticky
    //-------------------------------------------------------
    /**
     * Updates stickyCol
     */
    void setStickyCol( int col )
    {
        stickyCol = col;
    }

    void updateStickyCol( );

    /**
     * update stickCol to according to viewCursor
     */
    void updateStickyCol( YViewCursor* viewCursor );

    //-------------------------------------------------------
    // ----------------- Mode
    //-------------------------------------------------------

    /**
     * Accessor to the list of availables modes
     * @return a QMap of @ref YMode
     */
    YModePool* modePool() const
    {
        return mModePool;
    }

    /** Return the current key mode of the view
     */
    YMode * currentMode() const 
    {
        return mModePool->current();
    }

    //-------------------------------------------------------
    // ----------------- Modifier Keys
    //-------------------------------------------------------
    virtual void registerModifierKeys( const QString& )
    {}
    virtual void unregisterModifierKeys( const QString& )
    {}

    //-------------------------------------------------------
    // ----------------- Miscellaneous
    //-------------------------------------------------------
    /**
     * Display Intro text message 
     */
    void displayIntro();

    virtual void printToFile( const QString& path );

    QString getCharBelow( int delta );

    /*
     * @returns current screen YCursor relative to top-left screen corner
     */
    YCursor getRelativeScreenCursor() const;

    /**
     * returns a YSelection which fit view
     */
    YSelection clipSelection( const YSelection& sel ) const;

	/*
	 * TODO: docstring
	 */
	void updateBufferInterval( int bl, int bl_last );
	void updateBufferInterval( const YInterval& bi );

	// TODO: docstring
	YDrawLine drawLineFromYLine( const YLine* yl, int start_column = 0 );

protected:

    void setupKeys();

    bool stringHasOnlySpaces ( const QString& what );

    YDrawBuffer mDrawBuffer;

private:
	/* update internal attributes */
	void updateInternalAttributes();


	// TODO: docstring
	int setBufferLineContent( int bl, const YLine* yl );
	/* TODO: docstring */
	void deleteFromBufferLine( int bl );

	YDrawSection mWorkDrawSection;
	YDrawLine mWorkDrawLine;

    /*
     * scroll draw buffer and view
     */
    void internalScroll( int dx, int dy );

    /**
     * Used to store previous keystrokes which are not recognised as a command,
     * this should allow us to have commands like : 100g or gg etc ...
     */
    YKeySequence mPreviousChars;
    YKeySequence mLastPreviousChars;

    /**
     * The buffer we depend on
     */
    YBuffer *mBuffer;

    /**
      * This is the main cursor, the one which is displayed
     */
    YViewCursor mainCursor;

    /**
     * Searching backward
     */
    bool reverseSearch;

    /**
     * The current session, provided by the GUI
     */
    YSession *mSession;

    /**
     * Line search
     */
    YLineSearch* mLineSearch;

    /**
     * This is the worker cursor, the one which we directly modify in our draw engine
     */
    YViewCursor workCursor;

    YzisAttribute* mHighlightAttributes;

    void gotoy( int y );
    void gotody( int y );
    void gotox( int x, bool forceGoBehindEOL = false );
    void gotodx( int x );
    void applyGoto( YViewCursor* viewCursor, bool applyCursor = true );
    void initGoto( YViewCursor* viewCursor );

    int stickyCol;

    QChar m_lineFiller;
    QChar m_lineMarker;

    /// cached value of tabstop option
    int tabstop;
    bool wrap;
    bool rightleft;

    QMap<yzis::SelectionType, YInterval> mSelectionPool;
    YSelection mPaintSelection;


    /// which regs to store macros in
    QList<QChar> mRegs;
    int m_paintAutoCommit;
    YViewCursor keepCursor;

    YModePool* mModePool;

    /**
     * options cache
     */
    int opt_schema;
    bool opt_list;
    MapOption opt_listchars;
    YZFoldPool* mFoldPool;

    const int id;
};

#endif /*  YZ_VIEW_H */

