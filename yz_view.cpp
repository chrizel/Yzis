/**
 * $Id: yz_view.cpp,v 1.37 2003/04/25 20:00:54 mikmak Exp $
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
		postEvent(mk_event_setstatus(&QString( "Command mode") ));
		return;
	}
	switch(mode) {
		case YZ_VIEW_MODE_INSERT:
			/* handle adding a char */
			if ( c.unicode() == 13 ) {// <ENTER> 
				buffer->addNewLine(cursor->getX(),cursor->getY());
				cursor->incY();
				cursor->setX( 0 );
				redrawScreen();
			} else {
				buffer->addChar(cursor->getX(),cursor->getY(),c);
				cursor->incX();
			}
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

void YZView::postEvent (yz_event e) {
	events.push_back( e ); //append to the FIFO
	if ( gui_manager )
		gui_manager->postEvent( e );
}

void YZView::registerManager ( Gui *mgr ) {
	gui_manager = mgr;
	session = mgr->getCurrentSession();
}

void YZView::centerView(unsigned int line) {
	//update current
	current = line - lines_vis / 2;
	if ( current < 0 ) current = 0;
	if ( current > buffer->text.count() - lines_vis ) current = buffer->text.count() - lines_vis;
	//printf("Center : %i\n Lines vis : %i\n Current : %i\n",line,lines_vis,current);

	//redraw the screen
	redrawScreen();
}

void YZView::redrawScreen() {
	for (int i=current; i<current + lines_vis; i++) {
		QString l=buffer->findLine(i);
		if (l.isNull()) continue;
		postEvent(mk_event_setline(i,&l));
	}
}

/*
 * all the goto-like commands
 */
void	YZView::gotoxy(int nextx, int nexty)
{
	QString lin;


	// check positions
	if ( nexty < 0 ) nexty = 0;
	if ( nexty >= buffer->text.count() ) nexty = buffer->text.count() - 1;
	cursor->setY( nexty );

	lin = buffer->findLine(nexty);
	if ( !lin.isNull() ) current_maxx = lin.length()-1; 
	if ( nextx > current_maxx ) nextx = current_maxx;
	if ( nextx < 0 ) nextx = 0;
	cursor->setX( nextx );

	//make sure this line is visible
	if ( ! isLineVisible( nexty ) ) centerView( nexty );

	/* do it */
	updateCursor();

	//reset the input buffer
	purgeInputBuffer();
}

QString YZView::moveDown( QString inputsBuff ) {
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
	gotoxy(cursor->getX(), cursor->getY() + nb_lines);

	//return something
	return QString::null;
}

QString YZView::moveUp( QString inputsBuff ) {
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
	gotoxy(cursor->getX(), cursor->getY() - nb_lines);

	//return something
	return QString::null;
}

QString YZView::moveLeft( QString inputsBuff ) {
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
	gotoxy(cursor->getX() - nb_cols , cursor->getY());

	//return something
	return QString::null;
}

QString YZView::moveRight( QString inputsBuff ) {
	int nb_cols=1;//default : one column right
	
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
	gotoxy(cursor->getX() + nb_cols , cursor->getY());
	
	//return something
	return QString::null;
}

QString YZView::moveToStartOfLine( QString ) {
	//execute the code
	gotoxy(0 , cursor->getY());
	
	//return something
	return QString::null;
}

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
		if ( !test  &&  !inputsBuff.startsWith( "gg" ) )
				line=buffer->text.count()-1; //there shouldn't be any other solution
	}


	if ( inputsBuff.startsWith( "gg" ) )
		gotoxy( 0, line );

	gotoxy(cursor->getX(), line);

	//return something
	return QString::null;
}

// end of goto-like command


QString YZView::moveToEndOfLine( QString ) {
	QString lin;
	
	//execute the code
	lin = buffer->findLine(cursor->getY());
	if ( !lin.isNull() )
		current_maxx = lin.length()-1;
	else
		current_maxx = 0;

	cursor->setX( current_maxx );
	
	gotoxy( current_maxx , cursor->getY());
	
	//return something
	return QString::null;
}

QString YZView::deleteCharacter( QString inputsBuff ) {
	QString lin;
	int nb_cols=1;//default : one row right
	
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
	buffer->delChar(cursor->getX(), cursor->getY(), nb_cols);

	//reset the input buffer
	purgeInputBuffer();

	return QString::null;
}

QString YZView::gotoInsertMode(QString) {
	mode = YZ_VIEW_MODE_INSERT;
	postEvent(mk_event_setstatus(&QString( "-- INSERT --") ));
	purgeInputBuffer();
	return QString::null;
}

QString YZView::gotoReplaceMode(QString) {
	mode = YZ_VIEW_MODE_REPLACE;
	postEvent(mk_event_setstatus(&QString( "-- REPLACE --") ));
	purgeInputBuffer();
	return QString::null;
}

//QString YZView::gotoExMode(QString) {
//}

