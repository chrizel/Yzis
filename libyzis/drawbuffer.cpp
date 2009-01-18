/*  This file is part of the Yzis libraries
*  Copyright (C) 2006-2008 Loic Pauleve <panard@inzenet.org>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
**/

#include "drawbuffer.h"

#include "debug.h"
#define dbg()    yzDebug("YDrawBuffer")
#define err()    yzError("YDrawBuffer")

#include "drawline.h"
#include "drawcell.h"
#include "view.h"
#include "viewcursor.h"

/************************
 * YDrawBuffer
 ************************/

YDrawBuffer::YDrawBuffer( const YView* view, int columns, int lines ) :
	mScreenOffset(0,0),
	mView(view)
{
	mFirstBufferLine = 0;
	mScreenTopBufferLine = 0;
	mEOLCell.step(" ");
	setScreenSize(columns, lines);
	mContent << (YDrawSection() << YDrawLine());
}

void YDrawBuffer::setScreenSize( int columns, int lines ) 
{
	YASSERT(mScreenWidth > 0);
	YASSERT(mScreenHeight > 0);
	mScreenWidth = columns;
	mScreenHeight = lines;
}

int YDrawBuffer::currentHeight() const
{
	int dy = 0;
	for ( int bl = mScreenTopBufferLine; dy < mScreenHeight && bl <= lastBufferLine(); ++bl ) {
		dy += mContent[bl-mFirstBufferLine].count();
	}
	return dy;
}

YInterval YDrawBuffer::setBufferDrawSection( int bl, YDrawSection ds )
{
	int lid = bl - mFirstBufferLine;
	YASSERT(lid >= 0);
	YASSERT(lid <= mContent.count());
	YInterval affected;

	int delta = 0;
	if ( lid == mContent.count() ) {
		mContent << ds;
	} else {
		delta = ds.count() - mContent[lid].count();
		mContent.replace(lid, ds);
	}

	if ( bl >= mScreenTopBufferLine ) {
		int screen_line = bufferDrawSectionScreenLine(bl);
		if ( screen_line < mScreenHeight ) {
			affected.setFrom(YBound(YCursor(0, screen_line)));
			if ( delta != 0 ) { // repaint all bottom
				affected.setTo(YBound(YCursor(0, mScreenHeight),true));
			} else { // repaint only the line
				affected.setTo(YBound(YCursor(0, screen_line+ds.count()), true));
			}
		}
	}

	return affected;
}
YInterval YDrawBuffer::deleteFromBufferDrawSection( int bl )
{
	int lid = bl - mFirstBufferLine;
	YASSERT(0 <= lid)
	YInterval affected;
	if ( lid >= mContent.count() ) {
		return affected;
	}
	int height = 0;
	while ( lid < mContent.count() ) {
		height += mContent[lid].count();
		mContent.takeAt(lid);
	}
	if ( bl >= mScreenTopBufferLine ) {
		/* compute screenY */
		int dy = bufferDrawSectionScreenLine(bl);
		if ( dy < mScreenHeight ) {
			affected.setFrom(YBound(YCursor(0,dy)));
			affected.setTo(YBound(YCursor(0,qMin(mScreenHeight, dy+height)), true));
		}
	}
	return affected;
}

YInterval YDrawBuffer::addSelection( yzis::SelectionType sel, const YInterval& i, yzis::IntervalType itype )
{
	YDrawBufferIterator it = iterator(i, itype);
	if ( !it.isValid() ) {
		return YInterval();
	}
	YCursor begin(/*TODO*/0, it.screenLine());
	int lastScreenLine = 0;
	int lastScreenColumn = screenWidth(); /*TODO*/
	for ( ; it.isValid(); it.next() ) {
		it.cell()->addSelection(sel);
		lastScreenLine = it.screenLine();
		/*lastScreenColumn = it.screenColumn() + it.cell()->width();*/
	}
	return YInterval(begin, YCursor(lastScreenColumn, lastScreenLine));
}

YInterval YDrawBuffer::delSelection( yzis::SelectionType sel, const YInterval& i, yzis::IntervalType itype )
{
	YDrawBufferIterator it = iterator(i, itype);
	if ( !it.isValid() ) {
		return YInterval();
	}
	YCursor begin(/*TODO*/0, it.screenLine());
	int lastScreenLine = 0;
	int lastScreenColumn = screenWidth(); /*TODO*/
	for ( ; it.isValid(); it.next() ) {
		it.cell()->delSelection(sel);
		lastScreenLine = it.screenLine();
		/*lastScreenColumn = it.screenColumn() + it.cell()->width();*/
	}
	return YInterval(begin, YCursor(lastScreenColumn, lastScreenLine));
}

