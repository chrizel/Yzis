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
	TYZView(YZBuffer *buf, YZSession *sess, int lines);

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
		yzDebug( AREA_TESTS) << "TYZView::setCommandLineText '" << text << "'\n";
	}

	virtual void setFocusCommandLine() {
		yzDebug( AREA_TESTS) << "TYZView::setFocusCommandLine" << endl;
	}

	virtual void setFocusMainWindow() {
		yzDebug( AREA_TESTS) << "TYZView::setFocusMainWindow" << endl;
	}

	virtual QString getCommandLineText() const {
		yzDebug( AREA_TESTS) << "TYZView::getCommandLineText" << endl;
		return QString::null;
	}

	virtual void invalidateLine( unsigned int ) {
		yzDebug( AREA_TESTS) << "TYZView::invalidateLine" << endl;
	}

	virtual void setStatusBar( const QString& ) {
		yzDebug( AREA_TESTS) << "TYZView::setStatusBar" << endl;
	}

	virtual void updateCursor( unsigned int, unsigned int, unsigned int, const QString& ) {
		yzDebug( AREA_TESTS) << "TYZView::updateCursor" << endl;
	}

	virtual void refreshScreen( ) {
		yzDebug( AREA_TESTS) << "TYZView::refreshScreen" << endl;
	}

	virtual void syncViewInfo( ) {
		yzDebug( AREA_TESTS) << "TYZView::syncViewInfo" << endl;
	}
	
	virtual void displayInfo( const QString& ) {
		yzDebug( AREA_TESTS) << "TYZView::displayInfo" << endl;
	}

	virtual void modeChanged( ) {
		yzDebug( AREA_TESTS) << "TYZView::modeChanged" << endl;
	}

	virtual void paintEvent( unsigned int /*curx*/, unsigned int /*cury*/, unsigned int /*curw*/, unsigned int /*curh*/ ) {
		yzDebug( AREA_TESTS) << "TYZView::paintEvent" << endl;
	}

	virtual void scrollUp( int ) {
		yzDebug( AREA_TESTS) << "TYZView::scrollUp" << endl;
	}
	virtual void scrollDown( int ) {
		yzDebug( AREA_TESTS) << "TYZView::scrollDown" << endl;
	}

protected:
	class Mapping {
	public:
		Mapping( QString ptext="", int pkey=0) {
			text = ptext;
			key = pkey;
		}
		QString text;
		int key;
	};
	
	QValueList<Mapping> mKeyMap;
};

#endif // TYZ_VIEW_H
