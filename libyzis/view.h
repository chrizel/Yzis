#ifndef YZ_VIEW_H
#define YZ_VIEW_H
/**
 * $Id$
 */


#include "buffer.h"
#include "cursor.h"
#include "gui.h"

class YZCursor;
class YZBuffer;
class YZSession;

class YZView {
	public:
		/**
		 * Each view is bound to a buffer, @param lines is the initial
		 * number of lines that this view can display
		 */
		YZView(YZBuffer *_b, YZSession *sess, int lines);
		~YZView();

		/**
		 * Updates the number of visible lines
		 */
		void setVisibleLines (int);

		/**
		 * transfer a key event from GUI to core
		 */
		void sendKey(int , int);

		/** 
		 * Returns the index of the first line displayed on the view
		 */
		unsigned int getCurrent() { return mCurrentTop; }

		/**
		 * returns the number of line this view can display
		 */
		unsigned int getLinesVisible() { return mLinesVis; }

		/**
		 * Returns true if the line @param l is visible. False otherwise
		 */
		int	isLineVisible(unsigned int l) { return ( (l>=mCurrentTop) && ((l-mCurrentTop)<mLinesVis) ); }

		/**
		 * Returm my current buffer
		 */
		YZBuffer *myBuffer() { return mBuffer;}

		/**
		 * Return my current session
		 */
		YZSession *mySession() { return mSession; }

		/**
		 * Center view on the given @param line
		 */
		void centerView( unsigned int line );

		/**
		 * Clean out the current buffer of inputs
		 * Typically used after a command is recognized or when ESC is pressed
		 */
		void purgeInputBuffer() { mPreviousChars = ""; }

		/**
		 * moves the cursor of the current view down
		 */
		QString moveDown( const QString& inputsBuff = QString::null );

		/**
		 * moves the cursor of the current view up 
		 */
		QString moveUp( const QString& inputsBuff = QString::null );

		/**
		 * moves the cursor of the current view left
		 */
		QString moveLeft( const QString& inputsBuff = QString::null );

		/**
		 * moves the cursor of the current view right
		 */
		QString moveRight( const QString& inputsBuff = QString::null );

		/**
		 * moves the cursor of the current view to the start of the current line
		 */
		QString moveToStartOfLine( const QString& inputsBuff = QString::null );

		/**
		 * moves the cursor of the current view to the end of the current line
		 */
		QString moveToEndOfLine( const QString& inputsBuff = QString::null );

		/**
		 * deletes the character under the cursor
		 */
		QString deleteCharacter( const QString& inputsBuff = QString::null );

		/**
		 * Back to command mode
		 */
		QString gotoCommandMode( );

		/**
		 * Start insert mode
		 */
		QString gotoInsertMode( const QString& inputsBuff = QString::null );

		/**
		 * Start Ex mode
		 */
		QString gotoExMode( const QString& inputsBuff = QString::null );

		/**
		 * Start replace mode
		 */
		QString gotoReplaceMode( const QString& inputsBuff = QString::null );

		/**
		 * Go to line of file
		 */
		QString gotoLine( const QString& inputsBuff = QString::null );

		/**
		 * Deletes lines
		 */
		QString deleteLine ( const QString& inputsBuff = QString::null );

		/**
		 * Opens a new line after current line
		 */
		QString openNewLineAfter ( const QString& inputsBuff = QString::null );

		/**
		 * Opens a new line before current line
		 */
		QString openNewLineBefore ( const QString& inputsBuff = QString::null );

		/**
		 * Append after current character
		 */
		QString append ( const QString& inputsBuff = QString::null );

		/**
		 * Append at End of Line
		 */
		QString appendAtEOL ( const QString& inputsBuff = QString::null );

		/**
		 * Repaint the whole visible screen
		 */
		void redrawScreen();

		/**
		 * Moves the cursor to @param nextx , @param nexty
		 */
		void gotoxy(unsigned int nextx, unsigned int nexty);
  
		/**
		 * A global UID for this view
		 **/
		unsigned int myId;

	protected:
		void updateCursor();

		/**
		 * The buffer we depend on
		 */
		YZBuffer *mBuffer;

		/**
		 * Number of visible lines on the view
		 */
		unsigned int mLinesVis;

		/**
		 * Index of the first visible line
		 */
		unsigned int mCurrentTop;

		/**
		 * The cursor of the view
		 */
		YZCursor *mCursor;

		/**
		 * The maximal X position of the current line
		 */
		unsigned int mMaxX;

		enum {
			YZ_VIEW_MODE_INSERT, // insert
			YZ_VIEW_MODE_REPLACE, // replace
			YZ_VIEW_MODE_COMMAND, // normal
			YZ_VIEW_MODE_EX //script 
		} mMode;		/** mode of this view */

		/** 
		 * Used to store previous keystrokes which are not recognised as a command,
		 * this should allow us to have commands like : 100g or gg etc ...
		 */
		QString mPreviousChars;

	private:
		/**
		 * The current session, provided by the GUI
		 */
		YZSession *mSession;
};

#endif /*  YZ_VIEW_H */

