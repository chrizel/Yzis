/* This file is part of the Yzis libraries
 *  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
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

QString BufferOperation::toString() {
	QString ots;
	switch( type ) {
		case ADDTEXT: ots= "ADDTEXT"; break;
		case DELTEXT: ots= "DELTEXT"; break;
		case ADDLINE: ots= "ADDLINE"; break;
		case DELLINE: ots= "DELLINE"; break;
	}
	return QString("class BufferOperation: %1 '%2' line %3, col %4").arg(ots).arg(text).arg(line).arg(col) ;
}

void BufferOperation::performOperation( YZBuffer * buf, bool opposite)
{
	OperationType t = type;
	if (opposite = true) {
		switch( type ) {
			case ADDTEXT: t = DELTEXT; break;
			case DELTEXT: t = ADDTEXT; break; 
			case ADDLINE: t = DELLINE; break; 
			case DELLINE: t = ADDLINE; break; 
		}
	}

	switch( t) {
		case ADDTEXT:
			buf->addChar( col, line, text ); 
			break;
		case DELTEXT: 
			buf->delChar( col, line, text.length() ); 
			break;
		case ADDLINE:
			buf->addNewLine( 0, line );
			break;
		case DELLINE:
			buf->deleteLine( line );
			break;
	}
}


UndoBuffer::UndoBuffer( YZBuffer * buffer )
:  mBuffer(buffer), mFutureUndoItem( 0L ) /* , mCurrentUndoItem( mUndoItemList ) */
{
	mCurrentIndex = 0;
	mInsideUndo = false;
//	mCurrentUndoItem = QPtrListIterator<UndoItem>( mUndoItemList );
	mUndoItemList.setAutoDelete( true );
	commitUndoItem();
}

void UndoBuffer::commitUndoItem()
{
	if (mInsideUndo == true) return;
	removeUndoItemAfterCurrent();
//	yzDebug() << "UndoItem::commitUndoItem 1 " << toString() << endl;
	if (mFutureUndoItem) {
		mUndoItemList.append( mFutureUndoItem );
//		yzDebug() << "UndoItem::commitUndoItem 2 " << toString() << endl;
		// we need to re-initialise the iterator
		//mCurrentUndoItem = QPtrListIterator<UndoItem>( mUndoItemList );
		mCurrentIndex = mUndoItemList.count();
	}
//	yzDebug() << "UndoItem::commitUndoItem 3 " << toString() << endl;
	mFutureUndoItem = new UndoItem();
	mFutureUndoItem->setAutoDelete( true );
	yzDebug() << "UndoItem::commitUndoItem 4 " << toString() << endl;
}

void UndoBuffer::addBufferOperation( BufferOperation::OperationType type, 
									 const QString & text,
									 uint col, uint line )
{
	//yzDebug() << "UndoItem::addBufferOperation 4 " << toString() << endl;
	if (mInsideUndo == true) return;
	YZASSERT( mFutureUndoItem != NULL );
	BufferOperation * bufOperation = new BufferOperation();
	bufOperation->type = type;
	bufOperation->text = text;
	bufOperation->line = line;
	bufOperation->col = col;
	//yzDebug() << "UndoItem::addBufferOperation 4 " << toString() << endl;
	mFutureUndoItem->append( bufOperation );
	//yzDebug() << "UndoItem::addBufferOperation 4 " << toString() << endl;

	removeUndoItemAfterCurrent();
	//yzDebug() << "UndoItem::addBufferOperation 5 " << toString() << endl;
}

void UndoBuffer::removeUndoItemAfterCurrent()
{
	//yzDebug() << "UndoItem::removeUndoItemAfterCurrent 4 " << toString() << endl;
	// mCurrentUndoItem.toLast();
	while( mUndoItemList.count() > mCurrentIndex ) {
		mUndoItemList.removeLast();
	}
	yzDebug() << "UndoItem::removeUndoItemAfterCurrent 4 " << toString() << endl;
}

void UndoBuffer::undo()
{
	BufferOperation * bufOp;

	if (mayUndo() == false) {
		// notify the user that undo is not possible
		return;
	}
	setInsideUndo( true );

	UndoItemIterator it( *mUndoItemList.at(mCurrentIndex-1) );
	while( (bufOp = it.current()) ) {
		bufOp->performOperation( mBuffer, true );
		++it;
	}
	//mCurrentUndoItem--;
	mCurrentIndex--;
	setInsideUndo( false );
}

void UndoBuffer::redo()
{
	BufferOperation * bufOp;

	if (mayRedo() == false) {
		// notify the user that undo is not possible
		return;
	}
	setInsideUndo( true );

	//UndoItemIterator it( *mCurrentUndoItem );
	UndoItemIterator it( *mUndoItemList.at(mCurrentIndex-1) );
	while( (bufOp = it.current()) ) {
		bufOp->performOperation( mBuffer, false );
		++it;
	}
	//mCurrentUndoItem++;
	mCurrentIndex++;

	setInsideUndo( false );
}

bool UndoBuffer::mayRedo()
{
	bool ret;
	//yzDebug() << "UndoItem::mayRedo " << toString() << endl;
	ret = mCurrentIndex < mUndoItemList.count();
	/* (! mCurrentUndoItem.atLast() ); */
	//yzDebug() << "UndoItem::mayRedo 4 " << toString() << endl;
	/* return (! mCurrentUndoItem.atLast() ); */
	// mCurrentUndoItem.toLast();
	return ret;
}
	
bool UndoBuffer::mayUndo()
{
	bool ret;
	//yzDebug() << "UndoItem::mayUndo " << toString() << endl;
	ret = mCurrentIndex > 0;
	/* (*mCurrentUndoItem) != 0; */
	//yzDebug() << "UndoItem::mayRedo 4 " << toString() << endl;
	/* return (*mCurrentUndoItem) != 0; */
	return ret;
}

QString UndoBuffer::undoItemToString( UndoItem * undoItem )
{
	QString s;
	QString offsetS = "  ";
	s += offsetS + offsetS + "UndoItem:\n";
	if (! undoItem ) return s;
	UndoItemIterator it( *undoItem );
	BufferOperation * bufOp;
	while( (bufOp = it.current()) ) {
		s += offsetS + offsetS + offsetS + bufOp->toString() + "\n";
		++it;
	}
	return s;
}

QString UndoBuffer::toString(QString msg)
{
	QString s = msg + " UndoBuffer:\n";
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
