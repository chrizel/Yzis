/**
 * $Id: yz_view.cpp,v 1.34 2003/04/25 12:45:30 mikmak Exp $
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

	//events_nb_begin = 0;
	//events_nb_last = 0;

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
		purgeInputBuffer();
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
			//printf("Currently unknown MODE\n");
			purgeInputBuffer();
			return;
	};
	/* ok, here now we're in command */
	previous_chars+=c;
	//execute the command
	if ( session ) 
		session->getPool()->execCommand(this, previous_chars);
}

void YZView::updateCursor(int x, int y) {
	if ( y!=-1 ) {
		QString lin = buffer->findLine( y );
		if ( !lin.isNull() ) cursor->setY( y );
		else return; //abort. something's wrong
	}
	if ( x!=-1 ) {
		QString lin = buffer->findLine( y );
		if ( !lin.isNull() ) {
			current_maxx = lin.length()-1;
			cursor->setX( x > current_maxx ? current_maxx : x );
		}
	}
	postEvent( mk_event_setcursor(cursor->getX(),cursor->getY()));
}

yz_event YZView::fetchNextEvent() {
	if ( !events.empty() ) {
		yz_event e = events.first();
		events.pop_front(); //remove it
		return e;
	} else
		return mk_event_noop();
}

#if 0
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
#endif

void YZView::postEvent (yz_event e) {
	events.push_back( e ); //append to the FIFO
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
	if ( !inputsBuff.isNull() ) {
		int i=0;
		while ( inputsBuff[i].isDigit() )
			i++; //go on
		bool test;
		nb_lines = inputsBuff.left( i ).toInt( &test );
		if ( !test ) nb_lines=1;
	}
	
	//execute the code
	int nexty=cursor->getY() + nb_lines;
	int nextx=cursor->getX();

	//number too big, go to bottom of file
	if ( nexty > buffer->text.count() ) nexty = buffer->text.count() - 1;
	cursor->setY( nexty );
	lin = buffer->findLine(nexty);
	if ( !lin.isNull() ) current_maxx = lin.length()-1; 
	if ( nextx > current_maxx ) nextx = current_maxx;
	if ( nextx < 0 ) nextx = 0;
	cursor->setX( nextx );
	
	//check if we need to scroll
/**	if ( current + lines_vis < cursor->getY() ) {
		//scroll down => GUI
		if ( gui_manager ) //gui_manager necessarly exists here, these are useless now
			gui_manager->scrollDown( cursor->getY() - current - lines_vis);
		current+= nb_lines; //bof XXX
	} */
	updateCursor();

	//reset the input buffer
	purgeInputBuffer();
	
	//return something
	return QString::null;
}

QString YZView::moveUp( QString inputsBuff ) {
	QString lin;
	int nb_lines=1;//default : one line down
	
	//check the arguments
	if ( !inputsBuff.isNull() ) {
		int i=0;
		while ( inputsBuff[i].isDigit() )
			i++; //go on
		bool test;
		nb_lines = inputsBuff.left( i ).toInt( &test );
		if ( !test ) nb_lines=1;
	}
	
	//execute the code
	int nexty=cursor->getY() - nb_lines;
	int nextx=cursor->getX();

	//number too big, go to bottom of file
	if ( nexty < 0 ) nexty = 0;
	cursor->setY( nexty );
	lin = buffer->findLine(nexty);
	if ( !lin.isNull() ) current_maxx = lin.length()-1; 
	if ( nextx > current_maxx ) nextx = current_maxx;
	if ( nextx < 0 ) nextx = 0;
	cursor->setX( nextx );
	
	//check if we need to scroll
/**	if ( current + lines_vis < cursor->getY() ) {
		//scroll down => GUI
		if ( gui_manager ) //gui_manager necessarly exists here, these are useless now
			gui_manager->scrollDown( cursor->getY() - current - lines_vis);
		current+= nb_lines; //bof XXX
	} */
	updateCursor();

	//reset the input buffer
	purgeInputBuffer();
	
	//return something
	return QString::null;
}

