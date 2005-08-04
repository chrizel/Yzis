/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
 *  Copyright (C) 2003-2004 Philippe Fremy <phil@freehackers.org>
 *  Copyright (C) 2003-2004 Pascal "Poizon" Maillard <poizon@gmx.at>
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
 *  Copyright (C) 2005 Erlend Hamberg <ehamberg@online.no>
 *  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
 *  Copyright (C) 2005 Craig Howard	<craig@choward.ca>
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
 * $Id: mode_command.cpp 1927 2005-06-14 09:10:34Z scottn $
 */

#include "portability.h"

#include "tags_stack.h"

#include "debug.h"

#include "buffer.h"
#include "cursor.h"
#include "session.h"
#include "yzisinfojumplistrecord.h"

static YZYzisinfoJumpListRecord getRecord()
{
	YZBuffer *buffer = YZSession::me->currentView()->myBuffer();
	YZCursor *cursor = YZSession::me->currentView()->getCursor();
	return YZYzisinfoJumpListRecord( buffer->fileName(), cursor->x(), cursor->y() );
}

YZTagStack::YZTagStack()
{
}

YZTagStack::~YZTagStack()
{
}

void YZTagStack::push()
{
	mCurrentTags.push_back(qMakePair(MatchingTagsType(), static_cast<unsigned int>(0)));
	mStack.push_back(getRecord());
}

const YZYzisinfoJumpListRecord *YZTagStack::getHead() const
{
	if ( !mStack.empty() ) {
		return &mStack.back();
	} else {
		return NULL;
	}
}

const YZTagStackItem *YZTagStack::moveToNext()
{
	const YZTagStackItem *result = NULL;
	MatchingStackItem &pair = mCurrentTags.back();
	
	if ( pair.second < pair.first.size() - 1 ) {
		++pair.second;
		result = &pair.first[ pair.second ];
	}
	
	return result;
}

const YZTagStackItem *YZTagStack::moveToPrevious()
{
	const YZTagStackItem *result = NULL;
	MatchingStackItem &pair = mCurrentTags.back();
	
	if ( pair.second != 0 ) {
		--pair.second ;
		result = &pair.first[ pair.second ];
	}
	
	return result;
}

void YZTagStack::pop()
{
	mStack.pop_back();
	mCurrentTags.pop_back();
}

bool YZTagStack::empty() const
{
	return mStack.empty();
}

void YZTagStack::storeMatchingTags(const YZVector<YZTagStackItem> &tags)
{
	MatchingStackItem &pair = mCurrentTags.back();
	pair.first = tags;
	pair.second = static_cast<unsigned int>(0);
}

unsigned int YZTagStack::getNumMatchingTags() const
{
	if ( !mCurrentTags.empty() ) {
		const MatchingStackItem &pair = mCurrentTags.back();
		return pair.first.size();
	} else {
		return 0;
	}
}

unsigned int YZTagStack::getNumCurMatchingTag() const
{
	if( !mCurrentTags.empty() ) {
		const MatchingStackItem &pair = mCurrentTags.back();
		return pair.second;
	} else {
		return 0;
	}
}
