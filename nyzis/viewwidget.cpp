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
	modeDisplayed = false;
}

NYZView::~NYZView(){
}

void NYZView::map( void )
{
	window = newwin( 0,0,0,0 );

	wmove(window,0,0 );
	//active symbols for special keycodes
	keypad(window , true);

	// creates layout
	/*
	 * ------------------ infobar ---------------------
	 * ------------------ statusbar -------------------
	 */

	update_info();
	touchwin( window ); // throw away optimisations because we're going to subwin , as said in doc
//	WINDOW *window = subwin(screen, LINES-2, 0, 0, 0);
	infobar = subwin(window, 1, 0, h-2, 0);
	wattrset(infobar, A_STANDOUT || A_BOLD);
	wbkgd(infobar, A_REVERSE);
	statusbar  = subwin(window, 1, 0, h-1, 0);
	//	(void) notimeout(stdscr,TRUE);/* prevents the delay between hitting <escape> and when we actually receive the event */
	//	(void) notimeout(window,TRUE);/* prevents the delay between hitting <escape> and when we actually receive the event */

	if (has_colors())
		wattron(statusbar, COLOR_PAIR(1));

	// TODO  write something like :       "bernoulli.tex" [noeol] 65L, 1440C
	// in last line (vim-like)
	displayInfo ( QString("\"%1\" %2L, %3C" ).arg(mBuffer->fileName()).arg(mBuffer->lineCount()).arg(mBuffer->getWholeTextLength()));
	redrawScreen();
}


void NYZView::unmap( void )
{
	YZASSERT( statusbar ); delwin( statusbar );
	YZASSERT( infobar ); delwin( infobar );
	YZASSERT( window ); delwin( window );
	window = statusbar = infobar = NULL;
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
	unsigned int relline = line - getCurrentTop(); // relative line #

	// check
	QString str = mBuffer->textline(line);
	if ( str.isNull() ) return;

	// clipping 
	if ( relline > mLinesVis ) return;

	getyx(window,sy,sx); // save cursor

	/* not use addnstr here, will stop at \0  (i guess) */ 
	if ( myBuffer()->introShown() )
		wmove( window,relline, (w-str.length()>0)?(w-str.length())/2:0 );
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
	int mColor = mColormap[ Qt::white.rgb() ];


//yzDebug() << "at,a,atLen are " << ( int )at << " " <<  ( int )a << " " <<  atLen <<endl;
	bool noAttribs = !a;
	for (i=0; i<w && i<str.length(); i++) {
		// quickly handle the tab case
		if ( str[ i ] == tabChar ) {
			for ( int j=0; j<8; j++ ) waddch( window, ' ' );
			if ( a ) a++;
			continue;
		}

		YzisAttribute hl;
		//YzisAttribute *curAt=at;
		//if ( at && a && *a>=atLen) curAt = &at[*a];
		YzisAttribute *curAt = ( !noAttribs && (*a) >= atLen ) ?  &at[ 0 ] : &at[*a];
		if ( curAt ) {
			hl+=*curAt;
			//yzDebug(NYZIS ) << "hl.textColor is" <<  hl.textColor().rgb() << endl;
			mColor = mColormap.contains(hl.textColor().rgb())?
				mColormap[ hl.textColor().rgb() ]:
				mColormap[ Qt::white.rgb() ];
			//yzDebug()<< "nyzis highlighting *********changing color to  "<< mColor << endl;
		}
		waddch(window, COLOR_PAIR(mColor)|str[i].unicode());
//		addch(str[i].unicode());
	if ( a ) a++;
	}



	// end of line...
	for ( ; i< w; i++ ) waddch(window, ' ' );
	wmove(window,sy,sx ); // restore cursor
}

void NYZView::setCommandLineText( const QString& text )
{
	commandline = text;
	mvwaddstr(statusbar, 0, STATUSBARWIDTH,text.latin1());
	waddch( statusbar, ' ' ); // when doing backspace...
	waddch( statusbar, '\b' );
	wrefresh(statusbar);
}

QString NYZView::getCommandLineText() const {
	return commandline;
}

void NYZView::invalidateLine ( unsigned int line ) {
	printLine( line );
	wrefresh( window );
}

