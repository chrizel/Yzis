/*  This file is part of the Yzis libraries
*  Copyright (C) 2007 Lothar Braun <lothar@lobraun.de>
*  Copyright (C) 2003-2005 Mickael Marchand <mikmak@yzis.org>,
*  Copyright (C) 2003-2004 Lucijan Busch <lucijan@kde.org>
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

#include "kysession.h"
#include "kyview.h"

#include <kdebug.h>
#include <kmessagebox.h>

#include <libyzis/debug.h>
#include <libyzis/buffer.h>


KYSession::KYSession()
{}

KYSession::~KYSession()
{
}

KYSession* KYSession::me = NULL;

void KYSession::createInstance()
{
    if (!me) {
        me = new KYSession();
        setInstance(me);
    }
}

YView* KYSession::guiCreateView(YBuffer* buffer)
{
    yzDebug() << "doCreateView( " << buffer->toString() << ")" << endl;
    KYView* view = new KYView(buffer, 0);
    YASSERT_MSG(view, "KYSession::createView : failed creating a new KYView");
    return view;
}

YBuffer* KYSession::guiCreateBuffer()
{
    return new YBuffer();
}

void KYSession::guiDeleteBuffer( YBuffer* b )
{
    delete b;
}

void KYSession::guiSplitHorizontally(YView*)
{
}

bool KYSession::guiQuit( int )
{
    return true;
}

void KYSession::guiPopupMessage(const QString& message)
{
    KYView* v = static_cast< KYView* >( currentView() );
    KMessageBox::information( v, message );
}

bool KYSession::guiPromptYesNo(const QString& title, const QString& message)
{
    int v = KMessageBox::questionYesNo( static_cast< KYView* >( currentView() ), message, title );
    if ( v == KMessageBox::Yes )
        return true;
    else
        return false;
}

int KYSession::guiPromptYesNoCancel(const QString& title, const QString& message)
{
    int v = KMessageBox::questionYesNoCancel( static_cast< KYView* >( currentView() ), message, title );
    if ( v == KMessageBox::Yes )
        return 0;
    else if ( v == KMessageBox::No )
        return 1;

    return 2;
}

void KYSession::guiSetFocusCommandLine()
{
    KYView *v = static_cast<KYView*>( currentView() );
    v->guiSetFocusCommandLine();
}

void KYSession::guiSetFocusMainWindow()
{
    KYView *v = static_cast<KYView*>( currentView() );
    v->guiSetFocusMainWindow();
}

void KYSession::guiSetClipboardText(const QString&, Clipboard::Mode)
{
}

void KYSession::guiDeleteView(YView* view)
{
    KYView* v = static_cast< KYView* >( view );
    v->close();
}

void KYSession::guiChangeCurrentView( YView* view)
{
    KYView* kyzisview = static_cast< KYView* >( view );
    kyzisview->activateWindow();
    kyzisview->setFocus();
}

