#ifndef YZ_VIEW_H
#define YZ_VIEW_H
/**
 * $Id$
 */


#include "yz_buffer.h"
#include "yz_cursor.h"
#include "gui.h"

class YZCursor;
class YZBuffer;
class YZSession;

class YZView {
	public:
		/**
		 * constructor. Each view is bound to a buffer, @param lines is the initial
		 * number of lines that this view can display
		 */
		YZView(YZBuffer *_b, YZSession *sess, int _lines_vis);
		~YZView();

		/**
		 * Updates the number of visible lines
		 */
		void setVisibleLines (int);

		/**
		 * transfer a key event from gui to core
		 */
		void	sendKey( int , int);

		/** 
		 * returns the number of the line displayed on top of this view (refering to
		 * the whole file/buffer
		 */
		unsigned int	getCurrent(void) { return current; }

		/**
		 * returns the number of line this view can display
		 */
		unsigned int	getLinesVisible(void) { return lines_vis; }

		/**
		 * return true or false according to if the given line is
		 * visible or not
		 */
		int	isLineVisible(unsigned int l) { return ( (l>=current) && ((l-current)<lines_vis) ); }

		YZBuffer *myBuffer() { return buffer;}

		YZSession *mySession() { return session; }

		/**
		 * Register a GUI event manager
		 */
		//	void registerManager ( Gui *mgr );

		void centerView( unsigned int line );

		void purgeInputBuffer() { previous_chars="";}

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

		void redrawScreen();

		void	gotoxy(int nextx, int nexty);

		int myId; // the ID of this view

	protected:
		void updateCursor(void);

		YZBuffer 	*buffer; 	/** buffer we use */
		unsigned int	lines_vis;	/** number of visible lines */
		unsigned int	current;	/** current line on top of view */
		YZCursor *cursor;
		int		current_maxx;	/** maximum value for x, that s (lenght of current line -1), -1 means the line is empty */

		enum {
			YZ_VIEW_MODE_INSERT, // insert
			YZ_VIEW_MODE_REPLACE, // replace
			YZ_VIEW_MODE_COMMAND, // normal
			YZ_VIEW_MODE_EX //script 
		}		mode;		/** mode of this view */


		/*
		 * mode handling (on a per-view basis, no?)
		 * most of command handling and drawing stuff...
		 * yzview should update the core as to how many lines it can display (changegeometry stuff)
		 */

		/** Used to store previous keystrokes which are not recognised as a command,
		 * this should allow us to have commands like : 100g or gg etc ...
		 */
		QString previous_chars;

	private:
		/**
		 * The current session, provided by the GUI
		 */
		YZSession *session;

		//internal counters to identify a view by a number
		static int view_ids;

};

#endif /*  YZ_VIEW_H */

