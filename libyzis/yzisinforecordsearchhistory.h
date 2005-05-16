/* This file is part of the Yzis libraries
 *  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
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

#ifndef YZISINFORECORDSEARCHHISTORY_H
#define YZISINFORECORDSEARCHHISTORY_H

#include <qstring.h>
#include <qstringlist.h>
#if QT_VERSION < 0x040000
#include <qvaluevector.h>
#else
#include <QVector>
#endif

#include "cursor.h"
#include "yzisinforecord.h"

class YZYzisinfoRecord;

class YZYzisinfoRecordSearchHistory : public YZYzisinfoRecord {
	public:
		YZYzisinfoRecordSearchHistory();
		YZYzisinfoRecordSearchHistory( YZYzisinfoRecordSearchHistory & copy );
		YZYzisinfoRecordSearchHistory( const QString & keyword, const QString & filename, const unsigned int x, const unsigned int y );
		~YZYzisinfoRecordSearchHistory();
		YZYzisinfoRecordSearchHistory & operator=( YZYzisinfoRecordSearchHistory & copy );
		int indexCount() { return mCursorData.count(); }
		QString & filename();
		YZCursor * position();
		YZCursor * previousSearchPosition();
		void setFilename( const QString & filename );
		void setPosition( const unsigned int x, const unsigned int y );

	private:
		QString mFilename;
		int mIndex;
#if QT_VERSION < 0x040000
		QValueVector<YZCursor*> mCursorData;
#else
		QVector<YZCursor*> mCursorData;
#endif
};

#endif // YZISINFORECORDSEARCHHISTORY_H
