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

#ifndef YZ_EVENTS_H
#define YZ_EVENTS_H
/**
 * $Id$
 *
 * yzis events
 *
 */

#include <qstring.h>

class QString;

/** list of all events */
enum yz_events {
	YZ_EV_INVALIDATE_LINE,
	YZ_EV_SET_CURSOR,
	YZ_EV_SET_STATUS,
	YZ_EV_REDRAW,
	YZ_EV_NOOP //no, nothing :)
};

/**
 * Be careful, when the core uses yz_event_invalidateline, it is of its responsability
 * to handle the cursor position
 * for a whole screen refresh
 * 	yz_event_invalidateline for line 1
 * 	....
 * 	yz_event_invalidateline for line n
 *
 * 	yz_event_setcursor(wherever)
 *
 * for a modified line
 * 	yz_event_invalidateline for line y
 * 	yz_event_setcursor(wherever)
 *
 * There will be some special events for that, but not yet
 */

/**
 * Structure to signal the modification of a line of text
 */
struct yz_event_invalidateline {
	int	y;
	yz_event_invalidateline() {
		y=0;
	}
	yz_event_invalidateline(const yz_event_invalidateline& e) {
		y=e.y;
	}
};

/**
 * Structure to handle a cursor movement
 * x and y are NOT relative to the view
 */
struct yz_event_setcursor {
	int x,y,y2;
	QString percentage;
	yz_event_setcursor() {
		percentage=QString::null;
		x=y=y2=0;
	}
	yz_event_setcursor(const yz_event_setcursor& e) {
		percentage=e.percentage;
		x=e.x;
		y=e.y;
		y2=e.y2;
	}
};

/**
 * Signal to ask the GUI to repaint the whole screen
 */
struct yz_event_redraw {
	yz_event_redraw() {
	}
	yz_event_redraw(const yz_event_redraw& e) {
	}
};

/**
 * Signal to change the status bar text
 */
struct yz_event_setstatus {
	QString text;
	yz_event_setstatus() {
		text=QString::null;
	}
	yz_event_setstatus(const yz_event_setstatus& e) {
		text=e.text;
	}
};

/** The yz_event struct, mainly an union of all event args
 *
 * exemple of use : 
 * (e is of type yz_event)
 * switch (e.id) {
 case YZ_EV_SETLINE:
 here we use e.u.invalidateline.line_nb;
 break;

 case YZ_EV_SETCURSOR:
 }
 */
struct yz_event_t {
	int view;
	enum yz_events		id;
	struct yz_event_t	*next;
	struct yz_event_invalidateline		invalidateline;
	struct yz_event_setcursor	setcursor;
	struct yz_event_setstatus	setstatus;
	struct yz_event_redraw	redraw;

	yz_event_t() {
		view=0;
	}
	yz_event_t(const yz_event_t& e) {
		view=e.view;
		id=e.id;
		next=e.next;
		invalidateline=e.invalidateline;
		setcursor = e.setcursor;
		setstatus = e.setstatus;
		redraw = e.redraw;
	}
};

typedef struct yz_event_t yz_event;

/**
 * Class to handle communication between the core and the GUIs.
 * There are for now 5 event types defined :
 * <UL>
 * <LI>Status : updates the statusbar</LI>
 * <LI>Cursor : moves the cursor</LI>
 * <LI>InvalidateLine : a line has been changed</LI>
 * <LI>Redraw : repaint the whole visible screen</LI>
 * <LI>Noop : a fake event like the "idle" loop</LI>
 * </UL>
 */
class YZEvent {
	public:
		YZEvent();

		static yz_event mkEventStatus(int v,const QString&);
		static yz_event mkEventCursor(int v, int x, int y, int y2, const QString&);
		static yz_event mkEventInvalidateLine(int,int);
		static yz_event mkEventRedraw(int);
		static yz_event mkEventNoop();
};

#endif /*  YZ_EVENTS_H */

