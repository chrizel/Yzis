/* This file is part of the Yzis libraries
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

#include <qglobal.h>
#if QT_VERSION < 0x040000
#include <qstringlist.h>
#include <qmap.h>
#else
#include <QChar>
#include <QStringList>
#include <QMap>
#endif

#ifndef YZ_REGISTERS
#define YZ_REGISTERS

class YZRegisters {
	public:
		/**
		 * Default constructor
		 */
		YZRegisters();

		/**
		 * Fills the register @param r with the @param value
		 */
		void setRegister( QChar r, const QStringList& value );

		/**
		 * Gets the value of register @param r
		 * Returns a QString containing the register content
		 */
		QStringList& getRegister ( QChar r );

#if QT_VERSION < 0x040000
		/**
		 * Gets the list of registers
		 */
		QValueList<QChar> keys() { return mRegisters.keys(); }

		/**
		 * Gets the list of values
		 */
		QValueList<QStringList> values() { return mRegisters.values(); }
#else
		/**
		 * Gets the list of registers
		 */
		QList<QChar> keys() { return mRegisters.keys(); }

		/**
		 * Gets the list of values
		 */
		QList<QStringList> values() { return mRegisters.values(); }
#endif

	private:
		QMap<QChar,QStringList> mRegisters;
};

#endif
