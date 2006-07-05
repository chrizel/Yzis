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
/**
 * $Id$
 */

#include "color.h"
#include "selection.h"
#include "mode.h"
#include "option.h"
#include "viewid.h"

#include <qglobal.h>
#include <QVector>
#include <QCoreApplication>

#include "drawbuffer.h"
class YZDrawBuffer;
struct YZDrawCell;

class YZViewCursor;
class YZCursor;
class YZBuffer;
class YZSession;
class YZSelectionPool;
class YzisAttribute;
class YZLineSearch;
class YZView;
class YZModePool;
class YZMode;
class YZModeCompletion;
class YZOptionValue;
class YZFoldPool;

/**
 * MUST be reimplemented in the GUI. 
 * It's the basis to display the content of a buffer.
 * One view is the display of some part of a buffer, it is used to receive inputs and displays
 * corresponding outputs.
 * Each @ref YZBuffer can have multiple views.
 * @ref YZBuffer will take care of synchronizing every views so updates are propagated to all views.
 * 
 */
class YZIS_EXPORT YZView {

	friend class YZDrawBuffer;

	public:
		//-------------------------------------------------------
		// ----------------- Constructor/Destructor and ID
		//-------------------------------------------------------
		/**
		 * Each view is bound to a buffer, @arg lines is the initial
		 * number of lines that this view can display
		 */
		YZView(YZBuffer *_b, YZSession *sess, int lines);
		
		/**
		 * The destructor
		 */
		virtual ~YZView();

		/**
		 * Accessor to the list of foldings
		 */
		inline YZFoldPool* folds() const {
			return mFoldPool;
		}

		/**
		 * A global UID for this view
		 **/
		const YZViewId& getId() const;

		//-------------------------------------------------------
		// ----------------- Fonts
		//-------------------------------------------------------
		/**
		 * set font mode ( fixed or not )
		 * @arg fixed is true if used font is fixed
		 */
		void setFixedFont( bool fixed );

		//-------------------------------------------------------
		// ----------------- Visible Areas
		//-------------------------------------------------------
		/**
		 * Updates the number of visible @arg c columns and @arg l lines
		 * @arg c is the number of columns ( fixed fonts ) or width in pixel ( non-fixed fonts ) of the Area
		 * @arg l is the number of lines
		 * @arg resfresh if true, refreshView is called
		 */
		void setVisibleArea (int c, int l, bool refresh = true );
		
		//-------------------------------------------------------
		// ----------------- Send events to GUI
		//-------------------------------------------------------
		/**
		 * transfer key events from GUI to core
		 */
		void sendKey(const QString& key, const QString& modifiers="");

		//-------------------------------------------------------
		// ----------------- Line Visibility
		//-------------------------------------------------------
		/**
		 * Returns the index of the first line displayed on the view
		 */
		int getCurrentTop();
		int getDrawCurrentTop();

		/**
		 * Returns the index of the first "buffer" column displayed on the view
		 * (does not care about tabs, wrapping ...)
		 */
		int getCurrentLeft();

		/**
		 * Returns the index of the first "screen" column displayed on the view
		 * (does care about tabs, wrapping ...)
		 */
		int getDrawCurrentLeft();

		/**
		 * returns the number of line this view can display
		 */
		int getLinesVisible() { return mLinesVis; }

		/**
		 * returns the number of lines this view can display
		 * @return the number of visible lines
		 */
		int getColumnsVisible() { return mColumnsVis; }

		/**
		 * Returns true if the line @arg l is visible. False otherwise.
		 */
		bool	isLineVisible(int l);

		/**
		 * Returns true if the column @arg c is visible for @arg line ( expanding TABs ). False otherwise
		 */
		bool	isColumnVisible(int column, int line);

		//-------------------------------------------------------
		// ----------------- Associated Objects
		//-------------------------------------------------------
		/**
		 * Return my current buffer
		 */
		YZBuffer *myBuffer() const { return mBuffer; }
//		const YZBuffer *myBuffer() const { return mBuffer; }

		/**
		 * Return my current session
		 */
		YZSession *mySession() { return mSession; }

		/**
		 * Return my current line search
		 */
		YZLineSearch* myLineSearch() { return mLineSearch; }
		
		/**
		 * Accessor to the list of availables modes
		 * @return a QMap of @ref YZMode
		 */
		YZModePool* modePool() const { return mModePool; }
		
