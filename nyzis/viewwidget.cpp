/*
	  Copyright (c) 2003, 2004 Thomas Capricelli <orzel@freehackers.org>

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
	: YZView(b,NYZFactory::self,0)
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
	// main window, fullscreen
	window = newwin( 0,0,0,0 ); YZASSERT( window );
	wattrset(window, A_NORMAL );
	touchwin( window ); // throw away optimisations because we're going to subwin , as said in doc
	wmove(window,0,0 );
	keypad(window , true); //active symbols for special keycodes
	updateVis();

	// creates layout
	/*
	 * ------------------ infobar ---------------------
	 * ------------------ statusbar -------------------
	 */
	infobar = subwin(window, 1, 0, getLinesVisible(), 0); YZASSERT( infobar );
	wattrset(infobar, A_NORMAL );
	wbkgd(infobar, A_REVERSE);

	statusbar  = subwin(window, 1, 0, getLinesVisible()+1, 0); YZASSERT( statusbar );
	wattrset(statusbar, A_NORMAL );
	if (has_colors())
		wattron(statusbar, COLOR_PAIR(1));
//	(void) notimeout(stdscr,TRUE);/* prevents the delay between hitting <escape> and when we actually receive the event */
//	(void) notimeout(window,TRUE);/* prevents the delay between hitting <escape> and when we actually receive the event */

	setVisibleArea( mColumnsVis,mLinesVis ); //compatibility.., does a refresh()
}


void NYZView::unmap( void )
{
	YZASSERT( statusbar ); delwin( statusbar );
	YZASSERT( infobar ); delwin( infobar );
	YZASSERT( window ); delwin( window );
	window = statusbar = infobar = NULL;
}


void NYZView::printVoid( unsigned int relline )
{
	unsigned int i;

	// clipping
	if ( relline > getLinesVisible() ) return;
	wmove (window, relline, 0);
	waddch(window, '~');
	for (i=1 ; i< getColumnsVisible(); i++ ) waddch(window, ' ' );
}

void NYZView::printLine( int line )
{

	unsigned int i;
	int actuallen;
	int sx,sy; // save x,y
	unsigned int relline = line - getCurrentTop(); // relative line #

	// check
	QString str = mBuffer->textline(line);
	if ( str.isNull() ) return; // Erase line ??

	// clipping 
	if ( relline > getLinesVisible() ) return;

	getyx(window,sy,sx); // save cursor

	/* not use addnstr here, will stop at \0  (i guess) */ 
	if ( myBuffer()->introShown() )
		wmove( window,relline, (getColumnsVisible()-str.length()>0)?(getColumnsVisible()-str.length())/2:0 );
	else wmove (window, relline, 0);


	// syntax highlighting stuff : copy/paste/enhance from kyzis
	YzisAttribute *at = 0L;
	const uchar* a = NULL;
	YZLine *yl = myBuffer()->yzline( line );
	if ( yl->length() != 0 ) a = yl->attributes();
	YzisHighlighting *highlight = myBuffer()->highlight();
	if ( !has_colors() ) highlight = 0; // disable syntax highlighting
	if ( highlight )
		at = highlight->attributes( 0 /*only one schema*/ )->data( );
	uint atLen = at ? highlight->attributes( 0 /*only one schema*/ )->size() : 0;
	int mColor = mColormap[Qt::white.rgb()];


//yzDebug() << "at,a,atLen are " << ( int )at << " " <<  ( int )a << " " <<  atLen <<endl;
	bool noAttribs = !a;
	actuallen = - getCurrentLeft();
	for (i=0; actuallen<(int)getColumnsVisible() && i<str.length(); i++) {
		// quickly handle the tab case
		if ( str[i] == tabChar ) {
			for ( int j=0; j<TABSIZE && actuallen<getColumnsVisible(); actuallen++,j++ )
				if (actuallen>=0) waddch( window, ' ' );
			if ( a ) a++;
			continue;
		}

		YzisAttribute hl;
		YzisAttribute *curAt = ( !noAttribs && (*a) >= atLen ) ?  &at[0] : &at[*a];
		if ( curAt ) {
			hl+=*curAt;
			mColor = mColormap.contains(hl.textColor().rgb())?
				mColormap[hl.textColor().rgb()]:
				mColormap[Qt::white.rgb()];
		}
		if ( actuallen>= 0 )
			waddch(window, COLOR_PAIR(mColor)|str[i].unicode());
		actuallen++;
		if ( a ) a++;
	}

	if ( myBuffer()->introShown() ) return;

	// end of line...
	if ( actuallen<0 ) actuallen=0;
	for ( ; actuallen< getColumnsVisible(); actuallen++ ) waddch(window, ' ' );
//	for ( ; actuallen< getColumnsVisible()-1; actuallen++ ) waddch(window, COLOR_PAIR(1)|'X' ); // debug :)
	wmove(window,sy,sx ); // restore cursor
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


void NYZView::invalidateLine ( unsigned int line ) {
	printLine( line );
	wrefresh( window );
	refresh();
}


void NYZView::syncViewInfo( void )
{
	// older versions of ncurses want non const..
	char * myfmt;

	updateVis();

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
		" Search"
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
				viewInformation.l1+1,
				viewInformation.c1+1,
				viewInformation.c2+1 );
	} else {
		myfmt=( char * )"%d,%d";
		mvwprintw( infobar, 0, getColumnsVisible()-20, myfmt, viewInformation.l1+1,viewInformation.c1+1 );
	}
	mvwaddstr( infobar, 0, getColumnsVisible()-9, viewInformation.percentage.latin1() );

	wrefresh(infobar);

	wmove(window, viewInformation.l1-getCurrentTop() , viewInformation.c2 - getCurrentLeft() ) ;
	wrefresh( window );
}

void NYZView::refreshScreen() {
	unsigned int i;

	clear();
	for ( i=getCurrentTop(); i < ( getCurrentTop() + getLinesVisible() ) && i < mBuffer->lineCount(); i++ )
		printLine(i);
	i-=getCurrentTop();
	for ( ; i < getLinesVisible() ; i++ ) printVoid( i );

	refresh();
	wrefresh(window);
	updateCursor();
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
		yzWarning() << "Terminal doesn't has color capabilities, disabling syntax highlighting" <<endl;
		return;
	}

	yzDebug() << "COLOR_PAIRS is : " << COLOR_PAIRS << endl;

	// magenta = 1, is used to display info on statusbar..
	//
	if ( can_change_color() ) {
		yzWarning() << "Terminal can change colors"<< endl;

#define COLOR_QT2CURSES(a) ((a)*1000/256)
#define COLOR_CURSES2QT(a) ((a)*256/1000)
#define MAP( nb, color )       \
	init_color( nb, COLOR_QT2CURSES(qRed(color.rgb())), COLOR_QT2CURSES( qGreen(color.rgb())), COLOR_QT2CURSES( qBlue(color.rgb()))) ; \
	YZASSERT ( ERR != init_pair( nb, nb, COLOR_BLACK ) );    \
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
		yzWarning() << "Terminal can't change colors"<< endl;
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

