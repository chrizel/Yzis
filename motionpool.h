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

#ifndef YZ_MOTIONPOOL
#define YZ_MOTIONPOOL

#include <qmap.h>
#include <qregexp.h>
typedef QMap<QString,QRegExp> YZMotion;

/**
 * This is the main place for handling motion objects 
 * This is used for operators commands like "d".
 * The goal is to handle ocmmands like : "2d3w" to delete twice 3 words 
 * Basics: 
 * We use RegExps to define the motion objects ( but this can be extended ),
 * each object has an identifier key like "w" for words, "p" for paragraphs
 * etc...
 */
class YZMotionPool {
	public:
		YZMotionPool();
		~YZMotionPool();

		//adds a new motion to the pool of known motions :)
		void addMotion(const YZMotion& regexp, const QString& key);

		void initPool();

	private:
		QMap<QString,YZMotion> pool;
};

#endif
