/* This file is part of the Yzis libraries
 *  Copyright (C) 2003, 2004 Mickael Marchand <marchand@kde.org>
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

#include "motionpool.h"
#include "view.h"
#include "cursor.h"
#include "debug.h"

static YZMotion nullMotion("",REGEXP,0,0,false,false);

YZMotionPool::YZMotionPool(){
}

YZMotionPool::~YZMotionPool() {
	pool.clear();
}

void YZMotionPool::initPool() {
						//regexp, type, x, y, backward,after
	addMotion (YZMotion( "\\w*\\b", REGEXP, 0, 0, false, true ) , "[0-9]*w" );
	addMotion (YZMotion( "\\b\\w*", REGEXP, 0, 0, true, true ), "[0-9]*b");
	addMotion (YZMotion( "$", REGEXP, 0, 0, false, true ), "\\$" );
	addMotion (YZMotion( "^", REGEXP, 0, 0, true, true ), "0");
	addMotion (YZMotion( "", RELATIVE, -1, 0, true, true ), "[0-9]*h");
	addMotion (YZMotion( "", RELATIVE, 1, 0, false, true ), "[0-9]*l");
	addMotion (YZMotion( "", RELATIVE, 0, -1, true, true ), "[0-9]*k");
	addMotion (YZMotion( "", RELATIVE, 0, 1, false, true ), "[0-9]*j");
	addMotion (YZMotion( "^\\s*", REGEXP, 0, 0, true, false ), "\\^");
	addMotion (YZMotion( "", NORMAL, 0, 0, false, true ), "%");
	addMotion (YZMotion( "", NORMAL, 0, 0, false, false ), "(`|')[a-zA-Z0-9]");
}

void YZMotionPool::addMotion(const YZMotion& regexp, const QString& key){
	pool.insert( key,regexp );
}

YZMotion& YZMotionPool::findMotion ( const QString& inputs, bool *ok ) {
	QMap<QString,YZMotion>::iterator it;
	for ( it = pool.begin(); it!=pool.end(); it++) {
		YZMotion& t = it.data();
		QRegExp rex ( it.key() );
		if ( rex.exactMatch( inputs ) ) {
			*ok = true;
			return t;
		}
	}
	*ok = false;
	return nullMotion;
}

bool YZMotionPool::isValid( const QString& inputs ) {
	bool ok = false;
	findMotion( inputs, &ok );
	return ok;
}

bool YZMotionPool::applyMotion( const QString &inputsMotion, YZView *view, YZCursor& from, YZCursor& to) {
	bool ok = false;
	YZMotion& motion = findMotion(inputsMotion, &ok);
	if ( !ok ) return false;
	switch ( motion.type ) {
		case REGEXP :
			return applyRegexpMotion( inputsMotion, motion, view, from, to);
		case RELATIVE :
			return applyRelativeMotion( inputsMotion, motion, view, from, to);
		case NORMAL :
			return applyNormalMotion( inputsMotion, motion, view, from, to);
	}
	return false;
}

bool YZMotionPool::applyNormalMotion( const QString& inputsMotion, YZMotion& , YZView *view, YZCursor& from, YZCursor& to) {
	bool found = false;
	if ( inputsMotion == "%" ) {
		to = view->myBuffer()->action()->match( view, from, &found );
		if ( found ) { //adjust the cursor
			if ( from < to)
				to.setX( to.getX()+1 );
			else if ( from > to ) {
				from.setX( from.getX() + 1 );
			}
		}
		return found;
	} else if ( inputsMotion.startsWith("`") || inputsMotion.startsWith("'") ) {
		yzDebug() << "Delete to tag " << inputsMotion.mid( 1,1 ) << endl;
		YZCursorPos pos = view->myBuffer()->marks()->get(inputsMotion.mid( 1, 1 ), &found);
		to = pos.bPos;
		to.setX(to.getX() ? to.getX() - 1 : 0);
		return found;
	}
	return false;
}

bool YZMotionPool::applyRelativeMotion( const QString &inputsMotion, YZMotion& motion, YZView *view, YZCursor& from, YZCursor& to) {
	int counter = 1; //number of times we have to match
	QRegExp rx ( "([0-9]+).+" );
	if ( rx.exactMatch( inputsMotion ) ) counter = rx.cap(1).toInt();
	yzDebug() << "Loop " << counter << " times" << endl;
	QRegExp rex( motion.rex );
	to.setX(view->getBufferCursor()->getX());
	to.setY(view->getBufferCursor()->getY());
	int count = 0 ;

	while (count < counter) {
		to.setX( to.getX() + motion.x );
		to.setY( to.getY() + motion.y );
		count++;
	}
	if ( count ) return true;

	return false;
}

bool YZMotionPool::applyRegexpMotion( const QString &inputsMotion, YZMotion& motion, YZView *view, YZCursor& from, YZCursor& to) {
	int counter = 1; //number of times we have to match
	QRegExp rx ( "([0-9]+).+" );
	if ( rx.exactMatch( inputsMotion ) ) counter = rx.cap(1).toInt();
	yzDebug() << "Loop " << counter << " times" << endl;
	QRegExp rex( motion.rex );
	to.setX(view->getBufferCursor()->getX());
	to.setY(view->getBufferCursor()->getY());
	int idx=-1;
	int count = 0 ;
	if ( ! motion.backward ) {
		yzDebug() << "Forward motion" <<endl;
		while (count < counter) {
			const QString& current = view->myBuffer()->textline( to.getY() );
			if ( current.isNull() ) return false;
			idx = rex.search( current, to.getX() + 1 );
			if ( idx != -1 ) {
				yzDebug() << "Match at " << idx << " Matched length " << rex.matchedLength() << endl;
				count++; //one match
				to.setX( idx + ( motion.after ? rex.matchedLength() : 0 ) );
			} else {
				if ( to.getY() >= view->myBuffer()->lineCount() ) break;
				to.setX( 0 );
				to.setY( to.getY() + 1 );
			}
		}
	} else {
		yzDebug() << "Backward motion" <<endl;
		while ( count < counter ) {
			const QString& current = view->myBuffer()->textline( to.getY() );
			if ( current.isNull() ) return false;
			idx = rex.searchRev( current, to.getX() >= 1 ? to.getX() - 1 : to.getX() );
			if ( idx != -1 ) {
				yzDebug() << "Match at " << idx << " on line " << to.getY() << " Matched length " << rex.matchedLength() << endl;
				count++; //one match
				yzDebug() << "Motion after " << motion.after << endl;
				to.setX( idx + ( motion.after ? -1 : rex.matchedLength() ) );
			}
			if ( count >= counter ) break;
			if ( idx == -1 || idx == 0 ) { //no match or we matched at beginning of line => go to previous line for next search
				yzDebug() << "Previous line " << to.getY() - 1 << endl;
				if ( to.getY() == 0 ) break; //stop here
				const QString& ncurrent = view->myBuffer()->textline( to.getY() - 1 );
				if ( ncurrent.isNull() ) return false;
				to.setX( ncurrent.length() );
				to.setY( to.getY() - 1 );
			}
		}
	}
	return true;
}

