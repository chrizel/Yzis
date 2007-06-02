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

#include <libyzis/debug.h>
#include <libyzis/buffer.h>


KYZisSession* KYZisSession::me = NULL;

KYZisSession::KYZisSession()
{
}

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
	KYZisView* view = new KYZisView(buffer);
	YZASSERT_MSG(view, "KYZisSession::createView : failed creating a new KYZisView");
	return view;
}

YZBuffer* KYZisSession::guiCreateBuffer()
{
	return new YZBuffer();
}
