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
#include <libyzis/buffer.h>

#define GETX( x ) ( ( x ) * fontMetrics().maxWidth() )

KYEditor::KYEditor(KYView* parent)
        : QWidget(parent)
{
    m_parent = parent;
    setFocusPolicy( Qt::StrongFocus );

    setAutoFillBackground( false );
    setAttribute ( Qt::WA_PaintOutsidePaintEvent ); /* XXX */

    /* show an edit cursor */
    QWidget::setCursor( Qt::IBeamCursor );

    // TODO: make this one configurable
    setFont( QFont( "Monospace" ) );

    // for Input Method
    setAttribute( Qt::WA_InputMethodEnabled, true );

    mCursor = new KYCursor( this, KYCursor::SQUARE );

    QTimer::singleShot(0, static_cast<KYSession*>(YSession::self()), SLOT(frontendGuiReady()) );
}

KYEditor::~KYEditor()
{}

void KYEditor::setPalette( const QColor& fg, const QColor& bg, double opacity )
{
    QPalette p = palette();
    p.setColor( QPalette::WindowText, fg );
    p.setColor( QPalette::Window, bg );
    QWidget::setPalette( p );
    setWindowOpacity( opacity );
}

KYCursor::shape KYEditor::cursorShape()
{
    KYCursor::shape s;

    QString shape;
    YMode::ModeType m = m_parent->modePool()->current()->type();
    switch ( m ) {
    case YMode::ModeInsert :
        shape = m_parent->getLocalStringOption("cursorinsert");
        break;
    case YMode::ModeReplace :
        shape = m_parent->getLocalStringOption("cursorreplace");
        break;
    case YMode::ModeCompletion :
        shape = "keep";
        break;
    default :
        shape = m_parent->getLocalStringOption("cursor");
        break;
    }
    if ( shape == "hbar" ) {
        s = KYCursor::HBAR;
    } else if ( shape == "vbar" ) {
        s = KYCursor::VBAR;
    } else if ( shape == "keep" ) {
        s = mCursor->type();
    } else {
        if ( hasFocus() )
            s = KYCursor::SQUARE;
        else
            s = KYCursor::RECT;
    }
    return s;
}

QPoint KYEditor::translatePositionToReal( const YCursor& c ) const
{
    return QPoint( c.x() * fontMetrics().maxWidth(), c.y() * fontMetrics().lineSpacing() );
}

YCursor KYEditor::translateRealToPosition( const QPoint& p, bool ceil ) const
{
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
    return YCursor( x, y );
}

YCursor KYEditor::translateRealToAbsolutePosition( const QPoint& p, bool ceil ) const
{
    return translateRealToPosition( p, ceil ) + m_parent->getScreenPosition();
}

void KYEditor::updateCursor()
{
    mCursor->setCursorType( cursorShape() );
}


void KYEditor::updateArea( )
{
    mCursor->resize( fontMetrics().maxWidth(), fontMetrics().lineSpacing() );
    updateCursor();

    int lines = height() / fontMetrics().lineSpacing();
    int columns = width() / GETX( 1 );
    m_parent->setVisibleArea( columns, lines );
}

/**
 * QWidget event handling
 */
bool KYEditor::event(QEvent *e)
{
    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent *ke = (QKeyEvent *)e;
        if ( ke->key() == Qt::Key_Tab ) {
            keyPressEvent(ke);
            return true;
        }
    }
    return QWidget::event(e);
}

void KYEditor::keyPressEvent ( QKeyEvent * e )
{
    Qt::KeyboardModifiers st = e->modifiers();
    QString modifiers;
    if ( st & Qt::ShiftModifier )
        modifiers = "<SHIFT>";
    if ( st & Qt::AltModifier )
        modifiers += "<ALT>";
    if ( st & Qt::ControlModifier )
        modifiers += "<CTRL>";

    //int lmode = m_parent->modePool()->currentType();
    QString k;
    if ( m_parent->containsKey( e->key() ) ) //to handle some special keys
        k = m_parent->getKey( e->key() );
    else
        k = e->text();

    KYSession::self()->sendKey(m_parent, k, modifiers);

    // TODO: find out what this did! seems to be useless right now
    //if ( lmode == YMode::ModeInsert || lmode == YMode::ModeReplace ) {
    //KYTextEditorIface *d = static_cast<KYTextEditorIface*>(document());
    //emit d->emitChars(mCursor->y(), mCursor->x(),k);
    //}
    e->accept();
}

void KYEditor::mousePressEvent ( QMouseEvent * e )
{
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
        if (m_parent->modePool()->currentType() != YMode::ModeEx) {
            m_parent->gotodxdy( e->x() / ( GETX( 1 ) ) + m_parent->getDrawCurrentLeft( ),
                                e->y() / fontMetrics().lineSpacing() + m_parent->getDrawCurrentTop( ) );
            m_parent->updateStickyCol();
        }
    } else if ( e->button() == Qt::MidButton ) {
        QString text = KApplication::clipboard()->text( QClipboard::Selection );
        // TODO: is this still necessary in kde 4?
        if ( text.isNull() )
            text = QApplication::clipboard()->text( QClipboard::Clipboard );
        if ( ! text.isNull() ) {
            if ( m_parent->modePool()->current()->isEditMode() ) {
                QChar reg = '\"';
                KYSession::self()->setRegister( reg, text.split( "\n" ) );
                m_parent->pasteContent( reg, false );
                m_parent->moveRight();
            }
        }
    }
}

