/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Philippe Fremy <phil@freehackers.org>
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

/**
 * This file contains the classes necessary to handle undo/redo
 */

#ifndef YZ_UNDO_H
#define YZ_UNDO_H

#include <qstring.h>
#include <qptrlist.h>

class YZView;
class YZBuffer;

struct buffer_operation
{
	enum OperationType {
		ADDTEXT, // insert some characters inside the line
		DELTEXT, // delete some characters from the line
		
		// for ADDLINE and DELLINE, the arguments col and text are ignored.
		ADDLINE, // insert a line before the specified line.
		DELLINE  // delete the line from the buffer

	};

	/**  Perform the buffer operation on the buffer passed in argument.
	  *  If opposite is true, perform the opposite operation (used for undo)
	  */
	void performOperation( YZView* pView, bool opposite=false );

	OperationType type;
	QString text;
	uint line;
	uint col;

	QString toString();
};
typedef struct buffer_operation YZBufferOperation;

typedef QPtrListIterator<YZBufferOperation> UndoItemContentIterator;
typedef QPtrList<YZBufferOperation> UndoItemBase;

class UndoItem : public UndoItemBase
{
public:
	UndoItem();
	
	int startCursorX, startCursorY;
	int endCursorX, endCursorY;
};

class YZUndoBuffer {
public:
	YZUndoBuffer( YZBuffer * );

	/** Store the previous undo item (if any) and start a new one
	 */
	void commitUndoItem( uint cursorX, uint cursorY );

	void addBufferOperation( YZBufferOperation::OperationType type, const QString & text, uint col, uint line );

	/** 
	 * Undo the last operations on the buffer, move backward in the undo list.
	 * cursorX and cursorY will be set to the new cursor position
	 */ 
	void undo( YZView* pView );

	/** 
	 * Redo the current operation on the buffer, move forward in the undo list
	 * cursorX and cursorY will be set to the new cursor position
	 */
	void redo( YZView* pView );

	/*! Return whether it is possibe to issue a redo */
	bool mayRedo();

	/*! Return whether it is possibe to issue an undo */
	bool mayUndo();

	QString toString(QString msg="");

	/** Sets this while performing undo and redo, so that the operations
	 * are not registred as new buffer commands */
	void setInsideUndo( bool set ) { mInsideUndo = set; }
	bool isInsideUndo() { return mInsideUndo; }

protected:
	/** purge the undo list after the current item */
	void removeUndoItemAfterCurrent();

	QString undoItemToString( UndoItem * item);

	YZBuffer * mBuffer;
	UndoItem * mFutureUndoItem;
	QPtrList<UndoItem> mUndoItemList;
//	QPtrListIterator<UndoItem> mCurrentUndoItem;
	uint mCurrentIndex;
	bool mInsideUndo;
};

#endif // YZ_UNDO_H

