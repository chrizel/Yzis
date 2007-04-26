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

#include "registers.h"
#include "debug.h"

#define dbg()    yzDebug("YZRegisters")
#define err()    yzError("YZRegisters")

YZRegisters::YZRegisters() {
}

void YZRegisters::setRegister( QChar c, const QStringList& value ) {
	mRegisters[ c ] = value;
	dbg() << "Register : " << QString( c ) << " Value : " << value << endl;
}

QStringList& YZRegisters::getRegister( QChar c ) {
	return /*( QStringList& )*/mRegisters[ c ];
}
