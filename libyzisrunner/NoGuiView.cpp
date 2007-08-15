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
*  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
*  Boston, MA 02110-1301, USA.
**/


#include "NoGuiView.h"
#include "debug.h"

#define dbg() yzDebug("NoGuiView")

NoGuiView::NoGuiView(YZBuffer *buf, YZSession *sess, int cols, int lines)
        : YZView(buf, sess, cols, lines)
{
    dbg().sprintf("NoGuiView( %s, cols=%d, lines=%d )", qp(buf->toString()), cols, lines );
}

uint NoGuiView::getCursorX()
{
    dbg() << HERE() << endl;
    return viewCursor().bufferX();
}

uint NoGuiView::getCursorY()
{
    dbg() << HERE() << endl;
    return viewCursor().bufferY();
}

uint NoGuiView::getCursorLine()
{
    dbg() << HERE() << endl;
    return viewCursor().bufferY();
}

uint NoGuiView::getCursorCol()
{
    dbg() << HERE() << endl;
    return viewCursor().bufferX();
}

// Reimplemented to please compilation

void NoGuiView::guiSetCommandLineText( const QString& text)
{
    // dbg() << "NoGuiView::guiSetCommandLineText( text='" << text << "') \n";
    mCommandLine = text;
}

void NoGuiView::guiSetFocusCommandLine()
{
    dbg() << "NoGuiView::guiSetFocusCommandLine" << endl;
}

void NoGuiView::guiSetFocusMainWindow()
{
    dbg() << "NoGuiView::guiSetFocusMainWindow" << endl;
}

QString NoGuiView::guiGetCommandLineText() const
{
    // dbg() << "NoGuiView::guiGetCommandLineText" << endl;
    return mCommandLine;
}

void NoGuiView::invalidateLine( unsigned int )
{
    dbg() << "NoGuiView::invalidateLine" << endl;
}

void NoGuiView::setStatusBar( const QString& )
{
    dbg() << "NoGuiView::setStatusBar" << endl;
}

void NoGuiView::updateCursor( unsigned int, unsigned int, unsigned int, const QString& )
{
    dbg() << "NoGuiView::updateCursor" << endl;
}

void NoGuiView::refreshScreen( )
{
    dbg() << "NoGuiView::refreshScreen" << endl;
}

void NoGuiView::guiSyncViewInfo( )
{
    dbg() << "NoGuiView::guiSyncViewInfo" << endl;
}

void NoGuiView::guiDisplayInfo( const QString& )
{
    dbg() << "NoGuiView::guiDisplayInfo" << endl;
}

void NoGuiView::modeChanged( )
{
    dbg() << "NoGuiView::modeChanged" << endl;
}

void NoGuiView::paintEvent( unsigned int /*curx*/, unsigned int /*cury*/, unsigned int /*curw*/, unsigned int /*curh*/ )
{
    dbg() << "NoGuiView::paintEvent" << endl;
}

void NoGuiView::scrollUp( int )
{
    dbg() << "NoGuiView::scrollUp" << endl;
}
void NoGuiView::scrollDown( int )
{
    dbg() << "NoGuiView::scrollDown" << endl;
}
void NoGuiView::registerModifierKeys(const QString& s)
{
    dbg() << "NoGuiView::registerModifierKeys(" << s << ")" << endl;
    return ;
}

void NoGuiView::paintEvent( const YZSelection& )
{
    dbg() << "NoGuiView::paintEvent( )" << endl;
}

void NoGuiView::guiScroll( int dx, int dy )
{
    dbg() << "NoGuiView::Scroll" << dx << dy << endl;
}

void NoGuiView::guiNotifyContentChanged( const YZSelection& )
{
    dbg() << "NoGuiView::guiNotifyContentChanged" << endl;
}

bool NoGuiView::guiPopupFileSaveAs()
{
    dbg() << HERE() << endl;
    return false;
}
void NoGuiView::guiFilenameChanged()
{
    dbg() << HERE() << endl;
}

void NoGuiView::guiHighlightingChanged()
{
    dbg() << HERE() << endl;
}
void NoGuiView::guiPreparePaintEvent(int, int)
{
    dbg() << HERE() << endl;
}
void NoGuiView::guiEndPaintEvent()
{
    dbg() << HERE() << endl;
}
void NoGuiView::guiDrawCell(QPoint, const YZDrawCell&, void*)
{
    dbg() << HERE() << endl;
}
void NoGuiView::guiDrawClearToEOL(QPoint, const QChar&)
{
    dbg() << HERE() << endl;
}
void NoGuiView::guiDrawSetMaxLineNumber(int)
{
    dbg() << HERE() << endl;
}
void NoGuiView::guiDrawSetLineNumber(int, int, int)
{
    dbg() << HERE() << endl;
}


