/**
 * $Id$
 */

#include <cstdlib>
#include <ctype.h>
#include "yz_view.h"
#include "yz_debug.h"
#include <qkeysequence.h>

//initialise view IDs counter (static)
int YZView::view_ids = 0;

YZView::YZView(YZBuffer *_b, YZSession *sess, int _lines_vis) {
	myId = view_ids++;
	session = sess;
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

YZView::~YZView() {
}

void YZView::setVisibleLines(int nb) {
	redrawScreen();
	lines_vis = nb;
}

/* Used by the buffer to post events */
void YZView::sendKey( int c, int modifiers) {
	//ignore some keys
	if ( c == Qt::Key_Shift || c == Qt::Key_Alt || c == Qt::Key_Meta ||c == Qt::Key_Control || c == Qt::Key_CapsLock ) return;

	//map other keys
	if ( c == Qt::Key_Insert ) c = Qt::Key_I;
	
	QString lin;
	QString key = QChar( tolower( c ) );// = QKeySequence( c );
	//default is lower case unless some modifiers
	if ( modifiers & YZIS::Shift ) {
		key = key.upper();
	}

	if (Qt::Key_Escape == c) {
		//when escpaping while adding char in end of line
		if (cursor->getX() > current_maxx) {
			gotoxy(current_maxx, cursor->getY());
		}
		gotoCommandMode( );
		return;
	}

	switch(mode) {

		case YZ_VIEW_MODE_INSERT:
			switch ( c ) {
				case Qt::Key_Return:
					buffer->addNewLine(cursor->getX(),cursor->getY());
					gotoxy(0, cursor->getY()+1 );
					return;
				case Qt::Key_Down:
					moveDown( QString::null );
					return;
				case Qt::Key_Left:
					moveLeft( QString::null );
					return;
				case Qt::Key_Right:
					moveRight( QString::null );
					return;
				case Qt::Key_Up:
					moveUp( QString::null );
					return;
				default:
					buffer->addChar(cursor->getX(),cursor->getY(),key);
					gotoxy(cursor->getX()+1, cursor->getY() );
					return;
			}
			break;

		case YZ_VIEW_MODE_REPLACE:
			switch ( c ) {
				case Qt::Key_Return:
					buffer->addNewLine(cursor->getX(),cursor->getY());
					gotoxy(0, cursor->getY()+1 );
					return;
				case Qt::Key_Down:
					moveDown( QString::null );
					return;
				case Qt::Key_Left:
					moveLeft( QString::null );
					return;
				case Qt::Key_Right:
					moveRight( QString::null );
					return;
				case Qt::Key_Up:
					moveUp( QString::null );
					return;
				default:
					buffer->chgChar(cursor->getX(),cursor->getY(),key);
					gotoxy(cursor->getX()+1, cursor->getY() );
					return;
			}
			break;

		case YZ_VIEW_MODE_EX:
			switch ( c ) {
				case Qt::Key_Return:
					yzDebug() << "Current command EX : " << session->gui_manager->getCommandLineText();
					session->getExPool()->execExCommand( this, session->gui_manager->getCommandLineText() );
					session->gui_manager->setCommandLineText( "" );
					session->gui_manager->setFocusMainWindow();
					gotoCommandMode();
					return;
				case Qt::Key_Down:
				case Qt::Key_Left:
				case Qt::Key_Right:
				case Qt::Key_Up:
					return;
				case Qt::Key_Escape:
					session->gui_manager->setCommandLineText( "" );
					session->gui_manager->setFocusMainWindow();
					gotoCommandMode();
					return;
				default:
					buffer->chgChar(cursor->getX(),cursor->getY(),key);
					gotoxy(cursor->getX()+1, cursor->getY() );
					return;
			}
			break;

		case YZ_VIEW_MODE_COMMAND:
			switch ( c ) {
				case Qt::Key_Down:
					moveDown( QString::null );
					return;
				case Qt::Key_Left:
					moveLeft( QString::null );
					return;
				case Qt::Key_Right:
					moveRight( QString::null );
					return;
				case Qt::Key_Up:
					moveUp( QString::null );
					return;
				default:
					previous_chars+=key;
					yzDebug() << "Previous chars : " << previous_chars << endl;
					if ( session ) {
						int error = 0;
						session->getPool()->execCommand(this, previous_chars, &error);
						if ( error == 1 ) purgeInputBuffer(); // no matching command
					}
			}
			break;

		default:
			yzDebug() << "Unknown MODE" << endl;
			purgeInputBuffer();
	};
}

void YZView::updateCursor(void)
{
	static int lasty = -10; // small speed optimisation
	static QString percentage("All");
	int y = cursor->getY();

	if ( y != lasty ) {
		int nblines = buffer->getLines();
		percentage = QString("%1%").arg( int( y*100/ ( nblines==0 ? 1 : nblines )));
		if ( current < 1 )  percentage="Top";
		if ( current+lines_vis >= nblines )  percentage="Bot";
		if ( (current<1 ) &&  ( current+lines_vis >= nblines ) ) percentage="All";
		lasty=y;
	}

	session->postEvent(YZEvent::mkEventCursor(myId,
				cursor->getX(),
				y,
				y,
				percentage
				));
}

void YZView::centerView(unsigned int line) {

	//update current
	int newcurrent = line - lines_vis / 2;

	if ( newcurrent > ( int( buffer->text.count() ) - int( lines_vis ) ) )
		newcurrent = buffer->text.count() - lines_vis;
	if ( newcurrent < 0 ) newcurrent = 0;

	if ( newcurrent== int( current ) ) return;

	//redraw the screen
	current = newcurrent;
	redrawScreen();
}

void YZView::redrawScreen() {
	session->postEvent(YZEvent::mkEventRedraw(myId) );
	updateCursor();
}

/*
 * all the goto-like commands
 */

/**
  * Dont put unsigned here, some function may call with negative here, as
  * checking is done inside gotoxy
  */
void YZView::gotoxy(int nextx, int nexty)
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
	int nb_lines=1;//default : one line up

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
	gotoxy( cursor->getX(), cursor->getY());

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
	session->postEvent(YZEvent::mkEventStatus(0,"Command mode"));
	return QString::null;
}

QString YZView::gotoExMode(const QString&) {
	mode = YZ_VIEW_MODE_EX;
	session->postEvent(YZEvent::mkEventStatus(0,"-- EX --"));
	session->gui_manager->setFocusCommandLine();
	purgeInputBuffer();
	return QString::null;
}

QString YZView::gotoInsertMode(const QString&) {
	mode = YZ_VIEW_MODE_INSERT;
	session->postEvent(YZEvent::mkEventStatus(0,"-- INSERT --"));
	purgeInputBuffer();
	return QString::null;
}

QString YZView::gotoReplaceMode(const QString&) {
	mode = YZ_VIEW_MODE_REPLACE;
	session->postEvent(YZEvent::mkEventStatus(0,"-- REPLACE --") );
	purgeInputBuffer();
	return QString::null;
}

