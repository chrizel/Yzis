/* This file is part of the Yzis libraries
 *  Copyright (C) 2007 Lothar Braun <lothar@lobraun.de>
 *  Copyright (C) 2005 Erlend Hamberg <hamberg@stud.ntnu.no>
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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kyzisview.h"
#include "kyzissession.h"

#include <QSignalMapper>
#include <QAction>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QClipboard>
#include <QTimer>

#include <kdebug.h>
#include <kapplication.h>
#include <kactioncollection.h>
#include <kshortcut.h>
#include <kaction.h>

#include <libyzis/debug.h>

// TODO: do we still have something like isFontFixed?
#define GETX( x ) ( isFontFixed ? ( x ) * fontMetrics().maxWidth() : x )

KYZisView::KYZisView(YZBuffer* buffer, QWidget* parent)
	: YZView(buffer, KYZisSession::self(),0,0), QWidget(parent)
{
	setFocusPolicy( Qt::StrongFocus );

	setAutoFillBackground( false );
	setAttribute ( Qt::WA_PaintOutsidePaintEvent ); /* XXX */

	/* show an edit cursor */
	QWidget::setCursor( Qt::IBeamCursor );

	isFontFixed = fontInfo().fixedPitch();

	// for Input Method
	setAttribute( Qt::WA_InputMethodEnabled, true );

	mCursor = new KYZisCursor( this, KYZisCursor::SQUARE );

	initKeys();

	marginLeft = 0;

	QTimer::singleShot(0, static_cast<KYZisSession*>(YZSession::self()), SLOT(frontendGuiReady()) );
}

KYZisView::~KYZisView()
{
	delete signalMapper;
	for( int i = actionCollection->count() - 1; i>= 0; --i )
		delete actionCollection->takeAction( actionCollection->action(i) );
	delete actionCollection;
}

void KYZisView::setPalette( const QColor& fg, const QColor& bg, double opacity ) {
	QPalette p = palette();
	p.setColor( QPalette::WindowText, fg );
	p.setColor( QPalette::Window, bg );
	QWidget::setPalette( p );
	setWindowOpacity( opacity );
}

