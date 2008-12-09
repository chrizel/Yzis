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
#include "statusbariface.h"

class YDrawCell;

/** \brief The view interface that must be re-implemented by the GUI
  * frontend.
  *
  * The GUI should inherit from YView, not directly from this interface.
  *
  * All the methods prefixed by gui are methods called by libyzis, to ask the
  * GUI to perform a task or inform the GUI about something. Those are the
  * methods that should be implemented by each GUI.
  */
class YZIS_EXPORT YViewIface
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

    /** Tells the GUI to update the editor's cursor position.
     * Status bar feedback is handled by YView.
     */
    virtual void guiUpdateCursorPosition() = 0;

    /** Called when buffer's filename has changed.
     * Basic feedback is already handled by YView,
     * so implementation is optional in the GUI.
     */
    virtual void guiUpdateFileName()
    {}

    /** Called when current mode has changed.
     * The GUI must update the editor's cursor shape.
     * Status bar feedback is handled by YView.
     */
    virtual void guiUpdateMode() = 0;

    /** Called when buffer status information has changed.
     * Basic feedback is already handled by YView,
     * so implementation is optional in the GUI.
     */
    virtual void guiUpdateFileInfo()
    {}

    /** Called when an informational message has been sent to the user.
     * Basic feedback is already handled by YView,
     * so implementation is optional in the GUI.
     */
    virtual void guiDisplayInfo(const QString&)    {}

    /**
     * Notify GUIs that HL changed
     */
    virtual void guiHighlightingChanged(void	) {} ;

    /** Notify GUIs that the selected text has changed (clipboard).
      *
      * The GUI may want to re-implement this method to catch selection
      * change events.
      */
    virtual void guiSelectionChanged()    {}

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

    /** Focus on the command line
     */
    virtual void guiSetFocusCommandLine() = 0;

    /** Focus on the editing area
     */
    virtual void guiSetFocusMainWindow() = 0;

    //-------------------------------------------------------
    //                  GUI Status Bar
    //-------------------------------------------------------
    /** Returns the GUI status bar.
     * It is used by libyzis to export information about
     * the buffer and the view to the GUI.
     */
    virtual YStatusBarIface* guiStatusBar() = 0;

    //-------------------------------------------------------
    //                  GUI Painting
    //-------------------------------------------------------
	
	/** setup user interface using options */
	virtual void guiSetup() = 0;

    /** Notify gui that the following content has changed.
      *
      * The GUI should repaint the area that has changed.
      *
      * XXX This needs more doc.
      */
    virtual void guiNotifyContentChanged( const YSelection& s ) = 0;

    /** Inform GUI that a paint event is going to arrive. */
    virtual void guiPreparePaintEvent() = 0;

    /** Inform GUI that the paint event is finished. */
    virtual void guiEndPaintEvent() = 0;

    /** XXX to be written */
    virtual void guiDrawCell( YCursor pos, const YDrawCell& cell ) = 0;

    /** XXX to be written */
    virtual void guiDrawClearToEOL( YCursor pos, const YDrawCell& clearCell ) = 0;

    /** XXX to be written */
    virtual void guiDrawSetMaxLineNumber( int max ) = 0;

    /*!
     * This method is called by libyzis to change the line
     * number displayed.
     * @arg y the y-coordinate within the view where to display the line number
     * @arg n the actual number to display
     * @arg h line height, starting at 0
     */
    virtual void guiDrawSetLineNumber( int y, int n, int h ) = 0;

    //-------------------------------------------------------
    // ----------------- Misc
    //-------------------------------------------------------

    /** Virtual destructor */
    virtual ~YViewIface();

};


#endif // YZ_VIEW_IFACE


