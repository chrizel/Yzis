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

#ifndef YZ_ACTION_H
#define YZ_ACTION_H

/* Yzis */
#include "yzismacros.h"
#include "cursor.h"
class YZBuffer;
class YZView;
class YZCursor;
class YZInterval;

/* Qt */
#include <QList>
#include <QPoint>
class QString;
class QStringList;
class QChar;

/**
 * Class that holds all buffer modification methods.
 *
 * Here are all buffer modifications methods. 
 * If you need to add a buffer modification method, you are at the _RIGHT_ place.
 */
class YZIS_EXPORT YZAction
{

public:
	YZAction( YZBuffer* buffer );
	virtual ~YZAction( );

	// YZCursor versions
	void insertChar( YZView* pView, const YZCursor pos, const QString& text );
	void replaceChar( YZView* pView, const YZCursor pos, const QString& text );
	void deleteChar( YZView* pView, const YZCursor pos, int len );
	void insertLine( YZView* pView, const YZCursor pos, const QString &text );
	void insertNewLine( YZView* pView, const YZCursor pos );
	void replaceLine( YZView* pView, const YZCursor pos, const QString &text );
	void deleteLine( YZView* pView, const YZCursor pos, int len, const QList<QChar> &reg );

	// X,Y versions
	void insertChar( YZView* pView, const int X, const int Y, const QString& text ) { YZCursor pos( X, Y ); insertChar( pView, pos, text ); }
	void replaceChar( YZView* pView, const int X, const int Y, const QString& text ) { YZCursor pos( X, Y ); replaceChar( pView, pos, text ); }
	void deleteChar( YZView* pView, const int X, const int Y, int len ) { YZCursor pos( X, Y ); deleteChar( pView, pos, len ); }
	void insertLine( YZView* pView, int Y, const QString &text ) { YZCursor pos( 0, Y ); insertLine( pView, pos, text ); }
	void insertNewLine( YZView* pView, const int X, const int Y ) { YZCursor pos( X, Y ); insertNewLine( pView, pos ); }
	void deleteLine( YZView* pView, int Y, int len, const QList<QChar>& regs ) { YZCursor pos( 0, Y ); deleteLine( pView, pos, len, regs ); }
	void replaceLine( YZView* pView, int Y, const QString& text ) { YZCursor pos( 0, Y ); replaceLine( pView, pos, text ); }


	void mergeNextLine( YZView* pView, int Y, bool stripSpaces = true );

	void appendLine( YZView* pView, const QString& text );


	void deleteArea( YZView* pView, const YZCursor begin, const YZCursor end, const QList<QChar> &reg );
	void deleteArea( YZView* pView, const YZInterval& i, const QList<QChar> &reg );

	void copyLine( YZView* pView, const YZCursor pos, int len, const QList<QChar> &reg );
	void copyArea( YZView* pView, const YZCursor begin,const YZCursor end, const QList<QChar> &reg );
	void copyArea( YZView* pView, const YZInterval& i, const QList<QChar> &reg );

	void replaceArea( YZView* pView, const YZInterval& i, const QStringList& text );


	void replaceText( YZView* pView, const YZCursor pos, int replacedLength, const QString& text );

	void indentLine( YZView* pView, int Y, int count ); // if count is < 0, unindent line
	
	YZCursor match( YZView* pView, const YZCursor cursor, bool *found ) const;
	YZCursor search( YZBuffer* pBuffer, const QString& what, const YZCursor mBegin, const YZCursor mEnd, int *matchlength, bool *found ) const;

private:
	YZBuffer* mBuffer;
	YZCursor* mPos;

};

#endif

