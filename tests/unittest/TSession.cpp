/*
    Copyright (c) 2005 Mickael Marchand <mikmak@yzis.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "TSession.h"

void TYZSession::initTestCase() {
	
}

void TYZSession::testCursor2() { 
}

void TYZSession::testCursor() { 
	YZViewCursor *v = new YZViewCursor(YZSession::me->currentView()); 
	QVERIFY(v->bufferX()==1);
}

void TYZSession::cleanupTestCase() {
	
}

#include "TSession.moc"
