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

#include "cursor.h"

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

		void deleteLine( YZView* pView, const YZCursor& pos, unsigned int len, const QValueList<QChar> &reg );
		void deleteLine( YZView* pView, unsigned int Y, unsigned int len , const QValueList<QChar> &reg );
		void deleteArea( YZView* pView, const YZCursor& begin, const YZCursor& end, const QValueList<QChar> &reg );

		void copyLine( YZView* pView, const YZCursor& pos, unsigned int len, const QValueList<QChar> &reg );
		void copyArea( YZView* pView, const YZCursor& begin,const YZCursor& end, const QValueList<QChar> &reg );

		void replaceLine( YZView* pView, const YZCursor& pos, const QString &text );
		void replaceLine( YZView* pView, unsigned int Y, const QString &text );

		YZCursor match( YZView* pView, YZCursor& mCursor, bool *found );
		YZCursor search( YZView* pView, const QString& what, const YZCursor& mBegin, const YZCursor& mEnd, bool reverseSearch, unsigned int *matchlength, bool *found );

	private:
		YZBuffer* mBuffer;
		YZCursor* mPos;

};

#endif

