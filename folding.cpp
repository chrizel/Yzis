/*  This file is part of the Yzis libraries
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
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
 * $Id$
 */

#include "folding.h"

#include "debug.h"

YZFoldPool::YZFoldPool( YZView* view ) {
	m_view = view;
}
YZFoldPool::~YZFoldPool() {
}

void YZFoldPool::create( int from, int to ) {
	yzDebug() << "FOLDING: create from " << from << " to " << to << endl;
	int head = from;
	bool need_update = true;
	if ( isHead( from ) || contains( from, &head ) ) {
		if ( m_folds[ head ].to < to ) {
			m_folds[ head ].to = to;
		} else {
			need_update = false;
		}
	} else {
		YZFold fold;
		fold.to = to;
		fold.opened = false;
		m_folds.insert( head, fold );
	}
	if ( need_update ) {
		m_view->sendRefreshEvent();
	}
	yzDebug() << "" << *this;
}

bool YZFoldPool::isHead( int line ) const {
	return m_folds.contains( line );
}
bool YZFoldPool::contains( int line, int* head ) const {
	bool contains = false;
	QList<int> keys = m_folds.keys();
	if ( keys.size() > 0 ) {
		int i;
		for( i = keys.size() - 1; !contains && i >= 0 && m_folds[ keys[ i ] ].to >= line; --i )
			contains = ( keys[ i ] < line );
		if ( contains && head != NULL )
			*head = keys[ ++i ];
	}
	return contains;
}
bool YZFoldPool::isFolded( int line, int* head ) const {
	int fh;
	bool ret = contains( line, &fh );
	if ( head != NULL )
		*head = fh;
	return ret && !m_folds[ fh ].opened;
}

int YZFoldPool::lineAfterFold( int line ) const {
	int head;
	if ( contains( line, &head ) ) {
		if ( !m_folds[ head ].opened )
			return m_folds[ head ].to + 1;
	}
	return line;
}
int YZFoldPool::lineHeadingFold( int line ) const {
	int head;
	if ( contains( line, &head ) ) {
		return head;
	}
	return line;
}

YZDebugStream& operator<<( YZDebugStream& out, const YZFoldPool& f ) {
	QList<int> keys = f.m_folds.keys();
	for( int i = 0; i < keys.size(); ++i ) {
		out << "fold from line " << keys[ i ] 
			<< " to line " << f.m_folds[ keys[ i ] ].to
			<< ". Opened ? " << f.m_folds[ keys[ i ] ].opened << endl;
	}
	return out;
}


