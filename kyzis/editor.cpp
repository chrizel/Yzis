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
#include <qclipboard.h>
#include <qcursor.h>


KYZisEdit::KYZisEdit(KYZisView *parent, const char *name)
: QWidget( parent, name, WNoAutoErase ) 
{
	QFont f ("fixed");
	f.setFixedPitch(true);
	f.setStyleHint(QFont::TypeWriter);
	myFont = f;
	QWidget::setCursor( IbeamCursor );
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
	setBackgroundMode( PaletteBase );
	//setPaletteBackgroundColor(QColor("white"));
	setBackgroundColor(QColor("black"));
	setPaletteForegroundColor(QColor("white"));
	mCursorShown = false; //cursor is not shown
	mCursorY = mCursorX = 0;
}

KYZisEdit::~KYZisEdit() {
}

void KYZisEdit::resizeEvent(QResizeEvent *ev) {
	QSize s = ev->size();
	int lines = s.height() / fontMetrics().lineSpacing();
	int columns = s.width() / fontMetrics().maxWidth() - marginLeft;
	erase( );
	mParent->setVisibleArea( columns, lines );
}

void KYZisEdit::paintEvent( QPaintEvent * pe ) {
	QRect r = pe->rect( );
	int clipx = r.x();
	int clipy = r.y();
	int clipw = r.width();
	int cliph = r.height();
	unsigned int linespace = fontMetrics().lineSpacing();
	unsigned int maxwidth = fontMetrics().maxWidth();
	clipx = clipx ? clipx / maxwidth : 0;
	clipy = clipy ? clipy / linespace : 0;
	clipw = clipw ? clipw / maxwidth + ( int )ceil( clipw % maxwidth ) : 0;
	cliph = cliph ? cliph / linespace + ( int )ceil( cliph % linespace ) : 0;
	if ( mCursorY >= clipy && mCursorY <= clipy + cliph ) {
		if ( pe->erased() && ! ( mCursorX >= clipx && mCursorX <= clipx + clipw ) )
			mCursorShown = true;
		else
			mCursorShown = ! mCursorShown;
	}
//	yzDebug() << "KYZisEdit::paintEvent: " << clipx << "," << clipy << "," << clipw << "," << cliph << "," << pe->erased() << "," << mCursorShown << endl;
	drawContents( clipx, clipy, clipw, cliph, pe->erased() );
}

void KYZisEdit::paintEvent( unsigned int clipx, unsigned int clipy, unsigned int clipw, unsigned int cliph ) {
	clipx -= mParent->getDrawCurrentLeft( ) + marginLeft;
	clipy -= mParent->getDrawCurrentTop( );
	if ( ( unsigned int )mCursorY >= clipy && ( unsigned int )mCursorY <= clipy + cliph )
		mCursorShown = false;
	drawContents( clipx, clipy, clipw, cliph, false );
}

void KYZisEdit::setCursor(int c, int l) {
	c = c - mParent->getDrawCurrentLeft () + marginLeft;
	l -= mParent->getDrawCurrentTop ();
//	yzDebug() << "setCursor : mCursorShow=" << mCursorShown << "; (" << mCursorX << ", " << mCursorY << ") - (" << c << ", " << l << ")" << endl;
	if ( mCursorShown && c == mCursorX && l == mCursorY ) return;
	if ( mCursorShown ) drawCursorAt( mCursorX, mCursorY );
	mCursorX = c;
	mCursorY = l;
	drawCursorAt( mCursorX, mCursorY );
	mCursorShown = true;
}

void KYZisEdit::scrollUp( int n ) {
	drawCursorAt( mCursorX, mCursorY );
	bitBlt( this, 0, n * fontMetrics().lineSpacing(),
		this, 0, 0,
		width(), ( mParent->getLinesVisible() - n ) * fontMetrics().lineSpacing(),
		Qt::CopyROP, true );
	mCursorShown = false;
	drawContents( 0, 0, mParent->getColumnsVisible(), n, false );
}

