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
#include "factory.h"
#include <cstdlib>
#include <cstdio>
#include <csignal>

static void finish(int sig);
static void finish2(void);
void nyz_init_screen(void);
void handle_event(yz_event *);


FILE *debugstr;

int
main(int argc, char *argv[])
{

	debugstr = fopen("/tmp/yzis.log", "a");

	(void) signal(SIGINT, finish);      /* arrange interrupts to terminate */
	atexit(finish2);

	(new NYZSession(argc,argv)) -> event_loop();

//	error("should never reach this point");
	finish(0);               /* we're done */
}

static void finish2(void)
{
	//debug("finish2 called (from atexit)");
	finish(0);
}

static void finish(int /*sig*/)
{
	/* ncurses stuff */
	endwin();

	//debug("finish called (sigint)");
	/* other */

	/* exit */
	exit(0);
}

 
