/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>
 *  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
 *  Copyright (C) 2007 Philippe Fremy <phil@freehackers.org>
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

#ifndef YZ_VIEW_IFACE
#define YZ_VIEW_IFACE

// Qt
#include <QPoint>

// Project
#include "yzismacros.h"
#include "selection.h"
#include "option.h"
#include "drawbuffer.h"

/** \brief The view interface that must be re-implemented by the GUI
  * frontend.
  *
  * The GUI should inherit from YZView, not directly from this interface.
  *
  * All the methods prefixed by gui are methods called by libyzis, to ask the
  * GUI to perform a task or inform the GUI about something. Those are the
  * methods that should be implemented by each GUI.
  */
class YZIS_EXPORT YZViewIface 
{
    public:

		//-------------------------------------------------------
		//                GUI Notifications
		//-------------------------------------------------------

		/**
		 * Ask the GUI to popup for a filename
		 * @return whether a file name was successfully chosen
		 */
		virtual bool guiPopupFileSaveAs() = 0;

		/**
		 * Called whenever the filename is changed
		 */
		virtual void guiFilenameChanged() = 0;

		/**
		 * Notify GUIs that HL changed
		 */
		virtual void guiHighlightingChanged() = 0;

        /** Notify GUIs that the selected text has changed (clipboard).
          *
          * The GUI may want to re-implement this method to catch selection
          * change events.
          */
		virtual void guiSelectionChanged() {}

		/**
		 * scroll dx to the right and dy downward
		 */
		virtual void guiScroll( int dx, int dy ) = 0;


		//-------------------------------------------------------
		//                  GUI Command Line
		//-------------------------------------------------------
		/**
		 * Retrieve the text from command line
		 */
		virtual QString guiGetCommandLineText() const = 0;

		/**
		 * Sets the command line text
		 */
		virtual void guiSetCommandLineText( const QString& ) = 0;

		//-------------------------------------------------------
		//                  GUI Status Bar 
		//-------------------------------------------------------
		/**
		 * Displays an informational message
		 */
		virtual void guiDisplayInfo( const QString& info ) = 0;

		/**
		 * Display informational status about the current file and cursor
		 */
		virtual void guiSyncViewInfo() = 0;

		//-------------------------------------------------------
		//                  GUI Painting
		//-------------------------------------------------------

        /** Notify gui that the following content has changed.
          *
          * The GUI should repaint the area that has changed.
          *
          * XXX This needs more doc.
          */
		virtual void guiNotifyContentChanged( const YZSelection& s ) = 0;

        /** Inform GUI that a paint event is going to arrive. */
		virtual void guiPreparePaintEvent( int y_min, int y_max ) = 0;

        /** Inform GUI that the paint event is finished. */
		virtual void guiEndPaintEvent() = 0;

        /** XXX to be written */
		virtual void guiDrawCell( QPoint pos, const YZDrawCell& cell, void* arg ) = 0;

        /** XXX to be written */
		virtual void guiDrawClearToEOL( QPoint pos, const QChar& clearChar ) = 0;

        /** XXX to be written */
		virtual void guiDrawSetMaxLineNumber( int max ) = 0;

		/*!
		 * This method is called by libyzis to change the line
		 * number displayed.
		 * @arg y the y-coordinate within the view where to display the line number
		 * @arg n the actual number to display
		 * @arg h ??? it seems to be some kind of boolean..
		 */
		virtual void guiDrawSetLineNumber( int y, int n, int h ) = 0;

		//-------------------------------------------------------
		// ----------------- Misc
		//-------------------------------------------------------

        /** Virtual destructor */
        virtual ~YZViewIface();

};


#endif // YZ_VIEW_IFACE


