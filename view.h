/* This file is part of the Yzis libraries
 *  Copyright (C) 2003, 2004 Mickael Marchand <marchand@kde.org>,
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

#ifndef YZ_VIEW_H
#define YZ_VIEW_H
/**
 * $Id$
 */

#include <qvaluevector.h>
#include <qapplication.h>

#include "commands.h"
#include "selection.h"
//#include "linesearch.h"

class YZViewCursor;
class YZCursor;
class YZBuffer;
class YZSession;
class YZSelectionPool;
class YzisAttribute;
class YZLineSearch;
class YZView;

typedef QValueVector<QString> StringVector;

/**
 * MUST be reimplemented in the GUI. It's the basis to display the content of a buffer
 * One view is the display of some part of a buffer, it is used to receive inputs and displays
 * corresponding outputs
 */
class YZView {

	public:
		/**
		 * Each view is bound to a buffer, @arg lines is the initial
		 * number of lines that this view can display
		 */
		YZView(YZBuffer *_b, YZSession *sess, int lines);
		virtual ~YZView();

		/**
		 * set font mode ( fixed or not )
		 * @arg fixed is true if used font is fixed
		 */
		void setFixedFont( bool fixed );

		/**
		 * Updates the number of visible @arg c columns and @arg l lines
		 * @arg c is the number of columns ( fixed fonts ) or width in pixel ( non-fixed fonts ) of the Area
		 * @arg l is the number of lines
		 * @arg resfresh if true, refreshView is called
		 */
		void setVisibleArea (int c, int l, bool refresh = true );
		/**
		 * transfer key events from GUI to core
		 */
		void sendKey(const QString& key, const QString& modifiers="");

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

		/**
		 * Returm my current buffer
		 */
		YZBuffer *myBuffer() { return mBuffer; }

		/**
		 * Return my current session
		 */
		YZSession *mySession() { return mSession; }

		/**
		 * Return my current line search
		 */
		 YZLineSearch* myLineSearch() { return mLineSearch; }

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

		/**
		 * Clean out the current buffer of inputs
		 * Typically used after a command is recognized or when ESC is pressed
		 */
		void purgeInputBuffer() { mPreviousChars = ""; mapMode = 0; }

		/**
		 * moves the cursor to x,y (buffer) and save sticky column )
		 */
		void moveXY( unsigned int x, unsigned int y );
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
		 * Start command mode
		 */
		QString gotoCommandMode( );

		/**
		 * Start command mode
		 */
		QString gotoSearchMode( bool reverse = false );

		/**
		 * Start insert mode
		 */
		QString gotoInsertMode();

		/**
		 * Start Ex mode
		 */
		QString gotoExMode();

		/**
		 * Start Open mode
		 */
		QString gotoOpenMode();

		/**
		 * Goto completion mode
		 */
		 QString gotoCompletionMode();

		/* Prepend enough spaces to string so line is "centered" */
		QString centerLine( QString );

		/* Display Intro text message */
		void displayIntro();

		/* Clear intro text message */
		void clearIntro();

		/**
		 * Start replace mode
		 */
		QString gotoReplaceMode();

		/**
		 * Start intro mode
		 */
		QString gotoIntroMode();

		/**
		 * Start visual mode
		 */
		QString gotoVisualMode( bool isVisualLine=false );

		/**
		 * Leave insert mode
		 */
		void leaveInsertMode( );

		/**
		 * Leave replace mode
		 */
		void leaveReplaceMode( );

		/**
		 * Leave visual mode
		 */
		void leaveVisualMode( );

		/**
		 * Leave completion mode
		 */
		void leaveCompletionMode( );

		/**
		 * Get the selected area
		 */
		YZSelectionMap getVisualSelection();

		/**
		 * Go back to either open mode or command mode, depending on how the
		 * previous mode is set.
		 */
		QString gotoPreviousMode();

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
		 * Append after current character
		 */
		QString append ( );

		/**
		 * Undo last action
		 */
		void undo ( unsigned int count = 1 );

		/**
		 * Redo last undoed action
		 */
		void redo ( unsigned int count = 1 );

		virtual void printToFile( const QString& path );

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
		 * Pastes the content of default or given register
		 */
		void paste( QChar registr, bool after = true );

		/**
		 * A global UID for this view
		 **/
		unsigned int myId;

		/**
		 * Get the current mode
		 */
		int getCurrentMode() { return mMode; }

		/**
		 * Get the previous mode
		 */
		int getPreviousMode() { return mPrevMode; }

