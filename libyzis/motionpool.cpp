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

static YZMotion nullMotion("",REGEXP,0,0,false);

YZMotionPool::YZMotionPool(){
}

YZMotionPool::~YZMotionPool() {
	pool.clear();
}

void YZMotionPool::initPool() {
	addMotion (YZMotion( "\\b\\w+\\b", REGEXP, 0, 0, false ) , "[0-9]*w" );
	addMotion (YZMotion( "\\b\\w+\\b", REGEXP, 0, 0, true ), "[0-9]*b");
	addMotion (YZMotion( "$", REGEXP, 0, 0, false ), "\\$" );
	addMotion (YZMotion( "^", REGEXP, 0, 0, true ), "0");
	addMotion (YZMotion( "", RELATIVE, -1, 0, true ), "[0-9]*h");
	addMotion (YZMotion( "", RELATIVE, 1, 0, false ), "[0-9]*l");
	addMotion (YZMotion( "", RELATIVE, 0, -1, true ), "[0-9]*k");
	addMotion (YZMotion( "", RELATIVE, 0, 1, false ), "[0-9]*j");
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

bool YZMotionPool::applyMotion( const QString &inputsMotion, YZView *view, bool *backward, YZCursor *result ) {
	bool ok = false;
	YZMotion& motion = findMotion(inputsMotion, &ok);
	if ( !ok ) return false;
	*backward = motion.backward;
	if ( motion.type == REGEXP )
		return applyRegexpMotion( inputsMotion, motion, view, result );
	else if ( motion.type == RELATIVE )
		return applyRelativeMotion( inputsMotion, motion, view, result );
	return false;
}

bool YZMotionPool::applyRelativeMotion( const QString &inputsMotion, YZMotion& motion, YZView *view, YZCursor *result ) {
	int counter = 1; //number of times we have to match
	QRegExp rx ( "([0-9]+).+" );
	if ( rx.exactMatch( inputsMotion ) ) counter = rx.cap(1).toInt();
	yzDebug() << "Loop " << counter << " times" << endl;
	QRegExp rex( motion.rex );
	result->setX(view->getBufferCursor()->getX());
	result->setY(view->getBufferCursor()->getY());
	int count = 0 ;

	while (count < counter) {
		result->setX( result->getX() + motion.x );
		result->setY( result->getY() + motion.y );
		count++;
	}
	if ( count ) return true;

	return false;
}

bool YZMotionPool::applyRegexpMotion( const QString &inputsMotion, YZMotion& motion, YZView *view, YZCursor *result ) {
	int counter = 1; //number of times we have to match
	QRegExp rx ( "([0-9]+).+" );
	if ( rx.exactMatch( inputsMotion ) ) counter = rx.cap(1).toInt();
	yzDebug() << "Loop " << counter << " times" << endl;
	QRegExp rex( motion.rex );
	result->setX(view->getBufferCursor()->getX());
	result->setY(view->getBufferCursor()->getY());
	int idx=-1;
	int count = 0 ;
	if ( ! motion.backward ) {
		yzDebug() << "Forward motion" <<endl;
		while (count < counter) {
			const QString& current = view->myBuffer()->textline( result->getY() );
			if ( current.isNull() ) return false;
			idx = rex.search( current, result->getX() );
			if ( idx != -1 ) {
				yzDebug() << "Match at " << idx << " Matched length " << rex.matchedLength() << endl;
				count++; //one match
				result->setX( idx + rex.matchedLength() + 1 );
			} else {
				if ( result->getY() >= view->myBuffer()->lineCount() ) break;
				result->setX( 0 );
				result->setY( result->getY() + 1 );
			}
		}
	} else {
		yzDebug() << "Backward motion" <<endl;
		while ( count < counter ) {
			const QString& current = view->myBuffer()->textline( result->getY() );
			if ( current.isNull() ) return false;
			idx = rex.searchRev( current, result->getX() >= 1 ? result->getX() - 1 : result->getX() );
			if ( idx != -1 ) {
				yzDebug() << "Match at " << idx << " Matched length " << rex.matchedLength() << endl;
				count++; //one match
				result->setX( idx );
			}
			if ( idx == -1 || idx == 0 ) { //no match or we matched at beginning of line => go to previous line for next search
				yzDebug() << "Previous line " << result->getY() - 1 << endl;
				if ( result->getY() == 0 ) break; //stop here
				const QString& ncurrent = view->myBuffer()->textline( result->getY() - 1 );
				if ( ncurrent.isNull() ) return false;
				result->setX( ncurrent.length() );
				result->setY( result->getY() - 1 );
			}
		}
	}
	return true;
}

