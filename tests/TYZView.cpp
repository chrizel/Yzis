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

TYZView::TYZView(YZBuffer *buf, YZSession *sess, int lines)
: YZView( buf, sess, lines )
{
	mKeyMap.append( Mapping("<esc>",	Qt::Key_Escape) );
	mKeyMap.append( Mapping("<left>", 	Qt::Key_Left) );
	mKeyMap.append( Mapping("<right>", 	Qt::Key_Right) );
	mKeyMap.append( Mapping("<up>", 	Qt::Key_Up) );
	mKeyMap.append( Mapping("<down>", 	Qt::Key_Down) );
	mKeyMap.append( Mapping("\n", 		Qt::Key_Return) );
	mKeyMap.append( Mapping("\t", 		Qt::Key_Tab) );
}

void TYZView::sendText( QString text )
{
	int consumed = 0;
	while( text.length() ) {
		consumed = 0;

		for( QValueList<Mapping>::iterator it = mKeyMap.begin(); it != mKeyMap.end(); ++it  ) {
			if ((*it).text == text.left((*it).text.length()).lower()) {
				sendKey( (*it).key , 0 );
				consumed = (*it).text.length();
				break;			
			}
		}
		
		if ( consumed == 0) {
			QChar c = text[0];
			int modifier = 0;
			consumed = 1;
		
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