		/**
		 * Accessor to the list of current selections
		 */
		YZSelectionPool* getSelectionPool() const { return selectionPool; }
		
		/**
		 * Accessor to the list of recorded registers
		 * @return a QList of @ref YZRegisters
		 */
		const QList<QChar> registersRecorded() { return mRegs; }
		
		YZSelectionMap visualSelection();

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

		virtual void scrollUp( int ) = 0;
		virtual void scrollDown( int ) = 0;

		//-------------------------------------------------------
		// ----------------- Command Input Buffer
		//-------------------------------------------------------
		/**
		 * Clean out the current buffer of inputs
		 * Typically used after a command is recognized or when ESC is pressed
		 */
		void purgeInputBuffer() { mPreviousChars = ""; }
		void saveInputBuffer();
		QString getInputBuffer() { return mPreviousChars; }
		QString getLastInputBuffer() { return mLastPreviousChars; }

		//-------------------------------------------------------
		// ----------------- Cursor Motion
		//-------------------------------------------------------
		/**
		 * moves the cursor of the current view down
		 */
		QString moveDown( int nb_lines = 1, bool applyCursor = true );
		QString moveDown( YZViewCursor* viewCursor, int nb_lines = 1, bool applyCursor = true );

		/**
		 * moves the cursor of the current view up
		 */
		QString moveUp( int nb_lines = 1, bool applyCursor = true );
		QString moveUp( YZViewCursor* viewCursor,  int nb_lines = 1, bool applyCursor = true );

		/**
		 * moves the cursor of the current view to the left
		 */
		QString moveLeft(int nb_cols=1, bool wrap=false, bool applyCursor = true);
		QString moveLeft( YZViewCursor* viewCursor,int nb_cols=1, bool wrap=false, bool applyCursor = true);

		/**
		 * moves the cursor of the current view to the right
		 */
		QString moveRight(int nb_cols=1, bool wrap=false, bool applyCursor = true);
		QString moveRight( YZViewCursor* viewCursor,int nb_cols=1, bool wrap=false, bool applyCursor = true);

		/**
		 * moves the cursor of the current view to the first non-blank character
		 * of the current line
		 */
		QString moveToFirstNonBlankOfLine();
		QString moveToFirstNonBlankOfLine( YZViewCursor* viewCursor, bool applyCursor = true );

		/**
		 * moves the cursor of the current view to the start of the current line
		 */
		QString moveToStartOfLine();
		QString moveToStartOfLine( YZViewCursor* viewCursor, bool applyCursor = true );

		/**
		 * moves the cursor of the current view to the end of the current line
		 */
		QString moveToEndOfLine();
		QString moveToEndOfLine( YZViewCursor* viewCursor, bool applyCursor = true );

		/**
		 * Moves the draw cursor to @arg nextx, @arg nexty
		 */
		void gotodxdy(int nextx, int nexty, bool applyCursor = true );
		void gotodxdy( YZViewCursor* viewCursor,int nextx, int nexty, bool applyCursor = true );

		/**
		 * Moves the cursor to @arg buffer nextx, @arg draw nexty
		 */
		void gotoxdy(int nextx, int nexty, bool applyCursor = true );
		void gotoxdy( YZViewCursor* viewCursor,int nextx, int nexty, bool applyCursor = true );

		/**
		 * Moves the cursor to @arg draw nextx, @arg buffer nexty
		 */
		void gotodxy(int nextx, int nexty, bool applyCursor = true );
		void gotodxy( YZViewCursor* viewCursor,int nextx, int nexty, bool applyCursor = true );

		/**
		 * Moves the buffer cursor to @arg nextx, @arg nexty
		 */
		void gotoxy(int nextx, int nexty, bool applyCursor = true );
		void gotoxy( YZViewCursor* viewCursor, int nextx, int nexty, bool applyCursor = true );

		/**
		 * Moves the buffer cursor to @arg cursor and stick the column
		 */
		void gotoxyAndStick( const YZCursor& cursor );
		void gotoxyAndStick( int x, int y );
		void gotodxdyAndStick( const YZCursor& cursor );
		void gotodxdyAndStick( int x, int y );

		/**
		 * Go to line of file
		 */
		void gotoLine( int line );
		void gotoLine( YZViewCursor* viewCursor, int line, bool applyCursor = true );

		/**
		 * Go to last line of the file
		 */
		void gotoLastLine();
		void gotoLastLine( YZViewCursor* viewCursor, bool applyCursor = true );

