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
#include <qnamespace.h>
#include <ctype.h>
#include "viewwidget.h"
#include "factory.h"

NYZView::NYZView(WINDOW *_window, YZBuffer *b)
	: YZView(b,NYZFactory::self,0)
{
	window = _window;

	update_info();
	wmove(window,0,0 );
	//active symbols for special keycodes
	keypad(window , true);

	// TODO  write something like :       "bernoulli.tex" [noeol] 65L, 1440C
	// in last line (vim-like)
	NYZFactory::self->setStatusText (
			b->fileName() + QString(" %1L" ).arg(b->lineCount())
			);
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
	wmove (window, relline, 0);
	for (i=0; i<w && i<str.length(); i++)
		waddch(window, str[i].unicode());
//		addch(str[i].unicode());
	for ( ; i< w; i++ ) waddch(window, ' ' );
	wmove(window,sy,sx ); // restore cursor
}

void NYZView::setCommandLineText( const QString& text )
{
	NYZFactory::self->setCommandLine( text );
}

QString NYZView::getCommandLineText() const {
	return NYZFactory::self->getCommandLine();
}

void NYZView::invalidateLine ( unsigned int line ) {
	printLine( line );
	wrefresh( window );
}

void NYZView::setStatusBar( const QString& text ) {
	NYZFactory::self->setStatusText (text);
}

void NYZView::updateCursor ( unsigned int line, unsigned int x1, unsigned int x2, const QString& percentage) {
	NYZFactory::self->update_infobar(line+1, x1+1, x2+1, percentage);
	wmove(window, line-getCurrentTop() , x1 ) ;
	wrefresh( window );
}

void NYZView::refreshScreen() {
	unsigned int i;
	for ( i=getCurrentTop(); i < ( getCurrentTop() + mLinesVis ) && i < mBuffer->lineCount(); i++ )
		printLine(i);
	i-=getCurrentTop();
	for ( ; i < mLinesVis ; i++ ) printVoid( i );
}

void NYZView::displayInfo( const QString& info ) {
//TODO
}

void NYZView::setInformation( const QString& info ) {
//TODO
}
