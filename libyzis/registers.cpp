/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Mickael Marchand <marchand@kde.org>
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

#include "registers.h"
#include "debug.h"

YZRegisters::YZRegisters() {
}

void YZRegisters::setRegister( QChar c, const QStringList& value ) {
	mRegisters[ c ] = value;
	yzDebug() << "Register : " << c.latin1() << " Value : " << value << endl;
}

QStringList& YZRegisters::getRegister( QChar c ) const {
	return ( QStringList& )mRegisters[ c ];
}
