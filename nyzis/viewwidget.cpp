/*
	  Copyright (c) 2003 Yzis Team <yzis-dev@yzis.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/**
 * $Id$
 */

#include "debug.h"
#include <qnamespace.h>
#include <ctype.h>
#include "viewwidget.h"
#include "factory.h"

NYZView::NYZView(WINDOW *_window, YZBuffer *b)
	: YZView(b,NYZFactory::session,0)
{
	window = _window;

	update_info();
	wmove(window,0,0 );
	//active symbols for special keycodes
	keypad(window , true);

	// TODO  write something like :       "bernoulli.tex" [noeol] 65L, 1440C
	// in last line (vim-like)
	NYZFactory::self->setCommandLineText (
			b->fileName() + QString(" %1L" ).arg(b->lineCount())
			);
}

NYZView::~NYZView(){
}

void NYZView::handle_event(yz_event e)
{

	switch(e.id) {
		case YZ_EV_INVALIDATE_LINE:
			printLine( e.invalidateline.y );

			wrefresh( window );
//			debug("YZ_EV_SET_LINE: received, line is %d", l);
			break;
		case YZ_EV_SET_CURSOR:
			NYZFactory::self->update_infobar(
				e.setcursor.l+1,
				e.setcursor.c+1,
				e.setcursor.c2+1,
				e.setcursor.percentage
				);
			wmove(window, e.setcursor.l, e.setcursor.c - getCurrent()) ;
			wrefresh( window );
//			debug("YZ_EV_SET_CURSOR: received");
			break;
		case YZ_EV_SET_STATUS:
			NYZFactory::self->setCommandLineText (e.setstatus.text);
			break;
		case YZ_EV_REDRAW: {
			unsigned int i;
			for ( i=getCurrent(); i < ( getCurrent() + mLinesVis ) && i < mBuffer->lineCount(); i++ ) {
				printLine(i);
			}
			i-=getCurrent();
			for ( ; i < mLinesVis ; i++ ) printVoid( i );
		}
			break;
		case YZ_EV_NOOP:
			yzDebug() << "nyzis : event NOOP " << e.id << endl;
			break;
		default:
			yzWarning() << "Unhandled event from yzis core : " <<  e.id;
			break;
	}
}

void NYZView::printVoid( unsigned int relline ) {
	unsigned int i;

	// clipping
	if ( relline > mLinesVis ) return;
	wmove (window, relline, 0);
	waddch(window, '~');
	for (i=1 ; i< w; i++ ) waddch(window, ' ' );
}

void NYZView::printLine( int line ) {

	unsigned int i;
	int sx,sy; // save x,y
	unsigned int relline = line - getCurrent(); // relative line #

	// check
	QString str = mBuffer->data(line);
	if ( str.isNull() ) return;

	// clipping 
	if ( relline > mLinesVis ) return;

	getyx(window,sy,sx); // save cursor

	/* not use addnstr here, will stop at \0  (i guess) */
	wmove (window, relline, 0);
	for (i=0; i<w && i<str.length(); i++)
		waddch(window, str[i].unicode());
//		addch(str[i].unicode());
	for ( ; i< w; i++ ) waddch(window, ' ' );
	wmove(window,sy,sx ); // restore cursor
}



