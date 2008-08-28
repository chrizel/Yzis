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
#include "buffer.h"
class YView;
class YCursor;
class YInterval;

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
    YZAction( YBuffer* buffer );
    virtual ~YZAction( );

    // YCursor versions
    void insertChar( YView* pView, const YCursor pos, const QString& text );
    bool replaceChar( YView* pView, const YCursor pos, const QString& text );
    bool deleteChar( YView* pView, const YCursor pos, int len );
    void insertLine( YView* pView, const YCursor pos, const QString &text );
    void insertNewLine( YView* pView, const YCursor pos );
    void replaceLine( YView* pView, const YCursor pos, const QString &text );
    void deleteLine( YView* pView, const YCursor pos, int len, const QList<QChar> &reg );

    // X,Y versions
    void insertChar( YView* pView, const int X, const int Y, const QString& text )
    {
        YCursor pos( X, Y ); insertChar( pView, pos, text );
    }
    bool replaceChar( YView* pView, const int X, const int Y, const QString& text )
    {
        YCursor pos( X, Y ); return replaceChar( pView, pos, text );
    }
    bool deleteChar( YView* pView, const int X, const int Y, int len )
    {
        YCursor pos( X, Y ); return deleteChar( pView, pos, len );
    }
    void insertLine( YView* pView, int Y, const QString &text )
    {
        YCursor pos( 0, Y ); insertLine( pView, pos, text );
    }
    void insertNewLine( YView* pView, const int X, const int Y )
    {
        YCursor pos( X, Y ); insertNewLine( pView, pos );
    }
    void deleteLine( YView* pView, int Y, int len, const QList<QChar>& regs )
    {
        YCursor pos( 0, Y ); deleteLine( pView, pos, len, regs );
    }
    void replaceLine( YView* pView, int Y, const QString& text )
    {
        YCursor pos( 0, Y ); replaceLine( pView, pos, text );
    }


    void mergeNextLine( YView* pView, int Y, bool stripSpaces = true );

    void deleteArea( YView* pView, const YCursor begin, const YCursor end, const QList<QChar> &reg );
    void deleteArea( YView* pView, const YInterval& i, const QList<QChar> &reg );

    void copyLine( YView* pView, const YCursor pos, int len, const QList<QChar> &reg );
    void copyArea( YView* pView, const YCursor begin, const YCursor end, const QList<QChar> &reg );
    void copyArea( YView* pView, const YInterval& i, const QList<QChar> &reg );

    void replaceArea( YView* pView, const YInterval& i, const YRawData& text );


    void replaceText( YView* pView, const YCursor pos, int replacedLength, const QString& text );

    void indentLine( YView* pView, int Y, int count ); // if count is < 0, unindent line

    YCursor match( YView* pView, const YCursor cursor, bool *found ) const;
    YCursor search( YBuffer* pBuffer, const QString& what, const YCursor mBegin, const YCursor mEnd, int *matchlength, bool *found ) const;

    /**
     * Pastes the content of default or given register
     */
    void pasteContent( YView *pView, QChar registr, bool after = true );

private:
    YBuffer* mBuffer;
    YCursor* mPos;

};

#endif

