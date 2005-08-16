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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef YZ_VIEW_H
#define YZ_VIEW_H
/**
 * $Id$
 */

#include "selection.h"
#include "mode.h"
#include "option.h"
#include "viewid.h"

#include <qglobal.h>
#include <qvaluevector.h>
#include <qapplication.h>

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

/**
 * MUST be reimplemented in the GUI. It's the basis to display the content of a buffer
 * One view is the display of some part of a buffer, it is used to receive inputs and displays
 * corresponding outputs
 */
class YZView {

	public:
		//-------------------------------------------------------
		// ----------------- Constructor/Destructor and ID
		//-------------------------------------------------------
		/**
		 * Each view is bound to a buffer, @arg lines is the initial
		 * number of lines that this view can display
		 */
		YZView(YZBuffer *_b, YZSession *sess, int lines);
		
		virtual ~YZView();

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
		unsigned int getCurrentTop();
		unsigned int getDrawCurrentTop();

		/**
		 * Returns the index of the first column displayed on the view
		 */
		unsigned int getCurrentLeft();
		unsigned int getDrawCurrentLeft();

		/**
		 * returns the number of line this view can display
		 */
		unsigned int getLinesVisible() { return mLinesVis; }

		/**
		 * returns the number of line this view can display
		 */
		unsigned int getColumnsVisible() { return mColumnsVis; }

		/**
		 * Returns true if the line @arg l is visible. False otherwise
		 */
		bool	isLineVisible(unsigned int l);

		/**
		 * Returns true if the column @arg c is visible for @arg line ( expanding TABs ). False otherwise
		 */
		bool	isColumnVisible(unsigned int column, unsigned int line);

		//-------------------------------------------------------
		// ----------------- Associated Objects
		//-------------------------------------------------------
		/**
		 * Return my current buffer
		 */
		YZBuffer *myBuffer() { return mBuffer; }
		const YZBuffer *myBuffer() const { return mBuffer; }

		/**
		 * Return my current session
		 */
		YZSession *mySession() { return mSession; }

		/**
		 * Return my current line search
		 */
		YZLineSearch* myLineSearch() { return mLineSearch; }
		
		YZModePool* modePool() { return mModePool; }
		YZSelectionPool* getSelectionPool() const { return selectionPool; }
		
		const QValueList<QChar> registersRecorded() { return mRegs; }
		
		YZSelectionMap visualSelection();

		//-------------------------------------------------------
		// ----------------- Scrolling
		//-------------------------------------------------------
		/**
		 * Adjust view vertically to show @arg line on bottom
		 */
		void bottomViewVertically( unsigned int line );

		/**
		 * Center view vertically on the given @arg line
		 */
		void centerViewVertically( unsigned int line );

		/**
		 * Center view horizontally on the given @arg column
		 */
		void centerViewHorizontally( unsigned int column );

		/**
		 * align view vertically on the given buffer @arg line
		 */
		void alignViewBufferVertically(unsigned int line);
		
		/**
		 * align view vertically on the given screen @arg line
		 */
		void alignViewVertically(unsigned int line);

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
		QString moveDown( unsigned int nb_lines = 1, bool applyCursor = true );
		QString moveDown( YZViewCursor* viewCursor, unsigned int nb_lines = 1, bool applyCursor = true );

		/**
		 * moves the cursor of the current view up
		 */
		QString moveUp( unsigned int nb_lines = 1, bool applyCursor = true );
		QString moveUp( YZViewCursor* viewCursor,  unsigned int nb_lines = 1, bool applyCursor = true );

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
		void gotodxdy(unsigned int nextx, unsigned int nexty, bool applyCursor = true );
		void gotodxdy( YZViewCursor* viewCursor,unsigned int nextx, unsigned int nexty, bool applyCursor = true );

		/**
		 * Moves the cursor to @arg buffer nextx, @arg draw nexty
		 */
		void gotoxdy(unsigned int nextx, unsigned int nexty, bool applyCursor = true );
		void gotoxdy( YZViewCursor* viewCursor,unsigned int nextx, unsigned int nexty, bool applyCursor = true );

