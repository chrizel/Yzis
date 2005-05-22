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
 * $Id: editor.cpp 1418 2004-12-01 18:23:46Z panard $
 */

#include "editor.h"
#include "debug.h"
#include "yzis.h"
#include "factory.h"

#include <kglobalsettings.h>
#include <math.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <kaction.h>
#include <qsignalmapper.h>
#include <kshortcut.h>
#include <ctype.h>

#include "settings.h"

#define GETX( x ) ( isFontFixed ? ( x ) * fontMetrics().maxWidth() : x )

KYZisEdit::KYZisEdit(KYZisView *parent, const char *name)
: QWidget( parent, name)
{
	mTransparent = false;
	mParent = parent;

	marginLeft = 0;
	lastLineNumber = 0;

	setFocusPolicy( StrongFocus );
	QWidget::setCursor( IbeamCursor );
	rootxpm = new KRootPixmap( this );
	setTransparent( false );

	mCursor = new KYZisCursor( this, KYZisCursor::KYZ_CURSOR_SQUARE );

	initKeys();
}


KYZisEdit::~KYZisEdit() {
	delete mCursor;
}

void KYZisEdit::setTransparent ( bool t, double opacity, const QColor& color ) {
	if ( opacity == 1 )	t = false;	// opactity is max, let use scroll optimisation
	mTransparent = t;
	if ( t ) {
		rootxpm->setFadeEffect( opacity, color );
		rootxpm->start();
	} else rootxpm->stop();
}

void KYZisEdit::resizeEvent(QResizeEvent* ) {
	updateArea();
}

void KYZisEdit::updateArea( ) {

	isFontFixed = fontInfo().fixedPitch();
	mParent->setFixedFont( isFontFixed );
	spaceWidth = mParent->spaceWidth;
	mCursor->resize( fontMetrics().maxWidth(), fontMetrics().lineSpacing() );
	if ( isFontFixed )
		mCursor->setCursorType( KYZisCursor::KYZ_CURSOR_SQUARE );
	else
		mCursor->setCursorType( KYZisCursor::KYZ_CURSOR_LINE );

	int lines = height() / fontMetrics().lineSpacing();
	// if font is fixed, calculate the number of columns fontMetrics().maxWidth(), else give the width of the widget
	int columns = width() / GETX( 1 ) - marginLeft;
	erase( );
	mParent->setVisibleArea( columns, lines );
}

void KYZisEdit::paintEvent( QPaintEvent * pe ) {
	QRect r = pe->rect( );
	int clipx = r.x();
	int clipy = r.y();
	int clipw = r.width();
	int cliph = r.height();
	if ( isFontFixed ) {
		unsigned int linespace = fontMetrics().lineSpacing();
		unsigned int maxwidth = fontMetrics().maxWidth();
		clipx = clipx ? clipx / maxwidth : 0;
		clipy = clipy ? clipy / linespace : 0;
		clipw = clipw ? clipw / maxwidth + ( int )ceil( clipw % maxwidth ) : 0;
		cliph = cliph ? cliph / linespace + ( int )ceil( cliph % linespace ) : 0;
	}
	drawContents( clipx, clipy, clipw, cliph, pe->erased() );
}

void KYZisEdit::paintEvent( unsigned int clipx, unsigned int clipy, unsigned int clipw, unsigned int cliph ) {
	clipx = clipx - mParent->getDrawCurrentLeft( ) + marginLeft;
	unsigned int dTop = mParent->getDrawCurrentTop();
	clipy = clipy > dTop ? clipy - dTop : 0;
	drawContents( clipx, clipy, clipw, cliph, false );
}

void KYZisEdit::setCursor( int c, int l ) {
//	yzDebug() << "setCursor" << endl;
	c = c - mParent->getDrawCurrentLeft () + marginLeft;
	l -= mParent->getDrawCurrentTop ();
	unsigned int x = GETX( c );
	if ( mParent->getLocalBoolOption( "rightleft" ) ) {
		x = width() - x - mCursor->width();
	}
	mCursor->move( x, l * fontMetrics().lineSpacing() );
}

