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
		void setX(int x);
		void setY(int y);
		int getX();
		int getY();
		void incX(int nb=1);
		void incY(int nb=1);
		void decX(int nb=1);
		void decY(int nb=1);
		
	private :
		YZView *parentView;
		
		/* position relative to the whole file */
		int x_pos;
		int y_pos;
};

#endif
