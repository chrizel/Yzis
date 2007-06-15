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

/* Yzis */
#include "NoGuiSession.h"
#include "NoGuiView.h"
#include "debug.h"

/* Qt */
#include <QCoreApplication>


#define dbg() yzDebug("NoGuiSession")

void NoGuiSession::createInstance()
{
    dbg() << HERE();
	// such allocation (i.e. not "new QYZisSession") will ensure that
	// "instance" object will be properly and automatically deleted 
	// when program exits
    
	static NoGuiSession instance;
	setInstance(&instance);
}

NoGuiSession::NoGuiSession()
: YZSession()
{
    dbg() << HERE() << endl;
}

void NoGuiSession::frontendGuiReady()
{
    dbg() << "frontendGuiReady()" << endl;
    YZSession::self()->frontendGuiReady();
}

YZView* NoGuiSession::createView ( YZBuffer* buf) {
    dbg() << "createView( " << buf->toString() << " ) " << endl;
    NoGuiView * view = new NoGuiView( buf, YZSession::self() );
    return view;
}

YZBuffer * NoGuiSession::createBuffer(const QString& path) 
{
    dbg() << "NoGuiSession::createBuffer( path=" << path << " ) " << endl;
    YZBuffer * buf = new YZBuffer();
    setCurrentView( createView( buf ) );
    buf->load( path );
    currentView()->refreshScreen();
    return buf;
}

void  NoGuiSession::guiPopupMessage( const QString& message) 
{
    dbg() << "NoGuiSession::guiPopupMessage: '" << message << "' \n";
    printf("popupMessage:\n");
    printf("============\n");
    printf("%s\n\n", message.toLatin1().constData() );
}

void  NoGuiSession::guiQuit(bool savePopup) 
{
    dbg() << "NoGuiSession::guiQuit(" << savePopup << ")" << endl;
    QCoreApplication::exit(0);
}

void  NoGuiSession::guiDeleteBuffer ( YZBuffer * b ) 
{
    dbg() << "guiDeleteBuffer( " << b->toString() << " )" << endl;
}
void  NoGuiSession::guiChangeCurrentView( YZView* v ) 
{
    // notification
    dbg() << "changeCurrentView( " << v->toString() << " )" << endl;
}
void  NoGuiSession::guiSetFocusCommandLine( ) 
{
    dbg() << "NoGuiSession::guiSetFocusCommandLine" << endl;
}

void  NoGuiSession::guiSetFocusMainWindow( ) 
{
    dbg() << "NoGuiSession::guiSetFocusMainWindow" << endl;
}

bool  NoGuiSession::guiQuit(int errorCode ) 
{
    dbg() << "guiQuit( errorCode=" << errorCode << " ) " << endl;
    QCoreApplication::exit(0);
    return true;
}

bool  NoGuiSession::guiPromptYesNo(const QString&, const QString&) 
{
    dbg() << "NoGuiSession::guiPromptYesNo" << endl;
    return true;
}

int  NoGuiSession::guiPromptYesNoCancel( const QString&, const QString& ) 
{
    dbg() << "NoGuiSession::guiPromptYesNoCancel" << endl;
    return 0;
}

void  NoGuiSession::guiSplitHorizontally(YZView*) 
{
    dbg() << "NoGuiSession::guiSplitHorizontally" << endl;
}

YZView * NoGuiSession::guiCreateView(YZBuffer*b) 
{ 
    dbg().sprintf("guiCreateView( %s )", qp(b->toString() ) );
    return new NoGuiView(b,YZSession::self()); 
}

void  NoGuiSession::guiDeleteView(YZView*v) 
{ 
    dbg().sprintf("guiDeleteView( %s )", qp(v->toString() ) );
    delete v; 
} 

YZBuffer*  NoGuiSession::guiCreateBuffer() 
{ 
    dbg().sprintf("doCreateBuffer()");
    return new YZBuffer(); 
}

void  NoGuiSession::guiSetClipboardText(const QString& text, Clipboard::Mode) 
{
    dbg().sprintf("guiSetClipboardText( text='%s' )", qp(text) );
}

