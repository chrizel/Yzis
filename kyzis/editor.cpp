/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Mickael Marchand <marchand@kde.org>,
 *  Loic Pauleve <panard@inzenet.org>
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

/**
 * $Id$
 */

#include "editor.h"
#include "debug.h"
#include "yzis.h"
#include "factory.h"
#include <kglobalsettings.h>
#include <math.h>


KYZisEdit::KYZisEdit(KYZisView *parent, const char *name)
: QScrollView( parent, name,WStaticContents | WNoAutoErase ) 
{
	QFont f ("fixed");
	f.setFixedPitch(true);
	f.setStyleHint(QFont::TypeWriter);
	myFont = f;
	setFont(f);
//	standard = new QFontMetrics( myFont );
//	f.setBold( true );
//	standardBold = new QFontMetrics( f );
//	f.setItalic( true );
//	standardBoldItalic = new QFontMetrics( f );
	mParent = parent;

	marginLeft = 0;
	lastLineNumber = 0;

	setFocusPolicy( StrongFocus );
	viewport()->setBackgroundMode( PaletteBase );
	//viewport()->setPaletteBackgroundColor(QColor("white"));
	viewport()->setBackgroundColor(QColor("black"));
	viewport()->setPaletteForegroundColor(QColor("white"));
	mCursorShown = false; //cursor is not shown
	mCursorY = mCursorX = 0;
}

KYZisEdit::~KYZisEdit() {
}

void KYZisEdit::viewportResizeEvent(QResizeEvent *ev) {
	QSize s = ev->size();
	int lines = s.height() / fontMetrics().lineSpacing();
	int columns = s.width() / fontMetrics().maxWidth() - marginLeft;
	mParent->setVisibleArea( columns, lines );
}

//c,l => draw positions
void KYZisEdit::setCursor(int c, int l) {
	//erase the previous cursor by redrawing the line 
	mCursorShown = false; //lock
	repaintContents( mCursorX * fontMetrics().maxWidth(), mCursorY * fontMetrics().lineSpacing(), fontMetrics().maxWidth(), fontMetrics().lineSpacing() );
	mCursorShown = true; //unlock
	mCursorX = c - mParent->getDrawCurrentLeft () + marginLeft;
	mCursorY = l - mParent->getDrawCurrentTop ();
	
	drawCursorAt( mCursorX, mCursorY );
}

void KYZisEdit::setTextLine(int , const QString &/*str*/) {
	updateContents( 0, mCursorY * fontMetrics().lineSpacing(), width(),
		YZSession::getBoolOption( "General\\wrap" ) ? height() - mCursorY * fontMetrics().lineSpacing() : fontMetrics().lineSpacing() );
}

bool KYZisEdit::event(QEvent *e) {
	if ( e->type() == QEvent::KeyPress ) {
		QKeyEvent *ke = (QKeyEvent *)e;
		if ( ke->key() == Key_Tab ) {
			keyPressEvent(ke);
			return TRUE;
		}
	}
	return QWidget::event(e);
}

void KYZisEdit::keyPressEvent ( QKeyEvent * e ) {
	if ( e->key() != 0 ) {
		ButtonState st = e->state();
		if (e->key() != Qt::Key_unknown) mParent->sendKey(e->key(), st);
		e->accept();
	}
}

void KYZisEdit::contentsMousePressEvent ( QMouseEvent * e ) {
	if (mParent->getCurrentMode() != YZView::YZ_VIEW_MODE_EX) {
		mParent->gotodxdy( e->x( ) / fontMetrics().maxWidth() + mParent->getDrawCurrentLeft( ) - marginLeft,
					e->y( ) / fontMetrics().lineSpacing() + mParent->getDrawCurrentTop( ) );
	}
}

void KYZisEdit::drawCursorAt(int x, int y) {
	bitBlt (
			viewport(),
			x*fontMetrics().maxWidth(),y * fontMetrics().lineSpacing(),
			viewport(),
			x*fontMetrics().maxWidth(), y * fontMetrics().lineSpacing(),
			fontMetrics().maxWidth(), fontMetrics().lineSpacing(),
			Qt::NotROP,	    // raster Operation
			true );		    // ignoreMask
}

