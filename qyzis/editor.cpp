/* This file is part of the QYzis
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2004-2006 Loic Pauleve <panard@inzenet.org>
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

#include <QApplication>
#include <QSignalMapper>
#include <QWidget>

#include "editor.h"
#include "debug.h"
#include "yzis.h"
#include "factory.h"
#include "registers.h"
#include "viewwidget.h"

#include <math.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <ctype.h>

#include "qyzis.h"

#define GETX( x ) ( isFontFixed ? ( x ) * fontMetrics().maxWidth() : x )

QYZisEdit::QYZisEdit(QYZisView *parent)
: QWidget( parent )
{
	mParent = parent;

	m_useArea.setCoords(0,0,0,0);

	setFocusPolicy( Qt::StrongFocus );

	setAutoFillBackground( true );

	// for Input Method
	setAttribute(Qt::WA_InputMethodEnabled, true);

	/* show an edit cursor */
	QWidget::setCursor( Qt::IBeamCursor );

	isFontFixed = fontInfo().fixedPitch();

	mCursor = new QYZisCursor( this, QYZisCursor::SQUARE );

	initKeys();
}


QYZisEdit::~QYZisEdit() {
	delete signalMapper;
	/*
	for( int i = actionCollection->count() - 1; i>= 0; --i )
		delete actionCollection->take( actionCollection->action(i) );
	delete actionCollection;
	*/
}

QYZisView* QYZisEdit::view() const {
	return mParent;
}

QPoint QYZisEdit::translatePositionToReal( const YZCursor& c ) const {
	return QPoint( GETX(c.x()), c.y() * fontMetrics().lineSpacing() );
}
YZCursor QYZisEdit::translateRealToPosition( const QPoint& p, bool ceil ) const {
	int height = fontMetrics().lineSpacing();
	int width = isFontFixed ? fontMetrics().maxWidth() : 1;

	int x = p.x() / width;
	int y = p.y() / height;
	if ( ceil ) {
		if ( p.y() % height )
			++y;
		if ( p.x() % width )
			++x;
	}
	return YZCursor( x, y );
}
YZCursor QYZisEdit::translateRealToAbsolutePosition( const QPoint& p, bool ceil ) const {
	return translateRealToPosition( p, ceil ) + mParent->getScreenPosition();
}

void QYZisEdit::setPalette( const QPalette& p, qreal opacity ) {
	QWidget::setPalette( p );
	//setWindowOpacity( opacity ); XXX doesn't work...
	Qyzis::me->setWindowOpacity(opacity);
}

QYZisCursor::shape QYZisEdit::cursorShape() {
	QYZisCursor::shape s;
	if ( !isFontFixed ) {
		s = QYZisCursor::VBAR;
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
			s = QYZisCursor::HBAR;
		} else if ( shape == "vbar" ) {
			s = QYZisCursor::VBAR;
		} else if ( shape == "keep" ) {
			s = mCursor->type();
		} else {
			if ( hasFocus() ) 
				s = QYZisCursor::SQUARE;
			else
				s = QYZisCursor::RECT;
		}
	}
	return s;
}
void QYZisEdit::updateCursor() {
	mCursor->setCursorType( cursorShape() );
	mCursor->update();
}


void QYZisEdit::updateArea( ) {

	isFontFixed = fontInfo().fixedPitch();
	mParent->setFixedFont( isFontFixed );
	spaceWidth = mParent->getSpaceWidth();
	updateCursor();

	yzDebug() << "isFontFixed = " << isFontFixed << endl;
	yzDebug() << "lineheight = " << fontMetrics().lineSpacing() << endl;
	yzDebug() << "maxwidth = " << fontMetrics().maxWidth() << endl;
	yzDebug() << "height = " << height();

	int lines = height() / fontMetrics().lineSpacing();
	// if font is fixed, calculate the number of columns fontMetrics().maxWidth(), else give the width of the widget
	int columns = width() / GETX(1);

	yzDebug() << "lines = " << lines;

	m_useArea.setBottomRight( QPoint(GETX(columns), lines * fontMetrics().lineSpacing()) );

	mParent->setVisibleArea( columns, lines );
}

/**
 * QWidget event handling
 */
bool QYZisEdit::event(QEvent *e) {
	if ( e->type() == QEvent::KeyPress ) {
		QKeyEvent *ke = (QKeyEvent *)e;
		if ( ke->key() == Qt::Key_Tab ) {
			keyPressEvent(ke);
			return true;
		}
	}
	return QWidget::event(e);
}

void QYZisEdit::keyPressEvent ( QKeyEvent * e ) {
	Qt::KeyboardModifiers st = e->modifiers();
	QString modifiers;
	if ( st & Qt::ShiftModifier )
		modifiers = "<SHIFT>";
	if ( st & Qt::AltModifier )
		modifiers += "<ALT>";
	if ( st & Qt::ControlModifier )
		modifiers += "<CTRL>";

	QString k;
	if ( keys.contains( e->key() ) ) //to handle some special keys
		k = keys[ e->key() ];
	else
		k = e->text();

	mParent->sendKey(k, modifiers);
	e->accept();
}