		/**
		 * Switch to the given mode; if the mode is the same as
		 * the current one, do nothing.
		 */
		void switchModes(int mode);

		enum modeType {
			YZ_VIEW_MODE_INSERT=0, // insert
			YZ_VIEW_MODE_REPLACE, // replace
			YZ_VIEW_MODE_COMMAND, // normal
			YZ_VIEW_MODE_EX, //script
			YZ_VIEW_MODE_SEARCH, //search mode
			YZ_VIEW_MODE_OPEN, // open mode
			YZ_VIEW_MODE_INTRO, // Intro displayed 
			YZ_VIEW_MODE_COMPLETION, // completion mode (CTRL-X) 
			YZ_VIEW_MODE_VISUAL, // visual mode //keep these 2 at the end of the list
			YZ_VIEW_MODE_VISUAL_LINE, // visual mode
		} mMode,		/** mode of this view */
			mPrevMode;	/** previous mode of this view */
#define	YZ_VIEW_MODE_LAST (YZ_VIEW_MODE_VISUAL_LINE+1) // <-- update that if you touch the enum
		
		/**
		 * Mode value for key mappings
		 */
		int mapMode;

		/**
		 * Get the text describing the mode
		 */
		QString mode( int mode );

		//GUI
		/**
		 * Retrieve the text from command line
		 */
		virtual QString getCommandLineText() const = 0;

		/**
		 * Sets the command line text
		 */
		virtual void setCommandLineText( const QString& ) = 0;

		virtual void paintEvent( unsigned int curx, unsigned int cury, unsigned int curw, unsigned int curh ) = 0;

		void sendPaintEvent( const YZCursor& from, const YZCursor& to );
		void sendPaintEvent( unsigned int curx, unsigned int cury, unsigned int curw, unsigned int curh );
		void removePaintEvent( const YZCursor& from, const YZCursor& to );
		void setPaintAutoCommit( bool enable = true );
		void abortPaintEvent();
		void commitPaintEvent();

		/**
		  * called when the mode is changed, so that gui can
		  * update information diplayed to the user
		  */
		virtual void modeChanged() = 0;

		/**
		 * Asks a redraw of the whole view
		 */
		virtual void refreshScreen() = 0;

		/**
		 * recalcScreen refresh the screen and recalculate cursor position
		 */
		void recalcScreen();

		/**
		 * Internal use
		 */
		QString refreshScreenInternal();

		/**
		 * Displays an informational message
		 */
		virtual void displayInfo( const QString& info ) = 0;

		/**
		 * Display informational status about the current file and cursor
		 */
		virtual void syncViewInfo() = 0;

		/**
		 * Get the view cursor
		 * @return a constant ref to the view cursor ( YZViewCursor )
		 */
		const YZViewCursor &viewCursor() { return *mainCursor; }

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

		/**
		 * Search for text and moves the cursor to the position of match
		 * @param search a regexp to look for
		 * @return true if a match is found
		 */
		bool doSearch( const QString& search, const YZCursor& begin, const YZCursor& end, bool moveToMatch=true, YZCursor *result=NULL );

		/**
		 * Continue previous search
		 */
		QString searchAgain( unsigned int count = 1, bool inverse = false );

		/**
		 * Updates the position of the cursor
		 */
		void updateCursor();

		QString tr( const char *source, const char* = 0) { return qApp->translate( "YZView", source ); }

		/**
		 * init r and s Cursor
		 */
		void initDraw( );
		void initDraw( unsigned int sLeft, unsigned int sTop, unsigned int rLeft, unsigned int rTop, bool draw = true );

		/**
		 * go to previous line
		 */
		bool drawPrevLine( );

		/**
		 * go to next line
		 */
		bool drawNextLine( );

		/*
		 * go to prev col
		 */
		bool drawPrevCol( );

		/*
		 * go to next col
		 */
		bool drawNextCol( );

		const QChar& fillChar() const;

		/**
		 * draw char
		 */
		const QChar& drawChar( );

		/**
		 * char length
		 */
		unsigned int drawLength( );

		/**
		 * line height
		 */
		unsigned int drawHeight( );


		/**
		 * line height
		 */
		 unsigned int lineHeight( );

		/**
		 * char color
		 */
		const QColor& drawColor( );

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

		virtual void scrollUp( int ) = 0;
		virtual void scrollDown( int ) = 0;

		//Local Options management
		/**
		 * Retrieve an int option
		 */
		int getLocalIntOption( const QString& option );