QPoint KYZisEdit::cursorCoordinates( ) {
	QPoint position( mCursor->x(), mCursor->y() );
	return position;
}

void KYZisEdit::scrollUp( int n ) {
	mCursor->hide();
	if ( ! mTransparent ) {
		bitBlt( this, 0, n * fontMetrics().lineSpacing(),
			this, 0, 0,
			width(), ( mParent->getLinesVisible() - n ) * fontMetrics().lineSpacing(),
			Qt::CopyROP, true );
		drawContents( 0, 0, mParent->getColumnsVisible(), n, false );
	} else {
		mParent->abortPaintEvent();
		drawContents( 0, 0, mParent->getColumnsVisible(), mParent->getLinesVisible(), false );
	}
}

void KYZisEdit::scrollDown( int n ) {
	mCursor->hide();
	if ( ! mTransparent ) {
		bitBlt( this, 0, 0,
			this, 0, n * fontMetrics().lineSpacing(),
			width(), ( mParent->getLinesVisible() - n ) * fontMetrics().lineSpacing(),
			Qt::CopyROP, true );
		drawContents( 0, mParent->getLinesVisible() - n, mParent->getColumnsVisible(), n, false );
	} else {
		mParent->abortPaintEvent();
		drawContents( 0, 0, mParent->getColumnsVisible(), mParent->getLinesVisible(), false );
	}
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
	ButtonState st = e->state();
	QString modifiers;
	if ( st & Qt::ShiftButton )
		modifiers = "<SHIFT>";
	if ( st & Qt::AltButton )
		modifiers += "<ALT>";
	if ( st & Qt::ControlButton )
		modifiers += "<CTRL>";

	if ( keys.contains( e->key() ) ) //to handle some special keys
		mParent->sendKey(keys[ e->key() ], modifiers);
	else
		mParent->sendKey( e->text(), modifiers );
	e->accept();
}

void KYZisEdit::mousePressEvent ( QMouseEvent * e ) {
	/*
	FIXME: How to handle mouse events commented out now so kyzis will compile
	
	if ( mParent->myBuffer()->introShown() ) {
		mParent->myBuffer()->clearIntro();
		mParent->gotodxdy( 0, 0 );
		return;
	}
	*/
	
	if (( e->button() == Qt::LeftButton ) || ( e->button() == Qt::RightButton )) {
		if (mParent->getCurrentMode() != YZView::YZ_VIEW_MODE_EX) {
			mParent->gotodxdy( e->x( ) / ( isFontFixed ? fontMetrics().maxWidth() : 1 ) + mParent->getDrawCurrentLeft( ) - marginLeft,
						e->y( ) / fontMetrics().lineSpacing() + mParent->getDrawCurrentTop( ) );
		}
	} else if ( e->button() == Qt::MidButton ) {
		QString text = QApplication::clipboard()->text( QClipboard::Selection );
		if ( text.isNull() )
			text = QApplication::clipboard()->text( QClipboard::Clipboard );
		if ( ! text.isNull() ) {
			if ( mParent->getCurrentMode() == mParent->YZ_VIEW_MODE_INSERT || mParent->getCurrentMode() == mParent->YZ_VIEW_MODE_REPLACE ) {
				QChar reg = '\"';
				YZSession::mRegisters.setRegister( reg, QStringList::split( "\n", text ) );
				mParent->paste( reg, true );
			}
		}
	}
}

void KYZisEdit::selectRect( unsigned int x, unsigned int y, unsigned int w, unsigned int h ) {
	if ( mParent->getLocalBoolOption( "rightleft" ) ) x = width() - x - w;
	bitBlt( this, x, y, this, x, y, w, h, Qt::NotROP, true );
}

