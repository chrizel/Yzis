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

#include "kyzissession.h"
#include "kyzisview.h"

#include <kdebug.h>
#include <kmessagebox.h>

#include <libyzis/debug.h>
#include <libyzis/buffer.h>


KYZisSession::KYZisSession()
{
}

KYZisSession::~KYZisSession()
{

}

KYZisSession* KYZisSession::me = NULL;

void KYZisSession::createInstance()
{
	if (!me) {
		me = new KYZisSession();
		setInstance(me);
	}
}

YZView* KYZisSession::guiCreateView(YZBuffer* buffer)
{
	yzDebug() << "doCreateView( " << buffer->toString() << ")" << endl;
	KYZisView* view = new KYZisView(buffer, 0);
	YZASSERT_MSG(view, "KYZisSession::createView : failed creating a new KYZisView");
	return view;
}

YZBuffer* KYZisSession::guiCreateBuffer()
{
	return new YZBuffer();
}

void KYZisSession::guiDeleteBuffer( YZBuffer* b )
{
	delete b;
}

void KYZisSession::guiSplitHorizontally(YZView*) 
{

}

bool KYZisSession::guiQuit( int ) 
{ 
	return true;
}

void KYZisSession::guiPopupMessage(const QString& message)
{
	KYZisView* v = static_cast< KYZisView* >( currentView() );
	KMessageBox::information( v, message );
}

bool KYZisSession::guiPromptYesNo(const QString& title, const QString& message) 
{
	int v = KMessageBox::questionYesNo( static_cast< KYZisView* >( currentView() ), message, title );
	if ( v == KMessageBox::Yes )
		return true;
	else 
		return false;
}

int KYZisSession::guiPromptYesNoCancel(const QString& title, const QString& message) 
{
	int v = KMessageBox::questionYesNoCancel( static_cast< KYZisView* >( currentView() ), message, title );
        if ( v == KMessageBox::Yes )
		return 0;
        else if ( v == KMessageBox::No )
		return 1;

        return 2;
}

void KYZisSession::guiSetFocusCommandLine() 
{
	KYZisView *v = static_cast<KYZisView*>( currentView() );
	v->setFocusCommandLine();
}

void KYZisSession::guiSetFocusMainWindow() 
{
	KYZisView *v = static_cast<KYZisView*>( currentView() );
	v->setFocusMainWindow();
}

void KYZisSession::guiSetClipboardText(const QString&, Clipboard::Mode) 
{

}

void KYZisSession::guiDeleteView(YZView* view) 
{
	KYZisView* v = static_cast< KYZisView* >( view );
	v->close();
}

void KYZisSession::guiChangeCurrentView( YZView* view) 
{
	KYZisView* kyzisview = static_cast< KYZisView* >( view );
	kyzisview->activateWindow();
	kyzisview->setFocus();
}

