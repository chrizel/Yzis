
#include <curses.h>
#include <signal.h>
#include <stdlib.h>

#include "nyzis.h"

static void finish(int sig);
static void finish2(void);
void nyz_init_screen(void);
void handle_event(yz_event *);


// global variables..
YZBuffer *buffer;
NYZView *view;


FILE *debugstr;

int
main(int argc, char *argv[])
{

	debugstr = fopen("/tmp/yzis.log", "a");

	(void) signal(SIGINT, finish);      /* arrange interrupts to terminate */

	atexit(finish2);
	nyz_init_screen();


	/*
	 * create an empty buffer and a view on it
	 */
	buffer = new YZBuffer("tests/test1");
	view = new NYZView(buffer, LINES);

	debug("before event_loop");
	view->event_loop();	// should not return
	debug("after event_loop");

	error();

	finish(0);               /* we're done */
}

void nyz_init_screen(void)
{
    (void) initscr();      /* initialize the curses library */
    keypad(stdscr, TRUE);  /* enable keyboard mapping */
    (void) nonl();         /* tell curses not to do NL->CR/NL on output */
    (void) cbreak();       /* take input chars one at a time, no wait for \n */
    (void) noecho();         /* echo input - in color */
    (void) nodelay(stdscr, TRUE);

    if (has_colors()) {

        start_color();

        /*
         * Simple color assignment, often all we need.  Color pair 0 cannot
         * be redefined.  This example uses the same value for the color
         * pair as for the foreground color, though of course that is not
         * necessary:
         */
        init_pair(1, COLOR_RED,     COLOR_BLACK);
        init_pair(2, COLOR_GREEN,   COLOR_BLACK);
        init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(4, COLOR_BLUE,    COLOR_BLACK);
        init_pair(5, COLOR_CYAN,    COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_WHITE,   COLOR_BLACK);
    }
}

static void finish2(void)
{
	debug("finish2 called (from atexit)");
	finish(0);
}
static void finish(int sig)
{
	/* ncurses stuff */
	endwin();

	debug("finish called (sigint)");
	/* other */

	/* exit */
	exit(0);
}

 
