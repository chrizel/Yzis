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
#include <libyzis/viewcursor.h>
#include <libyzis/debug.h>

class TYZView : public YZView
{
public:
	TYZView(YZBuffer *buf, YZSession *sess, int lines);

	uint getCursorX() { return mainCursor->bufferX(); }
	uint getCursorY() { return mainCursor->bufferY(); }
	uint getCursorLine() { return mainCursor->bufferY(); }
	uint getCursorCol() { return mainCursor->bufferX(); }

	// Reimplemented to please compilation
	
	virtual void setCommandLineText( const QString& text) {
		yzDebug( "TYZView") << "TYZView::setCommandLineText '" << text << "'\n";
	}

	virtual void setFocusCommandLine() {
		yzDebug( "TYZView") << "TYZView::setFocusCommandLine" << endl;
	}

	virtual void setFocusMainWindow() {
		yzDebug( "TYZView") << "TYZView::setFocusMainWindow" << endl;
	}

	virtual QString getCommandLineText() const {
		yzDebug( "TYZView") << "TYZView::getCommandLineText" << endl;
		return QString::null;
	}

	virtual void invalidateLine( unsigned int ) {
		yzDebug( "TYZView") << "TYZView::invalidateLine" << endl;
	}

	virtual void setStatusBar( const QString& ) {
		yzDebug( "TYZView") << "TYZView::setStatusBar" << endl;
	}

	virtual void updateCursor( unsigned int, unsigned int, unsigned int, const QString& ) {
		yzDebug( "TYZView") << "TYZView::updateCursor" << endl;
	}

	virtual void refreshScreen( ) {
		yzDebug( "TYZView") << "TYZView::refreshScreen" << endl;
	}

	virtual void syncViewInfo( ) {
		yzDebug( "TYZView") << "TYZView::syncViewInfo" << endl;
	}
	
	virtual void displayInfo( const QString& ) {
		yzDebug( "TYZView") << "TYZView::displayInfo" << endl;
	}

	virtual void modeChanged( ) {
		yzDebug( "TYZView") << "TYZView::modeChanged" << endl;
	}

	virtual void paintEvent( unsigned int /*curx*/, unsigned int /*cury*/, unsigned int /*curw*/, unsigned int /*curh*/ ) {
		yzDebug( "TYZView") << "TYZView::paintEvent" << endl;
	}

	virtual void scrollUp( int ) {
		yzDebug( "TYZView") << "TYZView::scrollUp" << endl;
	}
	virtual void scrollDown( int ) {
		yzDebug( "TYZView") << "TYZView::scrollDown" << endl;
	}
	virtual unsigned int stringWidth(const QString&str) const {
		return str.length();
	}
	virtual unsigned int charWidth(const QChar&) const {
		return 1;
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