void QYZisEdit::mousePressEvent ( QMouseEvent * e ) {
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
		if ( mParent->modePool()->currentType() != YZMode::MODE_EX ) {
			mParent->gotodxdyAndStick( translateRealToAbsolutePosition( e->pos() ) );
		}
	} else if ( e->button() == Qt::MidButton ) {
		QString text = QApplication::clipboard()->text( QClipboard::Selection );
		if ( text.isNull() )
			text = QApplication::clipboard()->text( QClipboard::Clipboard );
		if ( ! text.isNull() ) {
			if ( mParent->modePool()->current()->isEditMode() ) {
				QChar reg = '\"';
				YZSession::me->setRegister( reg, text.split("\n") );
				mParent->pasteContent( reg, false );
				mParent->moveRight();
			}
		}
	}
}

void QYZisEdit::mouseMoveEvent( QMouseEvent *e ) {
	if (e->buttons() == Qt::LeftButton) {
		if (mParent->modePool()->currentType() == YZMode::MODE_COMMAND) {
			// start visual mode when user makes a selection with the left mouse button
			mParent->modePool()->push( YZMode::MODE_VISUAL );
		} else if (mParent->modePool()->current()->isSelMode() ) {
			// already in visual mode - move cursor if the mouse pointer has moved over a new char
			YZCursor pos = translateRealToAbsolutePosition( e->pos() );
			if ( pos != mParent->getCursor() ) {
				mParent->gotodxdy( pos.x(), pos.y() );
			}
		}
	}
}

void QYZisEdit::focusInEvent ( QFocusEvent * ) {
	QYZisFactory::self()->setCurrentView( mParent );
	updateCursor();
}
void QYZisEdit::focusOutEvent ( QFocusEvent * ) {
	updateCursor();
}

void QYZisEdit::resizeEvent(QResizeEvent* e) {
	e->accept();
	updateArea();
}

void QYZisEdit::paintEvent( QPaintEvent* pe ) {
	// convert QPaintEvent rect to yzis coordinate
	QRect r = pe->rect();
	r.setTopLeft( translateRealToAbsolutePosition( r.topLeft() ) );
	r.setBottomRight( translateRealToAbsolutePosition( r.bottomRight() ) );
	//yzDebug() << "QYZisEdit::paintEvent : " << pe->rect().topLeft() << "," << pe->rect().bottomRight() << 
	//				" => " << r.topLeft() << "," << r.bottomRight() << endl;
	// paint it
	mParent->paintEvent( mParent->clipSelection( YZSelection( r ) ) );
}

void QYZisEdit::setCursor( int c, int l ) {
//	yzDebug() << "setCursor" << endl;
	c = c - mParent->getDrawCurrentLeft();
	l -= mParent->getDrawCurrentTop();
	unsigned int x = GETX( c );
	if ( mParent->getLocalBooleanOption( "rightleft" ) ) {
		x = width() - x - mCursor->width();
	}
	mCursor->move( x, l * fontMetrics().lineSpacing() );
	mCursor->show();

	// need for InputMethod (OverTheSpot)
//	setMicroFocusHint( mCursor->x(), mCursor->y(), mCursor->width(), mCursor->height() );
}

void QYZisEdit::scroll( int dx, int dy ) {
	int rx = GETX(dx);
	int ry = dy * fontMetrics().lineSpacing();
	mCursor->hide();
	QRect cursorRect = mCursor->rect();
	cursorRect.moveTo( mCursor->pos() );
	update(cursorRect);
	QWidget::scroll( rx, ry, m_useArea );
}

void QYZisEdit::drawCell( int x, int y, const YZDrawCell& cell, QPainter* p ) {
	//yzDebug() << "QYZisEdit::drawCell(" << x << "," << y <<",'" << cell.c << "')" << endl;
	p->save();
	bool has_bg = false;
	if ( !cell.sel ) {
		if ( cell.fg.isValid() )
			p->setPen( cell.fg.rgb() );
		if ( cell.bg.isValid() ) {
			has_bg = true;
			p->setBackground( QColor(cell.bg.rgb()) );
		}
	} else if ( cell.sel & YZSelectionPool::Visual ) {
		has_bg = true;
		p->setBackground( QColor(181, 24, 181)  ); //XXX setting
		p->setPen( Qt::white );
	} else {
		has_bg = true;
		p->setBackground( cell.fg.isValid() ? QColor(cell.fg.rgb()) : palette().color( QPalette::WindowText ) );
		p->setPen( cell.bg.isValid() ? QColor(cell.bg.rgb()) : palette().color( QPalette::Window ) );
	}
	QRect r( GETX(x), y*fontMetrics().lineSpacing(), cell.c.length()*fontMetrics().maxWidth(), fontMetrics().lineSpacing() );

	//yzDebug() << "drawCell: r=" << r.topLeft() << "," << r.bottomRight() << " has_bg=" << has_bg << endl;
	//yzDebug() << "drawCell: fg=" << p->pen().color().name() << endl;
	if ( has_bg )
		p->eraseRect( r ); 
	p->drawText( r, cell.c );
	p->restore();
}