KYZisCursor::shape KYZisView::cursorShape() {
	KYZisCursor::shape s;
	if ( !isFontFixed ) {
		s = KYZisCursor::VBAR;
	} else {
		QString shape;
		YZMode::ModeType m = modePool()->current()->type();
		switch( m ) {
			case YZMode::ModeInsert :
				shape = getLocalStringOption("cursorinsert");
				break;
			case YZMode::ModeReplace :
				shape = getLocalStringOption("cursorreplace");
				break;
			case YZMode::ModeCompletion :
				shape = "keep";
				break;
			default :
				shape = getLocalStringOption("cursor");
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
void KYZisView::updateCursor() {
	mCursor->setCursorType( cursorShape() );
}


void KYZisView::updateArea( ) {

	isFontFixed = fontInfo().fixedPitch();
	//TODO: What about the next lines???
	//setFixedFont( isFontFixed );
	//spaceWidth = getSpaceWidth();
	
	mCursor->resize( fontMetrics().maxWidth(), fontMetrics().lineSpacing() );
	updateCursor();

	int lines = height() / fontMetrics().lineSpacing();
	// if font is fixed, calculate the number of columns fontMetrics().maxWidth(), else give the width of the widget
	int columns = width() / GETX( 1 ) - marginLeft;
	//erase();
	setVisibleArea( columns, lines );
}

/**
 * QWidget event handling
 */
bool KYZisView::event(QEvent *e) {
	if ( e->type() == QEvent::KeyPress ) {
		QKeyEvent *ke = (QKeyEvent *)e;
		if ( ke->key() == Qt::Key_Tab ) {
			keyPressEvent(ke);
			return true;
		}
	}
	return QWidget::event(e);
}

void KYZisView::keyPressEvent ( QKeyEvent * e ) {
	Qt::KeyboardModifiers st = e->modifiers();
	QString modifiers;
	if ( st & Qt::ShiftModifier )
		modifiers = "<SHIFT>";
	if ( st & Qt::AltModifier )
		modifiers += "<ALT>";
	if ( st & Qt::ControlModifier )
		modifiers += "<CTRL>";

	int lmode = modePool()->currentType();
	QString k;
	if ( keys.contains( e->key() ) ) //to handle some special keys
		k = keys[ e->key() ];
	else
		k = e->text();

	KYZisSession::self()->sendKey(this, k, modifiers);
	if ( lmode == YZMode::ModeInsert || lmode == YZMode::ModeReplace ) {
		//KYZTextEditorIface *d = static_cast<KYZTextEditorIface*>(document());
		//emit d->emitChars(mCursor->y(), mCursor->x(),k);
	}
	e->accept();
}

void KYZisView::mousePressEvent ( QMouseEvent * e ) {
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
	if ( modePool()->current()->isSelMode() )
		modePool()->pop();
	
	if (( e->button() == Qt::LeftButton ) || ( e->button() == Qt::RightButton )) {
		if (modePool()->currentType() != YZMode::ModeEx) {
			gotodxdy( e->x() / ( isFontFixed ? fontMetrics().maxWidth() : 1 ) + getDrawCurrentLeft( ) - marginLeft,
						e->y() / fontMetrics().lineSpacing() + getDrawCurrentTop( ) );
			updateStickyCol();
		}
	} else if ( e->button() == Qt::MidButton ) {
		QString text = KApplication::clipboard()->text( QClipboard::Selection );
		if ( text.isNull() )
			text = QApplication::clipboard()->text( QClipboard::Clipboard );
		if ( ! text.isNull() ) {
			if ( modePool()->current()->isEditMode() ) {
				QChar reg = '\"';
				KYZisSession::self()->setRegister( reg, text.split( "\n" ) );
				pasteContent( reg, false );
				moveRight();
			}
		}
	}
}

void KYZisView::mouseMoveEvent( QMouseEvent *e ) {
	if (e->button() == Qt::LeftButton) {
		if (modePool()->currentType() == YZMode::ModeCommand) {
			// start visual mode when user makes a selection with the left mouse button
			modePool()->push( YZMode::ModeVisual );
		} else if (modePool()->current()->isSelMode() ) {
			// already in visual mode - move cursor if the mouse pointer has moved over a new char
			int newX = e->x() / ( isFontFixed ? fontMetrics().maxWidth() : 1 )
				+ getDrawCurrentLeft() - marginLeft;
			int newY = e->y() / fontMetrics().lineSpacing()
				+ getDrawCurrentTop();

			if (newX != getCursor().x() || newY != getCursor().y()) {
				gotodxdy( newX, newY );
			}
		}
	}
}

void KYZisView::focusInEvent ( QFocusEvent * ) {
	KYZisSession::self()->setCurrentView( this );
	updateCursor();
}
void KYZisView::focusOutEvent ( QFocusEvent * ) {
	updateCursor();
}


void KYZisView::resizeEvent(QResizeEvent* e) {
	e->accept();
	updateArea();
}

void KYZisView::paintEvent( QPaintEvent* pe ) {
	QRect r = pe->rect();
	int fx = r.left();
	int fy = r.top();
	int tx = r.right();
	int ty = r.bottom();
	yzDebug() << "KYZisView < QPaintEvent( " << fx << "," << fy << " -> " << tx << "," << ty << " )" << endl;
	m_insidePaintEvent = true;
	if ( isFontFixed ) {
		int linespace = fontMetrics().lineSpacing();
		int maxwidth = fontMetrics().maxWidth();
		fx /= maxwidth;
		fy /= linespace;
		int old_tx = tx, old_ty = ty;
		tx /= maxwidth;
		ty /= linespace;
		if ( tx < old_tx ) ++tx;
		if ( ty < old_ty ) ++ty;
	}
	fx = qMax( marginLeft, fx ) - marginLeft;
	tx = qMax( marginLeft, tx ) - marginLeft;
	fy += getDrawCurrentTop();
	ty += getDrawCurrentTop();
	if ( fx == (int)getDrawCurrentLeft() && tx - fx == (int)(getColumnsVisible() + 1) ) {
		sendPaintEvent( YZCursor( fx, fy ), YZCursor( tx, ty ) );
	} else {
		setPaintAutoCommit( false );
		for( ; fy <= ty; ++fy ) {
			sendPaintEvent( YZCursor( fx, fy ), YZCursor( tx, fy ) );
		}
		commitPaintEvent();
	}
	m_insidePaintEvent = false;
	yzDebug() << "KYZisView > QPaintEvent" << endl;
}
void KYZisView::paintEvent( const YZSelection& drawMap ) {
	yzDebug() << "KYZisView::paintEvent" << endl;
	YZSelectionMap m = drawMap.map();
	for( int i = 0; i < m.size(); ++i ) {
		int left = GETX( qMin( m[i].fromPos().x(), m[i].toPos().x() ) );
		int right = GETX( qMax( m[i].fromPos().x(), m[i].toPos().x() ) );
		int top = qMin( m[i].fromPos().y(), m[i].toPos().y() ) * fontMetrics().lineSpacing();
		int bottom = qMax( m[i].fromPos().y(), m[i].toPos().y() ) * fontMetrics().lineSpacing();
		
		update( QRect(left, top, right - left, bottom - top) );
	}
	yzDebug() << "KYZisView::paintEvent ends" << endl;
}

void KYZisView::setCursor( int c, int l ) {
//	yzDebug() << "setCursor" << endl;
	c = c - getDrawCurrentLeft() + marginLeft;
	l -= getDrawCurrentTop();
	unsigned int x = GETX( c );
	if ( getLocalBooleanOption( "rightleft" ) ) {
		x = width() - x - mCursor->width();
	}
	mCursor->move( x, l * fontMetrics().lineSpacing() );

	// need for InputMethod (OverTheSpot)
//	setMicroFocusHint( mCursor->x(), mCursor->y(), mCursor->width(), mCursor->height() );
}

QPoint KYZisView::cursorCoordinates( ) {
	return QPoint( mCursor->x(), mCursor->y() );
}

void KYZisView::scrollUp( int n ) {
	mCursor->hide();
	scroll( 0, n * fontMetrics().lineSpacing() );
	mCursor->show();
}
void KYZisView::scrollDown( int n ) {
	scrollUp( -n );
}

void KYZisView::guiDrawCell( int x, int y, const YZDrawCell& cell, QPainter* p ) {
	p->save();
	if ( cell.fg.isValid() )
		p->setPen( cell.fg.rgb() );
	if ( !fakeLine )
		x += marginLeft;
	QRect r( GETX(x), y*fontMetrics().lineSpacing(), cell.c.length()*fontMetrics().maxWidth(), fontMetrics().lineSpacing() );
	p->eraseRect( r );
	p->drawText( r, cell.c );
	p->restore();
}

void KYZisView::guiDrawClearToEOL( int x, int y, const QChar& clearChar, QPainter* p ) {
	QRect r( GETX(x), y*fontMetrics().lineSpacing(), width(), fontMetrics().lineSpacing() );
	p->eraseRect( r );
}

void KYZisView::guiDrawSetMaxLineNumber( int max ) {
	int my_marginLeft = 2 + QString::number( max ).length();
	if ( my_marginLeft != marginLeft ) {
		marginLeft = my_marginLeft;
		updateArea();
	}
}
void KYZisView::guiDrawSetLineNumber( int y, int n, int h, QPainter* p ) {
	fakeLine = n <= 0;

	QString num;
	if ( !fakeLine && h == 0 )
		num = QString::number( n );
	num = num.rightJustified( marginLeft - 1, ' ' );

	p->save();
	p->setPen( Qt::yellow );

	QRect r( 0, y*fontMetrics().lineSpacing(), GETX(marginLeft /*- spaceWidth*/), fontMetrics().lineSpacing() );
	p->eraseRect( r );
	p->drawText( r, num );

	p->restore();
}

void KYZisView::drawMarginLeft( int min_y, int max_y, QPainter* p ) {
	if ( marginLeft > 0 ) {
		int x = GETX( marginLeft ) /*- GETX( spaceWidth )*//2;
		p->save();
		// TODO:
		p->setPen( Qt::red );
		p->drawLine( x, min_y*fontMetrics().lineSpacing(), x, max_y*fontMetrics().lineSpacing() );
		p->restore();
	}
}

void KYZisView::initKeys() {
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
	//keys[ Qt::Key_Prior ] = "<PUP>" ; seems like it doesn't exist any longer with qt 4.3
	//keys[ Qt::Key_Next ] = "<PDOWN>" ; seems like it doesn't exist any longer with qt 4.3
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


	actionCollection = new KActionCollection( this );
	signalMapper = new QSignalMapper( this );
	connect( signalMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( sendMultipleKey( const QString& ) ) );
}

QString KYZisView::keysToShortcut( const QString& keys ) {
	QString ret = keys;
	ret = ret.replace( "<CTRL>", "CTRL+" );
	ret = ret.replace( "<SHIFT>", "SHIFT+" );
	ret = ret.replace( "<ALT>", "ALT+" );
	return ret;
}

void KYZisView::registerModifierKeys( const QString& keys ) {
	KAction* k = new KAction( actionCollection );
	k->setShortcut( KShortcut( keysToShortcut( keys ) ) );
	connect( k, SIGNAL( triggered() ), signalMapper, SLOT( map() ) );
		
	signalMapper->setMapping( k, keys );
}
void KYZisView::unregisterModifierKeys( const QString& keys ) {
	QByteArray ke = keys.toUtf8();
	QAction* q = actionCollection->action( ke.data() );
	if ( q == NULL ) {
		yzDebug() << "No KAction for " << keys << endl;
		return;
	}
	actionCollection->takeAction( q );
#warning port to KDE4 API
#if 0
	KAccel* accel = actionCollection->kaccel();
	if ( accel ) {
		accel->remove( keys );
		accel->updateConnections();
	}
#endif
	signalMapper->removeMappings( q );
	delete q;
}

//void KYZisView::sendMultipleKey( const QString& keys ) {
//	sendMultipleKey( keys );
//}

const QString& KYZisView::convertKey( int key ) {
	return keys[ key ];
}

void KYZisView::inputMethodEvent ( QInputMethodEvent * ) {
	//TODO
}

QVariant KYZisView::inputMethodQuery ( Qt::InputMethodQuery query ) {
	return QWidget::inputMethodQuery( query );
}

YZDrawCell KYZisView::getCursorDrawCell( )
{
	return m_drawBuffer.at( getCursor() - YZCursor(getDrawCurrentLeft(), getDrawCurrentTop( )) );
}

// for InputMethod (OnTheSpot)
/*
void KYZisView::imStartEvent( QIMEvent *e )
{
	if ( mParent->modePool()->current()->supportsInputMethod() ) {
		mParent->modePool()->current()->imBegin( mParent );
	}
	e->accept();
}*/

// for InputMethod (OnTheSpot)
/*
void KYZisView::imComposeEvent( QIMEvent *e ) {
	//yzDebug() << "KYZisView::imComposeEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
	if ( mParent->modePool()->current()->supportsInputMethod() ) {
		mParent->modePool()->current()->imCompose( mParent, e->text() );
		e->accept();
	} else {
		e->ignore();
	}
}*/

// for InputMethod (OnTheSpot)
/*
void KYZisView::imEndEvent( QIMEvent *e ) {
//	yzDebug() << "KYZisView::imEndEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
	if ( mParent->modePool()->current()->supportsInputMethod() ) {
		mParent->modePool()->current()->imEnd( mParent, e->text() );
	} else {
		mParent->sendKey( e->text() );
	}
	e->accept();
}*/

