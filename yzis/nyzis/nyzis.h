/**
 * nyzis.h
 *
 * ncurses-based GUI for yzis
 *
 */



#include <curses.h>
#include "yz_interface.h"



class NYZView : public YZView  {
public:
	/**
	  * constructor. Each view is binded to a buffer, @param lines is the initial number of lines that
	  * this view can display
	  */
	NYZView(YZBuffer *b, int lines) : YZView(b,lines) { }

};

