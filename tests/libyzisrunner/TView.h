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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/


#ifndef TYZ_VIEW_H
#define TYZ_VIEW_H

#include "libyzis/view.h"
#include "libyzis/viewcursor.h"
#include "libyzis/debug.h"

class TYZView : public YZView
{
public:
	TYZView(YZBuffer *buf, YZSession *sess, int lines=50) : YZView(buf,sess,lines) {}

	uint getCursorX() { return viewCursor().bufferX(); }
	uint getCursorY() { return viewCursor().bufferY(); }
	uint getCursorLine() { return viewCursor().bufferY(); }
	uint getCursorCol() { return viewCursor().bufferX(); }

	// Reimplemented to please compilation

	virtual void setCommandLineText( const QString& text) {
		yzDebug( "TYZView") << "TYZView::setCommandLineText '" << text << "'\n";
		mCommandLine = text;
	}

	virtual void setFocusCommandLine() {
		yzDebug( "TYZView") << "TYZView::setFocusCommandLine" << endl;
	}

	virtual void setFocusMainWindow() {
		yzDebug( "TYZView") << "TYZView::setFocusMainWindow" << endl;
	}

	virtual QString getCommandLineText() const {
		yzDebug( "TYZView") << "TYZView::getCommandLineText" << endl;
		return mCommandLine;
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
	virtual void registerModifierKeys(const QString&) {
		return;
	}

	virtual void paintEvent( const YZSelection& ) {
	}
	
	virtual bool popupFileSaveAs() { return false; }
	virtual void filenameChanged() {}
	virtual void highlightingChanged() {}
	void preparePaintEvent(int, int) {}
	void endPaintEvent() {}
	void drawCell(int, int, const YZDrawCell&, void*) {}
	void drawClearToEOL(int, int, const QChar&) {}
	void drawSetMaxLineNumber(int) {}
	void drawSetLineNumber(int, int, int) {}
	
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

	QString mCommandLine;
	QList<Mapping> mKeyMap;
};

#endif // TYZ_VIEW_H