YDebugStream& operator<< ( YDebugStream& out, const YDrawBuffer& buff )
{
	int dy = 0;
    for ( int i = 0; i < buff.mContent.size(); ++i ) {
		for ( int j = 0; j < buff.mContent[i].count(); ++j ) {
			out << dy++ << ": " << buff.mContent[i][j] << endl;
		}
    }
    return out;
}

void YDrawBuffer::setEOLCell( const YDrawCell& cell )
{
	mEOLCell = cell;
}

YDrawBufferConstIterator YDrawBuffer::const_iterator( const YInterval& i, yzis::IntervalType itype )
{
	YDrawBufferConstIterator it = YDrawBufferConstIterator(this);
	it.setup(i, itype);
	return it;
}
YDrawBufferIterator YDrawBuffer::iterator( const YInterval& i, yzis::IntervalType itype )
{
	YDrawBufferIterator it = YDrawBufferIterator(this);
	it.setup(i, itype);
	return it;
}

const YDrawSection YDrawBuffer::bufferDrawSection( int bl ) const
{
	int sid = bl - mFirstBufferLine;
	YASSERT(0 <= sid)
	YASSERT(sid < mContent.count());
	return mContent[sid];
}
int YDrawBuffer::bufferDrawSectionScreenLine( int bl ) const
{
	YASSERT(bl >= mScreenTopBufferLine);
	YASSERT(bl <= lastBufferLine()+1);
	int sl = 0;
	for ( int l = mScreenTopBufferLine; l < bl; ++l ) {
		sl += mContent[l-mFirstBufferLine].count();
	}
	return sl;
}

int YDrawBuffer::screenTopBufferLine() const
{
	return mScreenTopBufferLine;
}
int YDrawBuffer::screenBottomBufferLine() const
{
	int height = 0;
	for ( int sid = mScreenTopBufferLine - mFirstBufferLine; sid < mContent.count(); ++sid ) {
		height += mContent[sid].count();
		if ( height >= mScreenHeight ) {
			return sid + mFirstBufferLine;
		}
	}
	return mContent.count() - 1 + mFirstBufferLine;
}

bool YDrawBuffer::scrollForViewCursor( const YViewCursor& vc, int* scroll_horizontal, int* scroll_vertical )
{
	int sid;
	targetBufferLine( vc.line(), &sid );
	*scroll_horizontal = 0;
	int oldScreenTopBufferLine = mScreenTopBufferLine;
	if ( vc.line() < screenTopBufferLine() ) {
		mScreenTopBufferLine = vc.line();
		int delta = 0;
		for ( int bl = mScreenTopBufferLine; bl < oldScreenTopBufferLine; ++bl ) {
			delta += mContent[bl-mFirstBufferLine].count();
		}
		*scroll_vertical = -delta;
		return true;
	} else if ( vc.line() > screenBottomBufferLine() ) {
		int height = 0;
		int bl = vc.line();
		for ( ; bl >= 0; --bl ) {
			height += mContent[bl-mFirstBufferLine].count();
			if ( height == mScreenHeight ) {
				mScreenTopBufferLine = bl;
				break;
			} else if ( height > mScreenHeight ) {
				mScreenTopBufferLine = bl + 1; //TODO: what if height(vc.line()) > mScreenHeight?
				break;
			}
		}
		if ( bl < 0 ) mScreenTopBufferLine = 0;

		YASSERT(mScreenTopBufferLine >= oldScreenTopBufferLine);
		int delta = 0;
		for ( bl = mScreenTopBufferLine; bl > oldScreenTopBufferLine; --bl ) {
			delta += mContent[bl-mFirstBufferLine].count();
		}
		*scroll_vertical = delta;
		return true;
	}
	return false;
}

bool YDrawBuffer::scrollLineToTop( int line, int* scroll_horizontal, int* scroll_vertical )
{
	int sid;
	targetBufferLine(line, &sid);
	*scroll_horizontal = 0;
	if ( line < mScreenTopBufferLine ) {
		int delta = 0;
		for ( int bl = line; bl < mScreenTopBufferLine; ++bl ) {
			delta += mContent[bl-mFirstBufferLine].count();
		}
		mScreenTopBufferLine = line;
		*scroll_vertical = -delta;
		return true;
	} else if ( line > mScreenTopBufferLine ) {
		int delta = 0;
		for ( int bl = mScreenTopBufferLine; bl < line; ++bl ) {
			delta += mContent[bl-mFirstBufferLine].count();
		}
		mScreenTopBufferLine = line;
		*scroll_vertical = delta;
		fill();
		return true;
	}
	return false;
}

