/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>
 *  Mickael Marchand <marchand@kde.org>
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

#ifndef YZ_SEARCH_H
#define YZ_SEARCH_H

#include "cursor.h"
#include "view.h"
#include "buffer.h"

class YZView;
class YZCursor;
class YZBuffer;

/**
 * YZSearch : 
 *	Searches are common to all buffers and views. 
 *	When doing a search if hlsearch is set, we have to highlight
 *	all matching strings on all views of each buffers.
 */

class YZSearch {

	public :
		YZSearch();
		~YZSearch();

		/**
		 * search after current cursor position
		 */
		YZCursor forward( YZView* mView, const QString& pattern, bool* found, YZCursor* from = NULL );

		/**
		 * search before current cursor position
		 */
		YZCursor backward( YZView* mView, const QString& pattern, bool* found, YZCursor* from = NULL );

		/**
		 * replay search forward
		 */
		YZCursor replayForward( YZView* mView, bool* found, YZCursor* from = NULL, bool skipline = false );

		/**
		 * replay search backward
		 */
		YZCursor replayBackward( YZView* mView, bool* found, YZCursor* from = NULL, bool skipline = false );

		/**
		 * Highlight given line
		 */
		void highlightLine( YZBuffer* buffer, unsigned int line );

		/**
		 * return current search
		 */
		const QString& currentSearch() const;

		/**
		 * true if currentSearch is not null
		 */
		bool active();


	private :

		void setCurrentSearch( const QString& pattern );
		YZCursor doSearch( YZView* mView, YZCursor* from, const QString& pattern, bool reverse, bool skipline, bool* found );
		void highlightSearch( YZView* mView, YZSelectionMap searchMap );

		QString mCurrentSearch;

};

#endif

