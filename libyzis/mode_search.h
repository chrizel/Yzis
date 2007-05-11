/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
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

#ifndef YZ_MODE_SEARCH_H
#define YZ_MODE_SEARCH_H

#include "mode.h"
#include "yzismacros.h"
#include "cursor.h"

class YZView;
class YZHistory;

/**
  * @short (forward) search is handled by a special mode.
  */
class YZIS_EXPORT YZModeSearch : public YZMode {
	public:
		YZModeSearch();
		virtual ~YZModeSearch();

		virtual void enter( YZView* view );
		virtual void leave( YZView* view );
		virtual void initModifierKeys();

		virtual CmdState execCommand( YZView* view, const QString& key );

		virtual YZCursor search( YZView* view, const QString& s, bool* found );
		virtual YZCursor search( YZView* view, const QString& s, const YZCursor begin, int* matchlength, bool* found );
		virtual YZCursor replaySearch( YZView* view, bool* found );
		
		YZHistory *getHistory() { return mHistory; }
		
	private:
		YZHistory *mHistory;
		
		//search mode cursors
		YZCursor mSearchBegin;
		bool incSearchFound;
		YZCursor incSearchResult;
};


/**
  * @short Backward search is a special mode.
  */
class YZIS_EXPORT YZModeSearchBackward : public YZModeSearch {
	public :
		YZModeSearchBackward();
		virtual ~YZModeSearchBackward();

		virtual YZCursor search( YZView* view, const QString& s, bool* found );
		virtual YZCursor search( YZView* view, const QString& s, const YZCursor begin, int* matchlength, bool* found );
		virtual YZCursor replaySearch( YZView* view, bool* found );
};


#endif

