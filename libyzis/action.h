/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>
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
 * $Id$
 */

#ifndef YZ_ACTION_H
#define YZ_ACTION_H

#include <qstring.h>

#include "buffer.h"
#include "view.h"
#include "cursor.h"
#include "commands.h"

class YZBuffer;
class YZView;
class YZCursor;

class YZAction {

	public:
		YZAction( YZBuffer* buffer );
		virtual ~YZAction( );

		void insertChar( YZView* pView, const YZCursor& pos, const QString& text );
		void insertChar( YZView* pView, unsigned int X, unsigned int Y, const QString& text );

		void replaceChar( YZView* pView, const YZCursor& pos, const QString& text );
		void replaceChar( YZView* pView, unsigned int X, unsigned int Y, const QString& text );

		void deleteChar( YZView* pView, const YZCursor& pos, unsigned int len );
		void deleteChar( YZView* pView, unsigned int X, unsigned int Y, unsigned int len );

		void insertLine( YZView* pView, const YZCursor& pos, const QString &text );
		void insertLine( YZView* pView, unsigned int Y, const QString &text );

		void insertNewLine( YZView* pView, const YZCursor& pos );
		void insertNewLine( YZView* pView, unsigned int X, unsigned int Y );

		void deleteLine( YZView* pView, const YZCursor& pos, unsigned int len );
		void deleteLine( YZView* pView, unsigned int Y, unsigned int len );
		void deleteArea( YZView* pView, YZCursor& begin, YZCursor& end, const QChar& reg );

	private:
		YZBuffer* mBuffer;
		YZCursor* mPos;

};

#endif

