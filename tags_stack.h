/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
 *  Copyright (C) 2003-2004 Philippe Fremy <phil@freehackers.org>
 *  Copyright (C) 2003-2004 Pascal "Poizon" Maillard <poizon@gmx.at>
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
 *  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
 *  Copyright (C) 2005 Craig Howard <craig@choward.ca>
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

#ifndef YZ_TAGS_STACK_H
#define YZ_TAGS_STACK_H

#include <QPair>
#include <QVector>

class YZYzisinfoJumpListRecord;

struct YZTagStackItem {
	YZTagStackItem() {}
	YZTagStackItem(const YZTagStackItem &o) : pattern(o.pattern), filename(o.filename) {}
	YZTagStackItem(const QString &p, const QString &f) : pattern(p), filename(f) {}
	QString pattern;
	QString filename;
};

class YZTagStack {
	public:
		YZTagStack();
		virtual ~YZTagStack();
		
		void push();
		const YZYzisinfoJumpListRecord *getHead() const;
		const YZTagStackItem *moveToPrevious();
		const YZTagStackItem *moveToNext();
		void pop();
		bool empty() const;
		
		unsigned int getNumMatchingTags() const;
		unsigned int getNumCurMatchingTag() const;
		
		void storeMatchingTags(const QVector<YZTagStackItem> &tags);
		
	private:
		typedef QVector<YZYzisinfoJumpListRecord> StackType;
		typedef QVector<YZTagStackItem> MatchingTagsType;
		typedef QPair<MatchingTagsType, unsigned int> MatchingStackItem;
		typedef QVector<MatchingStackItem> MatchingStackType;
		
		StackType mStack;
		MatchingStackType mCurrentTags;
};

#endif
