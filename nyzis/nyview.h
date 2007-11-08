/*
Copyright (c) 2003 Thomas Capricelli <orzel@freehackers.org>
Copyright (c) 2004-2005 Mickael Marchand <marchand@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of version 2 of the GNU General Public
   License as published by the Free Software Foundation

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef NYZ_VIEW_H
#define NYZ_VIEW_H 
/**
 * ncurses-based GUI for yzis
 */

/* Std */
// This define is needed on Macinotsh. Else the ncurses wide char API 
// is not exported
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif
#include <ncurses.h> 
// This is an ugly hack preventing the compiler to choke on scroll (used in
// both Qt API and (as a macro!) in ncurses
#define curses_scroll scroll
#undef scroll

/* Qt */
#include <QMap>
#include <QDataStream>
#include <QStringList>

/* Yzis */
#include "view.h"
#include "cursor.h"
#include "nystatusbar.h"

class NYSession;

/**
 * Implementation of YView for the NCurses frontend.
 */
class NYView : public YView
{

public:
    /**
      * constructor. Each view is binded to a buffer, @arg lines is the initial number of lines that
      * this view can display
      */
    NYView(YBuffer *b);
    virtual ~NYView();

    virtual QString guiGetCommandLineText(void) const
    {
        return commandline;
    }
    virtual void guiSetCommandLineText( const QString& );
    virtual void refreshScreen();
    virtual void guiPaintEvent( const YSelection& drawMap );

    void guiScroll( int dx, int dy );

    /**
      * Used when this view becomes viewable, that
      * is on front of others
      */
    void map( void );
    /**
      * This view is not the front one anymore, hide it
      */
    void unmap( void );
    virtual void registerModifierKeys( const QString& )
    { }
    virtual void unregisterModifierKeys( const QString& )
    { }

    bool guiPopupFileSaveAs();

    /** Returns the status bar */
    virtual YStatusBarIface* guiStatusBar();
    virtual void guiUpdateFileName();
    virtual void guiUpdateMode();
    virtual void guiUpdateFileInfo();
    virtual void guiUpdateCursor();
    virtual void guiDisplayInfo(const QString&);

    virtual void guiHighlightingChanged();


    virtual void guiSetFocusCommandLine();
    virtual void guiSetFocusMainWindow();
    void restoreFocus();

protected :

public slots:

protected :
    virtual void guiDrawCell( QPoint pos, const YDrawCell& cell, void* arg );

    virtual void guiNotifyContentChanged( const YSelection& s );

    void guiPreparePaintEvent(int, int);
    void guiEndPaintEvent();
    virtual void guiDrawClearToEOL( QPoint pos, const QChar& clearChar );
    virtual void guiDrawSetMaxLineNumber( int max );
    virtual void guiDrawSetLineNumber( int y, int n, int h );

    bool fakeLine; /* true if current line is a fake one (eg: ~) */


private:
    WINDOW *window; /* ncurses window to write to */
    /**
     * update visible area
     */
    void updateVis( bool refresh = true );

    /**
      * draw cursor
      */
    void drawCursor();

    /* layout */
    WINDOW *editor;
    NYStatusBar infobar;
    WINDOW *statusbar; // TODO: rename to commandbar

    enum e_focusable {
        w_editor,
        w_statusbar
    };

    bool statusbarHasCommand; // true if status bar contains a command

    /**
      * used to implement set/get CommandLine, as we have no
      * special widget for that
      */
    QString commandline;
    int marginLeft;
    int height;
    int width;

    void initialiseAttributesMap();
    static int attributesMapInitialised;
    /**
      * maps QRgb to ncurses attributes, as those used in
      * mAttributesMap[ QRgb ] is the # of the corresponding pair in ncurses
      */
    static QMap<QRgb, unsigned long int> mAttributesMap;

    e_focusable m_focus;
};

#endif // NYZ_VIEW_H