void KYZisEdit::drawContents( int /*clipx*/, int clipy, int /*clipw*/, int cliph, bool ) {
	//int flag = ( mParent->myBuffer()->introShown() ? Qt::AlignCenter : Qt::AlignLeft )| Qt::AlignVCenter | Qt::SingleLine;
	QPainter p;
	p.begin( this );

//	yzDebug() << "==> drawContent at " << clipx << "," << clipy << " " << clipw << " x " << cliph << endl;

	unsigned int linespace = fontMetrics().lineSpacing();
	QRect myRect;
	bool number = mParent->getLocalBoolOption( "number" );
	bool wrap = mParent->getLocalBoolOption( "wrap" );
	bool rightleft = mParent->getLocalBoolOption( "rightleft" );

	unsigned int lineCount = mParent->myBuffer()->lineCount();
	unsigned int my_marginLeft = 0;
	if ( number ) { // update marginLeft
		my_marginLeft = ( isFontFixed ? QString::number( lineCount ).length() + 2 : mParent->stringWidth( " " + QString::number( lineCount ) + "  " ) );
		lastLineNumber = 0;
	}
	if ( marginLeft != my_marginLeft ) {
		if ( mCursor->visible() ) {
			mCursor->move( mCursor->x() + GETX( marginLeft - my_marginLeft ), mCursor->y() );
			mCursor->hide();
		}
		marginLeft = my_marginLeft;
		updateArea();
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
	unsigned int w;

	mCursor->hide();

	//FIXME introShown eliminate conditional
	//if ( ! mParent->myBuffer()->introShown() ) {
		while ( cliph > 0 && mParent->drawNextLine( ) ) {
			lineNumber = mParent->drawLineNumber();
			if ( currentY >= ( uint )clipy ) {
				unsigned int currentX = 0;

				if ( number ) { // draw current line number
					myRect.setRect( 0, currentY * linespace, GETX( marginLeft - spaceWidth ), linespace );
					if ( rightleft ) {
						w = myRect.width();
						myRect.setLeft( width() - w );
						myRect.setWidth( w );
					}
					erase( myRect );
					QPen old_pen = p.pen( );
					if ( lineNumber != lastLineNumber ) { // we don't draw it twice
						p.setPen( Qt::yellow );
						p.setBackgroundMode( Qt::TransparentMode );
						p.setFont(font());
						// FIXME p.drawText( myRect, flag | ( rightleft ? Qt::AlignLeft : Qt::AlignRight ), QString::number( lineNumber ) );
						p.drawText( myRect, ( rightleft ? Qt::AlignLeft : Qt::AlignRight ), QString::number( lineNumber ) );
						lastLineNumber = lineNumber;
					}
					currentX += marginLeft;
					p.setPen( old_pen );
				}
				myRect.setRect( GETX( currentX ), currentY * linespace, width() - GETX( currentX ), linespace );
				if ( rightleft ) {
					w = myRect.width();
					myRect.setLeft( width() - myRect.left() - w );
					myRect.setWidth( w );
				}
				erase( myRect );

				while ( mParent->drawNextCol( ) ) {
					myRect.setX( GETX( currentX ) );
					myRect.setWidth( GETX( mParent->drawLength() ) );
					if ( rightleft ) {
						w = myRect.width();
						myRect.setLeft( width() - myRect.left() - w );
						myRect.setWidth( w );
					}
					QColor c = mParent->drawColor( );
					if ( c.isValid() ) p.setPen( c );
					else p.setPen( foregroundColor() );
					//other flags
					QFont myFont(font());
					myFont.setItalic(mParent->drawItalic());
					myFont.setBold(mParent->drawBold());
					myFont.setOverline(mParent->drawOverline());
					myFont.setStrikeOut(mParent->drawStrikeOutLine());
					myFont.setUnderline(mParent->drawUnderline());
					p.setFont(myFont);
					//FIXME remove flag...
					//p.drawText(myRect, flag, mParent->drawChar( ) );
					QString disp = mParent->drawChar();
					if ( rightleft )
						disp = disp.rightJustify( mParent->drawLength(), mParent->fillChar() );
					else
						disp = disp.leftJustify( mParent->drawLength(), mParent->fillChar() );
					QColor bgColor = mParent->drawBgColor();
					if ( bgColor.isValid() && bgColor != backgroundColor() ) {
						p.setBackgroundMode(Qt::OpaqueMode);
						p.setBackgroundColor( bgColor );
					} else {
						p.setBackgroundMode( Qt::TransparentMode );
					}
					
					p.drawText(myRect, 0, disp );

					if ( mParent->drawSelected() ) {
						selectRect( GETX( currentX ), currentY * linespace, GETX( mParent->drawLength() ), linespace );
						if ( mParent->getCursor()->getY() == currentY && mParent->getCursor()->getX() == currentX - marginLeft )
							mCursor->hide();
					}

					currentX += mParent->drawLength( );
				}
				if ( currentY == mY ) mCursor->refresh();
				currentY += mParent->drawHeight( );
				cliph -= mParent->lineHeight( );
			} else {
				if ( wrap ) while ( mParent->drawNextCol( ) ) ;
				currentY += mParent->drawHeight( );
				lastLineNumber = lineNumber;
			}
		}
		p.setPen( Settings::colorFG() );
		if ( number ) {
			if ( rightleft )
				w = width() - GETX( marginLeft ) + GETX( spaceWidth ) / 2;
			else
				w = GETX( marginLeft ) - GETX( spaceWidth ) / 2;
			p.drawLine( w, clipy * linespace, w, currentY * linespace );
		}
		unsigned int fh = height() / linespace;
		while ( cliph > 0 && currentY < fh ) {
			myRect.setRect ( 0, currentY * linespace, width(), linespace );
			erase( myRect );
			p.setPen( Qt::cyan );
			//FIXME p.drawText( myRect, rightleft ? flag | Qt::AlignRight : flag,"~" );
			p.drawText( myRect, rightleft ? Qt::AlignRight : 0,"~" );
			++currentY;
			--cliph;
		}
	//FIXME eliminate introShown conditional } else {
	//	while ( currentY < lineCount ) {
	//		myRect.setRect (0, currentY * linespace, width(), linespace);
	//		erase(myRect);
	//		//FIXME p.drawText(myRect,flag, mParent->myBuffer()->textline(currentY ) );
	//		p.drawText(myRect, 0, mParent->myBuffer()->textline(currentY ) );
	//		++currentY;
	//	}
	//}
	p.end( );
}

void KYZisEdit::focusInEvent ( QFocusEvent * ) {
	KYZisFactory::s_self->setCurrentView( mParent );
}

void KYZisEdit::initKeys() {
	keys[ Qt::Key_Escape ] = "<ESC>" ;
	keys[ Qt::Key_Tab ] = "<TAB>" ;
	keys[ Qt::Key_Backtab ] = "<BTAB>" ;
	keys[ Qt::Key_Backspace ] = "<BS>" ;
	keys[ Qt::Key_Return ] = "<ENTER>" ;
	keys[ Qt::Key_Enter ] = "<ENTER>" ;
	keys[ Qt::Key_Insert ] = "<INS>" ;
	keys[ Qt::Key_Delete ] = "<DEL>" ;
	keys[ Qt::Key_Pause ] = "<PAUSE>" ;
	keys[ Qt::Key_Print ] = "<PRINT>" ;
	keys[ Qt::Key_SysReq ] = "<SYSREQ>" ;
	keys[ Qt::Key_Home ] = "<HOME>" ;
	keys[ Qt::Key_End ] = "<END>" ;
	keys[ Qt::Key_Left ] = "<LEFT>" ;
	keys[ Qt::Key_Up ] = "<UP>" ;
	keys[ Qt::Key_Right ] = "<RIGHT>" ;
	keys[ Qt::Key_Down ] = "<DOWN>" ;
	keys[ Qt::Key_Prior ] = "<PUP>" ;
	keys[ Qt::Key_Next ] = "<PDOWN>" ;
	keys[ Qt::Key_PageUp ] = "<PUP>" ;
	keys[ Qt::Key_PageDown ] = "<PDOWN>" ;
	keys[ Qt::Key_Shift ] = "<SHIFT>" ;
	keys[ Qt::Key_Control ] = "<CTRL>" ;
	keys[ Qt::Key_Meta ] = "<META>" ;
	keys[ Qt::Key_Alt ] = "<ALT>" ;
//hmm ignore it	keys[ Qt::Key_CapsLock ] = "<CAPSLOCK>" ;
//hmm ignore it	keys[ Qt::Key_NumLock ] = "<NUMLOCK>" ;
//hmm ignore it	keys[ Qt::Key_ScrollLock ] = "<SCROLLLOCK>" ;
	keys[ Qt::Key_Clear ] = "<CLEAR>" ;
	keys[ Qt::Key_F1 ] = "<F1>" ;
	keys[ Qt::Key_F2 ] = "<F2>" ;
	keys[ Qt::Key_F3 ] = "<F3>" ;
	keys[ Qt::Key_F4 ] = "<F4>" ;
	keys[ Qt::Key_F5 ] = "<F5>" ;
	keys[ Qt::Key_F6 ] = "<F6>" ;
	keys[ Qt::Key_F7 ] = "<F7>" ;
	keys[ Qt::Key_F8 ] = "<F8>" ;
	keys[ Qt::Key_F9 ] = "<F9>" ;
	keys[ Qt::Key_F10 ] = "<F10>" ;
	keys[ Qt::Key_F11 ] = "<F11>" ;
	keys[ Qt::Key_F12 ] = "<F12>" ;
	keys[ Qt::Key_F13 ] = "<F13>" ;
	keys[ Qt::Key_F14 ] = "<F14>" ;
	keys[ Qt::Key_F15 ] = "<F15>" ;
	keys[ Qt::Key_F16 ] = "<F16>" ;
	keys[ Qt::Key_F17 ] = "<F17>" ;
	keys[ Qt::Key_F18 ] = "<F18>" ;
	keys[ Qt::Key_F19 ] = "<F19>" ;
	keys[ Qt::Key_F20 ] = "<F20>" ;
	keys[ Qt::Key_F21 ] = "<F21>" ;
	keys[ Qt::Key_F22 ] = "<F22>" ;
	keys[ Qt::Key_F23 ] = "<F23>" ;
	keys[ Qt::Key_F24 ] = "<F24>" ;
	keys[ Qt::Key_F25 ] = "<F25>" ;
	keys[ Qt::Key_F26 ] = "<F26>" ;
	keys[ Qt::Key_F27 ] = "<F27>" ;
	keys[ Qt::Key_F28 ] = "<F28>" ;
	keys[ Qt::Key_F29 ] = "<F29>" ;
	keys[ Qt::Key_F30 ] = "<F30>" ;
	keys[ Qt::Key_F31 ] = "<F31>" ;
	keys[ Qt::Key_F32 ] = "<F32>" ;
	keys[ Qt::Key_F33 ] = "<F33>" ;
	keys[ Qt::Key_F34 ] = "<F34>" ;
	keys[ Qt::Key_F35 ] = "<F35>" ;
//	keys[ Qt::Key_BracketLeft ] = "[";
//	keys[ Qt::Key_BracketRight ] = "]";


	actionCollection = new KActionCollection( this, mParent );
	signalMapper = new QSignalMapper( this );
	connect( signalMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( sendMultipleKey( const QString& ) ) );
}

QString KYZisEdit::keysToShortcut( const QString& keys ) {
	QString ret = keys;
	ret = ret.replace( "<CTRL>", "CTRL+" );
	ret = ret.replace( "<SHIFT>", "SHIFT+" );
	ret = ret.replace( "<ALT>", "ALT+" );
	return ret;
}

void KYZisEdit::registerModifierKeys( const QString& keys ) {
	KAction* k = new KAction( "", KShortcut( keysToShortcut( keys ) ), signalMapper, SLOT( map() ), actionCollection, keys );
	signalMapper->setMapping( k, keys );
}

void KYZisEdit::sendMultipleKey( const QString& keys ) {
	mParent->sendMultipleKey( keys );
}

const QString& KYZisEdit::convertKey( int key ) {
	return keys[ key ];
}

#include "editor.moc"