void KYEditor::mouseMoveEvent( QMouseEvent *e )
{
    if (e->button() == Qt::LeftButton) {
        if (m_parent->modePool()->currentType() == YMode::ModeCommand) {
            // start visual mode when user makes a selection with the left mouse button
            m_parent->modePool()->push( YMode::ModeVisual );
        } else if (m_parent->modePool()->current()->isSelMode() ) {
            // already in visual mode - move cursor if the mouse pointer has moved over a new char
            int newX = e->x() / fontMetrics().maxWidth()
                       + m_parent->getDrawCurrentLeft();
            int newY = e->y() / fontMetrics().lineSpacing()
                       + m_parent->getDrawCurrentTop();

            if (newX != m_parent->getCursor().x() || newY != m_parent->getCursor().y()) {
                m_parent->gotodxdy( newX, newY );
            }
        }
    }
}

void KYEditor::focusInEvent ( QFocusEvent * )
{
    KYSession::self()->setCurrentView( m_parent );
    updateCursor();
}
void KYEditor::focusOutEvent ( QFocusEvent * )
{
    updateCursor();
}


void KYEditor::resizeEvent(QResizeEvent* e)
{
    e->accept();
    updateArea();
}

void KYEditor::paintEvent( QPaintEvent* pe )
{
    updateCursor();

    // convert QPaintEvent rect to yzis coordinate
    QRect r = pe->rect();
    r.setTopLeft( translateRealToAbsolutePosition( r.topLeft() ) );
    r.setBottomRight( translateRealToAbsolutePosition( r.bottomRight() ) );
    //dbg() << "QYisEdit::paintEvent : " << pe->rect().topLeft() << "," << pe->rect().bottomRight() <<
    //                              " => " << r.topLeft() << "," << r.bottomRight() << endl;
    // paint it
    m_parent->guiPaintEvent( m_parent->clipSelection( YSelection( r ) ) );
}

void KYEditor::setCursor( int c, int l )
{
    // yzDebug() << "setCursor" << endl;
    c = c - m_parent->getDrawCurrentLeft();
    l -= m_parent->getDrawCurrentTop();
    unsigned int x = GETX( c );
    if ( m_parent->getLocalBooleanOption( "rightleft" ) ) {
        x = width() - x - mCursor->width();
    }
    mCursor->move( x, l * fontMetrics().lineSpacing() );
    updateCursor();

    // need for InputMethod (OverTheSpot)
    // setMicroFocusHint( mCursor->x(), mCursor->y(), mCursor->width(), mCursor->height() );
}

QPoint KYEditor::cursorCoordinates( )
{
    return QPoint( mCursor->x(), mCursor->y() );
}

void KYEditor::scroll( int x, int y )
{
    mCursor->hide();
    QWidget::scroll( x * fontMetrics().maxWidth(), y * fontMetrics().lineSpacing() );
    mCursor->show();
}

void KYEditor::guiDrawCell( QPoint pos, const YDrawCell& cell, QPainter* p )
{
    p->save();
    if ( cell.fg.isValid() ) {
        p->setPen( cell.fg.rgb() );
    }

    QRect r( GETX(pos.x()), pos.y()*fontMetrics().lineSpacing(), cell.c.length()*fontMetrics().maxWidth(), fontMetrics().lineSpacing() );
    p->eraseRect( r );
    p->drawText( r, cell.c );
    p->restore();
}

void KYEditor::guiDrawClearToEOL( QPoint pos, const QChar& /*clearChar*/, QPainter* p )
{
    QRect r( GETX(pos.x()), pos.y()*fontMetrics().lineSpacing(), width(), fontMetrics().lineSpacing() );
    p->eraseRect( r );
}

//void KYEditor::sendMultipleKey( const QString& keys ) {
// sendMultipleKey( keys );
//}

void KYEditor::inputMethodEvent ( QInputMethodEvent * )
{
    //TODO
}

QVariant KYEditor::inputMethodQuery ( Qt::InputMethodQuery query )
{
    return QWidget::inputMethodQuery( query );
}



// for InputMethod (OnTheSpot)
/*
void KYEditor::imStartEvent( QIMEvent *e )
{
 if ( mParent->m_parent->modePool()->current()->supportsInputMethod() ) {
  mParent->m_parent->modePool()->current()->imBegin( mParent );
 }
 e->accept();
}*/

// for InputMethod (OnTheSpot)
/*
void KYEditor::imComposeEvent( QIMEvent *e ) {
 //yzDebug() << "KYEditor::imComposeEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
 if ( mParent->m_parent->modePool()->current()->supportsInputMethod() ) {
  mParent->m_parent->modePool()->current()->imCompose( mParent, e->text() );
  e->accept();
 } else {
  e->ignore();
 }
}*/

// for InputMethod (OnTheSpot)
/*
void KYEditor::imEndEvent( QIMEvent *e ) {
// yzDebug() << "KYEditor::imEndEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
 if ( mParent->m_parent->modePool()->current()->supportsInputMethod() ) {
  mParent->m_parent->modePool()->current()->imEnd( mParent, e->text() );
 } else {
  mParent->sendKey( e->text() );
 }
 e->accept();
}*/



#include "kyziseditor.moc"
