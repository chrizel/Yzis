/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>
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

#include "action.h"
#include "debug.h"

YZAction::YZAction( YZBuffer* buffer ) {
	mBuffer = buffer;
}
YZAction::~YZAction( ) {
}

void YZAction::insertChar( YZView* pView, const YZCursor& pos, const QString& text ) {
	YZCursor mPos( pos );
	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() )
		it->initInsertChar( mPos, text.length(), pView->myId == it->myId );

	mBuffer->insertChar( mPos.getX(), mPos.getY(), text );

	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() )
		it->applyInsertChar( mPos, text.length(), pView->myId == it->myId );
}

void YZAction::replaceChar( YZView* pView, const YZCursor& pos, const QString& text ) {
	YZCursor mPos( pos );
	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() )
		it->initReplaceChar( mPos, text.length(), pView->myId == it->myId );

	mBuffer->delChar( mPos.getX(), mPos.getY(), text.length() );
	mBuffer->insertChar( mPos.getX(), mPos.getY(), text );

	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() )
		it->applyReplaceChar( mPos, text.length(), pView->myId == it->myId );
}

void YZAction::deleteChar( YZView* pView, const YZCursor& pos, unsigned int len ) {
	YZCursor mPos( pos );
	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() )
		it->initDeleteChar( mPos, len, pView->myId == it->myId );

	mBuffer->delChar( mPos.getX(), mPos.getY(), len );

	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() )
		it->applyDeleteChar( mPos, len, pView->myId == it->myId );
}

void YZAction::insertNewLine( YZView* pView, const YZCursor& pos ) {
	YZCursor mPos( pos );
	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() )
		it->initInsertLine( mPos, pView->myId == it->myId );

	mBuffer->insertNewLine( mPos.getX(), mPos.getY() );

	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() )
		it->applyInsertLine( mPos, pView->myId == it->myId );
}

void YZAction::insertLine( YZView* pView, const YZCursor& pos, const QString &text ) {
	YZCursor mPos( pos );
	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() )
		it->initInsertLine( mPos, pView->myId == it->myId );

	mBuffer->insertLine( text, mPos.getY() );

	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() )
		it->applyInsertLine( mPos, pView->myId == it->myId );
}

void YZAction::deleteLine( YZView* pView, const YZCursor& pos, unsigned int len ) {
	YZCursor mPos( pos );
	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() )
		it->initDeleteLine( mPos, len, pView->myId == it->myId );

	for ( unsigned int i = 0; i < len && mPos.getY() < mBuffer->lineCount(); i++ )
		mBuffer->deleteLine( mPos.getY() );

	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() )
		it->applyDeleteLine( mPos, len, pView->myId == it->myId );
}

void YZAction::deleteArea( YZView* pView, YZCursor& begin, YZCursor& end, const QChar& reg ) {
	YZCursor mPos( begin );
	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() )
		it->initDeleteLine( mPos, end, pView->myId == it->myId );

	QStringList buff;
	unsigned int bX = begin.getX();
	unsigned int bY = begin.getY();
	unsigned int eX = end.getX();
	unsigned int eY = end.getY();

	bool lineDeleted = bY != eY;

	/* 1. delete the part of the current line */
	QString b = mBuffer->textline( bY );
	if ( !lineDeleted ) {
		buff << b.mid( bX, eX - bX + 1 );
		QString b2 = b.left( bX ) + b.mid( eX );
		mBuffer->replaceLine( b2 , bY );
	} else {
		buff << b.mid( bX );
		QString b2 = b.left( bX );
		mBuffer->replaceLine( b2 , bY );
	}

	/* 2. delete whole lines */
	unsigned int curY = bY + 1;
	if ( bY == 0 ) curY = 0; //dont loop ;)
	if ( bY == mBuffer->lineCount() - 1 ) curY = mBuffer->lineCount() - 1; // and ... dont loop

	while ( end.getY() > curY ) {
		mBuffer->deleteLine( curY );
		end.setY( end.getY() - 1 );
	}

	/* 3. delete the part of the last line */
	if ( end.getY() == curY && !lineDeleted ) {
		b = mBuffer->textline( curY );
		buff << b.left( end.getX() );
		mBuffer->replaceLine( b.mid( end.getX() ), curY );
	} else if ( end.getY() == curY ) {
		b = mBuffer->textline( curY );
		buff << b.left( end.getX() );
		mBuffer->replaceLine( b.mid( end.getX() ), curY );
		mBuffer->mergeNextLine( curY - 1 );
	}
	YZSession::mRegisters.setRegister( reg, buff );

	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() )
		it->applyDeleteLine( mPos, end, pView->myId == it->myId );
}

void YZAction::insertChar( YZView* pView, unsigned int X, unsigned int Y, const QString& text ) {
	YZCursor pos( pView, X, Y );
	insertChar( pView, pos, text );
}
void YZAction::replaceChar( YZView* pView, unsigned int X, unsigned int Y, const QString& text ) {
	YZCursor pos( pView, X, Y );
	replaceChar( pView, pos, text );
}
void YZAction::deleteChar( YZView* pView, unsigned int X, unsigned int Y, unsigned int len ) {
	YZCursor pos( pView, X, Y );
	deleteChar( pView, pos, len );
}
void YZAction::insertLine( YZView* pView, unsigned int Y, const QString &text ) {
	YZCursor pos( pView, 0, Y );
	insertLine( pView, pos, text );
}
void YZAction::insertNewLine( YZView* pView, unsigned int X, unsigned int Y ) {
	YZCursor pos( pView, X, Y );
	insertNewLine( pView, pos );
}
void YZAction::deleteLine( YZView* pView, unsigned int Y, unsigned int len ) {
	YZCursor pos( pView, 0, Y );
	deleteLine( pView, pos, len );
}

