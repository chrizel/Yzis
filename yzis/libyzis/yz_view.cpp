/**
 * yz_interface.c
 *
 */

#include <stdlib.h>
#include "yz_view.h"

YZView::YZView(YZBuffer *_b, int _lines_vis)
{
	gui_manager = NULL;
	buffer		= _b;
	lines_vis	= _lines_vis;
	cursor = new YZCursor(this);
	current=0;
	current_maxx = 0;
	mode 		= YZ_VIEW_MODE_COMMAND;
	QString line = buffer->find_line(cursor->getY());
	if (line) current_maxx = line.length()-1;

	events_nb_begin = 0;
	events_nb_last = 0;

	buffer->add_view(this);
}

void YZView::setVisibleLines(int nb) {
	if ( lines_vis < nb )
		for (int i=lines_vis; i<nb; i++) {
			QString l=buffer->find_line(i);
			if (!l) continue;
			post_event(mk_event_setline(i,&l));
		}
	lines_vis = nb;
}

/* Used by the buffer to post events */
void YZView::send_char( QChar c)
{
	QString lin;

	if ('\033'==c) {
		//why do we check that ?
		if (cursor->getX() > current_maxx) {
			cursor->setX(current_maxx);
			update_cursor();
		}
		mode = YZ_VIEW_MODE_COMMAND;
		post_event(mk_event_setstatus("Command mode"));
		return;
	}
	switch(mode) {
		case YZ_VIEW_MODE_INSERT:
			/* handle adding a char */
			buffer->add_char(cursor->getX(),cursor->getY(),c);
			cursor->incX();
			lin = buffer->find_line(cursor->getY());
			if ( lin ) current_maxx = lin.length()-1;
			update_cursor();
			return;
		case YZ_VIEW_MODE_REPLACE:
			/* handle replacing a char */
			buffer->chg_char(cursor->getX(),cursor->getY(),c);
			cursor->incX();
			lin = buffer->find_line(cursor->getY());
			if ( lin ) current_maxx = lin.length()-1;
			update_cursor();
			return;
		case YZ_VIEW_MODE_COMMAND:
			/* will be handled after the switch */
			break;
		default:
			/* ?? */
			//printf("Currently unknown MODE\n"); error("unknown mode, ignoring");
			return;
	};
	/* ok, here we now we're in command */
	switch (c) {
		default:
			post_event(mk_event_setstatus("*Unknown command*"));
			break;
		case 'A': /* append -> insert mode */
			/* go to end of line */
			cursor->setX(current_maxx);
		/* pass through */
		case 'a': /* append -> insert mode */
			cursor->incX();
		/* pass through */
		case 'i': /* insert mode */
			mode = YZ_VIEW_MODE_INSERT;
			post_event(mk_event_setstatus("-- INSERT --"));
			update_cursor();
			break;
		case 'R': /* -> replace mode */
			mode = YZ_VIEW_MODE_REPLACE;
			post_event(mk_event_setstatus("-- REPLACE --"));
			break;
		case 'j': /* move down */
			if (cursor->getY() < buffer->text.count()-1) {
				cursor->incY();
				lin = buffer->find_line(cursor->getY());
				if ( lin ) current_maxx = lin.length()-1; 
				//cursor->setX( cursor_x_ghost );
				if (cursor->getX() > current_maxx) cursor->setX( current_maxx );
				if (cursor->getX() < 0) cursor->setX( 0 );
				update_cursor();
			}
			break;
		case 'k': /* move up */
			if (cursor->getY() > 0) {
				cursor->decY();
				lin = buffer->find_line(cursor->getY());
				if ( lin ) current_maxx = lin.length()-1;
				//cursor->setX( cursor_x_ghost );
				if (cursor->getX() > current_maxx) cursor->setX( current_maxx );
				if (cursor->getX() < 0) cursor->setX( 0 );
				update_cursor();
			}
			break;
		case 'h': /* move left */
			if (cursor->getX() > 0) {
				cursor->decX();
				//cursor_x_ghost = cursor->getX();
				update_cursor();
			}
			break;
		case 'l': /* move right */
			if (cursor->getX() < current_maxx) {
				cursor->incX();
				//cursor_x_ghost = cursor->getX();
				update_cursor();
			}
			break;
		case '0': /* move beginning of line */
			//cursor_x_ghost = 0;
			cursor->setX( 0 );
			update_cursor();
			break;
		case '$': /* move end of line */
			if (! (current_maxx < 0)) {
				//cursor_x_ghost = current_maxx;
				cursor->setX( current_maxx );
				update_cursor();
			}
			break;
	}
}

void YZView::update_cursor(void)
{
	post_event( mk_event_setcursor(cursor->getX(),cursor->getY()-current));
}

yz_event *YZView::fetch_event(int idx)
{
	if ( idx!=-1 )
		return &events[idx];

	if (events_nb_last==events_nb_begin)
		return NULL;

	yz_event *e = &events[events_nb_begin++];

	if (events_nb_begin>=YZ_EVENT_EVENTS_MAX)
		events_nb_last=0;

	return e;
}

void YZView::post_event (yz_event e)
{
	events[events_nb_last++] = e;
	if (events_nb_last>=YZ_EVENT_EVENTS_MAX)
		events_nb_last=0;
	if ( gui_manager )
		gui_manager->postEvent( e );
}

void YZView::register_manager ( Gui *mgr )
{
	gui_manager = mgr;
}