		/**
		 * move the cursor to the sticky column
		 */
		void gotoStickyCol( int Y );
		void gotoStickyCol( YZViewCursor* viewCursor, int Y, bool applyCursor = true );

		void applyStartPosition( const YZCursor& pos );

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
		const YZColor& drawColor();

		/**
		 * char color if selected
		 */
		const YZColor& drawSelColor();

		/**
		 * char background color
		 */
		const YZColor& drawBgColor();

		/**
		 * char background color if selected
		 */
		const YZColor& drawBgSelColor();

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
		const YZColor& drawOutline();

		/**
		 * Character color at column line
		 */
		const YZColor& drawColor ( int col, int line ) const;

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
		// ----------------- GUI Status Notifications
		//-------------------------------------------------------
		/**
		 * Retrieve the text from command line
		 */
		virtual QString getCommandLineText() const = 0;

		/**
		 * Sets the command line text
		 */
		virtual void setCommandLineText( const QString& ) = 0;

		/**
		 * Displays an informational message
		 */
		virtual void displayInfo( const QString& info ) = 0;

		/**
		 * Display informational status about the current file and cursor
		 */
		virtual void syncViewInfo() = 0;

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
		bool isRecording() { return mRegs.count() > 0; }

		//-------------------------------------------------------
		// ----------------- Buffer Modification
		//-------------------------------------------------------
		/**
		 * Append after current character
		 */
		QString append ( );

		/**
		 * Pastes the content of default or given register
		 */
		void pasteContent( QChar registr, bool after = true );

		/**
		 * Reindent given line ( cindent )
		 */
		void reindent( int X, int Y );

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

		YZOptionValue* getLocalOption( const QString& option ) const;
		
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
		virtual void paintEvent( const YZSelection& drawMap );

		void sendPaintEvent( const YZCursor& from, const YZCursor& to );
		void sendPaintEvent( int curx, int cury, int curw, int curh );
		void sendPaintEvent( YZSelectionMap map, bool isBufferMap = true );

		/**
		 * ask to draw from buffer line @arg line to @arg line + @arg n
		 */
		void sendBufferPaintEvent( int line, int n );

		/**
		 * Ask for refresh screen
		 */
		void sendRefreshEvent();

		void removePaintEvent( const YZCursor& from, const YZCursor& to );
		void setPaintAutoCommit( bool enable = true );
		void abortPaintEvent();
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
		void sendCursor( YZViewCursor* cursor );
		
		/**
		 * Get the view cursor
		 * @return a constant ref to the view cursor ( YZViewCursor )
		 */
		const YZViewCursor &viewCursor() const { return *mainCursor; }

		/**
		 * Get the current cursor information
		 * @return a reference on the current cursor
		 */
		const YZCursor &getCursor() const;

		/**
		 * Get the current buffer cursor information
		 * @return a reference on the current buffer cursor
		 */
		const YZCursor &getBufferCursor() const;

		YZViewCursor* visualCursor() { return mVisualCursor; }

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
		void setStickyCol( int col ) { stickyCol = col; }

		void updateStickyCol( );
		
		/**
		 * update stickCol to according to viewCursor
		 */
		void updateStickyCol( YZViewCursor* viewCursor );

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

		/**
		 * width of a space ( in pixel or in cols )
		 */
		int getSpaceWidth() const;

		/**
		 * Returns pixel width of given string str, must be implemented in ui.
		 * Used only with non-Fixed fonts
		 */
		virtual int stringWidth( const QString& str ) const = 0;

		/**
		 * Returns pixel width of given char ch, must be implemented in ui.
		 * Used only with non-Fixed fonts
		 */
		virtual int charWidth( const QChar& ch ) const = 0;

		//-------------------------------------------------------
		// ----------------- Mode
		//-------------------------------------------------------
		/**
		 * Get the text describing the mode
		 */
		QString mode();

		/**
		 * called when the mode is changed, so that gui can
		 * update information diplayed to the user
		 */
		virtual void modeChanged() {};

		//-------------------------------------------------------
		// ----------------- GUI Notifications
		//-------------------------------------------------------
		/**
		 * Ask the GUI to popup for a filename
		 * @return whether a file name was successfully chosen
		 */
		virtual bool popupFileSaveAs() = 0;

		/**
		 * Called whenever the filename is changed
		 */
		virtual void filenameChanged() = 0;

