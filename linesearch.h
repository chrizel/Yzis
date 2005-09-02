/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Adam Connell <adam@argoncorp.com>,
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

#ifndef YZ_LINE_SEARCH_H
#define YZ_LINE_SEARCH_H

class YZViewCursor;
class YZCursor;
class YZBuffer;
class YZView;

class YZLineSearch {

public:
	/**
	 * Each search is bound to a view
	 */
	YZLineSearch( YZView *_view );
	~YZLineSearch();

	/**
	 * Searches start from current cursor location to line boundary
	 * for times instances of ch and return a YZCursor
	 * containing location of successful search
	 * found is modified to indicate search success/failure
	 * times instances of ch must be found for a search to be successful
	 */

	/**
	 * Search from cursor to end of line for times instances of ch
	 */
	YZCursor forward( const QString& ch, bool& found, unsigned int times );

	/**
	 * Search from cursor to end of line for times instances of ch
	 */
	YZCursor forwardBefore( const QString& ch, bool& found, unsigned int times );

	/**
	 * Return location of ch searching backwards
	 */
	YZCursor reverse( const QString& ch, bool& found, unsigned int times );
	
	/**
	 * Return locate right after ch
	 */
	YZCursor reverseAfter( const QString& ch, bool& found, unsigned int times );

	/**
	 * Searches for the next instance of a previously searched character
	 */
	YZCursor searchAgain( bool &found, unsigned int times );

	/**
	 * Searches for previously searched character in opposite direction
	 */
	YZCursor YZLineSearch::searchAgainOpposite( bool &found, unsigned int times );
	
	/**
	 * Defines types of searches for history
	 * Don't change the order b/c will break searchAgainOpposite
	 */
	enum searchType {
		YZ_LINE_SEARCH_FORWARD = 0,
		YZ_LINE_SEARCH_FBEFORE,
		YZ_LINE_SEARCH_REVERSE,
		YZ_LINE_SEARCH_RAFTER
	};

private:

	/**
	 * View we are working for
	 */
	YZView* mView;
	
	/**
	 * Have we searched for anything yet?
	 */
	bool mFirstTime;
	
	/**
	 * Direction of last search forward/backwards
	 */
	searchType mType;
	
	/**
	 * Last character that was searched for
	 */
	QString mPrevSearched;
	
	/**
	 * Record search information
	 */
	void updateHistory( const QString&, searchType );
};

#endif /*  YZ_LINE_SEARCH_H */
