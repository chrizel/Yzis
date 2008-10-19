/* This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
*  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>
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

#ifndef YZ_VIEWCURSOR_H
#define YZ_VIEWCURSOR_H

#include "yzismacros.h"
#include "cursor.h"

/**
  * @short Handle both buffer/drawing cursors
  *
  * Coumpound object containing a YCursor for the buffer, and another one
  * for the display
  */
class YZIS_EXPORT YViewCursor
{

public:
    YViewCursor();
    YViewCursor( int line, int position, int column );
    YViewCursor( const YViewCursor &c);
    virtual ~YViewCursor();

    YViewCursor &operator=( const YViewCursor &c );

	/* buffer line */
	inline int line() const { return mBuffer.line(); } 
    inline void setLine( int value ) { mBuffer.setLine(value); }
	/* buffer position */
	inline int position() const { return mBuffer.column(); }
	inline void setPosition( int value ) { mBuffer.setColumn(value); }

	inline YCursor buffer() const { return mBuffer; }

	/* view column */
	inline int column() const { return mColumn; }
	inline void setColumn( int value ) { mColumn = value; }

    /*
     * operators
     */
    bool operator== ( const YViewCursor right ) const;
    bool operator!= ( const YViewCursor right ) const;
    bool operator< ( const YViewCursor right ) const;
    bool operator<= ( const YViewCursor right ) const;
    bool operator> ( const YViewCursor right ) const;
    bool operator>= ( const YViewCursor right ) const;

private :
	YCursor mBuffer;
	int mColumn;
};

#endif
