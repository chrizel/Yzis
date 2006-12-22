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


#ifndef NOGUI_VIEW_H
#define NOGUI_VIEW_H

#include "libyzis/view.h"
#include "libyzis/viewcursor.h"
#include "libyzis/debug.h"
#include "libyzis/buffer.h"

class NoGuiView : public YZView
{
public:
	NoGuiView(YZBuffer *buf, YZSession *sess, int lines=50) : YZView(buf,sess,lines) {}

	uint getCursorX() { return viewCursor().bufferX(); }
	uint getCursorY() { return viewCursor().bufferY(); }
	uint getCursorLine() { return viewCursor().bufferY(); }
	uint getCursorCol() { return viewCursor().bufferX(); }

	// Reimplemented to please compilation

	virtual void setCommandLineText( const QString& text) {
		yzDebug( "NoGuiView") << "NoGuiView::setCommandLineText '" << text << "'\n";
		mCommandLine = text;
	}

	virtual void setFocusCommandLine() {
		yzDebug( "NoGuiView") << "NoGuiView::setFocusCommandLine" << endl;
	}

	virtual void setFocusMainWindow() {
		yzDebug( "NoGuiView") << "NoGuiView::setFocusMainWindow" << endl;
	}

	virtual QString getCommandLineText() const {
		yzDebug( "NoGuiView") << "NoGuiView::getCommandLineText" << endl;
		return mCommandLine;
	}

	virtual void invalidateLine( unsigned int ) {
		yzDebug( "NoGuiView") << "NoGuiView::invalidateLine" << endl;
	}

	virtual void setStatusBar( const QString& ) {
		yzDebug( "NoGuiView") << "NoGuiView::setStatusBar" << endl;
	}

	virtual void updateCursor( unsigned int, unsigned int, unsigned int, const QString& ) {
		yzDebug( "NoGuiView") << "NoGuiView::updateCursor" << endl;
	}

	virtual void refreshScreen( ) {
		yzDebug( "NoGuiView") << "NoGuiView::refreshScreen" << endl;
	}

	virtual void syncViewInfo( ) {
		yzDebug( "NoGuiView") << "NoGuiView::syncViewInfo" << endl;
	}

	virtual void displayInfo( const QString& ) {
		yzDebug( "NoGuiView") << "NoGuiView::displayInfo" << endl;
	}

	virtual void modeChanged( ) {
		yzDebug( "NoGuiView") << "NoGuiView::modeChanged" << endl;
	}

	virtual void paintEvent( unsigned int /*curx*/, unsigned int /*cury*/, unsigned int /*curw*/, unsigned int /*curh*/ ) {
		yzDebug( "NoGuiView") << "NoGuiView::paintEvent" << endl;
	}

	virtual void scrollUp( int ) {
		yzDebug( "NoGuiView") << "NoGuiView::scrollUp" << endl;
	}
	virtual void scrollDown( int ) {
		yzDebug( "NoGuiView") << "NoGuiView::scrollDown" << endl;
	}
	virtual int stringWidth(const QString&str) const {
		return str.length();
	}
	virtual int charWidth(const QChar&) const {
		return 1;
	}
	virtual void registerModifierKeys(const QString&) {
		return;
	}

	virtual void paintEvent( const YZSelection& ) {
	}
	
    virtual void Scroll( int dx, int dy ) {
		yzDebug( "NoGuiView") << "NoGuiView::Scroll" << dx << dy << endl;
    }

    virtual void notifyContentChanged( const YZSelection& s ) {
		yzDebug( "NoGuiView") << "NoGuiView::notifyContentChanged" << endl;
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

#endif // NOGUI_VIEW_H
