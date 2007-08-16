/* This file is part of the Yzis libraries
*  Copyright (C) 2004-2006 Loic Pauleve <panard@inzenet.org>
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

#ifndef QYZISCURSOR_H
#define QYZISCURSOR_H

#include <QWidget>

class YDebugStream;
class QYEdit;
class QYView;

class QYCursor : public QWidget
{
    Q_OBJECT

public :
    enum CursorShape {
        CursorFilledRect,
        CursorVbar,
        CursorHbar,
        CursorFrameRect,
        CursorHidden,
    };

    QYCursor( QYEdit* parent, CursorShape shape );
    virtual ~QYCursor();

    void setCursorShape( CursorShape shape );
    CursorShape shape() const;


protected :
    virtual void paintEvent( QPaintEvent* pe );

private :
    CursorShape mCursorShape;
    QYEdit* mEdit;
    QYView* mView;

};

YDebugStream& operator<<( YDebugStream& out, const QYCursor::CursorShape & shape );

#endif