		/**
		 * Moves the cursor to @arg draw nextx, @arg buffer nexty
		 */
		void gotodxy(unsigned int nextx, unsigned int nexty, bool applyCursor = true );
		void gotodxy( YZViewCursor* viewCursor,unsigned int nextx, unsigned int nexty, bool applyCursor = true );

		/**
		 * Moves the buffer cursor to @arg nextx, @arg nexty
		 */
		void gotoxy(unsigned int nextx, unsigned int nexty, bool applyCursor = true );
		void gotoxy( YZViewCursor* viewCursor, unsigned int nextx, unsigned int nexty, bool applyCursor = true );

		/**
		 * Moves the buffer cursor to @arg cursor and stick the column
		 */
		void gotoxyAndStick( YZCursor* cursor );
		void gotoxyAndStick( unsigned int x, unsigned int y );

		/**
		 * Go to line of file
		 */
		void gotoLine( unsigned int line );
		void gotoLine( YZViewCursor* viewCursor, unsigned int line, bool applyCursor = true );

		/**
		 * Go to last line of the file
		 */
		void gotoLastLine();
		void gotoLastLine( YZViewCursor* viewCursor, bool applyCursor = true );

		/**
		 * move the cursor to the sticky column
		 */
		void gotoStickyCol( unsigned int Y );
		void gotoStickyCol( YZViewCursor* viewCursor, unsigned int Y, bool applyCursor = true );

		//-------------------------------------------------------
		// ----------------- Drawing
		//-------------------------------------------------------
		/**
		 * init r and s Cursor
		 */
		void initDraw( );
		void initDraw( unsigned int sLeft, unsigned int sTop, unsigned int rLeft, unsigned int rTop, bool draw = true );

		unsigned int initDrawContents( unsigned int clipy );

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

		/**
		 * draw char
		 */
		const QChar& drawChar( );

		/**
		 * char length
		 */
		unsigned int drawLength( );

		/**
		 * line increment (on screen)
		 */
		unsigned int drawHeight( );

		/**
		 * char color
		 */
		const QColor& drawColor( );

		/**
		 * char color if selected
		 */
		const QColor& drawSelColor( );

		/**
		 * char background color
		 */
		const QColor& drawBgColor( );

		/**
		 * char background color if selected
		 */
		const QColor& drawBgSelColor( );

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
		const QColor& drawOutline();

		/**
		 * Character color at column line
		 */
		const QColor& drawColor ( unsigned int col, unsigned int line );

		/**
		 * return current buffer line
		 */
		unsigned int drawLineNumber( );

		/**
		 * total height ( draw )
		 */
		unsigned int drawTotalHeight();

		bool drawSelected();

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
		void undo ( unsigned int count = 1 );

		/**
		 * Redo last undoed action
		 */
		void redo ( unsigned int count = 1 );

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
		void recordMacro( const QValueList<QChar> &regs );

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
		void paste( QChar registr, bool after = true );

		/**
		 * Reindent given line ( cindent )
		 */
		void reindent( unsigned int X, unsigned int Y );

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
		QString getLocalOptionKey();

		YZOptionValue* getLocalOption( const QString& option );
		
		/**
		 * Retrieve an int option
		 */
		int getLocalIntegerOption( const QString& option );

		/**
		 * Retrieve a bool option
		 */
		bool getLocalBooleanOption( const QString& option );

		/**
		 * Retrieve a string option
		 */
		QString getLocalStringOption( const QString& option );

		/**
		 * Retrieve a qstringlist option
		 */
		QStringList getLocalListOption( const QString& option );

		/**
		 * Retrieve a map option
		 */
		MapOption getLocalMapOption( const QString& option );

		//-------------------------------------------------------
		// ----------------- Paint Events
		//-------------------------------------------------------
		virtual void paintEvent( const YZSelection& drawMap ) = 0;

