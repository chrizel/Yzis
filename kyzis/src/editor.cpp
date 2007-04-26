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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

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
: QWidget( parent )
{
	mParent = parent;

	setFocusPolicy( Qt::StrongFocus );

	setAutoFillBackground( false );
	setAttribute ( Qt::WA_PaintOutsidePaintEvent ); /* XXX */

	/* show an edit cursor */
	QWidget::setCursor( Qt::IBeamCursor );

	isFontFixed = fontInfo().fixedPitch();

	// for Input Method
	setInputMethodEnabled( true );

	mCursor = new KYZisCursor( this, KYZisCursor::SQUARE );

	initKeys();

	marginLeft = 0;
}


KYZisEdit::~KYZisEdit() {
	delete signalMapper;
	for( int i = actionCollection->count() - 1; i>= 0; --i )
		delete actionCollection->take( actionCollection->action(i) );
	delete actionCollection;
}

void KYZisEdit::setPalette( const QColor& fg, const QColor& bg, double opacity ) {
	QPalette p = palette();
	p.setColor( QPalette::WindowText, fg );
	p.setColor( QPalette::Window, bg );
	QWidget::setPalette( p );
	setWindowOpacity( opacity );
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
	mCursor->setCursorType( cursorShape() );
}


void KYZisEdit::updateArea( ) {

	isFontFixed = fontInfo().fixedPitch();
	mParent->setFixedFont( isFontFixed );
	spaceWidth = mParent->getSpaceWidth();
	mCursor->resize( fontMetrics().maxWidth(), fontMetrics().lineSpacing() );
	updateCursor();

	int lines = height() / fontMetrics().lineSpacing();
	// if font is fixed, calculate the number of columns fontMetrics().maxWidth(), else give the width of the widget
	int columns = width() / GETX( 1 ) - marginLeft;
	//erase();
	mParent->setVisibleArea( columns, lines );
}

/**
 * QWidget event handling
 */
bool KYZisEdit::event(QEvent *e) {
	if ( e->type() == QEvent::KeyPress ) {
		QKeyEvent *ke = (QKeyEvent *)e;
		if ( ke->key() == Qt::Key_Tab ) {
			keyPressEvent(ke);
			return true;
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

void KYZisEdit::focusInEvent ( QFocusEvent * ) {
	KYZisFactory::self()->setCurrentView( mParent );
	updateCursor();
}
void KYZisEdit::focusOutEvent ( QFocusEvent * ) {
	updateCursor();
}


void KYZisEdit::resizeEvent(QResizeEvent* e) {
	e->accept();
	updateArea();
}
void KYZisEdit::paintEvent( QPaintEvent* pe ) {
	QRect r = pe->rect();
	int fx = r.left();
	int fy = r.top();
	int tx = r.right();
	int ty = r.bottom();
	yzDebug() << "KYzisEdit < QPaintEvent( " << fx << "," << fy << " -> " << tx << "," << ty << " )" << endl;
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
	m_insidePaintEvent = false;
	yzDebug() << "KYzisEdit > QPaintEvent" << endl;
}
void KYZisEdit::paintEvent( const YZSelection& drawMap ) {
	yzDebug() << "KYZisEdit::paintEvent" << endl;
	YZSelectionMap m = drawMap.map();
	for( int i = 0; i < m.size(); ++i ) {
		int left = GETX( qMin( m[i].fromPos().x(), m[i].toPos().x() ) );
		int right = GETX( qMax( m[i].fromPos().x(), m[i].toPos().x() ) );
		int top = qMin( m[i].fromPos().y(), m[i].toPos().y() ) * fontMetrics().lineSpacing();
		int bottom = qMax( m[i].fromPos().y(), m[i].toPos().y() ) * fontMetrics().lineSpacing();
		
		update( QRect(left, top, right - left, bottom - top) );
	}
	yzDebug() << "KYZisEdit::paintEvent ends" << endl;
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
	return QPoint( mCursor->x(), mCursor->y() );
}

void KYZisEdit::scrollUp( int n ) {
	mCursor->hide();
	scroll( 0, n * fontMetrics().lineSpacing() );
	mCursor->show();
}
void KYZisEdit::scrollDown( int n ) {
	scrollUp( -n );
}

void KYZisEdit::guiDrawCell( int x, int y, const YZDrawCell& cell, QPainter* p ) {
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

void KYZisEdit::guiDrawClearToEOL( int x, int y, const QChar& clearChar, QPainter* p ) {
	QRect r( GETX(x), y*fontMetrics().lineSpacing(), width(), fontMetrics().lineSpacing() );
	p->eraseRect( r );
}

void KYZisEdit::guiDrawSetMaxLineNumber( int max ) {
	int my_marginLeft = 2 + QString::number( max ).length();
	if ( my_marginLeft != marginLeft ) {
		marginLeft = my_marginLeft;
		updateArea();
	}
}
void KYZisEdit::guiDrawSetLineNumber( int y, int n, int h, QPainter* p ) {
	fakeLine = n <= 0;

	QString num;
	if ( !fakeLine && h == 0 )
		num = QString::number( n );
	num = num.rightJustified( marginLeft - 1, ' ' );

	p->save();
	p->setPen( Qt::yellow );

	QRect r( 0, y*fontMetrics().lineSpacing(), GETX(marginLeft - spaceWidth), fontMetrics().lineSpacing() );
	p->eraseRect( r );
	p->drawText( r, num );

	p->restore();
}

void KYZisEdit::drawMarginLeft( int min_y, int max_y, QPainter* p ) {
	if ( marginLeft > 0 ) {
		int x = GETX( marginLeft ) - GETX( spaceWidth )/2;
		p->save();
		p->setPen( Settings::colorFG() );
		p->drawLine( x, min_y*fontMetrics().lineSpacing(), x, max_y*fontMetrics().lineSpacing() );
		p->restore();
	}
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


	actionCollection = new KActionCollection( this );
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
#warning port to KDE4 API
#if 0
	KAccel* accel = actionCollection->kaccel();
	if ( accel ) {
		accel->remove( keys );
		accel->updateConnections();
	}
#endif
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
