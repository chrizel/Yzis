/*
    Copyright (c) 2003, 2004 Thomas Capricelli <orzel@freehackers.org>,
    Loic Pauleve <panard@inzenet.org>

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
/* Qt */
#include <qnamespace.h>
#include <qtimer.h>
#include <qcolor.h>

#include <ctype.h>

#include "viewwidget.h"
#include "factory.h"

static const QChar tabChar( '\t' );
static const QChar spaceChar( ' ' );

int NYZView::colormapinitialised = 0;
QMap<QRgb,int> NYZView::mColormap; // map Ncurses to Qt codes


NYZView::NYZView(YZBuffer *b)
	: YZView(b,NYZFactory::self,0), editor(0)
{
	
	if ( !colormapinitialised ) initialisecolormap();
	YZASSERT( b );
	yzDebug(NYZIS) << "NYZView::NYZView buffer is : " << ( int )b << endl;
	window = NULL;
}

NYZView::~NYZView(){
	if ( window ) unmap();
}

void NYZView::map( void )
{
	marginLeft = 0;
	updateVis(false);

	// main editor, fullscreen
	window = newwin( 0,0,0,0 ); YZASSERT( window );
	touchwin( window ); // throw away optimisations because we're going to subwin , as said in doc

	editor = subwin( window, getLinesVisible(), 0, 0, 0); YZASSERT( editor );
	wattrset( editor, A_NORMAL|A_BOLD );
	wmove( editor,0,0 );
	keypad( editor , true); //active symbols for special keycodes
	scrollok( editor, false ); 

	// creates layout
	/*
	 * ------------------ infobar ---------------------
	 * ------------------ statusbar -------------------
	 */
	infobar = subwin(window, 1, 0, getLinesVisible(), 0); YZASSERT( infobar );
	wattrset(infobar, A_NORMAL|A_BOLD );
	wbkgd(infobar, A_NORMAL|A_BOLD|A_REVERSE);

	statusbar  = subwin(window, 1, 0, getLinesVisible()+1, 0); YZASSERT( statusbar );
	wattrset(statusbar, A_NORMAL|A_BOLD );
	if (has_colors())
		wattron(statusbar, COLOR_PAIR(1));

	/* prevents the delay between hitting <escape> and when we actually receive the event */
	notimeout( stdscr, true );
	notimeout( editor, true );
	notimeout( statusbar, true );
	notimeout( infobar, true );
}


void NYZView::unmap( void )
{
	YZASSERT( statusbar ); delwin( statusbar );
	YZASSERT( infobar ); delwin( infobar );
	YZASSERT( editor ); delwin( editor );
	YZASSERT( window ); delwin( window );
	window = editor = statusbar = infobar = NULL;
}

void NYZView::updateVis( bool refresh ) {
	unsigned int width;
	unsigned int height;
	getmaxyx( stdscr, height, width ); 
	setVisibleArea( width - marginLeft, height - 2, refresh );
}

void NYZView::printVoid( unsigned int relline )
{
	unsigned int i;

	// clipping
	if ( relline > getLinesVisible() ) return;
	wmove (editor, relline, 0);
	waddch(editor, '~');
	for (i=1 ; i< getColumnsVisible(); i++ ) waddch(editor, ' ' );
}

void NYZView::scrollUp( int n ) {
	scrollok( editor, true ); 
	wscrl( editor, - n );
	scrollok( editor, false ); 
	drawContents( 0, n );
}

void NYZView::scrollDown( int n ) {
	scrollok( editor, true ); 
	wscrl( editor, n );
	scrollok( editor, false ); 
	drawContents( getLinesVisible() - n, n );
}

void NYZView::paintEvent( unsigned int , unsigned int clipy, unsigned int , unsigned int cliph ) {
	drawContents( clipy - getDrawCurrentTop( ), cliph );
}