void KYZisEdit::drawContents(QPainter *p, int , int clipy, int , int cliph) {
	int flag = ( mParent->myBuffer()->introShown() ? Qt::AlignCenter : Qt::AlignLeft )| Qt::AlignVCenter | Qt::SingleLine;

	unsigned int linespace = fontMetrics().lineSpacing();
	unsigned int maxwidth = fontMetrics().maxWidth();
	cliph = cliph ? cliph / linespace + ( int )ceil( cliph % linespace ): 0;
	clipy = clipy ? clipy / linespace : 0;
	QRect myRect;
	bool number = YZSession::getBoolOption( "General\\number" );
	bool wrap = YZSession::getBoolOption( "General\\wrap" );

	unsigned int lineCount = mParent->myBuffer()->lineCount();
	unsigned int my_marginLeft = 0;
	if ( number ) { // update marginLeft
		my_marginLeft = 2 + QString::number( lineCount ).length();
		lastLineNumber = 0;
	}
	if ( marginLeft != my_marginLeft ) {
		marginLeft = my_marginLeft;
		mParent->setVisibleArea( width() / maxwidth - marginLeft, height() / linespace );
		return;
	}

	mParent->initDraw( );

	unsigned int currentY = 0;
	unsigned int lineNumber = 0;

	if ( ! mParent->myBuffer()->introShown() ) {
		while ( mParent->drawNextLine( ) && cliph > 0 ) {
			lineNumber = mParent->drawLineNumber();
			if ( currentY >= ( uint )clipy ) {
				unsigned int currentX = 0;

				myRect.setRect (0, currentY * linespace, width(), linespace );
				p->eraseRect( myRect );
				if ( number ) { // draw current line number
					QPen old_pen = p->pen( );

					if ( lineNumber != lastLineNumber ) { // we don't draw it twice
						p->setPen( Qt::yellow );
						p->drawText( myRect, flag, QString::number( lineNumber ).rightJustify( marginLeft - 1, ' ' ) );
						lastLineNumber = lineNumber;
					}

					currentX += marginLeft;
					p->setPen( old_pen );
				}


				while ( mParent->drawNextCol( ) ) {
					const QChar& ch = mParent->drawChar( );
					myRect.setRect (currentX * maxwidth, currentY * linespace, 
							maxwidth * mParent->drawLength( ), linespace);
					QColor c = mParent->drawColor( );
					if ( c.isValid() )
						p->setPen( c );
					p->drawText(myRect, flag, ch );
					currentX += mParent->drawLength( );
				}
				currentY += mParent->drawHeight( );
				cliph -= mParent->lineHeight( );
			} else {
				if ( wrap ) while ( mParent->drawNextCol( ) ) mParent->drawChar( );
				currentY += mParent->drawHeight( );
				lastLineNumber = lineNumber;
			}
		}
		p->setPen( Qt::white );
		if ( number ) { // draw blank line
			p->drawLine( marginLeft * maxwidth - maxwidth / 2, clipy * linespace, \
					marginLeft * maxwidth - maxwidth / 2, ( currentY - clipy ) * linespace );
		}
		unsigned int fh = height() / linespace;
		while ( cliph > 0 && currentY < fh ) {
			myRect.setRect ( 0, currentY * linespace, width(), linespace );
			p->eraseRect( myRect );
			p->drawText(myRect,flag ,"~");
			++currentY;
			--cliph;
		}
		if ( mCursorShown )
			setCursor( mParent->getCursor()->getX(), mParent->getCursor()->getY() );
	} else {
		while ( currentY < lineCount ) {
			myRect.setRect (0, currentY * linespace, width(), linespace);
			p->eraseRect(myRect);

			p->drawText(myRect,flag, mParent->myBuffer()->textline(currentY ) );

			++currentY;
		}
	}
}

void KYZisEdit::focusInEvent ( QFocusEvent * ) {
	KYZisFactory::s_self->setCurrentView( mParent );
}

#include "editor.moc"
