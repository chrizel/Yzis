/**
 * $Id$
 */

#include "cursor.h"

YZCursor::YZCursor(YZView *vp) {
	parentView=vp;
	x_pos=0;
	y_pos=0;
}

YZCursor::~YZCursor() {
}


