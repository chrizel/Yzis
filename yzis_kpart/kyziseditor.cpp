/* This file is part of the Yzis libraries
 *  Copyright (C) 2007 Lothar Braun <lothar@lobraun.de>
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

#include "kyziseditor.h"
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

KYZisEditor::KYZisEditor(KYZisView* parent)
	: QWidget(parent)
{
	m_parent = parent;
	setFocusPolicy( Qt::StrongFocus );

	setAutoFillBackground( false );
	setAttribute ( Qt::WA_PaintOutsidePaintEvent ); /* XXX */

	/* show an edit cursor */
	QWidget::setCursor( Qt::IBeamCursor );

	isFontFixed = fontInfo().fixedPitch();

	// for Input Method
	setAttribute( Qt::WA_InputMethodEnabled, true );

	mCursor = new KYZisCursor( this, KYZisCursor::SQUARE );

	marginLeft = 0;

	//QTimer::singleShot(0, static_cast<KYZisSession*>(YZSession::self()), SLOT(frontendGuiReady()) );
}

KYZisEditor::~KYZisEditor()
{
}

void KYZisEditor::setPalette( const QColor& fg, const QColor& bg, double opacity ) {
	QPalette p = palette();
	p.setColor( QPalette::WindowText, fg );
	p.setColor( QPalette::Window, bg );
	QWidget::setPalette( p );
	setWindowOpacity( opacity );
}

