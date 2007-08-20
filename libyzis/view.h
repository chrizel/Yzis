/* This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
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

#ifndef YZ_VIEW_H
#define YZ_VIEW_H

#include <QString>

/* yzis */
#include "viewiface.h"
#include "viewcursor.h"
#include "mode_pool.h"

class YViewCursor;
class YColor;
class YCursor;
class YBuffer;
class YSession;
class YSelectionPool;
class YzisAttribute;
class YLineSearch;
class YMode;
class YModeCompletion;
class YOptionValue;
class YZFoldPool;
struct YDrawCell;

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

    friend class YDrawBuffer;

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
    const int getId() const;

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
     * @arg c is the number of columns
     * @arg l is the number of lines
     * @arg resfresh if true, refreshView is called
     */
    void setVisibleArea (int c, int l, bool refresh = true );

    //-------------------------------------------------------
    // ----------------- Line Visibility
    //-------------------------------------------------------
    /**
     * Returns the index of the first line displayed on the view
     */
    int getCurrentTop() const
    {
        return scrollCursor.bufferY();
    }
    int getDrawCurrentTop() const
    {
        return scrollCursor.screenY();
    }

    /**
     * Returns the index of the first "buffer" column displayed on the view
     * (does not care about tabs, wrapping ...)
     */
    int getCurrentLeft() const
    {
        return scrollCursor.bufferX();
    }

    /**
     * Returns the index of the first "screen" column displayed on the view
     * (does care about tabs, wrapping ...)
     */
    int getDrawCurrentLeft() const
    {
        return scrollCursor.screenX();
    }



    /**
     * returns the number of line this view can display
     */
    int getLinesVisible() const
    {
        return mLinesVis;
    }

    /**
     * returns the number of lines this view can display
     * @return the number of visible lines
     */
    int getColumnsVisible() const
    {
        return mColumnsVis;
    }

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
     * Accessor to the list of current selections
     */
    YSelectionPool* getSelectionPool() const
    {
        return selectionPool;
    }

    /**
     * Accessor to the list of recorded registers
     * @return a QList of @ref YRegisters
     */
    const QList<QChar> registersRecorded() const
    {
        return mRegs;
    }

    YSelectionMap visualSelection() const;

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
        mPreviousChars = "";
    }
    void appendInputBuffer( const QString & s )
    {
        mPreviousChars += s;
    }
    void saveInputBuffer();
    QString getInputBuffer() const
    {
        return mPreviousChars;
    }
    QString getLastInputBuffer() const
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
     * Moves the cursor to @arg buffer nextx, @arg draw nexty
     */
    void gotoxdy(int nextx, int nexty, bool applyCursor = true );
    void gotoxdy( YViewCursor* viewCursor, int nextx, int nexty, bool applyCursor = true );

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
     * Go to line of file
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
    // ----------------- Drawing
    //-------------------------------------------------------
    /**
     * init r and s Cursor
     */
    void initDraw( );
    void initDraw( int sLeft, int sTop, int rLeft, int rTop, bool draw = true );

    int initDrawContents( int clipy );

    /**
     * go to previous line
     */
    bool drawPrevLine( );

    /**
     * go to next line
     */
    bool drawNextLine( );

    /**
     * go to prev col
     */
    bool drawPrevCol( );

    /**
     * go to next col
     */
    bool drawNextCol( );

    const QChar& drawLineFiller() const;
    const QChar& drawLineMarker() const;

    const QChar& fillChar() const;

    /**
     * draw char
     */
    const QChar& drawChar() const;

    /**
     * char length
     */
    int drawLength() const;

    /**
     * line increment (on screen)
     */
    int drawHeight() const;

    /**
     * char color
     */
    const YColor& drawColor();

    /**
     * char color if selected
     */
    const YColor& drawSelColor();

    /**
     * char background color
     */
    const YColor& drawBgColor();

    /**
     * char background color if selected
     */
    const YColor& drawBgSelColor();

    /**
     * current char is bold
     */
    bool drawBold();

    /**
     * current char is italic
     */
    bool drawItalic();

    /**
     * current char is underlined
     */
    bool drawUnderline();

    /**
     * current char is overlined
     */
    bool drawOverline();

    /**
     * current char is striked-out
     */
    bool drawStrikeOutLine();

    /**
     * current char outline color
     */
    const YColor& drawOutline();

    /**
     * Character color at column line
     */
    const YColor& drawColor ( int col, int line ) const;

    /**
     * return current buffer line
     */
    int drawLineNumber() const;

    /**
     * total height ( draw )
     */
    int drawTotalHeight();

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

    void sendPaintEvent( const YCursor from, const YCursor to );
    void sendPaintEvent( int curx, int cury, int curw, int curh );
    void sendPaintEvent( YSelectionMap map, bool isBufferMap = true );

    /**
     * ask to draw from buffer line @arg line to @arg line + @arg n
     */
    void sendBufferPaintEvent( int line, int n );

    /**
     * Ask for refresh screen
     */
    void sendRefreshEvent();

    void removePaintEvent( const YCursor from, const YCursor to );

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
     * Asks a redraw of the whole view
     */
    virtual void refreshScreen();

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

    YViewCursor* visualCursor()
    {
        return &mVisualCursor;
    }

    /**
     * Updates the position of the cursor
     */
    void updateCursor();

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
    // ----------------- Dimensions
    //-------------------------------------------------------
    /**
     * line increment (on buffer)
     */
    int lineIncrement( ) const;

    /**
     * current line height (on screen)
     */
    int lineHeight( ) const;

    //-------------------------------------------------------
    // ----------------- Mode
    //-------------------------------------------------------
    /**
     * Get the text describing the mode, adding the text recording when
     * the view is recording.
     */
    QString modeString() const;

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
    // ----------------- Changes occurred around (x, y)
    //-------------------------------------------------------
    /**
     * initChanges and applyChanges are called by the buffer to inform the view that there are
     * changes around x,y. Each view have to find what they have to redraw, depending
     * of the wrap option, and of course window size.
     */
    void initChanges( QPoint pos );
    void applyChanges( int y );

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
     * @returns screen top-left corner position
     */
    YCursor getScreenPosition() const;

    /*
     * @returns current screen YCursor relative to top-left screen corner
     */
    YCursor getRelativeScreenCursor() const;

    /**
     * returns a YSelection which fit view
     */
    YSelection clipSelection( const YSelection& sel ) const;

