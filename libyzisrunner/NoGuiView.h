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
#include "libyzis/buffer.h"

class NoGuiView : public YZView
{
public:
	NoGuiView(YZBuffer *buf, YZSession *sess, int cols=50, int lines=50);

	uint getCursorX();
	uint getCursorY();
	uint getCursorLine();
	uint getCursorCol();

	// Reimplemented to please compilation

	virtual void guiSetCommandLineText( const QString& text);

	virtual void guiSetFocusCommandLine();

	virtual void guiSetFocusMainWindow();

	virtual QString guiGetCommandLineText() const;

	virtual void invalidateLine( unsigned int );

	virtual void setStatusBar( const QString& );

	virtual void updateCursor( unsigned int, unsigned int, unsigned int, const QString& );

	virtual void refreshScreen( );

	virtual void guiSyncViewInfo( );

	virtual void guiDisplayInfo( const QString& );

	virtual void modeChanged( );

	virtual void paintEvent( unsigned int /*curx*/, unsigned int /*cury*/, unsigned int /*curw*/, unsigned int /*curh*/ );

	virtual void scrollUp( int );
	virtual void scrollDown( int );
	virtual void registerModifierKeys(const QString& s);

	virtual void paintEvent( const YZSelection& );
	
    virtual void Scroll( int dx, int dy );

    virtual void guiNotifyContentChanged( const YZSelection& s );

	virtual bool guiPopupFileSaveAs();
	virtual void guiFilenameChanged();
	virtual void guiHighlightingChanged();
	void guiPreparePaintEvent(int, int);
	void guiEndPaintEvent();
	void guiDrawCell(int, int, const YZDrawCell&, void*);
	void guiDrawClearToEOL(int, int, const QChar&);
	void guiDrawSetMaxLineNumber(int);
	void guiDrawSetLineNumber(int, int, int);
	
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