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

	if (has_colors()) {
		//		wattron(infobar, COLOR_PAIR(4));
		wattron(statusbar, COLOR_PAIR(6));
	}

	// TODO  write something like :       "bernoulli.tex" [noeol] 65L, 1440C
	// in last line (vim-like)
	displayInfo ( QString("\"%1\" %2L, %3C" ).arg(b->fileName()).arg(b->lineCount()).arg(b->getWholeTextLength()));
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
			displayInfo( tr( "Entering Command mode" ));
			break;
		case YZ_VIEW_MODE_EX: //script·
			displayInfo( tr( "EX Mode :" ));
			break;
		case YZ_VIEW_MODE_SEARCH: //search mode
			displayInfo( reverseSearch ? tr( "Reverse Search:" ) : tr( "Search mode :" ));
			break;
	};
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
	wrefresh(statusbar);
//	QTimer::singleShot(2000, this, SLOT( resetInfo() ) );
}

void NYZView::resetInfo()
{
	syncViewInfo();
}

void NYZView::update_info(void)
{
	getmaxyx(window, h, w);
	mLinesVis = h-2;
}