protected:

    void setupKeys();

    bool stringHasOnlySpaces ( const QString& what );

    QString getLineStatusString() const;

    YDrawBuffer m_drawBuffer;

private:

    /*
     * scroll draw buffer and view
     */
    void internalScroll( int dx, int dy );

    /**
     * Information about the view. Used internally in @ref YView to create
     * the text for the statusbar displaying line/column numbers and how
     * much of the buffer that's being displayed.
     */
    class ViewInformation
    {
    public:
        /** buffer line */
        int l;

        /** buffer column */
        int c1;

        /** the virtual column of the cursor.  This is counting screen
         * cells from the left side of the window.  The leftmost column is
         * one. This will be the same as c1 if no tab characters are
         * present on this line. (1 tab = several screen columns) */
        int c2;

        /** how much of the buffer that is being displayed. */
        QString percentage;
    };

    ViewInformation viewInformation;

    /**
     * Used to store previous keystrokes which are not recognised as a command,
     * this should allow us to have commands like : 100g or gg etc ...
     */
    QString mPreviousChars;
    QString mLastPreviousChars;

    /**
     * The buffer we depend on
     */
    YBuffer *mBuffer;

    /**
      * This is the main cursor, the one which is displayed
     */
    YViewCursor mainCursor;

    /* screen top-left cursor */
    YViewCursor scrollCursor;

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

    /**
     * are we moving cursor in draw mode ?
     */
    bool drawMode;


    /**
     * Number of visible lines on the view
     */
    int mLinesVis;

    /**
     * Number of visible columns on the view
     */
    int mColumnsVis;

    /**
     * Index of the first visible line (buffer)
     */
    int sCurrentTop;

    /**
     * Index of the first visible line (buffer)
     */
    int sCurrentLeft;

    /**
     * Index of the first visible column (draw)
     */
    int rCurrentLeft;

    /**
     * Index of the first visible line (draw)
     */
    int rCurrentTop;

    int spaceWidth;

    const uchar* rHLa;

    bool rHLnoAttribs;

    int rHLAttributesLen;

    YzisAttribute *rHLAttributes;

    ///  current line
    QString sCurLine;
    ///  current line length
    int sCurLineLength;
    ///  current line max width ( tab is 8 spaces )
    int rCurLineLength;
    ///  current line min width( tab is 1 space )
    int rMinCurLineLength;

    void gotoy( int y );
    void gotody( int y );
    void gotox( int x, bool forceGoBehindEOL = false );
    void gotodx( int x );
    void applyGoto( YViewCursor* viewCursor, bool applyCursor = true );
    void initGoto( YViewCursor* viewCursor );
    void updateCurLine( );

    bool m_paintAll;

    int stickyCol;

    QChar mFillChar;
    QChar lastChar;
    bool listChar;

    QChar m_lineFiller;
    QChar m_lineMarker;


    YCursor origPos;
    int lineDY;

    YCursor beginChanges;

    /// cached value of tabstop option
    int tabstop;
    bool wrap;
    bool rightleft;

    /// tabstop * spaceWidth
    int tablength;

    /// tablength to wrap
    int areaModTab;

    /// if true, do not check for cursor visibility
    bool adjust;

    YSelectionPool * selectionPool;
    YSelection* mPaintSelection;

    //Visual Mode stuff
    YViewCursor mVisualCursor; // TODO : this one is only used by external class, not by this very one ?


    /// which regs to store macros in
    QList<QChar> mRegs;
    int m_paintAutoCommit;
    YViewCursor keepCursor;

    /// the current attribute being used by the GUI
    YzisAttribute * curAt;
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

