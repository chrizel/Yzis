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

/**
 * $Id$
 */

#ifndef GUI_H
#define GUI_H

#include "events.h"

class YZSession;
class YZBuffer;
class YZView;
class QString;

/**
 * This is the interface that all GUIs must implement
 * Once you created an instance of it after initializing your GUI
 * you will register it into the lib.
 * Then the lib will be able to talk with the GUIs using this itf :)
 * IMPORTANT:
 * Register your GUI the earlier you can in your constructor
 */ 
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

		/**
		 * Switch views
		 */
		virtual void setCurrentView ( YZView* ) = 0;

		/**
		 * Create a new view
		 */
		virtual YZView* createView ( YZBuffer* ) = 0;
		
		/**
		 * Creates a new buffer
		 */
		virtual	YZBuffer *createBuffer(const QString& path=QString::null) = 0;

		/**
		 * Display the specified error/information message
		 */
		virtual void popupMessage( const QString& message ) = 0;
};

#endif // GUI_H

