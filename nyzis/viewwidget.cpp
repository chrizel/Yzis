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

#include <ctype.h>

#include "viewwidget.h"
#include "factory.h"

NYZView::NYZView(WINDOW *_window, YZBuffer *b)
	: YZView(b,NYZFactory::self,0)
{
	window = _window;

	wmove(window,0,0 );
	//active symbols for special keycodes
	keypad(window , true);

	// creates layout
	/*
	 * ------------------ infobar ---------------------
	 * statusbar |     command 
	 */

	update_info();
	touchwin( window ); // throw away optimisations because we're going to subwin , as said in doc
//	WINDOW *window = subwin(screen, LINES-2, 0, 0, 0);
	infobar = subwin(window, 1, 0, h-2, 0);
	wattrset(infobar, A_STANDOUT || A_BOLD);
	wbkgd(infobar, A_REVERSE);
	statusbar  = subwin(window, 1, STATUSBARWIDTH, h-1, 0);
	commandbar = subwin(window, 1, 0, h-1, STATUSBARWIDTH);
	//	(void) notimeout(stdscr,TRUE);/* prevents the delay between hitting <escape> and when we actually receive the event */
	//	(void) notimeout(window,TRUE);/* prevents the delay between hitting <escape> and when we actually receive the event */

	if (has_colors()) {
		//		wattron(infobar, COLOR_PAIR(4));
		wattron(statusbar, COLOR_PAIR(6));
		wattron(commandbar, COLOR_PAIR(4));
	}

	// TODO  write something like :       "bernoulli.tex" [noeol] 65L, 1440C
	// in last line (vim-like)
	setStatusText ( b->fileName() + QString(" %1L" ).arg(b->lineCount()));
	redrawScreen();
}

NYZView::~NYZView(){
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
	for (i=0; i<w && i<str.length(); i++)
		waddch(window, str[i].unicode());
//		addch(str[i].unicode());
	for ( ; i< w; i++ ) waddch(window, ' ' );
	wmove(window,sy,sx ); // restore cursor
}

void NYZView::setCommandLineText( const QString& text )
{
	commandline = text;
	werase(commandbar);
	waddstr(commandbar, text.local8Bit());
	wrefresh(commandbar);
}

QString NYZView::getCommandLineText() const {
	return commandline;
}

void NYZView::invalidateLine ( unsigned int line ) {
	printLine( line );
	wrefresh( window );
}

void NYZView::setStatusBar( const QString& text ) {
	// TODO : factorize..
	setStatusText (text);
}

void NYZView::syncViewInfo( void )
{
	char * myfmt;

	update_info();

	/*
	 * ------------------ infobar ---------------------
	 * statusbar |     command 
	 */

	// update infobar
	werase(infobar);
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

	myfmt="%s"; // <- prevent %s in percentage to fubar everything, even if
	            // it's rather unlikely..
	mvwprintw( infobar, 0, w-9, myfmt, viewInformation.percentage.latin1() );

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

void NYZView::displayInfo( const QString& info ) {
//TODO
//
	QTimer::singleShot(2000, this, SLOT( resetInfo() ) );
}

void NYZView::resetInfo()
{
//	status->changeItem( "", 80 );
}

void NYZView::update_info(void)
{
	getmaxyx(window, h, w);
	mLinesVis = h;
}

void NYZView::setStatusText( const QString& text )
{
	werase(statusbar);
	waddstr(statusbar, text.latin1());
	wrefresh(statusbar);
}