void KYZisEdit::scrollDown( int n ) {
	drawCursorAt( mCursorX, mCursorY );
	bitBlt( this, 0, 0,
		this, 0, n * fontMetrics().lineSpacing(),
		width(), ( mParent->getLinesVisible() - n ) * fontMetrics().lineSpacing(),
		Qt::CopyROP, true );
	mCursorShown = false;
	drawContents( 0, mParent->getLinesVisible() - n, mParent->getColumnsVisible(), n, false );
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

void KYZisEdit::mousePressEvent ( QMouseEvent * e ) {
	if ( mParent->myBuffer()->introShown() ) {
		mParent->myBuffer()->clearIntro();
		mParent->gotodxdy( 0, 0 );
		return;
	} 
	if ( e->button() == Qt::LeftButton ) {
		if (mParent->getCurrentMode() != YZView::YZ_VIEW_MODE_EX) {
			mParent->gotodxdy( e->x( ) / fontMetrics().maxWidth() + mParent->getDrawCurrentLeft( ) - marginLeft,
						e->y( ) / fontMetrics().lineSpacing() + mParent->getDrawCurrentTop( ) );
		}
	} else if ( e->button() == Qt::MidButton ) {
		QString text = QApplication::clipboard()->text();
		if ( ! text.isNull() ) {
			if ( mParent->getCurrentMode() == mParent->YZ_VIEW_MODE_INSERT )
				mParent->myBuffer()->action()->insertChar( mParent, mParent->getCursor(), text );
			else if ( mParent->getCurrentMode() == mParent->YZ_VIEW_MODE_REPLACE )
				mParent->myBuffer()->action()->replaceChar( mParent, mParent->getCursor(), text );
		}
	}
}

void KYZisEdit::drawCursorAt(int x, int y) {
	bitBlt (
			this,
			x*fontMetrics().maxWidth(),y * fontMetrics().lineSpacing(),
			this,
			x*fontMetrics().maxWidth(), y * fontMetrics().lineSpacing(),
			fontMetrics().maxWidth(), fontMetrics().lineSpacing(),
			Qt::NotROP,	    // raster Operation
			true );		    // ignoreMask
}

void KYZisEdit::drawContents( int , int clipy, int , int cliph, bool ) {
	int flag = ( mParent->myBuffer()->introShown() ? Qt::AlignCenter : Qt::AlignLeft )| Qt::AlignVCenter | Qt::SingleLine;
	QPainter p;
	p.begin( this );

	unsigned int linespace = fontMetrics().lineSpacing();
	unsigned int maxwidth = fontMetrics().maxWidth();
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
		setCursor( mCursorX + marginLeft - my_marginLeft, mCursorY ); // move cursor
		marginLeft = my_marginLeft;
		myRect.setRect ( 0 , 0, marginLeft * maxwidth, height() ); // erase numbers
		p.eraseRect( myRect );
		mParent->setVisibleArea( width() / maxwidth - marginLeft, height() / linespace );
		return;
	}

	unsigned int currentY = 0;
	if (! wrap ) {
		mParent->initDraw( mParent->getCurrentLeft(), mParent->getCurrentTop() + clipy, mParent->getDrawCurrentLeft(), mParent->getDrawCurrentTop() + clipy  );
		currentY = clipy;
	} else {
		mParent->initDraw( );
	}

	unsigned int lineNumber = 0;
	unsigned int mY = mParent->getCursor()->getY() - mParent->getDrawCurrentTop();

	if ( ! mParent->myBuffer()->introShown() ) {
		while ( mParent->drawNextLine( ) && cliph > 0 ) {
			lineNumber = mParent->drawLineNumber();
			if ( currentY >= ( uint )clipy ) {
				unsigned int currentX = 0;

				if ( number ) { // draw current line number
					myRect.setRect ( currentX * maxwidth , currentY * linespace, ( marginLeft - 1 ) * maxwidth, linespace );
					p.eraseRect( myRect );
					QPen old_pen = p.pen( );
					if ( lineNumber != lastLineNumber ) { // we don't draw it twice
						p.setPen( Qt::yellow );
						p.drawText( myRect, flag, QString::number( lineNumber ).rightJustify( marginLeft - 1, ' ' ) );
						lastLineNumber = lineNumber;
					}
					currentX += marginLeft;
					p.setPen( old_pen );
				}
				myRect.setRect (currentX * maxwidth, currentY * linespace, width() - currentX * maxwidth, linespace );
				p.eraseRect( myRect );

				while ( mParent->drawNextCol( ) ) {
					myRect.setX( currentX * maxwidth );
					myRect.setWidth( mParent->drawLength() * maxwidth );

					QColor c = mParent->drawColor( );
					if ( c.isValid() ) p.setPen( c );
					p.drawText(myRect, flag, mParent->drawChar( ) );

					if ( mParent->drawSelected() ) {
						for ( unsigned int i = currentX; i < currentX + mParent->drawLength(); i++ )
							drawCursorAt( i, currentY );
					}

					currentX += mParent->drawLength( );
				}
				if ( currentY == mY ) setCursor( mParent->getCursor()->getX(), mParent->getCursor()->getY() );
				currentY += mParent->drawHeight( );
				cliph -= mParent->lineHeight( );
			} else {
				if ( wrap ) while ( mParent->drawNextCol( ) ) ;
				currentY += mParent->drawHeight( );
				lastLineNumber = lineNumber;
			}
		}
		p.setPen( Qt::white );
		if ( number ) { 
			p.drawLine( marginLeft * maxwidth - maxwidth / 2, clipy * linespace, \
					marginLeft * maxwidth - maxwidth / 2, currentY * linespace );
		}
		unsigned int fh = height() / linespace;
		while ( cliph > 0 && currentY < fh ) {
			myRect.setRect ( 0, currentY * linespace, width(), linespace );
			p.eraseRect( myRect );
			p.drawText(myRect,flag ,"~");
			++currentY;
			--cliph;
		}
	} else {
		while ( currentY < lineCount ) {
			myRect.setRect (0, currentY * linespace, width(), linespace);
			p.eraseRect(myRect);
			p.drawText(myRect,flag, mParent->myBuffer()->textline(currentY ) );
			++currentY;
		}
	}
	p.end( );
}

void KYZisEdit::focusInEvent ( QFocusEvent * ) {
	KYZisFactory::s_self->setCurrentView( mParent );
}

#include "editor.moc"
