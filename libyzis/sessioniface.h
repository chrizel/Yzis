/* This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <mikmak@yzis.org>,
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

#ifndef YZ_SESSION_IFACE_H
#define YZ_SESSION_IFACE_H

#include "yzis.h"
#include "keys.h"
#include "mode.h"

class YView;
class YBuffer;
class QString;
class YKeySequence;

/** @brief The session interface that must be re-implemented by the GUI
  * frontend.
  *
  * The GUI should inherit YSession which
  * inherits from this class.
  *
  * All the methods prefixed by gui are methods called by libyzis, to ask the
  * GUI to perform a task or inform the GUI about something. Those are the
  * methods that should be implemented by each GUI.
  */
class YZIS_EXPORT YSessionIface
{
public:

    //-------------------------------------------------------
    // ----------------- GUI Prompts
    //-------------------------------------------------------

    /** @brief Display the specified error/information message
     */
    virtual void guiPopupMessage( const QString& message ) = 0;

    /** @brief Prompt a Yes/No question for the user and returns the answer.
     */
    virtual bool guiPromptYesNo(const QString& title, const QString& message) = 0;

    /** @brief Prompt a Yes/No/Cancel question for the user
           *
     * @return 0 (Yes), 1 (No), 2 (Cancel)
     */
    virtual int guiPromptYesNoCancel(const QString& title, const QString& message) = 0;


    //-------------------------------------------------------
    // ----------------- Gui Events
    //-------------------------------------------------------

    /** @brief Ask the GUI to quit the app.
            *
            * The GUI should return control to libyzis after this function so
            * that libyzis can close itself properly.
            *
            * In Qt, this is typically the place where you will call
            * qApp->exit().
            *
            * @return XXX dunno
      */
    virtual bool guiQuit(int errorCode = 0) = 0;


    //-------------------------------------------------------
    // ----------------- Clipboard
    //-------------------------------------------------------

    /** Set the text of the clipboard.
      */
    virtual void guiSetClipboardText( const QString& text, Clipboard::Mode mode ) = 0;


    //-------------------------------------------------------
    // ----------------- View and Buffer management
    //-------------------------------------------------------

    /** @brief Create new views on an existing buffer.
      */
    virtual YView * guiCreateView( YBuffer *buffer ) = 0;

    /** @brief Switch the current view.
     */
    virtual void guiChangeCurrentView( YView* ) = 0;

    /** @brief Ask the frontend to delete the view.
      *
      * The @arg view pointer is still valid but is no longer used
     * after that call. The frontend should call delete on its
     * view instance.
      */
    virtual void guiDeleteView ( YView *view ) = 0;

    /** @brief Inform the frontend of the buffer creation.
      *
      */
    virtual void guiCreateBuffer( YBuffer * b ) { Q_UNUSED(b); }

    /** @brief Inform the frontend that the given buffer is being removed.
           *
           * The buffer @arg b is still valid when the call is made but is not
     * used afterward. 
     */
    virtual void guiRemoveBuffer( YBuffer *b ) { Q_UNUSED(b); }


    //-------------------------------------------------------
    // ----------------- Splitting
    //-------------------------------------------------------

    /** @brief Splits horizontally the mainwindow area to create a new view on the current buffer
     */
    virtual void guiSplitHorizontally ( YView* ) = 0;

    /** @brief Splits the screen vertically showing the 2 given views
     */ 
    //  virtual void splitHorizontallyWithViews( YView*, YView* ) = 0;

    /** @brief Splits the screen vertically to show the 2 given views
     */ 
    //  virtual void splitVerticallyOnView( YView*, YView* ) = 0;


    //-------------------------------------------------------
    // ----------------- Misc
    //-------------------------------------------------------

    /** Virtual destructor */
    virtual ~YSessionIface();
};

#endif // YZ_SESSION_IFACE_H


