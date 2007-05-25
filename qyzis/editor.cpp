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

/* Std */
#include <math.h>
#include <ctype.h>

/* Qt */
#include <QApplication>
#include <QSignalMapper>
#include <QWidget>
#include <qclipboard.h>
#include <qcursor.h>

/* Yzis */
#include "editor.h"
#include "yzis.h"
#include "qsession.h"
#include "registers.h"
#include "viewwidget.h"

/* QYzis */
#include "qyzis.h"

#define dbg() yzDebug("QYZisEdit")
#define err() yzError("QYZisEdit")

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

	mCursor = new QYZisCursor( this, QYZisCursor::CursorFilledRect );

	initKeys();
}


QYZisEdit::~QYZisEdit() {
    dbg() << "~QYZisEdit()" << endl;
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
	return QPoint( c.x() * fontMetrics().maxWidth(), c.y() * fontMetrics().lineSpacing() );
}
YZCursor QYZisEdit::translateRealToPosition( const QPoint& p, bool ceil ) const {
	int height = fontMetrics().lineSpacing();
	int width = fontMetrics().maxWidth();

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

QYZisCursor::CursorShape QYZisEdit::cursorShape() {
	QYZisCursor::CursorShape shape;
	YZMode::ModeType m = mParent->modePool()->current()->type();
    dbg() << "cursorShape(): mode=" << m << endl;
    shape = mCursor->shape();
    if ( ! hasFocus() ) {
        shape = QYZisCursor::CursorFrameRect;
    } else {
        switch( m ) {
        case YZMode::ModeInsert :
            shape = QYZisCursor::CursorVbar;
            break;
        case YZMode::ModeReplace :
            shape = QYZisCursor::CursorHbar;
            break;
		case YZMode::ModeIntro:
        case YZMode::ModeEx:
		case YZMode::ModeSearch:
		case YZMode::ModeSearchBackward:
            shape = QYZisCursor::CursorHidden;
            break;
        case YZMode::ModeCompletion :
            // do not change it
            break;
        case YZMode::ModeCommand:
		case YZMode::ModeVisual:
		case YZMode::ModeVisualLine:
		case YZMode::ModeVisualBlock:
            shape = QYZisCursor::CursorFilledRect;
            break;
        }
    }

    dbg() << "cursorShape(), cursorShape=" << shape << endl;
	return shape;
}

void QYZisEdit::updateCursor() {
	mCursor->setCursorShape( cursorShape() );
	mCursor->update();
}

void QYZisEdit::updateArea( ) {

	updateCursor();

	dbg() << "fixedPitch = " << fontInfo().fixedPitch() << endl;
	dbg() << "lineheight = " << fontMetrics().lineSpacing() << endl;
	dbg() << "maxwidth = " << fontMetrics().maxWidth() << endl;
	dbg() << "height = " << height();

	int lines = height() / fontMetrics().lineSpacing();
	int columns = width() / fontMetrics().maxWidth();

	dbg() << "lines = " << lines;

	m_useArea.setBottomRight( QPoint( columns * fontMetrics().maxWidth(), lines * fontMetrics().lineSpacing()) );

	mParent->setVisibleArea( columns, lines );
}

/**
 * QWidget event handling
 */
bool QYZisEdit::event(QEvent *e) {
	if ( e->type() == QEvent::KeyPress ) {
		QKeyEvent *ke = (QKeyEvent *)e;
        dbg() << "event( KeyEvent( key=" << ke->text() << ", mod=" << ke->modifiers() << ") )" << endl;;
		if ( ke->key() == Qt::Key_Tab ) {
			keyPressEvent(ke);
			return true;
		}
        dbg().sprintf("event: key event transferred to QWidget" );
	}
	return QWidget::event(e);
}

void QYZisEdit::keyPressEvent ( QKeyEvent * e ) {
	dbg() << "keyPressEvent( QKeyEVent(text=\"" << e->text() << "\", key=" << e->key() << ", modifiers=" << e->modifiers() << ")" << endl;
	Qt::KeyboardModifiers st = e->modifiers();
	QString modifiers;
	if ( st & Qt::ShiftModifier ) {
		modifiers = "<SHIFT>";
	}
	if ( st & Qt::AltModifier ) {
		modifiers += "<ALT>";
	}
	if ( st & Qt::ControlModifier ) {
		modifiers += "<CTRL>";
	}
	QString text;
	if ( !keys.contains(e->key()) ) {
		text = e->text();
	} else {
		text = keys[ e->key() ];
	}
	dbg().sprintf("Event transferred to YZSession");
	YZSession::self()->sendKey( static_cast<YZView*>( mParent ), text, modifiers);
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
		if ( mParent->modePool()->currentType() != YZMode::ModeEx ) {
			mParent->gotodxdyAndStick( translateRealToAbsolutePosition( e->pos() ) );
		}
	} else if ( e->button() == Qt::MidButton ) {
		QString text = QApplication::clipboard()->text( QClipboard::Selection );
		if ( text.isNull() )
			text = QApplication::clipboard()->text( QClipboard::Clipboard );
		if ( ! text.isNull() ) {
			if ( mParent->modePool()->current()->isEditMode() ) {
				QChar reg = '\"';
				YZSession::self()->setRegister( reg, text.split("\n") );
				mParent->pasteContent( reg, false );
				mParent->moveRight();
			}
		}
	}
}

