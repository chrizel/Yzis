/**
 * $Id$
 */

#ifndef YZIS_CURSOR
#define YZIS_CURSOR

#include "yz_view.h"

/**
 * This class handles the cursor information.
 * Maybe we can have more than one cursor per view someday :) ( multiple
 * developers remotely editing the same file :)
 */

class YZCursor {
	public :
		YZCursor(YZView *vp);
		~YZCursor();

		inline void setX(int x) { x_pos = x; }

		inline void setY(int y) { y_pos = y; }

		inline int getX() { return x_pos; }

		inline int getY() { return y_pos; }

/*
		inline void incX(int nb=1) { x_pos+=nb; }

		inline void incY(int nb=1) { y_pos+=nb; }

		inline void decX(int nb=1) { x_pos-=nb; }

		inline void decY(int nb=1) { y_pos-=nb; }
*/

	private :
		YZView *parentView;

		/* position relative to the whole file */
		int x_pos;
		int y_pos;
};

#endif
