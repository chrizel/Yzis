/**
 * $Id: yz_cursor.cpp,v 1.3 2003/04/25 12:45:30 mikmak Exp $
 */

#include "yz_cursor.h"

YZCursor::YZCursor(YZView *vp) {
	parentView=vp;
	x_pos=0;
	y_pos=0;
}

YZCursor::~YZCursor() {
}

void YZCursor::setX(int x) {
	x_pos = x;
}

void YZCursor::setY(int y) {
	y_pos = y;
}

int YZCursor::getX() {
	return x_pos;
}

int YZCursor::getY() {
	return y_pos;
}

void YZCursor::incX(int nb) {
	x_pos+=nb;
}

void YZCursor::incY(int nb) {
	y_pos+=nb;
}

void YZCursor::decX(int nb) {
	x_pos-=nb;
}

void YZCursor::decY(int nb) {
	y_pos-=nb;
}

