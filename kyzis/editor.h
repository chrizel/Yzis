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

#ifndef KYZISEDIT_H
#define KYZISEDIT_H

#include "viewwidget.h"
#include <qscrollview.h>
#include <qpainter.h>
#include <qevent.h>
#include <qmap.h>

class KYZisView;

/**
 * KYZis Painter Widget
 */
class KYZisEdit : public QScrollView {
	Q_OBJECT

	public :
		KYZisEdit(KYZisView *parent=0, const char *name=0);
		virtual ~KYZisEdit();

		//erase all text, and set new text
		void setText (const QString& );

		//append text
		void append ( const QString& );

		//set text at line ...
		void setTextLine(int l, const QString &);
		
		//move cursor to position column, line relative to viewport
		void setCursor(int c,int l);

	protected:
		//intercept tabs
		virtual bool event(QEvent*);
		
		//update view when the viewport gets resized
		void viewportResizeEvent(QResizeEvent*);

		//entry point for drawing events
		void drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph );

		//normal keypressEvents processing
		void keyPressEvent (QKeyEvent *);

		//mouse events
		void contentsMousePressEvent (QMouseEvent *);

		//insert text at line
		void insetTextAt(const QString&, int line);

		//insert a char at idx on line ....
		void insertCharAt(QChar,int);
		
		//replace a char at idx on line ....
		void replaceCharAt( QChar,int );
		
		//number of lines
		long lines();
		
		//draw the cursor at the given position
		void drawCursorAt(int x, int y);

		virtual void focusInEvent( QFocusEvent * );

		int getLineforY( int y );

	private :
		KYZisView *mParent;

		//cursor position (sync with libyzis one)
		int mCursorX;
		int mCursorY;
		bool mCursorShown;
};

#endif
