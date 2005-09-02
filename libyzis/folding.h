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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

/**
 * $Id$
 */

#ifndef YZ_FOLDING_H
#define YZ_FOLDING_H

#include <QMap>

#include "view.h"

struct YZFold {
	unsigned int to;
	bool opened;
};

class YZFoldPool {

	friend YZDebugStream& operator<<( YZDebugStream& out, const YZFoldPool& f );

	public:
		YZFoldPool( YZView* view );
		virtual ~YZFoldPool();

		/**
		 * create a new fold
		 */
		void create( unsigned int from, unsigned int to );

		/**
		 * returns true if line is the head of a fold
		 */
		bool isHead( unsigned int line ) const;
		
		/**
		 * returns true if line is inside a fold (head excluded)
		 * if head is not NULL, it will contains the line heading the fold
		 */
		bool contains( unsigned int line, unsigned int* head = NULL ) const;

		/**
		 * same as contains && fold is closed 
		 *  => line should be hidden
		 */
		bool isFolded( unsigned int line, unsigned int* head = NULL ) const;

		/**
		 * returns the line number under the fold containing line. If line isn't inside a fold, returns line
		 */
		unsigned int lineAfterFold( unsigned int line ) const;

		/**
		 * returns the head of the fold containing line. If line isn't inside a fold, returns line
		 */
		unsigned int lineHeadingFold( unsigned int line ) const;

	private:
		YZView* m_view;
		QMap<unsigned int, YZFold> m_folds;

};

#endif

