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

#include "libyzis/undo.h"
#include "libyzis/buffer.h"
#include "libyzis/debug.h"

QString YZBufferOperation::toString() {
	QString ots;
	switch( type ) {
		case ADDTEXT: ots= "ADDTEXT"; break;
		case DELTEXT: ots= "DELTEXT"; break;
		case ADDLINE: ots= "ADDLINE"; break;
		case DELLINE: ots= "DELLINE"; break;
	}
	return QString("%1 '%2' line %3, col %4").arg(ots).arg(text).arg(line).arg(col) ;
}

void YZBufferOperation::performOperation( YZBuffer * buf, bool opposite)
{
	OperationType t = type;

	yzDebug("YZUndoBuffer") << "YZBufferOperation: " << (opposite ? "undo " : "redo ") << toString().latin1() << endl;

	if (opposite == true) {
		switch( type ) {
			case ADDTEXT: t = DELTEXT; break;
			case DELTEXT: t = ADDTEXT; break; 
			case ADDLINE: t = DELLINE; break; 
			case DELLINE: t = ADDLINE; break; 
		}
	}

	switch( t) {
		case ADDTEXT:
			buf->insertChar( col, line, text ); 
			break;
		case DELTEXT: 
			buf->delChar( col, line, text.length() ); 
			break;
		case ADDLINE:
			buf->insertNewLine( 0, line );
			break;
		case DELLINE:
			buf->deleteLine( line );
			break;
	}

//	yzDebug("YZUndoBuffer") << "YZBufferOperation::performOperation Buf -> '" << buf->getWholeText() << "'\n";
}


YZUndoBuffer::YZUndoBuffer( YZBuffer * buffer )
:  mBuffer(buffer), mFutureUndoItem( 0L )
{
	mCurrentIndex = 0;
	mInsideUndo = false;
	mUndoItemList.setAutoDelete( true );
	// Create the mFutureUndoItem
	commitUndoItem();
}

void YZUndoBuffer::commitUndoItem()
{
	if (mInsideUndo == true) return;
	if (mFutureUndoItem && mFutureUndoItem->count() == 0) return;

	if (mFutureUndoItem) {
		removeUndoItemAfterCurrent();
		mUndoItemList.append( mFutureUndoItem );
		mCurrentIndex = mUndoItemList.count();
		yzDebug("YZUndoBuffer") << "UndoItem::commitUndoItem" << toString() << endl;
	}
	mFutureUndoItem = new UndoItem();
	mFutureUndoItem->setAutoDelete( true );
}

void YZUndoBuffer::addBufferOperation( YZBufferOperation::OperationType type, 
									 const QString & text,
									 uint col, uint line )
{
	if (mInsideUndo == true) return;
	YZASSERT( mFutureUndoItem != NULL );
	YZBufferOperation * bufOperation = new YZBufferOperation();
	bufOperation->type = type;
	bufOperation->text = text;
	bufOperation->line = line;
	bufOperation->col = col;
	mFutureUndoItem->append( bufOperation );
	removeUndoItemAfterCurrent();
}

void YZUndoBuffer::removeUndoItemAfterCurrent()
{
	while( mUndoItemList.count() > mCurrentIndex ) {
		mUndoItemList.removeLast();
	}
}

void YZUndoBuffer::undo()
{
	YZBufferOperation * bufOp;

	if (mayUndo() == false) {
		// notify the user that undo is not possible
		return;
	}
	setInsideUndo( true );

	UndoItemIterator it( *mUndoItemList.at(mCurrentIndex-1) );
	it.toLast();
	while( (bufOp = it.current()) ) {
		bufOp->performOperation( mBuffer, true );
		--it;
	}
	mCurrentIndex--;
	setInsideUndo( false );
}

void YZUndoBuffer::redo()
{
	YZBufferOperation * bufOp;

	if (mayRedo() == false) {
		// notify the user that undo is not possible
		return;
	}
	setInsideUndo( true );

	++mCurrentIndex;
	UndoItemIterator it( *mUndoItemList.at(mCurrentIndex-1) );
	while( (bufOp = it.current()) ) {
		bufOp->performOperation( mBuffer, false );
		++it;
	}
	setInsideUndo( false );
}

bool YZUndoBuffer::mayRedo()
{
	bool ret;
	ret = mCurrentIndex < mUndoItemList.count();
	return ret;
}
	
bool YZUndoBuffer::mayUndo()
{
	bool ret;
	ret = mCurrentIndex > 0;
	return ret;
}

QString YZUndoBuffer::undoItemToString( UndoItem * undoItem )
{
	QString s;
	QString offsetS = "  ";
	s += offsetS + offsetS + "UndoItem:\n";
	if (! undoItem ) return s;
	UndoItemIterator it( *undoItem );
	YZBufferOperation * bufOp;
	while( (bufOp = it.current()) ) {
		s += offsetS + offsetS + offsetS + bufOp->toString() + "\n";
		++it;
	}
	return s;
}

QString YZUndoBuffer::toString(QString msg)
{
	QString s = msg + " YZUndoBuffer:\n";
	QString offsetS = "  ";
	s += offsetS + "mUndoItemList\n";
	QPtrListIterator<UndoItem> it( mUndoItemList );
	UndoItem * undoItem;
	while( (undoItem = it.current()) ) {
		s += undoItemToString( undoItem );
		++it;
	}
	s += offsetS + "mFutureUndoItem\n";
	s += undoItemToString( mFutureUndoItem );
	s += offsetS + "current UndoItem\n";
	s += (mCurrentIndex > 0) ? undoItemToString( mUndoItemList.at( mCurrentIndex-1 ) )
		:offsetS + offsetS + "None\n";
	s += "\n";
	return s;
}
