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
#include "buffer.h"
#include "cursor.h"
#include "attribute.h"
#include "line.h"
#include "selection.h"

class YZCursor;
class YZBuffer;
class YZSession;
class YZSelectionPool;

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
		 * Updates the number of visible @arg c columns and @arg l lines
		 */
		void setVisibleArea (int c, int l, bool refresh = true );

		/**
		 * transfer a key event from GUI to core
		 * @arg c is the key value as in QKeyEvent::key()
		 * @arg modifiers is ored Qt::ShiftButton, Qt::ControlButton, Qt::AltButton 
		 * as in QKeyEvent::stat()
		 */
		void sendKey(int c, int modifiers);

		/**
		 * Translate Qt key/modifiers names into a full QString
		 * @param key the received key
		 * @param modifiers the received modifiers ( CTRL, SHIFT, ALT )
		 * @return a QString like <CTRL>key
		 */
		QString buildCommand( const QString& key, int modifiers );

		/** 
		 * Returns the index of the first line displayed on the view
		 */
		unsigned int getCurrentTop() { return mCurrentTop; }
		unsigned int getDrawCurrentTop() { return dCurrentTop; }

		/** 
		 * Returns the index of the first column displayed on the view
		 */
		unsigned int getCurrentLeft() { return mCurrentLeft; }
		unsigned int getDrawCurrentLeft() { return dCurrentLeft; }

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
		int	isLineVisible(unsigned int l) { return ( ( l >= dCurrentTop ) && ( l < mLinesVis + dCurrentTop ) ); }

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
		 * Center view vertically on the given @arg line
		 */
		void centerViewVertically( unsigned int line );

		/**
		 * Center view horizontally on the given @arg column
		 */
		void centerViewHorizontally( unsigned int column );

		/**
		 * align view vertically on the given @arg line
		 */
		void alignViewVertically(unsigned int line);

		/**
		 * Clean out the current buffer of inputs
		 * Typically used after a command is recognized or when ESC is pressed
		 */
		void purgeInputBuffer() { mPreviousChars = ""; }

		/**
		 * moves the cursor of the current view down
		 */
		QString moveDown( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * moves the cursor of the current view up 
		 */
		QString moveUp( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * moves the cursor of the current view left
		 */
		QString moveLeft( const QString& inputsBuff = QString::null , YZCommandArgs args = YZCommandArgs());

		/**
		 * moves the cursor of the current view right
		 */
		QString moveRight( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * moves the cursor of the current view to the first non-blank character 
		 * of the current line
		 */
		QString moveToFirstNonBlankOfLine( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * moves the cursor of the current view to the start of the current line
		 */
		QString moveToStartOfLine( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * moves the cursor of the current view to the end of the current line
		 */
		QString moveToEndOfLine( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/*
		 * ACTIONS
		 */

		/* insert text */
		void initInsertChar( const YZCursor& pos, unsigned int len, bool applyCursor );
		void applyInsertChar( const YZCursor& pos, unsigned int len, bool applyCursor );

		/* delete text */
		void initDeleteChar( const YZCursor& pos, unsigned int len, bool applyCursor );
		void applyDeleteChar( const YZCursor& pos, unsigned int len, bool applyCursor );

		/* replace text */
		void initReplaceChar( const YZCursor& pos, unsigned int len, bool applyCursor );
		void applyReplaceChar( const YZCursor& pos, unsigned int len, bool applyCursor );

		/* insert line */
		void initInsertLine( const YZCursor& pos, bool applyCursor );
		void applyInsertLine( const YZCursor& pos, bool applyCursor );

		/* delete line */
		void initDeleteLine( const YZCursor& pos, unsigned int len, bool applyCursor );
		void applyDeleteLine( const YZCursor& pos, unsigned int len, bool applyCursor );
		void initDeleteLine( const YZCursor& pos, const YZCursor& end, bool applyCursor );
		void applyDeleteLine( const YZCursor& pos, const YZCursor& end, bool applyCursor );

		/**
		 * deletes the character under the cursor
		 */
		QString deleteCharacter( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * Start command mode
		 */
		QString gotoCommandMode( );

		/**
		 * Start command mode
		 */
		QString gotoSearchMode( const QString& inputsBuff = QString::null, YZCommandArgs args  = YZCommandArgs() );

		/**
		 * Start insert mode
		 */
		QString gotoInsertMode( const QString& inputsBuff = QString::null, YZCommandArgs args  = YZCommandArgs());

		/**
		 * Start Ex mode
		 */
		QString gotoExMode( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * Start Open mode
		 */
		QString gotoOpenMode( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );


		/**
		 * Start replace mode
		 */
		QString gotoReplaceMode( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * Start visual mode
		 */
		QString gotoVisualMode( const QString& inputsBuff = QString::null, YZCommandArgs args  = YZCommandArgs());

		/**
		 * Go back to either open mode or command mode, depending on how the
		 * previous mode is set.
		 */
		QString gotoPreviousMode();

		/**
		 * Go to line of file
		 */
		QString gotoLine( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		QString changeLine ( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );
		/**
		 * Deletes lines
		 */
		QString deleteLine ( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * add a mark
		 */
		QString addMark( const QString& inputsBuff, YZCommandArgs args = YZCommandArgs() );


		/**
		 * Opens a new line after current line
		 */
		QString openNewLineAfter ( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * Opens a new line before current line
		 */
		QString openNewLineBefore ( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * Append after current character
		 */
		QString append ( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * Append at End of Line
		 */
		QString appendAtEOL ( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * Join current and next line
		 */
		QString joinLine ( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );
	
		void joinLine( unsigned int line );

		QString undo ( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );
		QString redo ( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );
		QString match( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );
		QString gotoMark( const QString&, YZCommandArgs );

		virtual void printToFile( const QString& path );

		/**
		 * Moves the draw cursor to @arg nextx, @arg nexty
		 */
		void gotodxdy(unsigned int nextx, unsigned int nexty, bool applyCursor = true );

		/**
		 * Moves the cursor to @arg buffer nextx, @arg draw nexty
		 */
		void gotoxdy(unsigned int nextx, unsigned int nexty, bool applyCursor = true );

		/**
		 * Moves the cursor to @arg draw nextx, @arg buffer nexty
		 */
		void gotodxy(unsigned int nextx, unsigned int nexty, bool applyCursor = true );

		/**
		 * Moves the buffer cursor to @arg nextx, @arg nexty
		 */
		void gotoxy(unsigned int nextx, unsigned int nexty, bool applyCursor = true );

		/**
		 * Copy from current to buffer to a register
		 */
		QString copy( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );
  
		/**
		 * Pastes the content of default or given register
		 */
		QString paste( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );
  
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
			YZ_VIEW_MODE_VISUAL, // visual mode
		} mMode,		/** mode of this view */
			mPrevMode;	/** previous mode of this view */
#define	YZ_VIEW_MODE_LAST (YZ_VIEW_MODE_VISUAL+1) // <-- update that if you touch the enum
		

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
		 * Internal use
		 */
		QString refreshScreenInternal(const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs());

		/**
		 * Displays an informational message
		 */
		virtual void displayInfo( const QString& info ) = 0;

		/**
		 * Display informational status about the current file and cursor
		 */
		virtual void syncViewInfo() = 0;

		/**
		 * Get the current cursor information
		 * @return a reference on the current cursor
		 */
		YZCursor* getCursor() { return dCursor; }

		/**
		 * Get the current buffer cursor information
		 * @return a reference on the current buffer cursor
		 */
		YZCursor* getBufferCursor() { return mCursor; }

		/**
		 * Search for text and moves the cursor to the position of match
		 * @param search a regexp to look for
		 * @return true if a match is found
		 */
		bool doSearch( const QString& search );

		/**
		 * Continue previous search
		 */
		QString searchAgain( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * Updates the position of the cursor
		 */
		void updateCursor();

		QString tr( const char *source, const char* = 0) { return qApp->translate( "YZView", source ); }

		/**
		 * init r and s Cursor
		 */
		void initDraw( );
		void initDraw( unsigned int sLeft, unsigned int sTop, 
					unsigned int rLeft, unsigned int rTop );

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
		 * return current buffer line
		 */
		unsigned int drawLineNumber( );

		/**
		 * total height ( draw )
		 */
		 unsigned int drawTotalHeight();
		
		bool drawSelected();

		/**
		 * Search and replace
		 */
		void substitute(const QString& range, const QString& search, const QString& replace, const QString& option);

		virtual void scrollUp( int ) = 0;
		virtual void scrollDown( int ) = 0;

		/* recalculate cursor position and refresh screen */
		void reset( );

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

	protected:
		/**
		 * The buffer we depend on
		 */
		YZBuffer *mBuffer;

		/**
		 * The cursor of the buffer
		 */
		YZCursor *mCursor;

		/**
		 * The cursor of the view
		 */
		YZCursor *dCursor;

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


	private:
		/**
		 * The current session, provided by the GUI
		 */
		YZSession *mSession;

		/* cursor members */
		unsigned int dColLength;
		unsigned int dLineLength;
		unsigned int dLineHeight;
		unsigned int mColLength;
		unsigned int mLineLength;
		unsigned int dSpaceFill;

		/* draw members */
		unsigned int rColLength;
		unsigned int rLineLength;
		unsigned int rLineHeight;
		unsigned int sColLength;
		unsigned int sLineLength;
		unsigned int rSpaceFill;
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
		unsigned int mCurrentTop;

		/**
		 * Index of the first visible line (buffer)
		 */
		unsigned int mCurrentLeft;

		/**
		 * Index of the first visible column (draw)
		 */
		unsigned int dCurrentLeft;

		/**
		 * Index of the first visible line (draw)
		 */
		unsigned int dCurrentTop;

		/**
		 * The cursor of the text
		 */
		YZCursor *sCursor;

		/**
		 * The cursor of the draw
		 */
		YZCursor *rCursor;
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

//		QColor rDrawColor;

		YzisAttribute *rHLAttributes;

		QString  sCurLine;
		unsigned int sCurLineLength;
		unsigned int rCurLineLength;

		void gotoy( unsigned int );
		void gotody( unsigned int );
		void gotox( unsigned int );
		void gotodx( unsigned int );
		void applyGoto( bool applyCursor = true );
		void initGoto( );
		void updateCurLine( );

		int stickyCol;

		bool wrapNextLine;
		bool dWrapNextLine;
		QChar lastChar;
		bool charSelected;

		YZCursor* origPos;
		unsigned int lineDY;
		void initChanges( const YZCursor& pos );
		void applyChanges( const YZCursor& pos, unsigned int len, bool applyCursor );

		//cached value of tabwidth option
		unsigned int tabwidth;	
		bool wrap;

		YZSelectionPool * selectionPool;
		
		//Visual Mode stuff
		YZCursor *mVisualCursor;
		YZCursor *dVisualCursor;
};

#endif /*  YZ_VIEW_H */

