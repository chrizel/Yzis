/**
 * $id$
 */

#include <cstdlib>
#include "yz_view.h"

YZView::YZView(YZBuffer *_b, int _lines_vis) {
	gui_manager = NULL;
	buffer		= _b;
	lines_vis	= _lines_vis;
	cursor = new YZCursor(this);
	current_maxx = 0;
	mode 		= YZ_VIEW_MODE_COMMAND;
	//could it be something else than 0 ?
	current = cursor->getY();
	QString line = buffer->findLine(current);
	if (!line.isNull()) current_maxx = line.length()-1;

	events_nb_begin = 0;
	events_nb_last = 0;

	buffer->addView(this);
}

void YZView::setVisibleLines(int nb) {
	if ( lines_vis < nb )
		for (int i=lines_vis; i<nb; i++) {
			QString l=buffer->findLine(i);
			if (l.isNull()) continue;
			postEvent(mk_event_setline(i,&l));
		}
	lines_vis = nb;
}

/* Used by the buffer to post events */
void YZView::sendChar( QChar c) {
	QString lin;

	if ('\033'==c) {
		//why do we check that ?
		if (cursor->getX() > current_maxx) {
			cursor->setX(current_maxx);
			updateCursor();
		}
		mode = YZ_VIEW_MODE_COMMAND;
		postEvent(mk_event_setstatus("Command mode"));
		return;
	}
	switch(mode) {
		case YZ_VIEW_MODE_INSERT:
			/* handle adding a char */
			buffer->addChar(cursor->getX(),cursor->getY(),c);
			cursor->incX();
			lin = buffer->findLine(cursor->getY());
			if ( !lin.isNull() ) current_maxx = lin.length()-1;
			updateCursor();
			return;
		case YZ_VIEW_MODE_REPLACE:
			/* handle replacing a char */
			buffer->chgChar(cursor->getX(),cursor->getY(),c);
			cursor->incX();
			lin = buffer->findLine(cursor->getY());
			if ( !lin.isNull() ) current_maxx = lin.length()-1;
			updateCursor();
			return;
		case YZ_VIEW_MODE_COMMAND:
			/* will be handled after the switch */
			break;
		default:
			/* ?? */
			//printf("Currently unknown MODE\n"); error("unknown mode, ignoring");
			return;
	};
	/* ok, here now we're in command */
	previous_chars+=c;
	//execute the command
	if ( session ) 
		session->getPool()->execCommand(this, previous_chars);
}
	
#if 0
	if ( ! previous_chars.isEmpty() ) {
		//append the char, then check is the current previous_chars has a sense
		previous_chars+=c;
		int step=0;
		QString number;
		while ( previous_chars[ step ].isDigit() )
			number+=previous_chars.at( step++ );
		//now we have the number in 'number' and the command from step index
		//the question is : did I forget a possibility ?
		//check the command name now
		switch ( ( ( QChar )previous_chars.at(step) ).latin1() ) {
		}
	} else {
		//one keystroke commands are below
		switch (c) {
			default:
				postEvent(mk_event_setstatus("*Unknown command*"));
				break;
			case 'A': /* append -> insert mode */
				/* go to end of line */
				cursor->setX(current_maxx);
				mode = YZ_VIEW_MODE_INSERT;
				postEvent(mk_event_setstatus("-- INSERT --"));
				break;
			case 'a': /* append -> insert mode */
				cursor->incX();
				mode = YZ_VIEW_MODE_INSERT;
				postEvent(mk_event_setstatus("-- INSERT --"));
				break;
			case 'i': /* insert mode */
				mode = YZ_VIEW_MODE_INSERT;
				postEvent(mk_event_setstatus("-- INSERT --"));
				updateCursor();
				break;
			case 'R': /* -> replace mode */
				mode = YZ_VIEW_MODE_REPLACE;
				postEvent(mk_event_setstatus("-- REPLACE --"));
				break;
			case 'j': /* move down */
				moveDown();
				break;
			case 'k': /* move up */
				if (cursor->getY() > 0) {
					cursor->decY();
					lin = buffer->findLine(cursor->getY());
					if ( !lin.isNull() ) current_maxx = lin.length()-1;
					if (cursor->getX() > current_maxx) cursor->setX( current_maxx );
					if (cursor->getX() < 0) cursor->setX( 0 );
					//check if we need to scroll
					if ( current > cursor->getY() ) {
						//scroll up => GUI
						if ( gui_manager )
							gui_manager->scrollUp(); //one line up
						current--;
					}
					updateCursor();
				}
				break;
			case 'h': /* move left */
				if (cursor->getX() > 0) {
					cursor->decX();
					updateCursor();
				}
				break;
			case 'l': /* move right */
				if (cursor->getX() < current_maxx) {
					cursor->incX();
					updateCursor();
				}
				break;
			case '0': /* move beginning of line */
				cursor->setX( 0 );
				updateCursor();
				break;
			case '$': /* move end of line */
				if (! (current_maxx < 0)) {
					cursor->setX( current_maxx );
					updateCursor();
				}
				break;
		}
	}
#endif

void YZView::updateCursor(int x, int y) {
	if ( x!=-1 ) cursor->setX( x );
	if ( y!=-1 ) cursor->setY( y );
	postEvent( mk_event_setcursor(cursor->getX(),cursor->getY()-current));
}

yz_event *YZView::fetchEvent(int idx) {
	if ( idx!=-1 )
		return &events[idx];

	if (events_nb_last==events_nb_begin)
		return NULL;

	yz_event *e = &events[events_nb_begin++];

	if (events_nb_begin>=YZ_EVENT_EVENTS_MAX)
		events_nb_last=0;

	return e;
}

void YZView::postEvent (yz_event e) {
	events[events_nb_last++] = e;
	if (events_nb_last>=YZ_EVENT_EVENTS_MAX)
		events_nb_last=0;
	if ( gui_manager )
		gui_manager->postEvent( e );
}

void YZView::registerManager ( Gui *mgr ) {
	gui_manager = mgr;
	session = mgr->getCurrentSession();
}

void YZView::centerView(int line) {
}

QString YZView::moveDown( QString inputsBuff ) {
	QString lin;
	int nb_lines=1;//default : one line down
	
	//check the arguments
/**	if ( !inputsBuff.isNull() ) {
		bool test;
		nb_lines = inputsBuff.toInt( &test );
		if ( !test ) return QString::null;
	}*/
	
	//execute the code
	if (cursor->getY() < buffer->text.count()-1) {
		cursor->incY();
		lin = buffer->findLine(cursor->getY());
		if ( !lin.isNull() ) current_maxx = lin.length()-1; 
		if (cursor->getX() > current_maxx) cursor->setX( current_maxx );
		if (cursor->getX() < 0) cursor->setX( 0 );

		//check if we need to scroll
		if ( current + lines_vis < cursor->getY() ) {
			//scroll down => GUI
			if ( gui_manager )
				gui_manager->scrollDown(); //one line down
			current++;
		}
		updateCursor();
	}
	//reset the input buffer
	purgeInputBuffer();
	
	//return something
	return QString::null;
}
