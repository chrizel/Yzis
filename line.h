/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Lucijan Busch <luci@yzis.org>,
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

#ifndef YZIS_LINE_H
#define YZIS_LINE_H

#include <qobject.h>
#include <qmap.h>

/**
 * this class represents a line in the buffer
 * it holds the actual data and metadata
 */
class YZLine
{
	public:
		YZLine(const QString &l);
		YZLine();
		~YZLine();

		const QString& data() const { return mData; }
		void setData(const QString &data);
		int length() { return mData.length(); }
		inline const QMemArray<short> &ctxArray () const { return m_ctx; };
		inline void setContext (QMemArray<short> &val) { m_ctx.assign (val); }
		inline bool hlLineContinue () const { return m_flags & YZLine::flagHlContinue; }

		inline void setHlLineContinue (bool cont)
		{
			if (cont) m_flags = m_flags | YZLine::flagHlContinue;
			else m_flags = m_flags & ~ YZLine::flagHlContinue;
		}


		void setAttribs(uchar attribute, uint start, uint end);
		inline uchar *attributes () const { return mAttributes.data(); }

		enum Flags
		{
			flagNoOtherData = 0x1, // ONLY INTERNAL USE, NEVER EVER SET THAT !!!!
			flagHlContinue = 0x2,
			flagVisible = 0x4,
			flagAutoWrapped = 0x8
		};

	private:
		QString mData;

		//rendering settings for each char
		QMemArray<uchar> mAttributes;
		//contexts for HL
		QMemArray<short> m_ctx;
		/**
		  Some bools packed
		  */
		uchar m_flags;
};

#endif
