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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/


/**
 * $Id: session.h 388 2004-03-03 23:44:13Z mikmak $
 */

#include "TYZView.h"

#include <ctype.h>

void TYZView::sendText( QString text )
{
	int consumed = 0;
	QChar c;
	int modifier;
	while( text.length() ) {
		modifier = 0;
		c = text[0];
		consumed = 1;
		if (c == '\n') {
			sendKey( Qt::Key_Return, 0 );
		} else if (c == '\t') {
			sendKey( Qt::Key_Tab, 0 );
		} else if (text.left(5) == "<Esc>") {
			sendKey( Qt::Key_Escape, 0 );
			consumed = 5;
		} else {
			if (c.upper() == c) {
				modifier |= Qt::ShiftButton;
				c = c.lower();
			}
			sendKey( c, modifier );
		}

		YZASSERT_MSG( consumed != 0, QString("TYZView::sendText() - nothing done with text '%1'").arg(text) );
		text.remove( 0, consumed );
	}
}