KYZisCursor::shape KYZisEditor::cursorShape() {
	KYZisCursor::shape s;
	if ( !isFontFixed ) {
		s = KYZisCursor::VBAR;
	} else {
		QString shape;
		YZMode::ModeType m = m_parent->modePool()->current()->type();
		switch( m ) {
			case YZMode::ModeInsert :
				shape = m_parent->getLocalStringOption("cursorinsert");
				break;
			case YZMode::ModeReplace :
				shape = m_parent->getLocalStringOption("cursorreplace");
				break;
			case YZMode::ModeCompletion :
				shape = "keep";
				break;
			default :
				shape = m_parent->getLocalStringOption("cursor");
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
void KYZisEditor::updateCursor() {
	mCursor->setCursorType( cursorShape() );
}


void KYZisEditor::updateArea( ) {

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
	m_parent->setVisibleArea( columns, lines );
}

/**
 * QWidget event handling
 */
bool KYZisEditor::event(QEvent *e) {
	if ( e->type() == QEvent::KeyPress ) {
		QKeyEvent *ke = (QKeyEvent *)e;
		if ( ke->key() == Qt::Key_Tab ) {
			keyPressEvent(ke);
			return true;
		}
	}
	return QWidget::event(e);
}

void KYZisEditor::keyPressEvent ( QKeyEvent * e ) {
	Qt::KeyboardModifiers st = e->modifiers();
	QString modifiers;
	if ( st & Qt::ShiftModifier )
		modifiers = "<SHIFT>";
	if ( st & Qt::AltModifier )
		modifiers += "<ALT>";
	if ( st & Qt::ControlModifier )
		modifiers += "<CTRL>";

	int lmode = m_parent->modePool()->currentType();
	QString k;
	if ( m_parent->containsKey( e->key() ) ) //to handle some special keys
		k = m_parent->getKey( e->key() );
	else
		k = e->text();

	KYZisSession::self()->sendKey(m_parent, k, modifiers);
	if ( lmode == YZMode::ModeInsert || lmode == YZMode::ModeReplace ) {
		//KYZTextEditorIface *d = static_cast<KYZTextEditorIface*>(document());
		//emit d->emitChars(mCursor->y(), mCursor->x(),k);
	}
	e->accept();
}

void KYZisEditor::mousePressEvent ( QMouseEvent * e ) {
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
	if ( m_parent->modePool()->current()->isSelMode() )
		m_parent->modePool()->pop();
	
	if (( e->button() == Qt::LeftButton ) || ( e->button() == Qt::RightButton )) {
		if (m_parent->modePool()->currentType() != YZMode::ModeEx) {
			m_parent->gotodxdy( e->x() / ( isFontFixed ? fontMetrics().maxWidth() : 1 ) + m_parent->getDrawCurrentLeft( ) - marginLeft,
						e->y() / fontMetrics().lineSpacing() + m_parent->getDrawCurrentTop( ) );
			m_parent->updateStickyCol();
		}
	} else if ( e->button() == Qt::MidButton ) {
		QString text = KApplication::clipboard()->text( QClipboard::Selection );
		if ( text.isNull() )
			text = QApplication::clipboard()->text( QClipboard::Clipboard );
		if ( ! text.isNull() ) {
			if ( m_parent->modePool()->current()->isEditMode() ) {
				QChar reg = '\"';
				KYZisSession::self()->setRegister( reg, text.split( "\n" ) );
				m_parent->pasteContent( reg, false );
				m_parent->moveRight();
			}
		}
	}
}

void KYZisEditor::mouseMoveEvent( QMouseEvent *e ) {
	if (e->button() == Qt::LeftButton) {
		if (m_parent->modePool()->currentType() == YZMode::ModeCommand) {
			// start visual mode when user makes a selection with the left mouse button
			m_parent->modePool()->push( YZMode::ModeVisual );
		} else if (m_parent->modePool()->current()->isSelMode() ) {
			// already in visual mode - move cursor if the mouse pointer has moved over a new char
			int newX = e->x() / ( isFontFixed ? fontMetrics().maxWidth() : 1 )
				+ m_parent->getDrawCurrentLeft() - marginLeft;
			int newY = e->y() / fontMetrics().lineSpacing()
				+ m_parent->getDrawCurrentTop();

			if (newX != m_parent->getCursor().x() || newY != m_parent->getCursor().y()) {
				m_parent->gotodxdy( newX, newY );
			}
		}
	}
}

void KYZisEditor::focusInEvent ( QFocusEvent * ) {
	KYZisSession::self()->setCurrentView( m_parent );
	updateCursor();
}
void KYZisEditor::focusOutEvent ( QFocusEvent * ) {
	updateCursor();
}


void KYZisEditor::resizeEvent(QResizeEvent* e) {
	e->accept();
	updateArea();
}

void KYZisEditor::paintEvent( QPaintEvent* pe ) {
	QRect r = pe->rect();
	int fx = r.left();
	int fy = r.top();
	int tx = r.right();
	int ty = r.bottom();
	yzDebug() << "KYZisEditor < QPaintEvent( " << fx << "," << fy << " -> " << tx << "," << ty << " )" << endl;
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
	fy += m_parent->getDrawCurrentTop();
	ty += m_parent->getDrawCurrentTop();

	m_parent->paintEvent( m_parent->clipSelection( YZSelection( r ) ) );
	/*
	if ( fx == (int)m_parent->getDrawCurrentLeft() && tx - fx == (int)(m_parent->getColumnsVisible() + 1) ) {
		m_parent->sendPaintEvent( YZCursor( fx, fy ), YZCursor( tx, ty ) );
	} else {
		m_parent->setPaintAutoCommit( false );
		for( ; fy <= ty; ++fy ) {
			m_parent->sendPaintEvent( YZCursor( fx, fy ), YZCursor( tx, fy ) );
		}
		m_parent->commitPaintEvent();
	}
	*/
	m_insidePaintEvent = false;
	yzDebug() << "KYZisEditor > QPaintEvent" << endl;
}
void KYZisEditor::paintEvent( const YZSelection& drawMap ) {
/*
	yzDebug() << "KYZisEditor::paintEvent" << endl;
	YZSelectionMap m = drawMap.map();
	for( int i = 0; i < m.size(); ++i ) {
		int left = GETX( qMin( m[i].fromPos().x(), m[i].toPos().x() ) );
		int right = GETX( qMax( m[i].fromPos().x(), m[i].toPos().x() ) );
		int top = qMin( m[i].fromPos().y(), m[i].toPos().y() ) * fontMetrics().lineSpacing();
		int bottom = qMax( m[i].fromPos().y(), m[i].toPos().y() ) * fontMetrics().lineSpacing();
		
		update( QRect(left, top, right - left, bottom - top) );
	}
	yzDebug() << "KYZisEditor::paintEvent ends" << endl;
*/
}

void KYZisEditor::setCursor( int c, int l ) {
//	yzDebug() << "setCursor" << endl;
	c = c - m_parent->getDrawCurrentLeft() + marginLeft;
	l -= m_parent->getDrawCurrentTop();
	unsigned int x = GETX( c );
	if ( m_parent->getLocalBooleanOption( "rightleft" ) ) {
		x = width() - x - mCursor->width();
	}
	mCursor->move( x, l * fontMetrics().lineSpacing() );

	// need for InputMethod (OverTheSpot)
//	setMicroFocusHint( mCursor->x(), mCursor->y(), mCursor->width(), mCursor->height() );
}

QPoint KYZisEditor::cursorCoordinates( ) {
	return QPoint( mCursor->x(), mCursor->y() );
}

void KYZisEditor::scrollUp( int n ) {
	mCursor->hide();
	scroll( 0, n * fontMetrics().lineSpacing() );
	mCursor->show();
}
void KYZisEditor::scrollDown( int n ) {
	scrollUp( -n );
}

void KYZisEditor::guiDrawCell( int x, int y, const YZDrawCell& cell, QPainter* p ) {
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

void KYZisEditor::guiDrawClearToEOL( int x, int y, const QChar& clearChar, QPainter* p ) {
	QRect r( GETX(x), y*fontMetrics().lineSpacing(), width(), fontMetrics().lineSpacing() );
	p->eraseRect( r );
}

void KYZisEditor::guiDrawSetMaxLineNumber( int max ) {
	int my_marginLeft = 2 + QString::number( max ).length();
	if ( my_marginLeft != marginLeft ) {
		marginLeft = my_marginLeft;
		updateArea();
	}
}
void KYZisEditor::guiDrawSetLineNumber( int y, int n, int h, QPainter* p ) {
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

void KYZisEditor::drawMarginLeft( int min_y, int max_y, QPainter* p ) {
	if ( marginLeft > 0 ) {
		int x = GETX( marginLeft ) /*- GETX( spaceWidth )*//2;
		p->save();
		// TODO:
		p->setPen( Qt::red );
		p->drawLine( x, min_y*fontMetrics().lineSpacing(), x, max_y*fontMetrics().lineSpacing() );
		p->restore();
	}
}




//void KYZisEditor::sendMultipleKey( const QString& keys ) {
//	sendMultipleKey( keys );
//}

void KYZisEditor::inputMethodEvent ( QInputMethodEvent * ) {
	//TODO
}

QVariant KYZisEditor::inputMethodQuery ( Qt::InputMethodQuery query ) {
	return QWidget::inputMethodQuery( query );
}



// for InputMethod (OnTheSpot)
/*
void KYZisEditor::imStartEvent( QIMEvent *e )
{
	if ( mParent->m_parent->modePool()->current()->supportsInputMethod() ) {
		mParent->m_parent->modePool()->current()->imBegin( mParent );
	}
	e->accept();
}*/

// for InputMethod (OnTheSpot)
/*
void KYZisEditor::imComposeEvent( QIMEvent *e ) {
	//yzDebug() << "KYZisEditor::imComposeEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
	if ( mParent->m_parent->modePool()->current()->supportsInputMethod() ) {
		mParent->m_parent->modePool()->current()->imCompose( mParent, e->text() );
		e->accept();
	} else {
		e->ignore();
	}
}*/

// for InputMethod (OnTheSpot)
/*
void KYZisEditor::imEndEvent( QIMEvent *e ) {
//	yzDebug() << "KYZisEditor::imEndEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
	if ( mParent->m_parent->modePool()->current()->supportsInputMethod() ) {
		mParent->m_parent->modePool()->current()->imEnd( mParent, e->text() );
	} else {
		mParent->sendKey( e->text() );
	}
	e->accept();
}*/



#include "kyziseditor.moc"
