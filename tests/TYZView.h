/* This file is part of the Yzis libraries
 *  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/


#ifndef TYZ_VIEW_H
#define TYZ_VIEW_H
/**
 * $Id: session.h 388 2004-03-03 23:44:13Z mikmak $
 */

#include <libyzis/view.h>
#include <libyzis/debug.h>

class TYZView : public YZView
{
public:
	TYZView(YZBuffer *buf, YZSession *sess, int lines)
	: YZView( buf, sess, lines )
	{}

	/** Send text to the view, that will be sent as individual chars
	  * with the sendKey()
	  *
	  * \t and \n are converted into the right keys. Upper case letters are
	  * converted into lower case letters with a shift modifier.
	  * <Esc>, <Tab>, <C-A> are also supported syntax.
	  */
	void sendText( QString text );

	uint getCursorX() { return mCursor->getX(); }
	uint getCursorY() { return mCursor->getY(); }
	uint getCursorLine() { return mCursor->getY(); }
	uint getCursorCol() { return mCursor->getX(); }

	// Reimplemented to please compilation
	
	virtual void setCommandLineText( const QString& text) {
		yzDebug() << "TYZView::setCommandLineText '" << text << "'\n";
	}

	virtual void setFocusCommandLine() {
		yzDebug() << "TYZView::setFocusCommandLine" << endl;
	}

	virtual void setFocusMainWindow() {
		yzDebug() << "TYZView::setFocusMainWindow" << endl;
	}

	virtual QString getCommandLineText() const {
		yzDebug() << "TYZView::getCommandLineText" << endl;
		return QString::null;
	}

};

#endif // TYZ_VIEW_H
