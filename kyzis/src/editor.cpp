/* This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2004-2005 Loic Pauleve <panard@inzenet.org>
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

/**
 * $Id$
 */

#include "editor.h"
#include "debug.h"
#include "yzis.h"
#include "factory.h"
#include "registers.h"
#include "viewwidget.h"

#include <kglobalsettings.h>
#include <math.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <kaction.h>
#include <qsignalmapper.h>
#include <kshortcut.h>
#include <ctype.h>
#include <QApplication>

#include <kaccel.h>

#include "settings.h"

#define GETX( x ) ( isFontFixed ? ( x ) * fontMetrics().maxWidth() : x )

KYZisEdit::KYZisEdit(KYZisView *parent)
: QWidget( parent, /*Qt::WStaticContents | */ Qt::WNoAutoErase )
{
	mTransparent = false;
	mParent = parent;

	marginLeft = 0;

	setFocusPolicy( Qt::StrongFocus );
	setAttribute ( Qt::WA_PaintOutsidePaintEvent );
	QWidget::setCursor( Qt::IBeamCursor );

	rootxpm = new KRootPixmap( this );
	setTransparent( false );

	// for Input Method
	setInputMethodEnabled( true );

	initKeys();
	mCell.clear();
	mCursor = new KYZisCursor( this, KYZisCursor::SQUARE );
	updateCursor();

	defaultCell.isValid = true;
	defaultCell.selected = false;
	defaultCell.c = " ";
	defaultCell.flag = 0;
}