		void sendPaintEvent( const YZCursor& from, const YZCursor& to );
		void sendPaintEvent( unsigned int curx, unsigned int cury, unsigned int curw, unsigned int curh );
		void sendPaintEvent( YZSelectionMap map, bool isBufferMap = true );

		/**
		 * ask to draw from buffer line @arg line to @arg line + @arg n
		 */
		void sendBufferPaintEvent( unsigned int line, unsigned int n );

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
		void refreshScreen();

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
		YZCursor* getCursor();

		/**
		 * Get the current buffer cursor information
		 * @return a reference on the current buffer cursor
		 */
		YZCursor* getBufferCursor();

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
		void setStickyCol( unsigned int col ) { stickyCol = col; }

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
		unsigned int lineIncrement( );

		/**
		 * current line height (on screen)
		 */
		unsigned int lineHeight( );

		/**
		 * width of a space ( in pixel or in cols )
		 */
		unsigned int getSpaceWidth() const;

		/**
		 * Returns pixel width of given string str, must be implemented in ui.
		 * Used only with non-Fixed fonts
		 */
		virtual unsigned int stringWidth( const QString& str ) const = 0;

		/**
		 * Returns pixel width of given char ch, must be implemented in ui.
		 * Used only with non-Fixed fonts
		 */
		virtual unsigned int charWidth( const QChar& ch ) const = 0;

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
		void initChanges( unsigned int x, unsigned int y );
		void applyChanges( unsigned int x, unsigned int y );

		//-------------------------------------------------------
		// ----------------- Miscellaneous
		//-------------------------------------------------------
		/**
		 * Display Intro text message 
		 */
		void displayIntro();

		virtual void printToFile( const QString& path );

		const QChar& fillChar() const;

		QString getCharBelow( int delta );

	public slots :
		void sendMultipleKey( const QString& keys );

	protected:

		void setupKeys();

		bool stringHasOnlySpaces ( const QString& what );
		
		QString getLineStatusString() const;
		
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
		unsigned int mLinesVis;

		/**
		 * Number of visible columns on the view
		 */
		unsigned int mColumnsVis;

		/**
		 * Index of the first visible line (buffer)
		 */
		unsigned int sCurrentTop;

		/**
		 * Index of the first visible line (buffer)
		 */
		unsigned int sCurrentLeft;

		/**
		 * Index of the first visible column (draw)
		 */
		unsigned int rCurrentLeft;

		/**
		 * Index of the first visible line (draw)
		 */
		unsigned int rCurrentTop;

		unsigned int spaceWidth;
		
		const uchar* rHLa;

		bool rHLnoAttribs;

		unsigned int rHLAttributesLen;

		YzisAttribute *rHLAttributes;

		// current line
		QString  sCurLine;
		// current line length
		unsigned int sCurLineLength;
		// current line max width ( tab is 8 spaces )
		unsigned int rCurLineLength;
		// current line min width( tab is 1 space )
		unsigned int rMinCurLineLength;

		void gotoy( unsigned int );
		void gotody( unsigned int );
		void gotox( unsigned int, bool forceGoBehindEOL = false );
		void gotodx( unsigned int );
		void applyGoto( YZViewCursor* viewCursor, bool applyCursor = true );
		void initGoto( YZViewCursor* viewCursor );
		void updateCurLine( );

		int stickyCol;

		QChar mFillChar;
		QChar lastChar;
		bool charSelected;
		bool listChar;

		YZCursor* origPos;
		unsigned int lineDY;

		YZCursor* beginChanges;

		//cached value of tabstop option
		unsigned int tabstop;
		bool wrap;
		bool rightleft;

		// tabstop * spaceWidth
		unsigned int tablength;

		// tablength to wrap
		unsigned int areaModTab;

		// if true, do not check for cursor visibility
		bool adjust;

		YZSelectionPool * selectionPool;
		YZSelection* mPaintSelection;

		//Visual Mode stuff
		YZViewCursor* mVisualCursor;


		//which regs to store macros in
		QValueList<QChar> mRegs;
		unsigned int m_paintAutoCommit;
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
		
		const YZViewId id;
};

#endif /*  YZ_VIEW_H */

