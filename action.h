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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id$
 */

#ifndef YZ_ACTION_H
#define YZ_ACTION_H

#include "cursor.h"
#include "selection.h"

#include <qstring.h>

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

		void mergeNextLine( YZView* pView, unsigned int Y, bool stripSpaces = true );

		void appendLine( YZView* pView, const QString& text );

		void insertNewLine( YZView* pView, const YZCursor& pos );
		void insertNewLine( YZView* pView, unsigned int X, unsigned int Y );

		void deleteLine( YZView* pView, const YZCursor& pos, unsigned int len, const QValueList<QChar> &reg );
		void deleteLine( YZView* pView, unsigned int Y, unsigned int len , const QValueList<QChar> &reg );
		void deleteArea( YZView* pView, const YZCursor& begin, const YZCursor& end, const QValueList<QChar> &reg );
		void deleteArea( YZView* pView, const YZInterval& i, const QValueList<QChar> &reg );

		void copyLine( YZView* pView, const YZCursor& pos, unsigned int len, const QValueList<QChar> &reg );
		void copyArea( YZView* pView, const YZCursor& begin,const YZCursor& end, const QValueList<QChar> &reg );
		void copyArea( YZView* pView, const YZInterval& i, const QValueList<QChar> &reg );

		void replaceArea( YZView* pView, const YZInterval& i, const QStringList& text );
		void replaceLine( YZView* pView, const YZCursor& pos, const QString &text );
		void replaceLine( YZView* pView, unsigned int Y, const QString &text );

		void replaceText( YZView* pView, const YZCursor& pos, unsigned int replacedLength, const QString& text );

		void indentLine( YZView* pView, unsigned int Y, int count ); // if count is < 0, unindent line
		
		YZCursor match( YZView* pView, YZCursor& mCursor, bool *found );
		YZCursor search( YZBuffer* pBuffer, const QString& what, const YZCursor& mBegin, const YZCursor& mEnd, unsigned int *matchlength, bool *found );

	private:
		YZBuffer* mBuffer;
		YZCursor* mPos;

};

#endif

