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


#ifndef NOGUI_SESSION_H
#define NOGUI_SESSION_H

#include "NoGuiView.h"

#include "libyzis/session.h"
#include "libyzis/buffer.h"

class NoGuiSession : public YZSession
{
	public:
        static void createInstance();


		virtual YZView* createView ( YZBuffer* buf);

		virtual	YZBuffer *createBuffer(const QString& path=QString::null);

		virtual void popupMessage( const QString& message);

		virtual void quit(bool /*savePopup=true */);

		virtual void deleteView ( int  );
		virtual void deleteBuffer ( YZBuffer * );
		virtual void changeCurrentView( YZView* );
		virtual void setFocusCommandLine( );
		virtual void setFocusMainWindow( );
		virtual bool quit(int);
		virtual bool promptYesNo(const QString&, const QString&);
		virtual int promptYesNoCancel( const QString&, const QString& );
		virtual void splitHorizontally(YZView*);
		virtual YZView *doCreateView(YZBuffer*b);
		virtual void doDeleteView(YZView*v);
		virtual YZBuffer* doCreateBuffer();
		virtual void setClipboardText(const QString&, Clipboard::Mode);

    private:
		NoGuiSession( const QString & sessionName="NoGuiSession" );

};

#endif // NOGUI_YZ_SESSION_H