void NYZView::modeChanged(void)
{
	switch ( mMode ) {
		case YZ_VIEW_MODE_INSERT: // insert
			displayInfo( tr( "Entering Insert mode" ));
			break;
		case YZ_VIEW_MODE_REPLACE: // replace
			displayInfo( tr( "Entering Replace mode" ));
			break;
		case YZ_VIEW_MODE_COMMAND: // normal
//			if ( modeDisplayed ) displayInfo( tr( "Entering Command mode" ));
			// else : nothing, we keep the previous message..
			break;
		case YZ_VIEW_MODE_EX: //script·
			displayInfo( tr( "EX Mode :" ));
			break;
		case YZ_VIEW_MODE_SEARCH: //search mode
			displayInfo( reverseSearch ? tr( "Reverse Search:" ) : tr( "Search mode :" ));
			break;
	};
	modeDisplayed = true;
}

void NYZView::syncViewInfo( void )
{
	char * myfmt;

	update_info();

	/*
	 * ------------------ infobar ---------------------
	 * ------------------ statusbar -------------------
	 */

	// update infobar
	werase(infobar);
	myfmt="%s%s"; // <- prevent %s in percentage to fubar everything, even if
	            // it's rather unlikely..
	mvwprintw( infobar, 0, 0, myfmt, 
			( mBuffer->fileIsNew() )?"[No File]":mBuffer->fileName().latin1(),
			( mBuffer->fileIsModified() )?" [+]":""
			);
	// prevent  gcc to use string
	if ( viewInformation.c1!=viewInformation.c2 ) {
		myfmt="%d,%d-%d";
		mvwprintw( infobar, 0, w-20, myfmt,
				viewInformation.l+1,
				viewInformation.c1+1,
				viewInformation.c2+1 );
	} else {
		myfmt="%d,%d";
		mvwprintw( infobar, 0, w-20, myfmt, viewInformation.l+1,viewInformation.c1+1 );
	}
	mvwaddstr( infobar, 0, w-9, viewInformation.percentage.latin1() );

	wrefresh(infobar);

	wmove(window, viewInformation.l-getCurrentTop() , viewInformation.c1 ) ;
	wrefresh( window );
}

void NYZView::refreshScreen() {
	unsigned int i;
	for ( i=getCurrentTop(); i < ( getCurrentTop() + mLinesVis ) && i < mBuffer->lineCount(); i++ )
		printLine(i);
	i-=getCurrentTop();
	for ( ; i < mLinesVis ; i++ ) printVoid( i );

	refresh();
	syncViewInfo();
}

void NYZView::displayInfo( const QString& info )
{
	werase(statusbar);
	mvwaddstr( statusbar, 0, 0, info.latin1() );
//	wrefresh(statusbar);
	refresh();
	modeDisplayed = false;
	yzDebug(NYZIS)<< "NYZView::displayInfo message is : " << info << endl;
}

void NYZView::update_info(void)
{
	getmaxyx(window, h, w);
	mLinesVis = h-2;
}


void NYZView::initialisecolormap()
{
	colormapinitialised = true;

	if ( !has_colors() ) {
		yzWarning() << "Terminal doesn't has color capabilities, disabling syntax highlighting" <<endl;
		return;
	}

	start_color();
	yzDebug() << "COLOR_PAIRS is : " << COLOR_PAIRS << endl;

	// magenta = 1, is used to display info on statusbar..
	//
	if ( can_change_color() ) {
		yzWarning() << "Terminal can change colors"<< endl;
#define MAP( nb, color )       \
	init_color( nb, (short)qRed(color.rgb()), (short)qGreen(color.rgb()), (short)qBlue(color.rgb())); \
	init_pair( nb, nb, COLOR_BLACK );    \
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
	init_pair( nb, color, COLOR_BLACK );    \
	mColormap[ qtcolor.rgb() ] = nb;

		MAP( 1, Qt::magenta, COLOR_MAGENTA );
		MAP( 2, Qt::red, COLOR_RED );
		MAP( 3, Qt::green, COLOR_GREEN );
		MAP( 4, Qt::yellow, COLOR_YELLOW );
		MAP( 5, Qt::cyan, COLOR_CYAN );
		MAP( 6, Qt::black, COLOR_BLACK );
		MAP( 7, Qt::blue, COLOR_BLUE );
		MAP( 8, Qt::white, COLOR_WHITE );
		MAP( 9, Qt::gray, COLOR_WHITE );
		MAP(10, Qt::darkGreen, COLOR_GREEN );
		MAP(11, Qt::darkMagenta, COLOR_MAGENTA );
		MAP(12, Qt::darkCyan, COLOR_CYAN );
		MAP(13, Qt::lightGray, COLOR_WHITE );
	}
}

