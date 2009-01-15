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
    // such allocation (i.e. not "new QYisSession") will ensure that
    // "instance" object will be properly and automatically deleted
    // when program exits

    static NoGuiSession instance;
    setInstance(&instance);
}

NoGuiSession::NoGuiSession()
        : YSession()
{
    dbg() << HERE() << endl;
}

void NoGuiSession::frontendGuiReady()
{
    dbg() << "frontendGuiReady()" << endl;
    YSession::self()->frontendGuiReady();
}

void NoGuiSession::guiPopupMessage( const QString& message)
{
    dbg() << "NoGuiSession::guiPopupMessage: '" << message << "' \n";
    printf("popupMessage:\n");
    printf("============\n");
    printf("%s\n\n", message.toLatin1().constData() );
}

void NoGuiSession::guiQuit(bool savePopup)
{
    dbg() << "NoGuiSession::guiQuit(" << savePopup << ")" << endl;
    QCoreApplication::exit(0);
}

void NoGuiSession::guiChangeCurrentView( YView* v )
{
    // notification
    dbg() << "changeCurrentView( " << v->toString() << " )" << endl;
}
void NoGuiSession::guiSetFocusCommandLine( )
{
    dbg() << "NoGuiSession::guiSetFocusCommandLine" << endl;
}

void NoGuiSession::guiSetFocusMainWindow( )
{
    dbg() << "NoGuiSession::guiSetFocusMainWindow" << endl;
}

bool NoGuiSession::guiQuit(int errorCode )
{
    dbg() << "guiQuit( errorCode=" << errorCode << " ) " << endl;
    QCoreApplication::exit(0);
    return true;
}

bool NoGuiSession::guiPromptYesNo(const QString&, const QString&)
{
    dbg() << "NoGuiSession::guiPromptYesNo" << endl;
    return true;
}

int NoGuiSession::guiPromptYesNoCancel( const QString&, const QString& )
{
    dbg() << "NoGuiSession::guiPromptYesNoCancel" << endl;
    return 0;
}

void NoGuiSession::guiSplitHorizontally(YView*)
{
    dbg() << "NoGuiSession::guiSplitHorizontally" << endl;
}

YView * NoGuiSession::guiCreateView(YBuffer*b)
{
    dbg().SPrintf("guiCreateView( %s )", qp(b->toString() ) );
    return new NoGuiView(b, YSession::self());
}

void NoGuiSession::guiDeleteView(YView*v)
{
    dbg().SPrintf("guiDeleteView( %s )", qp(v->toString() ) );
    NoGuiView * ngv = static_cast<NoGuiView *>( v );
    delete ngv;
}

void NoGuiSession::guiSetClipboardText(const QString& text, Clipboard::Mode)
{
    dbg().SPrintf("guiSetClipboardText( text='%s' )", qp(text) );
}

#include "NoGuiSession.moc"

