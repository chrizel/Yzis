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

#include "motionpool.h"
#include "view.h"
#include "cursor.h"

YZMotionPool::YZMotionPool(){
}

YZMotionPool::~YZMotionPool() {
	pool.clear();
}

void YZMotionPool::initPool() {
	YZMotion t = { "\b.*\b", REGEXP, 0, 0};
	addMotion (t, "([0-9]*)w" ); //drop the decimals ?
}

void YZMotionPool::addMotion(const YZMotion& regexp, const QString& key){
	pool.insert( key,regexp );
}

void YZMotionPool::findMotion ( const QString& inputs, YZMotion *motion ) {
	QMap<QString,YZMotion>::iterator it;
	for ( it = pool.begin(); it!=pool.end(); it++) {
		YZMotion t = it.data();
		QRegExp rex ( it.key() );
		if ( rex.exactMatch( inputs ) )
			motion = &t;
	}
	motion = NULL; //not found
}

void YZMotionPool::applyMotion( const YZMotion& motion, YZView *view ) {
	YZCursor* cursor = new YZCursor( view->getCursor() );
	QRegExp rex( motion.rex );
	int idx=-1;
	while ( idx == -1 ) {
		const QString& current = view->myBuffer()->textline( cursor->getY() );
		if ( current.isNull() ) return;
		idx = rex.search( current, cursor->getX() );
		cursor->setX( 0 );
		cursor->setY( cursor->getY() + 1 );
	}
}

