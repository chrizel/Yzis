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

/**
 * $Id$
 */

#include "events.h"

YZEvent::YZEvent() {
}

yz_event YZEvent::mkEventStatus(int view, const QString& text) {
	yz_event e;
	e.id=YZ_EV_SET_STATUS;
	e.setstatus.text=text;
	e.view=view;
	return e;
}


yz_event YZEvent::mkEventCursor(int view, int l, int c, int c2, const QString& p) {
	yz_event e;
	e.id=YZ_EV_SET_CURSOR;
	e.setcursor.l=l;
	e.setcursor.c=c;
	e.setcursor.c2=c2;
	e.setcursor.percentage=p;
	e.view = view;
	return e;
}

yz_event YZEvent::mkEventInvalidateLine(int view,int l) {
	yz_event e;
	e.id= YZ_EV_INVALIDATE_LINE;
	e.invalidateline.y=l;
	e.view = view;
	return e;
}

yz_event YZEvent::mkEventRedraw(int view) {
	yz_event e;
	e.id= YZ_EV_REDRAW;
	e.view = view;
	return e;
}

yz_event YZEvent::mkEventNoop() {
	yz_event e;
	e.id=YZ_EV_NOOP;
	return e;
}