QString YZView::moveLeft( QString inputsBuff ) {
	QString lin;
	int nb_cols=1;//default : one line left
	
	//check the arguments
	if ( !inputsBuff.isNull() ) {
		int i=0;
		while ( inputsBuff[i].isDigit() )
			i++; //go on
		bool test;
		nb_cols = inputsBuff.left( i ).toInt( &test );
		if ( !test ) nb_cols=1;
	}
	
	//execute the code
	lin = buffer->findLine(cursor->getY());
	if ( !lin.isNull() ) current_maxx = lin.length()-1; 
	int nextx=cursor->getX() - nb_cols;

	//number too big, go to bottom of file
	if ( nextx < 0 ) nextx = 0;
	cursor->setX( nextx );
	
	//check if we need to scroll
/**	if ( current + lines_vis < cursor->getY() ) {
		//scroll down => GUI
		if ( gui_manager ) //gui_manager necessarly exists here, these are useless now
			gui_manager->scrollDown( cursor->getY() - current - lines_vis);
		current+= nb_lines; //bof XXX
	} */
	updateCursor();

	//reset the input buffer
	purgeInputBuffer();
	
	//return something
	return QString::null;
}

QString YZView::moveRight( QString inputsBuff ) {
	QString lin;
	int nb_cols=1;//default : one line down
	
	//check the arguments
	if ( !inputsBuff.isNull() ) {
		int i=0;
		while ( inputsBuff[i].isDigit() )
			i++; //go on
		bool test;
		nb_cols = inputsBuff.left( i ).toInt( &test );
		if ( !test ) nb_cols=1;
	}
	
	//execute the code
	lin = buffer->findLine(cursor->getY());
	if ( !lin.isNull() ) current_maxx = lin.length()-1; 
	int nextx=cursor->getX()+nb_cols;

	//number too big, go to bottom of file
	if ( nextx > current_maxx ) nextx = current_maxx;
	if ( nextx < 0 ) nextx = 0;
	cursor->setX( nextx );
	
	//check if we need to scroll
/**	if ( current + lines_vis < cursor->getY() ) {
		//scroll down => GUI
		if ( gui_manager ) //gui_manager necessarly exists here, these are useless now
			gui_manager->scrollDown( cursor->getY() - current - lines_vis);
		current+= nb_lines; //bof XXX
	} */
	updateCursor();

	//reset the input buffer
	purgeInputBuffer();
	
	//return something
	return QString::null;
}

QString YZView::gotoInsertMode(QString) {
	mode = YZ_VIEW_MODE_INSERT;
	postEvent(mk_event_setstatus("-- INSERT --"));
	purgeInputBuffer();
	return QString::null;
}

QString YZView::gotoReplaceMode(QString) {
	mode = YZ_VIEW_MODE_REPLACE;
	postEvent(mk_event_setstatus("-- REPLACE --"));
	purgeInputBuffer();
	return QString::null;
}

//QString YZView::gotoExMode(QString) {
//}

QString YZView::gotoLine(QString inputsBuff) {
	int line=0;
	//check arguments
	//can be : 'gg' (goto Top),'G' or a number with one of them
	if ( !inputsBuff.isNull() ) {
		//try to find a number
		int i=0;
		while ( inputsBuff[i].isDigit() )
			i++; //go on
		bool test;
		line = inputsBuff.left( i ).toInt( &test );
		if ( !test ) {
			if ( inputsBuff.startsWith( "gg" ) ) line=0;
			else line=buffer->text.count(); //there shouldn't be any other solution
		}
	}
	
	if ( line==0 ) cursor->setX( 0 ); //for gg we go at beginning of line (should be first non blank character XXX)
	cursor->setY(line);
	//execute the code
	updateCursor( );

/**	if ( current + lines_vis < cursor->getY() ) {
		//scroll down => GUI
		if ( gui_manager ) //gui_manager necessarly exists here, these are useless now
			gui_manager->scrollDown( cursor->getY() - current - lines_vis);
		current+= nb_lines; //bof XXX
	} */

	//reset the input buffer
	purgeInputBuffer();
	
	//return something
	return QString::null;
}

