/**
 * $Id$
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
	redrawScreen();
	lines_vis = nb;
}

/* Used by the buffer to post events */
void YZView::sendChar( QChar c) {
	QString lin;

	if ('\033'==c) {
		//when escpaping while adding char in end of line
		if (cursor->getX() > current_maxx) {
			gotoxy(current_maxx, cursor->getY());
		}
		gotoCommandMode( );
		return;
	}
	switch(mode) {
		case YZ_VIEW_MODE_INSERT:
			/* handle adding a char */
			if ( c.unicode() == 13 ) {// <ENTER> 
				buffer->addNewLine(cursor->getX(),cursor->getY());
				gotoxy(0, cursor->getY()+1 );
			} else {
				buffer->addChar(cursor->getX(),cursor->getY(),c);
				gotoxy(cursor->getX()+1, cursor->getY() );
			}
			return;
		case YZ_VIEW_MODE_REPLACE:
			/* handle replacing a char */
			buffer->chgChar(cursor->getX(),cursor->getY(),c);
			gotoxy(cursor->getX()+1, cursor->getY() );
			return;
		case YZ_VIEW_MODE_COMMAND:
			/* will be handled after the switch */
			break;
		case YZ_VIEW_MODE_EX:
			if ( c.unicode() == 13 ) {// <ENTER> 
				//try to execute that stuff :)
				if ( session )
					session->getExPool()->execExCommand( this, gui_manager->getCommandLineText() );
			} // else nothing :)
			return; //we don't record anything
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

void YZView::updateCursor(void)
{
	postEvent( YZEvent::mkEventCursor(cursor->getX(),cursor->getY()));
}

yz_event YZView::fetchNextEvent() {
	if ( !events.empty() ) {
		yz_event e = events.first();
		events.pop_front(); //remove it
		return e;
	} else
		return YZEvent::mkEventNoop();
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
	int newcurrent = line - lines_vis / 2;

	if ( newcurrent > ( int( buffer->text.count() ) - int( lines_vis ) ) )
		newcurrent = buffer->text.count() - lines_vis;
	if ( newcurrent < 0 ) newcurrent = 0;
//	printf("Center : %i\n Lines vis : %i\n Current : %i\n",line,lines_vis,newcurrent);

	if ( newcurrent==current ) return;

	//redraw the screen
	current = newcurrent;
	redrawScreen();
}

void YZView::redrawScreen() {
	postEvent(YZEvent::mkEventRedraw() );
	updateCursor();
}

/*
 * all the goto-like commands
 */

/**
  * Dont put unsigned here, some function may call with negative here, as
  * checking is done inside gotoxy
  */
void	YZView::gotoxy(int nextx, int nexty)
{
	QString lin;

	// check positions
	if ( nexty >= int( buffer->text.count() ) ) nexty = buffer->text.count() - 1;
	if ( nexty < 0 ) nexty = 0;
	cursor->setY( nexty );

	lin = buffer->findLine(nexty);
	if ( !lin.isNull() ) current_maxx = lin.length()-1; 
	if ( YZ_VIEW_MODE_REPLACE == mode || YZ_VIEW_MODE_INSERT==mode ) {
		/* in edit mode, at end of line, cursor can be on +1 */
		if ( nextx > current_maxx+1 ) nextx = current_maxx+1;
	} else {
		if ( nextx > current_maxx ) nextx = current_maxx;
	}

	if ( nextx < 0 ) nextx = 0;
	cursor->setX( nextx );

	//make sure this line is visible
	if ( ! isLineVisible( nexty ) ) centerView( nexty );

	/* do it */
	updateCursor();

}

QString YZView::moveDown( const QString& inputsBuff ) {
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

	//reset the input buffer
	purgeInputBuffer();

	//return something
	return QString::null;
}

QString YZView::moveUp( const QString& inputsBuff ) {
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

	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::moveLeft( const QString& inputsBuff ) {
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

	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::moveRight( const QString& inputsBuff ) {
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
	
	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::moveToStartOfLine( const QString& ) {
	//execute the code
	gotoxy(0 , cursor->getY());
	
	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::gotoLine(const QString& inputsBuff) {
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
		if ( !test && !inputsBuff.startsWith( "gg" ) )
				line=buffer->text.count()-1; //there shouldn't be any other solution
	}

	if ( inputsBuff.startsWith( "gg" ) )
		gotoxy( 0, line );
	else
		gotoxy(cursor->getX(), line);

	purgeInputBuffer();
	//return something
	return QString::null;
}

// end of goto-like command


QString YZView::moveToEndOfLine( const QString& ) {
	QString lin;
	
	gotoxy( current_maxx+10 , cursor->getY());
	
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::deleteCharacter( const QString& inputsBuff ) {
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

QString YZView::deleteLine ( const QString& inputsBuff ) {
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

	for ( int i=0; i<nb_lines; ++i ) buffer->deleteLine( cursor->getY() );

	//reset the input buffer
	purgeInputBuffer();

	// prevent bug when deleting the last line
	gotoxy( cursor->getX(), cursor->getY());

	return QString::null;
}

QString YZView::openNewLineBefore ( const QString& ) {
	buffer->addNewLine(0,cursor->getY());
	//reset the input buffer
	purgeInputBuffer();
	gotoInsertMode();

	gotoxy(0,cursor->getY());

	//reset the input buffer
	purgeInputBuffer();
	return QString::null;
}

QString YZView::openNewLineAfter ( const QString& ) {
	buffer->addNewLine(0,cursor->getY()+1);
	//reset the input buffer
	purgeInputBuffer();
	gotoInsertMode();

	gotoxy( 0,cursor->getY()+1 );

	return QString::null;
}

QString YZView::append ( const QString& ) {
	//reset the input buffer
	purgeInputBuffer();
	gotoInsertMode();
	gotoxy(cursor->getX()+1, cursor->getY() );

	return QString::null;
}

QString YZView::appendAtEOL ( const QString& ) {
	//reset the input buffer
	purgeInputBuffer();
	moveToEndOfLine();
	append();

	return QString::null;
}


QString YZView::gotoCommandMode( ) {
	mode = YZ_VIEW_MODE_COMMAND;
	purgeInputBuffer();
	postEvent(YZEvent::mkEventStatus("Command mode"));
	return QString::null;
}

QString YZView::gotoInsertMode(const QString&) {
	mode = YZ_VIEW_MODE_INSERT;
	postEvent(YZEvent::mkEventStatus("-- INSERT --"));
	purgeInputBuffer();
	return QString::null;
}

QString YZView::gotoReplaceMode(const QString&) {
	mode = YZ_VIEW_MODE_REPLACE;
	postEvent(YZEvent::mkEventStatus("-- REPLACE --") );
	purgeInputBuffer();
	return QString::null;
}

//QString YZView::gotoExMode(QString) {
//}

