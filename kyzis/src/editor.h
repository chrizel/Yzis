/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
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

#ifndef KYZISEDIT_H
#define KYZISEDIT_H

#include "cursor.h"
#include "viewwidget.h"
#include <libyzis/drawbuffer.h>

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
class KYZisCursor;


struct KYZViewCell {
	bool isValid;
	int flag;
	bool selected;
	QFont font;
	QString c;
	QColor bg;
	QColor fg;
	KYZViewCell():
		isValid( false ),
		flag( 0 ),
		selected ( false ), font(), c(), bg(), fg() {
	}
};
typedef QMap<unsigned int,KYZViewCell> lineCell;

/**
 * KYZis Painter Widget
 */
class KYZisEdit : public QWidget {
	Q_OBJECT

	public :
		KYZisEdit(KYZisView *parent=0);
		virtual ~KYZisEdit();

		//erase all text, and set new text
		void setText (const QString& );

		//append text
		void append ( const QString& );

		void paintEvent( const YZSelection& drawMap );

		//move cursor to position column, line relative to viewport
		void setCursor(int c,int l);
		void scrollUp( int );
		void scrollDown( int );

		KYZisCursor::shape cursorShape();
		void updateCursor();
		// update text area
		void updateArea( );

		void setTransparent( bool t, double opacity = 0, const QColor& color = Qt::black );

		void drawCell( QPainter* p, const KYZViewCell& cell, const QRect& rect, bool reversed = false );
		void drawCell( unsigned int x, unsigned int y, const YZDrawCell& cell, QPainter* p );

		const QString& convertKey( int key );

		unsigned int spaceWidth;

		void registerModifierKeys( const QString& keys );
		void unregisterModifierKeys( const QString& keys );

		QPoint cursorCoordinates( );

		QMap<unsigned int,lineCell> mCell;

		QVariant inputMethodQuery ( Qt::InputMethodQuery query );

	public slots :
		void sendMultipleKey( const QString& keys );


	protected:
		//intercept tabs
		virtual bool event(QEvent*);

		void resizeEvent(QResizeEvent*);
		void paintEvent(QPaintEvent*);

		//normal keypressEvents processing
		void keyPressEvent (QKeyEvent *);

		//mouse events
		void mousePressEvent (QMouseEvent *);

		//mouse move event
		void mouseMoveEvent( QMouseEvent *);

		// mousebutton released
//		void mouseReleaseEvent( QMouseEvent *);

		//insert text at line
		void insetTextAt(const QString&, int line);

		//insert a char at idx on line ....
		void insertCharAt(QChar,int);

		//replace a char at idx on line ....
		void replaceCharAt( QChar,int );

		//number of lines
		long lines();

		virtual void focusInEvent( QFocusEvent * );
		virtual void focusOutEvent( QFocusEvent * );

		// for InputMethod
		void inputMethodEvent ( QInputMethodEvent * );

	private :
		void initKeys();
		KActionCollection* actionCollection;
		QSignalMapper* signalMapper;
		QString keysToShortcut( const QString& keys );

		KYZisView *mParent;
		KYZisCursor* mCursor;

		QFontMetrics *standard;
		QFontMetrics *standardBold;
		QFontMetrics *standardBoldItalic;

		bool isFontFixed;
		/**
		 * size of the left margin (used to draw line number)
		 */
		unsigned int marginLeft;

		// last line number
		QMap<int,QString> keys;
		KRootPixmap *rootxpm;
		bool mTransparent;
		KYZViewCell defaultCell;

	friend class KYZisCursor;
};

#endif
