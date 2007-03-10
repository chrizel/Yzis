/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Lucijan Busch <luci@yzis.org>,
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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

#ifndef YZIS_LINE_H
#define YZIS_LINE_H

#include <QVector>

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
		int length() const { return mData.length(); }
		inline const QVector<short> &ctxArray () const { return m_ctx; };
		inline void setContext (QVector<short> &val) { m_ctx = val; }
		inline bool hlLineContinue () const { return m_flags & YZLine::flagHlContinue; }

		inline void setHlLineContinue (bool cont)
		{
			if (cont) m_flags = m_flags | YZLine::flagHlContinue;
			else m_flags = m_flags & ~ YZLine::flagHlContinue;
		}

		void clearAttributes() { mAttributesList.clear(); }
		void addAttribute ( int start, int length, int attribute );


		inline uchar *attributes () { return mAttributes.data(); }
		bool initialized() const { return m_initialized; }

		int firstChar() const;
		int lastChar() const;
		int nextNonSpaceChar(uint pos) const;
		int previousNonSpaceChar(uint pos) const;

	private:
		enum Flags
		{
//			flagNoOtherData = 0x1, // ONLY INTERNAL USE, NEVER EVER SET THAT !!!!
			flagHlContinue = 0x2,
			flagVisible = 0x4,
			flagAutoWrapped = 0x8
		};
		QString mData;

		//rendering settings for each char
		QVector<uchar> mAttributes;
		QVector<int> mAttributesList;
		//contexts for HL
		QVector<short> m_ctx;
		/**
		  Some bools packed
		  */
		uchar m_flags;
		bool m_initialized;
};

#endif