void QYZisEdit::drawClearToEOL( int x, int y, const QChar& clearChar, QPainter* p ) {
	//yzDebug() << "QYZisEdit::drawClearToEOL("<< x << "," << y <<"," << clearChar << ")" << endl;
	if ( clearChar.isSpace() ) {
		// not needed as we called qt for repainting this widget, and autoFillBackground = True
		return;
	} else {
		QRect r;
		r.setTopLeft( translatePositionToReal( YZCursor(x,y) ) );
		r.setRight( width() );
		r.setHeight( fontMetrics().lineSpacing() );
		int nb_char = mParent->getColumnsVisible() - x;
		p->drawText( r, QString(nb_char, clearChar) );
	}
}

void QYZisEdit::drawSetMaxLineNumber( int max ) {
	// XXX do it in an other QWidget
}
void QYZisEdit::drawSetLineNumber( int y, int n, int h, QPainter* p ) {
	// XXX do it in an other QWidget
#if 0
	fakeLine = n <= 0;

	QRect r( 0, y*fontMetrics().lineSpacing(), GETX(marginLeft - spaceWidth), fontMetrics().lineSpacing() );
	p->eraseRect( r );
	if ( h == 0 ) {
		p->save();
		p->setPen( Qt::yellow ); // XXX Setting

		QString num;
		if ( !fakeLine && h == 0 )
			num = QString::number( n );
		num = num.rightJustified( marginLeft - 1, ' ' );
		p->drawText( r, num );

		p->restore();
	}
#endif
}

void QYZisEdit::drawMarginLeft( int min_y, int max_y, QPainter* p ) {
	// XXX do it in an other QWidget
#if 0
	if ( marginLeft > 0 ) {
		min_y *= fontMetrics().lineSpacing();
		max_y *= fontMetrics().lineSpacing();

		int x = GETX( marginLeft - spaceWidth );
		yzDebug() << "::drawMarginLeft bg : " << p->background().color().name() << endl;
		p->eraseRect( x, min_y, GETX(spaceWidth), max_y - min_y );

		x += GETX(spaceWidth) / 2;
		p->drawLine( x, min_y, x, max_y );
	}
#endif
}

void QYZisEdit::initKeys() {
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


	signalMapper = new QSignalMapper( this );
	connect( signalMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( sendMultipleKey( const QString& ) ) );
}

QString QYZisEdit::keysToShortcut( const QString& keys ) {
	QString ret = keys;
	ret = ret.replace( "<CTRL>", "CTRL+" );
	ret = ret.replace( "<SHIFT>", "SHIFT+" );
	ret = ret.replace( "<ALT>", "ALT+" );
	return ret;
}

void QYZisEdit::registerModifierKeys( const QString& keys ) {
	/*
	KAction* k = new KAction( "", KShortcut( keysToShortcut( keys ) ), signalMapper, SLOT( map() ), actionCollection, keys.ascii() );
	signalMapper->setMapping( k, keys );
	*/
}
void QYZisEdit::unregisterModifierKeys( const QString& keys ) {
	/*
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
	*/
}

void QYZisEdit::sendMultipleKey( const QString& keys ) {
	mParent->sendMultipleKey( keys );
}

const QString& QYZisEdit::convertKey( int key ) {
	return keys[ key ];
}

void QYZisEdit::inputMethodEvent ( QInputMethodEvent * ) {
	//TODO
}

QVariant QYZisEdit::inputMethodQuery ( Qt::InputMethodQuery query ) const {
	return QWidget::inputMethodQuery( query );
}

// for InputMethod (OnTheSpot)
/*
void QYZisEdit::imStartEvent( QIMEvent *e )
{
	if ( mParent->modePool()->current()->supportsInputMethod() ) {
		mParent->modePool()->current()->imBegin( mParent );
	}
	e->accept();
}*/

// for InputMethod (OnTheSpot)
/*
void QYZisEdit::imComposeEvent( QIMEvent *e ) {
	//yzDebug() << "QYZisEdit::imComposeEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
	if ( mParent->modePool()->current()->supportsInputMethod() ) {
		mParent->modePool()->current()->imCompose( mParent, e->text() );
		e->accept();
	} else {
		e->ignore();
	}
}*/

// for InputMethod (OnTheSpot)
/*
void QYZisEdit::imEndEvent( QIMEvent *e ) {
//	yzDebug() << "QYZisEdit::imEndEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
	if ( mParent->modePool()->current()->supportsInputMethod() ) {
		mParent->modePool()->current()->imEnd( mParent, e->text() );
	} else {
		mParent->sendKey( e->text() );
	}
	e->accept();
}*/

