/**
 * $Id$
 */

#ifndef GUI_H
#define GUI_H

#include "yz_events.h"

/**
 * This is the interface that all GUIs must implement
 * Once you created an instance of it after initializing your GUI
 * you will register it into the lib.
 * Then the lib will be able to talk with the GUIs using this itf :)
 * IMPORTANT:
 * Register your GUI the earlier you can in your constructor
 */ 
class YZSession;
class QString;

class Gui {
	public:

		/**
		 * libyzis posts events to the GUI
		 */
		virtual void postEvent(yz_event) = 0;

		/**
		 * scroll down the given number of lines
		 */
		virtual void scrollDown(int l = 1) = 0;

		/**
		 * scroll up the given number of lines
		 */
		virtual void scrollUp(int l = 1) = 0;

		/**
		 * clear the screen
		 */
//		virtual void clearScreen() = 0;

		/**
		 * setGeometry for one view ?
		 */

		/**
		 * create View ?
		 */

		/**
		 * show scrollbar left,right,top,bottom for view
		 */
	//	virtual void setScrollbar( int left, int right, int top, int bottom );


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
		 * Ask to quit the app
		 */
		virtual void quit(bool savePopup=true) = 0;

};

#endif // GUI_H

