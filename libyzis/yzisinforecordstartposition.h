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

#ifndef YZISINFORECORDSTARTPOSITION_H
#define YZISINFORECORDSTARTPOSITION_H

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluevector.h>

#include "cursor.h"
#include "yzisinforecord.h"

class YZYzisinfoRecord;

class YZYzisinfoRecordStartPosition : public YZYzisinfoRecord {
	public:
		YZYzisinfoRecordStartPosition();
		YZYzisinfoRecordStartPosition( YZYzisinfoRecordStartPosition & copy );
		YZYzisinfoRecordStartPosition( const QString & keyword, const QString & filename, const unsigned int x, const unsigned int y );
		~YZYzisinfoRecordStartPosition();
		YZYzisinfoRecordStartPosition & operator=( YZYzisinfoRecordStartPosition & copy );
		QString & filename();
		YZCursor * position();
		void setFilename( const QString & filename );
		void setPosition( const unsigned int x, const unsigned int y );

	private:
		QString mFilename;
		YZCursor * mPosition;
};

#endif // YZISINFORECORDSTARTPOSITION_H
