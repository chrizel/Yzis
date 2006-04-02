/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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

/**
 * $Id$
 */

#ifndef YZ_ACTION_H
#define YZ_ACTION_H

#include "cursor.h"
#include "selection.h"
#include "yzismacros.h"

#include <qstring.h>

class YZBuffer;
class YZView;
class YZCursor;

class YZIS_EXPORT YZAction {

	public:
		YZAction( YZBuffer* buffer );
		virtual ~YZAction( );

		void insertChar( YZView* pView, const YZCursor& pos, const QString& text );
		void insertChar( YZView* pView, int X, int Y, const QString& text );

		void replaceChar( YZView* pView, const YZCursor& pos, const QString& text );
		void replaceChar( YZView* pView, int X, int Y, const QString& text );

		void deleteChar( YZView* pView, const YZCursor& pos, int len );
		void deleteChar( YZView* pView, int X, int Y, int len );

		void insertLine( YZView* pView, const YZCursor& pos, const QString &text );
		void insertLine( YZView* pView, int Y, const QString &text );

		void mergeNextLine( YZView* pView, int Y, bool stripSpaces = true );

		void appendLine( YZView* pView, const QString& text );

		void insertNewLine( YZView* pView, const YZCursor& pos );
		void insertNewLine( YZView* pView, int X, int Y );

		void deleteLine( YZView* pView, const YZCursor& pos, int len, const QList<QChar> &reg );
		void deleteLine( YZView* pView, int Y, int len , const QList<QChar> &reg );
		void deleteArea( YZView* pView, const YZCursor& begin, const YZCursor& end, const QList<QChar> &reg );
		void deleteArea( YZView* pView, const YZInterval& i, const QList<QChar> &reg );

		void copyLine( YZView* pView, const YZCursor& pos, int len, const QList<QChar> &reg );
		void copyArea( YZView* pView, const YZCursor& begin,const YZCursor& end, const QList<QChar> &reg );
		void copyArea( YZView* pView, const YZInterval& i, const QList<QChar> &reg );

		void replaceArea( YZView* pView, const YZInterval& i, const QStringList& text );

		void replaceLine( YZView* pView, const YZCursor& pos, const QString &text );
		void replaceLine( YZView* pView, int Y, const QString &text );

		void replaceText( YZView* pView, const YZCursor& pos, int replacedLength, const QString& text );

		void indentLine( YZView* pView, int Y, int count ); // if count is < 0, unindent line
		
		YZCursor match( YZView* pView, const YZCursor& cursor, bool *found );
		YZCursor search( YZBuffer* pBuffer, const QString& what, const YZCursor& mBegin, const YZCursor& mEnd, int *matchlength, bool *found );

	private:
		YZBuffer* mBuffer;
		YZCursor* mPos;

};

#endif