void QYZisEdit::mouseMoveEvent( QMouseEvent *e ) {
	if (e->buttons() == Qt::LeftButton) {
		if (mParent->modePool()->currentType() == YZMode::ModeCommand) {
			// start visual mode when user makes a selection with the left mouse button
			mParent->modePool()->push( YZMode::ModeVisual );
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
    dbg() << "focusInEvent()" << endl;
	QYZisSession::self()->setCurrentView( mParent );
	updateCursor();
}
void QYZisEdit::focusOutEvent ( QFocusEvent * ) {
    dbg() << "focusOutEvent()" << endl;
	updateCursor();
}

void QYZisEdit::resizeEvent(QResizeEvent* e) {
	e->accept();
	updateArea();
}

void QYZisEdit::paintEvent( QPaintEvent* pe ) {
    updateCursor();
	// convert QPaintEvent rect to yzis coordinate
	QRect r = pe->rect();
	r.setTopLeft( translateRealToAbsolutePosition( r.topLeft() ) );
	r.setBottomRight( translateRealToAbsolutePosition( r.bottomRight() ) );
	//dbg() << "QYZisEdit::paintEvent : " << pe->rect().topLeft() << "," << pe->rect().bottomRight() << 
	//				" => " << r.topLeft() << "," << r.bottomRight() << endl;
	// paint it
	mParent->paintEvent( mParent->clipSelection( YZSelection( r ) ) );
}

void QYZisEdit::setCursor( int c, int l ) {
//	dbg() << "setCursor" << endl;
	c = c - mParent->getDrawCurrentLeft();
	l -= mParent->getDrawCurrentTop();
	unsigned int x = c * fontMetrics().maxWidth();
	if ( mParent->getLocalBooleanOption( "rightleft" ) ) {
		x = width() - x - mCursor->width();
	}
	mCursor->move( x, l * fontMetrics().lineSpacing() );
	if ( !mCursor->isVisible() )
		mCursor->show();

	// need for InputMethod (OverTheSpot)
//	setMicroFocusHint( mCursor->x(), mCursor->y(), mCursor->width(), mCursor->height() );
}

void QYZisEdit::scroll( int dx, int dy ) {
	int rx = dx * fontMetrics().maxWidth();
	int ry = dy * fontMetrics().lineSpacing();
	mCursor->hide();
	QRect cursorRect = mCursor->rect();
	cursorRect.moveTo( mCursor->pos() );
	update(cursorRect);
	QWidget::scroll( rx, ry, m_useArea );
}

void QYZisEdit::guiDrawCell( int x, int y, const YZDrawCell& cell, QPainter* p ) {
	//dbg() << "QYZisEdit::guiDrawCell(" << x << "," << y <<",'" << cell.c << "')" << endl;
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
	QRect r( x*fontMetrics().maxWidth(), y*fontMetrics().lineSpacing(), cell.c.length()*fontMetrics().maxWidth(), fontMetrics().lineSpacing() );

	//dbg() << "guiDrawCell: r=" << r.topLeft() << "," << r.bottomRight() << " has_bg=" << has_bg << endl;
	//dbg() << "guiDrawCell: fg=" << p->pen().color().name() << endl;
	if ( has_bg )
		p->eraseRect( r ); 
	p->drawText( r, cell.c );
	p->restore();
}

void QYZisEdit::guiDrawClearToEOL( int x, int y, const QChar& clearChar, QPainter* p ) {
	//dbg() << "QYZisEdit::guiDrawClearToEOL("<< x << "," << y <<"," << clearChar << ")" << endl;
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

void QYZisEdit::initKeys() {
	if ( keys.empty() ) {
		/* initialize keys */
		keys[ Qt::Key_A ] = "a";
		keys[ Qt::Key_B ] = "b";
		keys[ Qt::Key_C ] = "c";
		keys[ Qt::Key_D ] = "d";
		keys[ Qt::Key_E ] = "e";
		keys[ Qt::Key_F ] = "f";
		keys[ Qt::Key_G ] = "g";
		keys[ Qt::Key_H ] = "h";
		keys[ Qt::Key_I ] = "i";
		keys[ Qt::Key_J ] = "j";
		keys[ Qt::Key_K ] = "k";
		keys[ Qt::Key_L ] = "l";
		keys[ Qt::Key_M ] = "m";
		keys[ Qt::Key_N ] = "n";
		keys[ Qt::Key_O ] = "o";
		keys[ Qt::Key_P ] = "p";
		keys[ Qt::Key_Q ] = "q";
		keys[ Qt::Key_R ] = "r";
		keys[ Qt::Key_S ] = "s";
		keys[ Qt::Key_T ] = "t";
		keys[ Qt::Key_U ] = "u";
		keys[ Qt::Key_V ] = "v";
		keys[ Qt::Key_W ] = "w";
		keys[ Qt::Key_X ] = "x";
		keys[ Qt::Key_Y ] = "y";
		keys[ Qt::Key_Z ] = "z";
		keys[ Qt::Key_Escape ] = "<ESC>";
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
		keys[ Qt::Key_BracketLeft ] = "[";
		keys[ Qt::Key_BracketRight ] = "]";
	}
	signalMapper = new QSignalMapper( this );
	connect( signalMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( sendMappedKey( const QString& ) ) );
}

QString QYZisEdit::keysToShortcut( const QString& keys ) {
	QString ret = keys;
	ret = ret.replace( "<CTRL>", "CTRL+" );
	ret = ret.replace( "<SHIFT>", "SHIFT+" );
	ret = ret.replace( "<ALT>", "ALT+" );
    dbg().sprintf("keysToShortcut( %s ) --> %s", qp(keys), qp(ret) );
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
		dbg() << "No KAction for " << keys << endl;
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

void QYZisEdit::sendMappedKey( const QString& keys ) {
    dbg().sprintf("sendMappedKey( keys=%s )", qp(keys) );
	YZSession::self()->sendMultipleKeys( static_cast<YZView *>( mParent ), keys );
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
	//dbg() << "QYZisEdit::imComposeEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
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
//	dbg() << "QYZisEdit::imEndEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
	if ( mParent->modePool()->current()->supportsInputMethod() ) {
		mParent->modePool()->current()->imEnd( mParent, e->text() );
	} else {
		YZSession::self()->sendKey( static_cast<YZView *>( mParent ), e->text() );
	}
	e->accept();
}*/


QMap<int,QString> QYZisEdit::keys;