void NYZView::drawContents( int clipy, int cliph ) {
	bool number = YZSession::getBoolOption( "General\\number" );
	bool wrap = YZSession::getBoolOption( "General\\wrap" );

	if (!editor)	// Avoid segfaults and infinite recursion.
		return;

	unsigned int lineCount = myBuffer()->lineCount();
	unsigned int my_marginLeft = 0;
	if ( number ) { // update marginLeft
		my_marginLeft = 2 + QString::number( lineCount ).length();
		lastLineNumber = 0;
	}
	if ( marginLeft != my_marginLeft ) {
		marginLeft = my_marginLeft;
		updateVis();
		return;
	}

	unsigned int currentY = 0;
	if (! wrap ) {
		initDraw( getCurrentLeft(), getCurrentTop() + clipy, getDrawCurrentLeft(), getDrawCurrentTop() + clipy  );
		currentY = clipy;
	} else {
		initDraw( );
	}
	unsigned int lineNumber = 0;

	while ( drawNextLine( ) && cliph > 0 ) {
		lineNumber = drawLineNumber();
		if ( currentY >= ( uint )clipy ) {
			unsigned int currentX = 0;
			wmove( editor, currentY, currentX );
			if ( number ) { // draw current line number
				if ( lineNumber != lastLineNumber ) { // we don't draw it twice
					wattron( editor, COLOR_PAIR( mColormap[ Qt::yellow.rgb() ] ) );
					waddstr( editor, QString::number( lineNumber ).rightJustify( marginLeft - 1, ' ' ) );
					wattroff( editor, COLOR_PAIR( mColormap[ Qt::yellow.rgb() ] ) );
					waddch( editor, ' ' );
					lastLineNumber = lineNumber;
				} else for( unsigned int i = 0; i < marginLeft; i++) waddch( editor, ' ' );
				currentX += marginLeft;
			}
			while ( drawNextCol( ) ) {
				QColor c = drawColor( );
				int mColor = mColormap.contains( c.rgb() ) ? mColormap[ c.rgb() ] : mColormap[ Qt::white.rgb() ]; 
				bool invert = drawSelected( );
				waddch( editor, COLOR_PAIR( mColor ) | ( invert ? A_REVERSE : A_NORMAL ) | drawChar().unicode() );
				if ( drawLength() > 1 ) {
					for (unsigned int i = 1; i < drawLength(); i++ ) waddch( editor, ' ' | ( invert ? A_REVERSE : A_NORMAL ) );
				}
				currentX += drawLength( );
			}
			for( ; currentX < getColumnsVisible() + marginLeft; currentX++) waddch( editor, ' ' );
			currentY += drawHeight( );
			cliph -= lineHeight( );
		} else {
			if ( wrap ) while ( drawNextCol( ) ) ;
			currentY += drawHeight( );
			lastLineNumber = lineNumber;
		}
	}
	while ( cliph > 0 && currentY < getLinesVisible() ) {
		printVoid( currentY );
		++currentY;
		--cliph;
	}

	wmove(editor,
		getCursor()->getY() - getDrawCurrentTop (),
		getCursor()->getX() - getDrawCurrentLeft () + marginLeft
	);
	wrefresh( editor );
}

void NYZView::setCommandLineText( const QString& text )
{
	werase(statusbar);
	commandline = text;
	if ( !text.isEmpty() ) {
		waddstr(statusbar, text.latin1());
		waddch( statusbar, ' ' ); // when doing backspace...
		waddch( statusbar, '\b' );
	}
	wrefresh(statusbar);
}


void NYZView::syncViewInfo( void )
{
	// older versions of ncurses want non const..
	char * myfmt;

	/*
	 * ------------------ infobar ---------------------
	 * ------------------ statusbar -------------------
	 */

	YZASSERT( mMode<YZ_VIEW_MODE_LAST );
	static const char *modeName[] = {
		" Insert",
		"Replace",
		"Command",
		"     Ex",
		" Search",
		"   Open",
		" Visual"
	};

	werase(infobar);
	wmove( infobar,0,0 );
	for ( const char *ptr = modeName[mMode]; *ptr; ptr++ )
		waddch(infobar, COLOR_PAIR(1)|*ptr);

	waddch(infobar, ' ');

	// update infobar
	myfmt=( char* )"%s%s"; // <- prevent %s in percentage to fubar everything, even if
	            // it's rather unlikely..
	wprintw( infobar, myfmt, 
			( mBuffer->fileIsNew() )?"[No File]":mBuffer->fileName().latin1(),
			( mBuffer->fileIsModified() )?" [+]":""
			);
	// prevent  gcc to use string
	if ( viewInformation.c1!=viewInformation.c2 ) {
		myfmt=( char* )"%d,%d-%d";
		mvwprintw( infobar, 0, getColumnsVisible()-20, myfmt,
				viewInformation.l+1,
				viewInformation.c1+1,
				viewInformation.c2+1 );
	} else {
		myfmt=( char * )"%d,%d";
		mvwprintw( infobar, 0, getColumnsVisible()-20, myfmt, viewInformation.l+1,viewInformation.c1+1 );
	}
	mvwaddstr( infobar, 0, getColumnsVisible()-9, viewInformation.percentage.latin1() );

	wrefresh(infobar);

	wmove(editor,
		getCursor()->getY() - getDrawCurrentTop (),
		getCursor()->getX() - getDrawCurrentLeft () + marginLeft
		 );
	wrefresh( editor );
}

