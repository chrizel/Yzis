/* This file is part of the Yzis libraries
 *  Copyright (C) 2003, 2004 Mickael Marchand <marchand@kde.org>
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
#include <qfontmetrics.h>
#include <qfont.h>
#include <action.h>
#include <qnamespace.h>
#include <krootpixmap.h>
#include <qsignalmapper.h>
#include <kactioncollection.h>

class KYZisView;

/**
 * KYZis Painter Widget
 */
class KYZisEdit : public QWidget {
	Q_OBJECT

	public :
		KYZisEdit(KYZisView *parent=0, const char *name=0);
		virtual ~KYZisEdit();

		//erase all text, and set new text
		void setText (const QString& );

		//append text
		void append ( const QString& );

		void paintEvent( unsigned int curx, unsigned int cury, unsigned int curw, unsigned int curh );

		//move cursor to position column, line relative to viewport
		void setCursor(int c,int l);
		void scrollUp( int );
		void scrollDown( int );

		// update text area
		void updateArea( );

		void setTransparent( bool t, double opacity = 0, const QColor& color = Qt::black );

		const QString& convertKey( int key );

		unsigned int spaceWidth;

		void registerModifierKeys( const QString& keys );

		QPoint cursorCoordinates( );
	
	public slots :
		void sendMultipleKey( const QString& keys );


	protected:
		//intercept tabs
		virtual bool event(QEvent*);
		
		void resizeEvent(QResizeEvent*);
		void paintEvent(QPaintEvent*);

		//entry point for drawing events
		void drawContents( int clipx, int clipy, int clipw, int cliph, bool );

		//normal keypressEvents processing
		void keyPressEvent (QKeyEvent *);

		//mouse events
		void mousePressEvent (QMouseEvent *);

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

		void selectRect( unsigned int x, unsigned int y, unsigned int w, unsigned int h );

	private :
		void initKeys();
		KActionCollection* actionCollection;
		QSignalMapper* signalMapper;
		QString keysToShortcut( const QString& keys );
		
		KYZisView *mParent;

		//cursor position (sync with libyzis one)
		int mCursorX;
		int mCursorY;
		bool mCursorShown;
		QFontMetrics *standard;
		QFontMetrics *standardBold;
		QFontMetrics *standardBoldItalic;

		bool isFontFixed;
		/**
		 * size of the left margin (used to draw line number)
		 */
		unsigned int marginLeft;

		// last line number
		unsigned int lastLineNumber;
		QMap<int,QString> keys;
		KRootPixmap *rootxpm;
		bool mTransparent;
};

#endif
