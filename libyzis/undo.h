/* This file is part of the Yzis libraries
*  Copyright (C) 2004 Philippe Fremy <phil@freehackers.org>
*  Copyright (C) 2004-2005 Mickael Marchand <mikmak@yzis.org>
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
 * This file contains the classes necessary to handle undo/redo
 */

#ifndef YZ_UNDO_H
#define YZ_UNDO_H

#include <QString>
#include <QList>
#include <QPoint>
#include "yzismacros.h"

class YView;
class YBuffer;

/** An individual operation on a buffer, that can be done or undone. */

struct YBufferOperation
{
    enum OperationType {
        OpAddText,  //!< insert some characters inside the line
        OpDelText,  //!< delete some characters from the line

        // for OpAddLine and OpDelLine, the arguments col and text are ignored.
        OpAddLine,  //!< insert a line before the specified line.
        OpDelLine  //!< delete the line from the buffer

    };

    /**  Perform the buffer operation on the buffer passed in argument.
      *  If opposite is true, perform the opposite operation (used for undo)
      */
    void performOperation( YView* pView, bool opposite = false );

    OperationType type;
    QString text;
    QPoint pos;

    QString toString() const;
};

typedef QList<YBufferOperation*> UndoItemBase;

/** An UndoItem contains a list of individual buffer operations
  * and the two cursor positions: before and after the whole set of operations
  */
class UndoItem : public UndoItemBase
{
public:
    UndoItem();

    int startCursorX, startCursorY;
    int endCursorX, endCursorY;
};

/** This class contains all the UndoItem. It stores them (commitUndoItem), do
  * or undo them.
  */
class YZIS_EXPORT YZUndoBuffer
{
public:
    YZUndoBuffer( YBuffer * );
    virtual ~YZUndoBuffer();

    /*
     * * Store the previous undo item (if any) and start a new one
     */
    void commitUndoItem( uint cursorX, uint cursorY );

    void addBufferOperation( YBufferOperation::OperationType type, const QString & text, QPoint pos);

    /**
     * Undo the last operations on the buffer, move backward in the undo list.
     * cursorX and cursorY will be set to the new cursor position
     */
    void undo( YView* pView );

    /**
     * Redo the current operation on the buffer, move forward in the undo list
     * cursorX and cursorY will be set to the new cursor position
     */
    void redo( YView* pView );

    /*! Return whether it is possibe to issue a redo */
    bool mayRedo() const;

    /*! Return whether it is possibe to issue an undo */
    bool mayUndo() const;

    QString toString(const QString& msg = "") const;

    /** Sets this while performing undo and redo, so that the operations
     * are not registered as new buffer commands */
    void setInsideUndo( bool set )
    {
        mInsideUndo = set;
    }
    bool isInsideUndo() const
    {
        return mInsideUndo;
    }

    void clearUndo()
    {
        mUndoItemList.clear();
    }
    void clearRedo()
    {
        removeUndoItemAfterCurrent();
    }
    unsigned int undoCount() const
    {
        return mCurrentIndex;
    }
    unsigned int redoCount() const
    {
        return mUndoItemList.count() - mCurrentIndex;
    }

protected:
    /** purge the undo list after the current item */
    void removeUndoItemAfterCurrent();

    QString undoItemToString( UndoItem * item) const;

    YBuffer * mBuffer;
    UndoItem * mFutureUndoItem;
    QList<UndoItem*> mUndoItemList;
    uint mCurrentIndex;
    bool mInsideUndo;
};

#endif // YZ_UNDO_H