bool YDrawBuffer::scrollLineToBottom( int line, int* scroll_horizontal, int* scroll_vertical )
{
	int sid;
	targetBufferLine(line, &sid);
	int height = mContent[line-mFirstBufferLine].count();
	int topLine = line;
	while ( topLine > 0 && height < mScreenHeight ) {
		targetBufferLine(topLine-1, &sid);
		int h = mContent[sid].count();
		if ( height + h <= mScreenHeight ) {
			height += h;
			--topLine;
		} else {
			break;
		}
	}
	return scrollLineToTop(topLine, scroll_horizontal, scroll_vertical);
}

bool YDrawBuffer::scrollLineToCenter( int line, int* scroll_horizontal, int* scroll_vertical )
{
	int sid;
	targetBufferLine(line, &sid);
	int halfHeight = mScreenHeight/2 - 1 + mScreenHeight%2;
	int topLine = line;
	int height = 0;
	while ( topLine > 0 && height < halfHeight ) {
		targetBufferLine(topLine-1, &sid);
		int h = mContent[sid].count();
		if ( height + h <= halfHeight + 1 ) {
			--topLine;
			height += h;
		} else {
			break;
		}
	}
	return scrollLineToTop(topLine, scroll_horizontal, scroll_vertical);
}

YInterval YDrawBuffer::fill()
{
	YInterval affected;
	int h = currentHeight();
	if ( h < mScreenHeight ) {
		int bl = screenBottomBufferLine();
		int max_bl = mView->buffer()->lineCount();
		affected.setFrom(YBound(YCursor(0,h)));
		for ( ++bl; h < mScreenHeight && bl < max_bl; ++bl ) {
			YDrawSection ds = mView->drawSectionOfBufferLine(bl);
			h += ds.count();
			mContent << ds;
		}
		affected.setTo(YBound(YCursor(h,0),true));
	}
	return affected;
}

bool YDrawBuffer::targetBufferLine( int bline, int* sid )
{
	YASSERT(bline >= 0);
	if ( bline < mFirstBufferLine ) {
		for ( int bl = mFirstBufferLine - 1; bl >= bline; --bl ) {
			mContent.insert(0, mView->drawSectionOfBufferLine(bl));
		}
		mFirstBufferLine = bline;
	} else if ( bline - mFirstBufferLine >= mContent.count() ) {
		for ( int bl = mContent.count() + mFirstBufferLine; bl <= bline; ++bl ) {
			mContent << mView->drawSectionOfBufferLine(bl);
		}
	}
	*sid = bline - mFirstBufferLine;
	return true;
}
int YDrawBuffer::targetBufferColumn( int bcol, int sid, int* lid, int* cid, int* bshift, int* column ) const
{
	YASSERT(0 <= bcol);
	int w = 0;
	int my_lid = 0;
	*bshift = -1;
	int max_lid = mContent[sid].count() - 1;
	for( ; my_lid <= max_lid; ++my_lid ) {
		int lw = mContent[sid][my_lid].length();
		if ( w + lw > bcol || my_lid == max_lid ) {
			break;
		} else {
			w += lw;
		}
	}
	int my_column = 0;
	if ( column != NULL ) {
		my_column = my_lid * mScreenWidth;
	}
	int my_cid = 0;
	while( my_cid < mContent[sid][my_lid].count() ) { //; ++my_cid ) {
		int cw = mContent[sid][my_lid][my_cid].length();
		if ( w + cw > bcol ) {
			*bshift = bcol - w;
			if ( column != NULL ) {
				my_column += mContent[sid][my_lid][my_cid].widthForLength(*bshift);
			}
			break;
		} else {
			if ( my_cid < mContent[sid][my_lid].count() - 1 ) {
				*bshift = 0;
				w += cw;
				if ( column != NULL ) {
					my_column += mContent[sid][my_lid][my_cid].width();
				}
				++my_cid;
			} else { // last cell
				*bshift = cw - 1;
				if ( column != NULL ) {
					my_column += mContent[sid][my_lid][my_cid].widthForLength(*bshift);
				}
				break;
			}
		}
	}
	*lid = my_lid;
	*cid = my_cid;
	if ( column != NULL ) {
		*column = my_column;
	}
	int position = w + *bshift;
	return position;
}

