/* This file is part of the Yzis libraries
 *  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
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

#include "buffer.h"
#include "cursor.h"
#include "commands.h"

class YZCursor;
class YZBuffer;
class YZSession;

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
		void setVisibleArea (int c, int l);

		/**
		 * transfer a key event from GUI to core
		 * @arg c is the key value as in QKeyEvent::key()
		 * @arg modifiers is ored Qt::ShiftButton, Qt::ControlButton, Qt::AltButton 
		 * as in QKeyEvent::stat()
		 */
		void sendKey(int c, int modifiers);

		/** 
		 * Returns the index of the first line displayed on the view
		 */
		unsigned int getCurrentTop() { return mCurrentTop; }

		/** 
		 * Returns the index of the first column displayed on the view
		 */
		unsigned int getCurrentLeft() { return mCurrentLeft; }

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
		int	isLineVisible(unsigned int l) { return ( (l>=mCurrentTop) && ((l-mCurrentTop)<mLinesVis) ); }

		/**
		 * Returns true if the column @arg c is visible. False otherwise
		 */
		int	isColumnVisible(unsigned int c) { return ( (c>=mCurrentLeft) && ((c-mCurrentLeft)<mColumnsVis) ); }

		/**
		 * Returm my current buffer
		 */
		YZBuffer *myBuffer() { return mBuffer; }

		/**
		 * Return my current session
		 */
		YZSession *mySession() { return mSession; }

		/**
		 * Center view on the given @arg column and @arg line
		 */
		void centerView( unsigned int column, unsigned int line );

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
		 * Start replace mode
		 */
		QString gotoReplaceMode( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * Go to line of file
		 */
		QString gotoLine( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

		/**
		 * Deletes lines
		 */
		QString deleteLine ( const QString& inputsBuff = QString::null, YZCommandArgs args = YZCommandArgs() );

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
		 * Repaint the whole visible screen
		 */
		void redrawScreen();

		/**
		 * Moves the cursor to @arg nextx, @arg nexty
		 */
		void gotoxy(unsigned int nextx, unsigned int nexty);

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

		enum {
			YZ_VIEW_MODE_INSERT, // insert
			YZ_VIEW_MODE_REPLACE, // replace
			YZ_VIEW_MODE_COMMAND, // normal
			YZ_VIEW_MODE_EX, //script 
			YZ_VIEW_MODE_SEARCH //search mode
		} mMode;		/** mode of this view */
		

		//GUI
		/**
		 * Retrieve the text from command line
		 */
		virtual QString getCommandLineText() const = 0;

		/**
		 * Sets the command line text
		 */
		virtual void setCommandLineText( const QString& ) = 0;

		/**
		 * Focus on the command line
		 */
		virtual void setFocusCommandLine() = 0;

		/**
		 * Focus on the main window
		 */
		virtual void setFocusMainWindow() = 0;

		/**
		 * Inform a view that a line was changed
		 * @param line the line which was edited
		 */
		virtual void invalidateLine( unsigned int line ) = 0;

		/**
		 * Set status bar information text
		 * @param text the status information to display
		 */
		virtual void setStatusBar( const QString& text ) = 0;

		/**
		 * Updates cursor position and information text
		 * @param line Y position
		 * @param x1 real X position
		 * @param x2 fake X position ( counting tab stops )
		 * @param percentage informational percentage position in text
		 */
		virtual void updateCursor ( unsigned int line, unsigned int x1, unsigned int x2, const QString& percentage) = 0;

		/**
		 * Asks a redraw of the whole view
		 */
		virtual void refreshScreen () = 0;
					
		/**
		 * Displays an informational message
		 */
		virtual void displayInfo( const QString& info ) = 0;

		/**
		 * Display informational status about the current file
		 */
		virtual void setInformation( const QString& info ) = 0;

		/**
		 * Get the current cursor information
		 * @return a reference on the current cursor
		 */
		YZCursor* getCursor() { return mCursor; }

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

		QString tr( const char *source, const char* comment = 0) { return qApp->translate( "YZView", source ); }

	protected:
		/**
		 * The buffer we depend on
		 */
		YZBuffer *mBuffer;

		/**
		 * Number of visible lines on the view
		 */
		unsigned int mLinesVis;

		/**
		 * Number of visible columns on the view
		 */
		unsigned int mColumnsVis;

		/**
		 * Index of the first visible line
		 */
		unsigned int mCurrentTop;

		/**
		 * Index of the first visible line
		 */
		unsigned int mCurrentLeft;

		/**
		 * The cursor of the view
		 */
		YZCursor *mCursor;

		/**
		 * The maximal X position of the current line
		 */
		unsigned int mMaxX;

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

	private:
		/**
		 * The current session, provided by the GUI
		 */
		YZSession *mSession;

		/**
		 * Searching backward
		 */
		bool reverseSearch;
};

#endif /*  YZ_VIEW_H */