		/**
		 * sets an int option
		 */
		void setLocalIntOption( const QString& key, int option );

		/**
		 * Retrieve a bool option
		 */
		bool getLocalBoolOption( const QString& option );

		/**
		 * sets a bool option
		 */
		void setLocalBoolOption( const QString& key, bool option );

		/**
		 * Retrieve a string option
		 */
		QString getLocalStringOption( const QString& option );

		/**
		 * sets a qstring option
		 */
		void setLocalQStringOption( const QString& key, const QString& option );

		/**
		 * Retrieve a qstringlist option
		 */
		QStringList getLocalStringListOption( const QString& option );

		/**
		 * sets a qstringlist option
		 */
		void setLocalQStringListOption( const QString& key, const QStringList& option );

		/**
		 * Retrieve a qcolor option
		 */
		QColor getLocalColorOption( const QString& option );

		/**
		 * sets a qcolor option
		 */
		void setLocalQColorOption( const QString& key, const QColor& option );

		/**
		 * width of a space ( in pixel or in cols )
		 */
		unsigned int spaceWidth;

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

		/**
		 * Reindent given line ( cindent )
		 */
		void reindent( unsigned int X, unsigned int Y );

		/**
		 * Create new indented line
		 */
		void indent();

		/**
		 * move the cursor to the sticky column
		 */
		void gotoStickyCol( unsigned int Y );
		void gotoStickyCol( YZViewCursor* viewCursor, unsigned int Y, bool applyCursor = true );

		/**
		 * Updates stickyCol
		 */
		void setStickyCol( unsigned int col ) { stickyCol = col; }

		void updateStickyCol( );
		/**
		 * update stickCol to according to viewCursor
		 */
		void updateStickyCol( YZViewCursor* viewCursor );

		/**
		 * Prepares to record next undo item
		 */
		void commitNextUndo();

		/* inform view for init changes starting at x,y */
		void initChanges( unsigned int x, unsigned int y );
		/* inform view for apply changes ending at x,y */
		void applyChanges( unsigned int x, unsigned int y );

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

		const QValueList<QChar> registersRecorded() { return mRegs; }

		YZSelectionMap visualSelection();

		/**
		 * Starts a completion in direction forward
		 */
		void initCompletion();
		
		/**
		 * Find next completion match
		 * @param forward selects the direction
		 */
		QString doComplete(bool forward);

	public slots :
		void sendMultipleKey( const QString& keys );

	protected:

		void setupKeys();
		virtual void registerModifierKeys( const QString& keys ) = 0;

		bool stringHasOnlySpaces ( const QString& what );
		
		/**
		 * The buffer we depend on
		 */
		YZBuffer *mBuffer;

		/**
		 * Used to store previous keystrokes which are not recognised as a command,
		 * this should allow us to have commands like : 100g or gg etc ...
		 */
		QString mPreviousChars;

		/**
		 * command history
		 */
		StringVector mExHistory;

		/**
		 * search history
		 */
		StringVector mSearchHistory;

		/**
		 * current command history item
		 */
		unsigned int mCurrentExItem;

		/**
		 * current search history item
		 */
		unsigned int mCurrentSearchItem;

		struct {
			int l;  //buffer line
			int c1; //buffer column
			int c2; //buffer column as drawn
			QString percentage;
		} viewInformation;

		/**
		 * Searching backward
		 */
		bool reverseSearch;

		/**
		 * is font used fixed ?
		 */
		bool isFontFixed;

		/**
		 * This is the main cursor, the one which is displayed
		 */
		YZViewCursor* mainCursor;

		/* screen top-left cursor */
		YZViewCursor* scrollCursor;

	private:

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

		// tabstop * spaceWidth
		unsigned int tablength;

		// tablength to wrap
		unsigned int areaModTab;

		// if true, do not check for cursor visibility
		bool adjust;

		YZSelectionPool * selectionPool;

		//Visual Mode stuff
		YZCursor *mVisualCursor;
		YZCursor *dVisualCursor;

		//search mode cursors
		YZCursor *mSearchBegin;

		//which regs to store macros in
		QValueList<QChar> mRegs;
		QStringList mModes; //list of modes

		unsigned int m_paintAutoCommit;

		YZCursor *m_completionStart;
		YZCursor *m_completionCursor;
		QString m_word2Complete;
		QString m_lastMatch;
		QStringList m_oldProposals;
};

#endif /*  YZ_VIEW_H */