KYZisEdit::~KYZisEdit() {
	delete mCursor;
	mCell.clear();
	delete signalMapper;
	// dont delete rootxpm, this is done by KDE ;)
//	delete rootxpm;
	for( int i = actionCollection->count() - 1; i>= 0; --i )
		delete actionCollection->take( actionCollection->action(i) );
	delete actionCollection;
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

KYZisCursor::shape KYZisEdit::cursorShape() {
	KYZisCursor::shape s;
	if ( !isFontFixed ) {
		s = KYZisCursor::VBAR;
	} else {
		QString shape;
		YZMode::modeType m = mParent->modePool()->current()->type();
		switch( m ) {
			case YZMode::MODE_INSERT :
				shape = mParent->getLocalStringOption("cursorinsert");
				break;
			case YZMode::MODE_REPLACE :
				shape = mParent->getLocalStringOption("cursorreplace");
				break;
			case YZMode::MODE_COMPLETION :
				shape = "keep";
				break;
			default :
				shape = mParent->getLocalStringOption("cursor");
				break;
		}
		if ( shape == "hbar" ) {
			s = KYZisCursor::HBAR;
		} else if ( shape == "vbar" ) {
			s = KYZisCursor::VBAR;
		} else if ( shape == "keep" ) {
			s = mCursor->type();
		} else {
			if ( hasFocus() ) 
				s = KYZisCursor::SQUARE;
			else
				s = KYZisCursor::RECT;
		}
	}
	return s;
}
void KYZisEdit::updateCursor() {
	KYZisCursor::shape s = cursorShape();
	if ( s != mCursor->type() ) {
		mCursor->setCursorType( s );
		mCursor->refresh();
	}
}


void KYZisEdit::updateArea( ) {

	defaultCell.bg = backgroundColor();
	defaultCell.fg = foregroundColor();
	defaultCell.font = font();

	isFontFixed = fontInfo().fixedPitch();
	mParent->setFixedFont( isFontFixed );
	spaceWidth = mParent->getSpaceWidth();
	mCursor->resize( fontMetrics().maxWidth(), fontMetrics().lineSpacing() );
	updateCursor();

	int lines = height() / fontMetrics().lineSpacing();
	// if font is fixed, calculate the number of columns fontMetrics().maxWidth(), else give the width of the widget
	int columns = width() / GETX( 1 ) - marginLeft;
	erase();
	mCell.clear();
	mParent->setVisibleArea( columns, lines );
}

void KYZisEdit::paintEvent( QPaintEvent* pe ) {
	QRect r = pe->rect();
	int fx = r.left();
	int fy = r.top();
	int tx = r.right();
	int ty = r.bottom();
	if ( isFontFixed ) {
		unsigned int linespace = fontMetrics().lineSpacing();
		unsigned int maxwidth = fontMetrics().maxWidth();
		fx /= maxwidth;
		fy /= linespace;
		int old_tx = tx, old_ty = ty;
		tx /= maxwidth;
		ty /= linespace;
		if ( tx < old_tx ) ++tx;
		if ( ty < old_ty ) ++ty;
	}
	fx = qMax( (int)marginLeft, fx ) - marginLeft;
	tx = qMax( (int)marginLeft, tx ) - marginLeft;
	fy += mParent->getDrawCurrentTop();
	ty += mParent->getDrawCurrentTop();
	if ( fx == (int)mParent->getDrawCurrentLeft() && tx - fx == (int)(mParent->getColumnsVisible() + 1) ) {
		mParent->sendPaintEvent( YZCursor( fx, fy ), YZCursor( tx, ty ) );
	} else {
		mParent->setPaintAutoCommit( false );
		for( ; fy <= ty; ++fy ) {
			mParent->sendPaintEvent( YZCursor( fx, fy ), YZCursor( tx, fy ) );
		}
		mParent->commitPaintEvent();
	}
}

void KYZisEdit::setCursor( int c, int l ) {
//	yzDebug() << "setCursor" << endl;
	c = c - mParent->getDrawCurrentLeft() + marginLeft;
	l -= mParent->getDrawCurrentTop();
	unsigned int x = GETX( c );
	if ( mParent->getLocalBooleanOption( "rightleft" ) ) {
		x = width() - x - mCursor->width();
	}
	mCursor->move( x, l * fontMetrics().lineSpacing() );

	// need for InputMethod (OverTheSpot)
//	setMicroFocusHint( mCursor->x(), mCursor->y(), mCursor->width(), mCursor->height() );
}

QPoint KYZisEdit::cursorCoordinates( ) {
	QPoint position( mCursor->x(), mCursor->y() );
	return position;
}

void KYZisEdit::scrollUp( int n ) {
	mCursor->hide();
	scroll( 0, n * fontMetrics().lineSpacing() );
#if 0
	if ( ! mTransparent ) {
		mCursor->hide();
		bitBlt( this, 0, n * fontMetrics().lineSpacing(), this, 0, 0, width(), ( mParent->getLinesVisible() - n ) * fontMetrics().lineSpacing() );
		unsigned int lv = mParent->getLinesVisible();
		unsigned int i;
		for( i = lv; (int)i >= n; i-- )
			mCell[ i ] = mCell[ i - n ];
		setCursor( mParent->getCursor().x(), mParent->getCursor().y() );
		unsigned int top = mParent->getDrawCurrentTop();
		unsigned int left = mParent->getDrawCurrentLeft();
		mParent->sendPaintEvent( YZCursor( left , top ), YZCursor( left + mParent->getColumnsVisible(), top + n ) );
	} else {
		mCell.clear();
		mParent->sendRefreshEvent();
	}
#endif
}

void KYZisEdit::scrollDown( int n ) {
	mCursor->hide();
	scroll( 0, -n * fontMetrics().lineSpacing() );
#if 0
	if ( ! mTransparent ) {
		mCursor->hide();
		unsigned int h = mParent->getLinesVisible() - (unsigned int)n;
		bitBlt( this, 0, 0, this, 0, n * fontMetrics().lineSpacing(), width(), h * fontMetrics().lineSpacing() );
		unsigned int i;
		for( i = 0; i < h; i++ )
			mCell[ i ] = mCell[ i + n ];
		setCursor( mParent->getCursor().x(), mParent->getCursor().y() );
		h += mParent->getDrawCurrentTop();
		unsigned int left = mParent->getDrawCurrentLeft();
		mParent->sendPaintEvent( YZCursor( left, h ), YZCursor( left + mParent->getColumnsVisible(), h + n ) );
	} else {
		mCell.clear();
		mParent->sendRefreshEvent();
	}
#endif
}

bool KYZisEdit::event(QEvent *e) {
	if ( e->type() == QEvent::KeyPress ) {
		QKeyEvent *ke = (QKeyEvent *)e;
		if ( ke->key() == Qt::Key_Tab ) {
			keyPressEvent(ke);
			return TRUE;
		}
	}
	return QWidget::event(e);
}

void KYZisEdit::keyPressEvent ( QKeyEvent * e ) {
	Qt::KeyboardModifiers st = e->modifiers();
	QString modifiers;
	if ( st & Qt::ShiftButton )
		modifiers = "<SHIFT>";
	if ( st & Qt::AltButton )
		modifiers += "<ALT>";
	if ( st & Qt::ControlButton )
		modifiers += "<CTRL>";

	int lmode = mParent->modePool()->currentType();
	QString k;
	if ( keys.contains( e->key() ) ) //to handle some special keys
		k = keys[ e->key() ];
	else
		k = e->text();

	mParent->sendKey(k, modifiers);
	if ( lmode == YZMode::MODE_INSERT || lmode == YZMode::MODE_REPLACE ) {
		KYZTextEditorIface *d = static_cast<KYZTextEditorIface*>(mParent->document());
		emit d->emitChars(mCursor->y(), mCursor->x(),k);
	}
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

	// leave visual mode if the user clicks somewhere
	// TODO: this should only be done if the left button is used. Right button
	// should extend visual selection, like in vim.
	if ( mParent->modePool()->current()->isSelMode() )
		mParent->modePool()->pop();
	
	if (( e->button() == Qt::LeftButton ) || ( e->button() == Qt::RightButton )) {
		if (mParent->modePool()->currentType() != YZMode::MODE_EX) {
			mParent->gotodxdy( e->x() / ( isFontFixed ? fontMetrics().maxWidth() : 1 ) + mParent->getDrawCurrentLeft( ) - marginLeft,
						e->y() / fontMetrics().lineSpacing() + mParent->getDrawCurrentTop( ) );
			mParent->updateStickyCol();
		}
	} else if ( e->button() == Qt::MidButton ) {
		QString text = QApplication::clipboard()->text( QClipboard::Selection );
		if ( text.isNull() )
			text = QApplication::clipboard()->text( QClipboard::Clipboard );
		if ( ! text.isNull() ) {
			if ( mParent->modePool()->current()->isEditMode() ) {
				QChar reg = '\"';
				YZSession::me->setRegister( reg, QStringList::split( "\n", text ) );
				mParent->pasteContent( reg, false );
				mParent->moveRight();
			}
		}
	}
}

void KYZisEdit::mouseMoveEvent( QMouseEvent *e ) {
	if (e->state() == Qt::LeftButton) {
		if (mParent->modePool()->currentType() == YZMode::MODE_COMMAND) {
			// start visual mode when user makes a selection with the left mouse button
			mParent->modePool()->push( YZMode::MODE_VISUAL );
		} else if (mParent->modePool()->current()->isSelMode() ) {
			// already in visual mode - move cursor if the mouse pointer has moved over a new char
			unsigned int newX = e->x() / ( isFontFixed ? fontMetrics().maxWidth() : 1 )
				+ mParent->getDrawCurrentLeft() - marginLeft;
			unsigned int newY = e->y() / fontMetrics().lineSpacing()
				+ mParent->getDrawCurrentTop();

			if (newX != mParent->getCursor().x() || newY != mParent->getCursor().y()) {
				mParent->gotodxdy( newX, newY );
			}
		}
	}
}

void KYZisEdit::paintEvent( const YZSelection& drawMap ) {
//	yzDebug() << "KYZisEdit::paintEvent (top=" << mParent->getDrawCurrentTop()<< ",left="<< mParent->getDrawCurrentLeft() << ")" << endl << drawMap;
	if ( drawMap.isEmpty() )
		return;

	unsigned int linespace = fontMetrics().lineSpacing();
	QRect myRect;
	bool number = mParent->getLocalBooleanOption( "number" );
	bool rightleft = mParent->getLocalBooleanOption( "rightleft" );

	int flag = (rightleft ? Qt::AlignRight : Qt::AlignLeft);

	unsigned int lineCount = mParent->myBuffer()->lineCount();
	unsigned int my_marginLeft = 0;
	if ( number ) { // update marginLeft
		my_marginLeft = ( isFontFixed ? QString::number( lineCount ).length() + 2 : mParent->stringWidth( " " + QString::number( lineCount ) + "  " ) );
	}
	if ( marginLeft != my_marginLeft ) {
		if ( mCursor->visible() ) {
			mCursor->move( qMax( (int)( mCursor->x() + GETX( marginLeft - my_marginLeft ) ), 0 ), mCursor->y() );
		}
		marginLeft = my_marginLeft;
		updateArea();
		return;
	}

	defaultCell.flag = flag;

	QPainter p( this );

	unsigned int shiftY = mParent->getDrawCurrentTop();
	unsigned int shiftX = mParent->getDrawCurrentLeft();
	unsigned int maxX = shiftX + mParent->getColumnsVisible();

	unsigned int cursorY = mParent->getCursor().y();
//	unsigned int cursorX = mParent->getCursor()->x();
	bool refreshCursor = false;

	YZSelectionMap map = drawMap.map();
	unsigned int size = map.size();

	unsigned int fromY = map[ 0 ].fromPos().y();
	unsigned int toY = map[ size - 1 ].toPos().y();

	bool drawIt = false;
	unsigned int mapIdx = 0;

	unsigned int fX = map[ mapIdx ].fromPos().x();
	unsigned int fY = map[ mapIdx ].fromPos().y();
	unsigned int tX = map[ mapIdx ].toPos().x();
	unsigned int tY = map[ mapIdx ].toPos().y();

	unsigned int curY = mParent->initDrawContents( fromY );
	unsigned int curX = 0;

	myRect.setTop( ( curY - shiftY ) * linespace );
	myRect.setHeight( linespace );

#define REVERSE_MYRECT_IF_RIGHTLEFT \
	{ if ( rightleft ) \
		myRect.moveRight( width() - myRect.x() ); \
	}

	KYZViewCell cell = defaultCell;
	unsigned int mCellY;
	int mCellX;
	QList<unsigned int> mCellKeys;

	bool drawEntireLine;

	while( curY <= toY && mParent->drawNextLine() ) {
		curX = shiftX;

		mCellY = curY - shiftY;
		mCellX = 0;
		mCellKeys.clear();

		if ( tY < curY ) {
			++mapIdx;
			fX = map[ mapIdx ].fromPos().x();
			fY = map[ mapIdx ].fromPos().y();
			tX = map[ mapIdx ].toPos().x();
			tY = map[ mapIdx ].toPos().y();
		}

		drawEntireLine = !( curY == fY && fX > shiftX || curY == tY && tX < maxX );
		drawIt = curY == fY && fX <= shiftX || fY < curY && curY <= tY;
//		yzDebug() << curY << " : " << drawIt << "-" << drawEntireLine << endl;

		myRect.setLeft( 0 );

		if ( drawIt || !drawEntireLine && !drawIt ) { // this line will be drawn

			refreshCursor = ( curY == cursorY );

			mCellKeys = mCell[ mCellY ].keys();
			
			if ( number ) {
				myRect.setWidth( GETX( marginLeft - spaceWidth ) );
				REVERSE_MYRECT_IF_RIGHTLEFT;
				erase( myRect );

				if ( mParent->lineHeight() == 1 ) {
					p.setPen( Qt::yellow ); // XXX: custom
					p.setBackgroundMode( Qt::TransparentMode );
					p.setFont( font() );
					p.drawText( myRect, (rightleft ? Qt::AlignLeft : Qt::AlignRight), QString::number( mParent->drawLineNumber() ) );
				}
			}
		}
		if ( drawIt ) {
			myRect.setLeft( GETX( marginLeft ) );
			if ( drawEntireLine ) {
				myRect.setRight( width() );
				mCell[ mCellY ].clear();
			} else {
				if ( tY == curY ) {
					myRect.setWidth( GETX( tX - shiftX + 1 ) );
					for( mCellX = 0; mCellX < mCellKeys.size() && mCellKeys[ mCellX ] <= (tX - shiftX); ++mCellX )
						mCell[ mCellY ].remove( mCellKeys[ mCellX ] );
				} else {
					myRect.setRight( width() );
					for( mCellX = 0; mCellX < mCellKeys.size(); ++mCellX )
						mCell[ mCellY ].remove( mCellKeys[ mCellX ] );
				}
			}
			REVERSE_MYRECT_IF_RIGHTLEFT;
//			yzDebug() << "erase1(" << myRect.top() << "," << myRect.left() << "," << myRect.bottom() << "," << myRect.right() << ")" << endl;
			erase( myRect );
		}
		while( mParent->drawNextCol() ) {
			if ( ! drawEntireLine ) {
				if ( !drawIt && curY == fY ) { // start drawing ?
					drawIt = ( curX == fX );
					if ( drawIt ) {
						myRect.setLeft( GETX( marginLeft + curX - shiftX ) );
						while( mCellX < mCellKeys.size() && mCellKeys[ mCellX ] < (fX - shiftX) )
							++mCellX;
						if ( tY == curY ) {
							myRect.setRight( GETX( marginLeft + tX - shiftX + 1 ) - 1 );
							for( ; mCellX < mCellKeys.size() && mCellKeys[ mCellX ] <= (tX - shiftX); ++mCellX )
								mCell[ mCellY ].remove( mCellKeys[ mCellX ] );
						} else {
							myRect.setRight( width() );
							for( ; mCellX < mCellKeys.size(); ++mCellX )
								mCell[ mCellY ].remove( mCellKeys[ mCellX ] );
						}
						REVERSE_MYRECT_IF_RIGHTLEFT;
//						yzDebug() << "erase2(" << myRect.top() << "," << myRect.left() << "," << myRect.bottom() << "," << myRect.right() << ")" << endl;
						erase( myRect );
					}
				} else if ( drawIt && curY == tY ) { // stop drawing ?
					drawIt = !( curX > tX );
					if ( ! drawIt ) {
						++mapIdx;
						if ( mapIdx != size ) {
							fX = map[ mapIdx ].fromPos().x();
							fY = map[ mapIdx ].fromPos().y();
							tX = map[ mapIdx ].toPos().x();
							tY = map[ mapIdx ].toPos().y();
						} else {
							fX = fY = tX = tY = 0;
						}
					}
				}
			}
			if ( drawIt ) {
				QString disp = QString( mParent->drawChar() );
				cell.c = disp;

				myRect.setLeft( GETX( marginLeft + curX - shiftX ) );
				myRect.setWidth( GETX( mParent->drawLength() ) );
				REVERSE_MYRECT_IF_RIGHTLEFT;

				if ( rightleft )
					disp = disp.rightJustify( mParent->drawLength(), mParent->fillChar() );
				else
					disp = disp.leftJustify( mParent->drawLength(), mParent->fillChar() );

				QColor c( mParent->drawColor().rgb() );
				cell.fg = c.isValid() ? c : defaultCell.fg;
				c.setRgb( mParent->drawBgColor().rgb() );
				cell.bg = c.isValid() ? c : defaultCell.bg;
				cell.selected = mParent->drawSelected();

				QFont myFont( font() );
				if ( mParent->drawItalic() )
					myFont.setItalic( true );
				if ( mParent->drawBold() )
					myFont.setBold( true );
				myFont.setOverline( mParent->drawOverline() );
				myFont.setStrikeOut( mParent->drawStrikeOutLine() );
				myFont.setUnderline( mParent->drawUnderline() );
				cell.font = myFont;
				p.setFont( cell.font );

				if ( cell.selected ) {
					p.setPen( cell.bg );
					p.setBackgroundMode( Qt::OpaqueMode );
					p.setBackgroundColor( cell.fg );
					p.eraseRect( myRect );
				} else {
					p.setPen( cell.fg );
					if ( cell.bg != backgroundColor() ) {
						p.setBackgroundMode( Qt::OpaqueMode );
						p.setBackgroundColor( cell.bg );
						p.eraseRect( myRect );
					} else {
						p.setBackgroundMode( Qt::TransparentMode );
						if ( curX == tX && mParent->drawLength() > 1 )
							erase( myRect );
					}
				}
				p.drawText( myRect, flag, disp );

				mCell[ mCellY ][ rightleft ? myRect.right() : myRect.left() ] = cell;
			}
			curX += mParent->drawLength();
		}
		curY += mParent->drawHeight();
		myRect.moveBy( 0, linespace );

		if ( refreshCursor ) {
			mCursor->refresh( &p );
			refreshCursor = false;
		}
	}

	if ( number && fromY < curY ) {
		p.setPen( Settings::colorFG() );
		unsigned int w;
		if ( rightleft )
			w = width() - GETX( marginLeft ) + GETX( spaceWidth ) / 2;
		else
			w = GETX( marginLeft ) - GETX( spaceWidth ) / 2;
		p.drawLine( w, (fromY - shiftY) * linespace, w, (curY - shiftY) * linespace );
	}

	unsigned int fh = shiftY + height() / linespace;
	toY = qMin( toY, fh - 1 );
	myRect.setLeft( 0 );
	myRect.setHeight( linespace );
	myRect.setWidth( width() );
	for( ; curY <= toY; ++curY ) {
		erase( myRect );
		p.setPen( Qt::cyan );
		p.drawText( myRect, flag, "~" );
		myRect.moveBy( 0, linespace );
	}
}

void KYZisEdit::drawCell( QPainter* p, const KYZViewCell& cell, const QRect& rect, bool reversed  ) {
	if ( ! cell.isValid ) {
		drawCell( p, defaultCell, rect, reversed );
		return;
	}
	p->setFont( cell.font );
	if ( cell.selected || reversed ) {
		p->setPen( cell.bg );
		p->setBackgroundMode( Qt::OpaqueMode );
		p->setBackgroundColor( cell.fg );
		p->eraseRect( rect );
	} else {
		p->setPen( cell.fg );
		if ( cell.bg != backgroundColor() ) {
			p->setBackgroundMode( Qt::OpaqueMode );
			p->setBackgroundColor( cell.bg );
			p->eraseRect( rect );
		} else {
			p->setBackgroundMode( Qt::TransparentMode );
		}
	}
	p->drawText( rect, cell.flag, cell.c );
}

void KYZisEdit::focusInEvent ( QFocusEvent * ) {
	KYZisFactory::self()->setCurrentView( mParent );
	updateCursor();
}
void KYZisEdit::focusOutEvent ( QFocusEvent * ) {
	updateCursor();
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
	KAction* k = new KAction( "", KShortcut( keysToShortcut( keys ) ), signalMapper, SLOT( map() ), actionCollection, keys.ascii() );
	signalMapper->setMapping( k, keys );
}
void KYZisEdit::unregisterModifierKeys( const QString& keys ) {
	QByteArray ke = keys.toUtf8();
	KAction* k = actionCollection->action( ke.data() );
	if ( k == NULL ) {
		yzDebug() << "No KAction for " << keys << endl;
		return;
	}
	actionCollection->take( k );
	KAccel* accel = actionCollection->kaccel();
	if ( accel ) {
		accel->remove( keys );
		accel->updateConnections();
	}
	signalMapper->removeMappings( k );
	delete k;
}

void KYZisEdit::sendMultipleKey( const QString& keys ) {
	mParent->sendMultipleKey( keys );
}

const QString& KYZisEdit::convertKey( int key ) {
	return keys[ key ];
}

void KYZisEdit::inputMethodEvent ( QInputMethodEvent * ) {
	//TODO
}

QVariant KYZisEdit::inputMethodQuery ( Qt::InputMethodQuery query ) {
	return QWidget::inputMethodQuery( query );
}

// for InputMethod (OnTheSpot)
/*
void KYZisEdit::imStartEvent( QIMEvent *e )
{
	if ( mParent->modePool()->current()->supportsInputMethod() ) {
		mParent->modePool()->current()->imBegin( mParent );
	}
	e->accept();
}*/

// for InputMethod (OnTheSpot)
/*
void KYZisEdit::imComposeEvent( QIMEvent *e ) {
	//yzDebug() << "KYZisEdit::imComposeEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
	if ( mParent->modePool()->current()->supportsInputMethod() ) {
		mParent->modePool()->current()->imCompose( mParent, e->text() );
		e->accept();
	} else {
		e->ignore();
	}
}*/

// for InputMethod (OnTheSpot)
/*
void KYZisEdit::imEndEvent( QIMEvent *e ) {
//	yzDebug() << "KYZisEdit::imEndEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
	if ( mParent->modePool()->current()->supportsInputMethod() ) {
		mParent->modePool()->current()->imEnd( mParent, e->text() );
	} else {
		mParent->sendKey( e->text() );
	}
	e->accept();
}*/

#include "editor.moc"
