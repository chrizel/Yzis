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

#include "cursor.h"
#include "yzismacros.h"

class YView;

/**
  * @short Handle both buffer/drawing cursors
  *
  * Coumpound object containing a YCursor for the buffer, and another one
  * for the display
  */
struct YCursorPos
{
    YCursor mBuffer;  /* buffer position */
    YCursor mScreen; /* display position */

    QString toString() const
    {
        QString s;
        s = "buffer pos=" + mBuffer.toString() + ", screen pos=" + mScreen.toString();
        return s;
    }
};



/**
 * class YViewCursor : buffer and screen cursor with all members that YView needs to move it.
 * this is only an interface, it doesn't have to know how move itself ( this is YView stuff )
 */
class YZIS_EXPORT YViewCursor : public YCursorPos
{

    friend class YView;

public:
    explicit YViewCursor( YView* parent );
    YViewCursor( const YViewCursor &c);
    virtual ~YViewCursor();

    YViewCursor &operator=( const YViewCursor &c );

    void reset();

    int bufferX() const;
    int bufferY() const;
    int screenX() const;
    int screenY() const;

    // toggle valid token to false
    void invalidate();

    bool valid() const;

    inline const YCursor buffer() const
    {
        return mBuffer;
    }
    inline const YCursor screen() const
    {
        return mScreen;
    }

    /**
     * curLineHeight : line height at current cursor position
     */
    inline int curLineHeight() const
    {
        return lineHeight;
    }

    void debug();

    void setBuffer( const YCursor value )
    {
        mBuffer = value;
    }
    void setScreen( const YCursor value )
    {
        mScreen = value;
    }

    void setBufferX( int value );
    void setBufferY( int value );
    void setScreenX( int value );
    void setScreenY( int value );

private :
    void copyFields( const YViewCursor &rhs );

    /**
     * parent view
     */
    YView* mParent;

    /**
     * spaceFill is the shift for starting tabs
     * ( when wrapping a line, or scrolling horizontally )
     */
    int spaceFill;

    /**
     * buffer column increment
     */
    int bColIncrement;

    /**
     * buffer line increment
     */
    int bLineIncrement;

    /**
     * screen column increment
     */
    int sColIncrement;

    /**
     * screen line increment
     */
    int sLineIncrement;

    /**
     * current line height
     */
    int lineHeight;

    /**
     * last char was a tab ?
     */
    bool lastCharWasTab;

    /**
     * are we wrapping a tab ?
     */
    bool wrapTab;

    /**
     * are we wrapping a line ?
     */
    bool wrapNextLine;

    /**
     * valid token
     */
    bool mValid;

};

#endif
