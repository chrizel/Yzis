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
	NoGuiView(YZBuffer *buf, YZSession *sess, int lines=50);

	uint getCursorX();
	uint getCursorY();
	uint getCursorLine();
	uint getCursorCol();

	// Reimplemented to please compilation

	virtual void setCommandLineText( const QString& text);

	virtual void setFocusCommandLine();

	virtual void setFocusMainWindow();

	virtual QString getCommandLineText() const;

	virtual void invalidateLine( unsigned int );

	virtual void setStatusBar( const QString& );

	virtual void updateCursor( unsigned int, unsigned int, unsigned int, const QString& );

	virtual void refreshScreen( );

	virtual void syncViewInfo( );

	virtual void displayInfo( const QString& );

	virtual void modeChanged( );

	virtual void paintEvent( unsigned int /*curx*/, unsigned int /*cury*/, unsigned int /*curw*/, unsigned int /*curh*/ );

	virtual void scrollUp( int );
	virtual void scrollDown( int );
	virtual int stringWidth(const QString&str) const;
	virtual int charWidth(const QChar & c) const;
	virtual void registerModifierKeys(const QString& s);

	virtual void paintEvent( const YZSelection& );
	
    virtual void Scroll( int dx, int dy );

    virtual void notifyContentChanged( const YZSelection& s );

	virtual bool popupFileSaveAs();
	virtual void filenameChanged();
	virtual void highlightingChanged();
	void preparePaintEvent(int, int);
	void endPaintEvent();
	void drawCell(int, int, const YZDrawCell&, void*);
	void drawClearToEOL(int, int, const QChar&);
	void drawSetMaxLineNumber(int);
	void drawSetLineNumber(int, int, int);
	
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