		/**
		 * Notify GUIs that HL changed
		 */
		virtual void highlightingChanged() = 0;

		virtual void emitSelectionChanged() {}

		//-------------------------------------------------------
		// ----------------- Modifier Keys
		//-------------------------------------------------------
		virtual void registerModifierKeys( const QString& ) {}
		virtual void unregisterModifierKeys( const QString& ) {}

		//-------------------------------------------------------
		// ----------------- Changes occurred around (x, y)
		//-------------------------------------------------------
		/**
		 * initChanges and applyChanges are called by the buffer to inform the view that there are
		 * changes around x,y. Each view have to find what they have to redraw, depending
		 * of the wrap option, and of course window size.
		 */
		void initChanges( int x, int y );
		void applyChanges( int x, int y );

		//-------------------------------------------------------
		// ----------------- Miscellaneous
		//-------------------------------------------------------
		/**
		 * Display Intro text message 
		 */
		void displayIntro();

		virtual void printToFile( const QString& path );

		QString getCharBelow( int delta );

	public slots :
		void sendMultipleKey( const QString& keys );

	protected:

		virtual void notifyContentChanged( const YZSelection& s ) = 0;

		void setupKeys();

		bool stringHasOnlySpaces ( const QString& what );
		
		QString getLineStatusString() const;

		/*
		 * painting
		 */
		virtual void preparePaintEvent( int y_min, int y_max ) = 0;
		virtual void endPaintEvent() = 0;
		virtual void drawCell( int x, int y, const YZDrawCell& cell, void* arg ) = 0;
		virtual void drawClearToEOL( int x, int y, const QChar& clearChar ) = 0;
		virtual void drawSetMaxLineNumber( int max ) = 0;
		virtual void drawSetLineNumber( int y, int n, int h ) = 0;

		YZDrawBuffer m_drawBuffer;

	private:

		class  ViewInformation {
			public:
				int l;  //buffer line
				int c1; //buffer column
				int c2; //buffer column as drawn
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
		YZBuffer *mBuffer;

		/**
	 	 * This is the main cursor, the one which is displayed
		 */
		YZViewCursor* mainCursor;

		/* screen top-left cursor */
		YZViewCursor* scrollCursor;

		/**
		 * Searching backward
		 */
		bool reverseSearch;

		/**
		 * is font used fixed ?
		 */
		bool isFontFixed;

		/**
		 * The current session, provided by the GUI
		 */
		YZSession *mSession;

		/** 
		 * Line search
		 */
		 YZLineSearch* mLineSearch;

		/**
		 * This is the worker cursor, the one which we directly modify in our draw engine
		 */
		YZViewCursor* workCursor;

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

		// current line
		QString  sCurLine;
		// current line length
		int sCurLineLength;
		// current line max width ( tab is 8 spaces )
		int rCurLineLength;
		// current line min width( tab is 1 space )
		int rMinCurLineLength;

		void gotoy( int y );
		void gotody( int y );
		void gotox( int x, bool forceGoBehindEOL = false );
		void gotodx( int x );
		void applyGoto( YZViewCursor* viewCursor, bool applyCursor = true );
		void initGoto( YZViewCursor* viewCursor );
		void updateCurLine( );

		bool m_paintAll;

		int stickyCol;

		QChar mFillChar;
		QChar lastChar;
		bool listChar;

		QChar m_lineFiller;
		QChar m_lineMarker;


		YZCursor* origPos;
		int lineDY;

		YZCursor* beginChanges;

		//cached value of tabstop option
		int tabstop;
		bool wrap;
		bool rightleft;

		// tabstop * spaceWidth
		int tablength;

		// tablength to wrap
		int areaModTab;

		// if true, do not check for cursor visibility
		bool adjust;

		YZSelectionPool * selectionPool;
		YZSelection* mPaintSelection;

		//Visual Mode stuff
		YZViewCursor* mVisualCursor;


		//which regs to store macros in
		QList<QChar> mRegs;
		int m_paintAutoCommit;
		YZViewCursor* keepCursor;

		//the current attribute being used by the GUI
		YzisAttribute * curAt;
		YZModePool* mModePool;

		/**
		 * options cache
		 */
		int opt_schema;
		bool opt_list;
		MapOption opt_listchars;
		YZFoldPool* mFoldPool;
		
		const YZViewId id;
};

#endif /*  YZ_VIEW_H */

