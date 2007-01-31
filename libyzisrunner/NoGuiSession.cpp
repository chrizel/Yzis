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

#include <QCoreApplication>

#include "NoGuiSession.h"

#define dbg() yzDebug("NoGuiSession")

NoGuiSession::NoGuiSession( const QString & sessionName )
: YZSession( sessionName )
{
    dbg() << HERE() << endl;
}

YZView* NoGuiSession::createView ( YZBuffer* buf) {
    dbg() << HERE() << endl;
    NoGuiView * view = new NoGuiView( buf, YZSession::self() );
    buf->addView( view );
    return view;
}

YZBuffer * NoGuiSession::createBuffer(const QString& path) 
{
    dbg() << "NoGuiSession::createBuffer " << path << endl;
    YZBuffer * buf = new YZBuffer();
    setCurrentView( createView( buf ) );
    buf->load( path );
    currentView()->refreshScreen();
    return buf;
}

void  NoGuiSession::popupMessage( const QString& message) 
{
    dbg() << "NoGuiSession::popupMessage: '" << message << "' \n";
    printf("popupMessage:\n");
    printf("============\n");
    printf("%s\n\n", message.toLatin1().constData() );
}

void  NoGuiSession::quit(bool savePopup) 
{
    dbg() << "NoGuiSession::quit(" <<savePopup << ")" << endl;
    QCoreApplication::exit(0);
}

void  NoGuiSession::deleteView ( int  ) 
{
    dbg() << "NoGuiSession::deleteView" << endl;
}

void  NoGuiSession::deleteBuffer ( YZBuffer * ) 
{
    dbg() << "NoGuiSession::deleteBuffer" << endl;
}
void  NoGuiSession::changeCurrentView( YZView* ) 
{
    // notification
    dbg() << "NoGuiSession::changeCurrentView" << endl;
}
void  NoGuiSession::setFocusCommandLine( ) 
{
    dbg() << "NoGuiSession::setFocusCommandLine" << endl;
}

void  NoGuiSession::setFocusMainWindow( ) 
{
    dbg() << "NoGuiSession::setFocusMainWindow" << endl;
}

bool  NoGuiSession::quit(int) 
{
    dbg() << "NoGuiSession::quit" << endl;
    QCoreApplication::exit(0);
    return true;
}

bool  NoGuiSession::promptYesNo(const QString&, const QString&) 
{
    dbg() << "NoGuiSession::promptYesNo" << endl;
    return true;
}

int  NoGuiSession::promptYesNoCancel( const QString&, const QString& ) 
{
    dbg() << "NoGuiSession::promptYesNoCancel" << endl;
    return 0;
}

void  NoGuiSession::splitHorizontally(YZView*) 
{
    dbg() << "NoGuiSession::splitHorizontally" << endl;
}

YZView * NoGuiSession::doCreateView(YZBuffer*b) 
{ 
    dbg() << HERE() << endl;
    return new NoGuiView(b,YZSession::self()); 
}

void  NoGuiSession::doDeleteView(YZView*v) 
{ 
    dbg() << HERE() << endl;
    delete v; 
} 

YZBuffer*  NoGuiSession::doCreateBuffer() 
{ 
    dbg() << HERE() << endl;
    return new YZBuffer(); 
}

void  NoGuiSession::setClipboardText(const QString& text, Clipboard::Mode) 
{
    dbg() << HERE() << "text='" << text << "'" << endl;
}