bool YDrawBuffer::targetScreenLine( int sline, int* sid, int* lid, int* bline ) const
{
	YASSERT(0 <= sline);
	YASSERT(sline < screenHeight());
	bool found = false;
	int my_sid = mScreenTopBufferLine - mFirstBufferLine;
	int my_lid = 0;
	int h = 0;
	while( my_sid < mContent.count() ) {
		int sh = mContent[my_sid].count();
		if ( h + sh > sline ) {
			my_lid = sline - h;
			found = true;
			break;
		} else {
			if ( my_sid < mContent.count() - 1 ) {
				h += sh;
				++my_sid;
			} else {
				my_lid = mContent[my_sid].count() - 1;
				break;
			}
		}
	}
	YASSERT(0 <= my_lid);
	YASSERT(my_lid < mContent[my_sid].count());
	*sid = my_sid;
	*lid = my_lid;
	if ( bline != NULL ) *bline = my_sid + mFirstBufferLine;
	return found;
}
int YDrawBuffer::targetScreenColumn( int scol, int sid, int lid, int* cid, int* sshift, int* position ) const
{
	YASSERT(0 <= scol);
	YASSERT(scol < screenWidth());
	YASSERT(lid < mContent[sid].count());
	int my_cid = 0;
	int w = 0;
	*sshift = -1;
	int my_position = 0;
	if ( position != NULL ) {
		for ( int i = 0; i < lid; ++i ) {
			my_position += mContent[sid][i].length();
		}
	}
	while( my_cid < mContent[sid][lid].count() ) {
		int cw = mContent[sid][lid][my_cid].width();
		if ( w + cw > scol ) {
			*sshift = scol - w;
			if ( position != NULL ) {
				my_position += mContent[sid][lid][my_cid].lengthForWidth(*sshift);
			}
			break;
		} else {
			if ( my_cid < mContent[sid][lid].count() - 1 ) {
				*sshift = 0;
				w += cw;
				if ( position != NULL ) {
					my_position += mContent[sid][lid][my_cid].length();
				}
				++my_cid;
			} else {
				*sshift = cw - 1;
				if ( position != NULL ) {
					my_position += mContent[sid][lid][my_cid].lengthForWidth(*sshift);
				}
				break;
			}
		}
	}
	*cid = my_cid;
	if ( position != NULL ) *position = my_position;
	int column = w + *sshift;
	return column;
}

/* LOCK MANAGEMENT */
void YDrawBuffer::acquireLine( int line )
{
	bool found = false;
	int i = 0;
	for( ; i < mLocks.size(); i++ ) {
		if ( mLocks.at(i).line == line ) {
			mLocks[i].count++;
			found = true;
			break;
		} else if ( mLocks.at(i).line > line ) {
			break;
		}
	}
	if ( !found )
		mLocks.insert(i, YDrawLineLock(line, 1));
}
void YDrawBuffer::releaseLine( int line )
{
	for ( int i = 0; i < mLocks.size(); ++i ) {
		if ( mLocks.at(i).line == line ) {
			mLocks[i].count--;
			if ( mLocks.at(i).count == 0 ) {
				mLocks.removeAt(i);
			}
			break;
		}
	}
}
void YDrawBuffer::squeeze()
{
	int firstLine = mScreenTopBufferLine;
	int lastLine = screenBottomBufferLine();
	if ( mLocks.size() > 0 ) {
		firstLine = qMin(firstLine, mLocks.at(0).line);
		lastLine = qMax(lastLine, mLocks.at(mLocks.size()-1).line);
	}

	dbg() << "squeeze: firstLine=" << firstLine << "; lastLine=" << lastLine << "; mFirstBufferLine=" << mFirstBufferLine << "; size=" << mContent.size() << endl;
	int delIdx = lastLine - mFirstBufferLine + 1;
	int nbdel = mContent.size() - delIdx;
	while ( nbdel-- > 0 ) {
		mContent.takeAt(delIdx);
	}
	nbdel = firstLine - mFirstBufferLine;
	while ( nbdel-- > 0 ) {
		mContent.takeAt(0);
	}
	mFirstBufferLine = firstLine;
	dbg() << "      : firstLine=" << firstLine << "; lastLine=" << lastLine << "; mFirstBufferLine=" << mFirstBufferLine << "; size=" << mContent.size() << endl;
}