QString NYZView::refreshScreen(const QString&, YZCommandArgs) {
//	clear();
	drawContents( 0, getLinesVisible() );

	refresh();
	wrefresh(editor);
	updateCursor();
	return QString::null;
}

void NYZView::displayInfo( const QString& info )
{
	werase(statusbar);
	waddstr( statusbar, info.latin1() );
	wrefresh(statusbar);
	yzDebug(NYZIS)<< "NYZView::displayInfo message is : " << info << endl;
}


void NYZView::initialisecolormap()
{
	colormapinitialised = true;

	if ( !has_colors() ) {
		yzWarning() << "Terminal doesn't have color capabilities, disabling syntax highlighting" <<endl;
		return;
	}
	bool changecolorok = (can_change_color() == TRUE);
	yzWarning() << "Terminal can";
	if (!changecolorok)
		yzWarning() << " _not_";
	yzWarning() << " change colors" << endl;

	yzDebug() << "COLOR_PAIRS is : " << COLOR_PAIRS << endl;
	yzDebug() << "COLORS      is : " << COLORS << endl;

	// magenta = 1, is used to display info on statusbar..
	//
	if ( changecolorok ) {

#define COLOR_QT2CURSES(a) ((a)*1000/256)
#define COLOR_CURSES2QT(a) ((a)*256/1000)
#define MAP( nb, color )       \
	init_color( nb, COLOR_QT2CURSES(qRed(color.rgb())), COLOR_QT2CURSES( qGreen(color.rgb())), COLOR_QT2CURSES( qBlue(color.rgb()))) ; \
	YZASSERT ( ERR != init_pair( nb, nb%COLORS, COLOR_BLACK ) );    \
	mColormap[color.rgb()] = nb;

	/*
	init_color( 1,
			(short)qRed(Qt::magenta.rgb()),
			(short)qGreen(Qt::magenta.rg),
			(short)qBlue(Qt::magenta)); 
	init_pair( 1, 1, COLOR_BLACK );    
	mColormap[Qt::magenta] = 1;
			*/

		MAP( 1, Qt::magenta );
		MAP( 2, Qt::white );
		MAP( 3, Qt::darkGray );
		MAP( 4, Qt::gray );
		MAP( 5, Qt::lightGray );
		MAP( 6, Qt::red );
		MAP( 7, Qt::green );
		MAP( 8, Qt::blue );
		MAP( 9, Qt::cyan );
		MAP( 10, Qt::black );
		MAP( 11, Qt::yellow );
		MAP( 12, Qt::darkRed );
		MAP( 13, Qt::darkGreen );
		MAP( 14, Qt::darkBlue );
		MAP( 15, Qt::darkCyan );
		MAP( 16, Qt::darkMagenta );
		MAP( 17, Qt::darkYellow );

	} else {
#undef MAP
#define MAP( nb, qtcolor, color )               \
	YZASSERT( ERR != init_pair( nb, color, COLOR_BLACK ) );    \
	mColormap[qtcolor.rgb()] = nb;

		MAP( 1, Qt::magenta, COLOR_MAGENTA );
		MAP( 2, Qt::red, COLOR_RED );
		MAP( 3, Qt::green, COLOR_GREEN );
		MAP( 4, Qt::yellow, COLOR_YELLOW );
		MAP( 5, Qt::cyan, COLOR_CYAN );
		MAP( 6, Qt::black, COLOR_BLACK );
		MAP( 7, Qt::blue, COLOR_BLUE );
		MAP( 8, Qt::white, COLOR_WHITE | A_BOLD );
		MAP( 9, Qt::gray, COLOR_WHITE );
		MAP(10, Qt::darkGreen, COLOR_GREEN | A_BOLD);
		MAP(11, Qt::darkMagenta, COLOR_MAGENTA | A_BOLD);
		MAP(12, Qt::darkCyan, COLOR_CYAN | A_BOLD);
		MAP(13, Qt::lightGray, COLOR_WHITE | A_BOLD );
	}
}
