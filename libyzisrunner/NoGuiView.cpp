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

#define dbg() yzDebug("NoGuiView")

NoGuiView::NoGuiView(YZBuffer *buf, YZSession *sess, int cols, int lines) 
    : YZView(buf,sess,cols,lines) 
{
    dbg() << HERE() << endl;
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

void NoGuiView::setCommandLineText( const QString& text) {
    dbg() << "NoGuiView::setCommandLineText( text='" << text << "') \n";
    mCommandLine = text;
}

void NoGuiView::setFocusCommandLine() {
    dbg() << "NoGuiView::setFocusCommandLine" << endl;
}

void NoGuiView::setFocusMainWindow() {
    dbg() << "NoGuiView::setFocusMainWindow" << endl;
}

QString NoGuiView::getCommandLineText() const {
    // dbg() << "NoGuiView::getCommandLineText" << endl;
    return mCommandLine;
}

void NoGuiView::invalidateLine( unsigned int ) {
    dbg() << "NoGuiView::invalidateLine" << endl;
}

void NoGuiView::setStatusBar( const QString& ) {
    dbg() << "NoGuiView::setStatusBar" << endl;
}

void NoGuiView::updateCursor( unsigned int, unsigned int, unsigned int, const QString& ) {
    dbg() << "NoGuiView::updateCursor" << endl;
}

void NoGuiView::refreshScreen( ) {
    dbg() << "NoGuiView::refreshScreen" << endl;
}

void NoGuiView::syncViewInfo( ) {
    dbg() << "NoGuiView::syncViewInfo" << endl;
}

void NoGuiView::displayInfo( const QString& ) {
    dbg() << "NoGuiView::displayInfo" << endl;
}

void NoGuiView::modeChanged( ) {
    dbg() << "NoGuiView::modeChanged" << endl;
}

void NoGuiView::paintEvent( unsigned int /*curx*/, unsigned int /*cury*/, unsigned int /*curw*/, unsigned int /*curh*/ ) {
    dbg() << "NoGuiView::paintEvent" << endl;
}

void NoGuiView::scrollUp( int ) {
    dbg() << "NoGuiView::scrollUp" << endl;
}
void NoGuiView::scrollDown( int ) {
    dbg() << "NoGuiView::scrollDown" << endl;
}
void NoGuiView::registerModifierKeys(const QString& s) {
    dbg() << "NoGuiView::registerModifierKeys(" << s << ")" << endl;
    return;
}

void NoGuiView::paintEvent( const YZSelection& ) {
    dbg() << "NoGuiView::paintEvent( )" << endl;
}

void NoGuiView::Scroll( int dx, int dy ) {
    dbg() << "NoGuiView::Scroll" << dx << dy << endl;
}

void NoGuiView::notifyContentChanged( const YZSelection& s ) {
    dbg() << "NoGuiView::notifyContentChanged" << endl;
}

bool NoGuiView::popupFileSaveAs() 
{ 
    dbg() << HERE() << endl;
    return false; 
}
void NoGuiView::filenameChanged() 
{
    dbg() << HERE() << endl;
}

void NoGuiView::highlightingChanged() 
{
    dbg() << HERE() << endl;
}
void NoGuiView::preparePaintEvent(int, int) 
{
    dbg() << HERE() << endl;
}
void NoGuiView::endPaintEvent() 
{
    dbg() << HERE() << endl;
}
void NoGuiView::drawCell(int, int, const YZDrawCell&, void*) 
{
    dbg() << HERE() << endl;
}
void NoGuiView::drawClearToEOL(int, int, const QChar&) 
{
    dbg() << HERE() << endl;
}
void NoGuiView::drawSetMaxLineNumber(int) 
{
    dbg() << HERE() << endl;
}
void NoGuiView::drawSetLineNumber(int, int, int) 
{
    dbg() << HERE() << endl;
}


