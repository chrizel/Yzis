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

/* System */
#include <math.h>
#include <ctype.h>
#include <iostream>
using namespace std;

/* Qt */
#include <QApplication>
#include <QSignalMapper>
#include <QWidget>
#include <qclipboard.h>
#include <qcursor.h>

/* Yzis */
#include "yzis.h"
#include "registers.h"
#include "buffer.h"

/* QYzis */
#include "qyedit.h"
#include "qyview.h"
#include "qysession.h"


#define deepdbg() yzDeepDebug("QYEdit")
#define dbg() yzDebug("QYEdit")
#define err() yzError("QYEdit")

QYEdit::QYEdit(QYView * view )
        : QWidget( view )
        , signalMapper(this)
		, mCursor(view, this, QYCursor::CursorFilledRect)
{
    mView = view;

    mUseArea.setCoords(0, 0, 0, 0);

    setFocusPolicy( Qt::StrongFocus );

    setAutoFillBackground( true );

    // for Input Method
    setAttribute(Qt::WA_InputMethodEnabled, true);

    /* show an edit cursor */
    QWidget::setCursor( Qt::IBeamCursor );


    connect( &signalMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( sendMappedKey( const QString& ) ) );
}


QYEdit::~QYEdit()
{
    dbg() << "~QYEdit()" << endl;
    dbg() << "~QYEdit() done" << endl;
}

QYView* QYEdit::view() const
{
    return mView;
}

QPoint QYEdit::translatePositionToReal( const YCursor& c ) const
{
    return QPoint( c.x() * fontMetrics().maxWidth(), c.y() * fontMetrics().lineSpacing() );
}
YCursor QYEdit::translateRealToPosition( const QPoint& p, bool ceil ) const
{
    int height = fontMetrics().lineSpacing();
    int width = fontMetrics().maxWidth();

	int px = qMax(p.x(), 0);
	int py = qMax(p.y(), 0);

    int x = px / width;
    int y = py / height;
    if ( ceil ) {
        if ( py % height )
            ++y;
        if ( px % width )
            ++x;
    }
    return YCursor(x, y);
}
YCursor QYEdit::translateRealToAbsolutePosition( const QPoint& p, bool ceil ) const
{
    return translateRealToPosition( p, ceil );
}

QYCursor::CursorShape QYEdit::cursorShape()
{
    QYCursor::CursorShape shape;
    YMode::ModeType m = mView->modePool()->currentType();
    deepdbg() << "cursorShape(): mode=" << m << endl;
    shape = mCursor.shape();
    if ( ! hasFocus() ) {
        if (mView->mCommandLine->hasFocus()) {
            // command line has focus
            shape = QYCursor::CursorHidden;
        } else {
            // the widget no longer has focus
            shape = QYCursor::CursorFrameRect;
        }
    } else {
        switch ( m ) {
        case YMode::ModeInsert :
            shape = QYCursor::CursorVbar;
            break;
        case YMode::ModeReplace :
            shape = QYCursor::CursorHbar;
            break;
        case YMode::ModeIntro:
        case YMode::ModeEx:
        case YMode::ModeSearch:
        case YMode::ModeSearchBackward:
            shape = QYCursor::CursorHidden;
            break;
        case YMode::ModeCompletion :
            // do not change it
            break;
        case YMode::ModeCommand:
        case YMode::ModeVisual:
        case YMode::ModeVisualLine:
        case YMode::ModeVisualBlock:
            shape = QYCursor::CursorFilledRect;
            break;
        }
    }

    deepdbg() << "cursorShape(), cursorShape=" << shape << endl;
    return shape;
}

void QYEdit::updateCursor()
{
    mCursor.setCursorShape( cursorShape() );
    mCursor.update();
}

void QYEdit::modeChanged()
{
    updateCursor();
}

void QYEdit::updateArea( )
{
    dbg() << "updatearea()" << endl;
    updateCursor();

    dbg() << "updateArea(): fixedPitch = " << fontInfo().fixedPitch() << endl;
    dbg() << "updateArea(): lineheight = " << fontMetrics().lineSpacing() << endl;
    dbg() << "updateArea(): maxwidth = " << fontMetrics().maxWidth() << endl;
    dbg() << "updateArea(): height = " << height();

    int lines = height() / fontMetrics().lineSpacing();
    int columns = width() / fontMetrics().maxWidth();

    dbg().SPrintf("updateArea(): lines,col = %d,%d", lines, columns );

    mUseArea.setBottomRight( QPoint( columns * fontMetrics().maxWidth(), lines * fontMetrics().lineSpacing()) );

    mView->setVisibleArea( columns, lines );
}

/**
 * QWidget event handling
 */
bool QYEdit::event(QEvent *e)
{
    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent *ke = (QKeyEvent *)e;
        dbg() << "event( KeyEvent( key=" << ke->text() << ", mod=" << ke->modifiers() << ") )" << endl;;
        if ( ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab ) {
            keyPressEvent(ke);
            return true;
        }
        dbg().SPrintf("event: key event transferred to QWidget" );
    }
    return QWidget::event(e);
}

#define ALPHABET(k) ( (k) >= Qt::Key_A && (k) <= Qt::Key_Z)

void QYEdit::keyPressEvent ( QKeyEvent * e )
{
    dbg() << "keyPressEvent( QKeyEVent(text=\"" << e->text() << "\", key=" << e->key() << ", modifiers=" << e->modifiers() << ")" << endl;
    YKey key(Qt::Key_unknown, e->modifiers());
//    if ( !keys.contains(e->key()) ) {

    // some checks
	/*
	if ( e->text().isEmpty() ) {
			dbg() << "keyPressEvent: empty key text (key=" << e->key() << ")" << endl;
			e->ignore();
            return;
    }*/
    // TODO: comprehensive solution
#if 1
    if ( ALPHABET(e->key()) && !(e->modifiers() & Qt::ShiftModifier)) {
        key.setKey( QChar(e->key()).toLower() );
    }
    else
        key = YKey((Qt::Key)e->key(),e->modifiers());
#else
    key = YKey(e->key(),e->modifiers(), e->text());
#endif
    dbg().SPrintf("Event transferred to YSession");
    YSession::self()->sendKey( static_cast<YView*>( mView ), key);
    e->accept();
}

void QYEdit::mousePressEvent ( QMouseEvent * e )
{
    /*
    FIXME: How to handle mouse events commented out now so kyzis will compile

    if ( mView->buffer()->introShown() ) {
     mView->buffer()->clearIntro();
     mView->gotodxdy( 0, 0 );
     return;
    }
    */

    // leave visual mode if the user clicks somewhere
    // TODO: this should only be done if the left button is used. Right button
    // should extend visual selection, like in vim.
    if ( mView->modePool()->current()->isSelMode() )
        mView->modePool()->pop();

    if (( e->button() == Qt::LeftButton ) || ( e->button() == Qt::RightButton )) {
        if ( mView->modePool()->currentType() != YMode::ModeEx ) {
			YCursor screenPosition = translateRealToAbsolutePosition(e->pos());
			YViewCursor dest = mView->viewCursorFromRowColumn(screenPosition.line(), screenPosition.column());
			mView->gotoViewCursor(dest);
			mView->stickToColumn();
        }
    } else if ( e->button() == Qt::MidButton ) {
        QString text = QApplication::clipboard()->text( QClipboard::Selection );
        if ( text.isNull() )
            text = QApplication::clipboard()->text( QClipboard::Clipboard );
        if ( ! text.isNull() ) {
            if ( mView->modePool()->current()->isEditMode() ) {
                QChar reg = '\"';
                YSession::self()->setRegister( reg, text.split("\n") );
                                mView->buffer()->action()->pasteContent( mView, reg, false);
                //mView->pasteContent( reg, false );
				mView->gotoViewCursor(mView->viewCursorMoveHorizontal(1));
            }
        }
    }
}

void QYEdit::mouseMoveEvent( QMouseEvent *e )
{
    if (e->buttons() == Qt::LeftButton) {
        if (mView->modePool()->currentType() == YMode::ModeCommand) {
            // start visual mode when user makes a selection with the left mouse button
            mView->modePool()->push( YMode::ModeVisual );
        } else if (mView->modePool()->current()->isSelMode() ) {
            // already in visual mode - move cursor if the mouse pointer has moved over a new char
            YCursor pos = translateRealToAbsolutePosition( e->pos() );
            if ( pos != mView->getRowColumnCursor() ) {
                mView->gotoRowColumn( pos );
            }
        }
    }
}

void QYEdit::focusInEvent ( QFocusEvent * )
{
    dbg() << "focusInEvent() for " << mView->buffer()->fileNameShort() << endl;
    YSession::self()->setCurrentView( mView );
    updateCursor();
}
void QYEdit::focusOutEvent ( QFocusEvent * )
{
    dbg() << "focusOutEvent() for " << mView->buffer()->fileNameShort() << endl;
    updateCursor();
}

void QYEdit::resizeEvent(QResizeEvent* e)
{
    dbg() << "resizeEvent(" << *e << ") - filename=" << mView->buffer()->fileNameShort() << endl;
    updateArea();
}

void QYEdit::paintEvent( QPaintEvent* pe )
{
    updateCursor();
    // convert QPaintEvent rect to yzis coordinate
    QRect r = pe->rect();
    r.setTopLeft( translateRealToAbsolutePosition( r.topLeft() ) );
    r.setBottomRight( translateRealToAbsolutePosition( r.bottomRight() ) );
    //dbg() << "QYEdit::paintEvent : " << pe->rect().topLeft() << "," << pe->rect().bottomRight() <<
    //    " => " << r.topLeft() << "," << r.bottomRight() << endl;
    // paint it
    mView->guiPaintEvent( mView->clipSelection( YSelection( r ) ) );
}

void QYEdit::setCursor( int c, int l )
{
    // dbg() << "setCursor" << endl;
    unsigned int x = c * fontMetrics().maxWidth();
    if ( mView->getLocalBooleanOption( "rightleft" ) ) {
        x = width() - x - mCursor.width();
    }
    mCursor.move( x, l * fontMetrics().lineSpacing() );
    if ( !mCursor.isVisible() )
        mCursor.show();

    // need for InputMethod (OverTheSpot)
    // setMicroFocusHint( mCursor.x(), mCursor.y(), mCursor.width(), mCursor.height() );
}

void QYEdit::scroll( int dx, int dy )
{
    int rx = dx * fontMetrics().maxWidth();
    int ry = dy * fontMetrics().lineSpacing();
    mCursor.hide();
    QRect cursorRect = mCursor.rect();
    cursorRect.moveTo( mCursor.pos() );
    update(cursorRect);
    QWidget::scroll( rx, ry, mUseArea );
}

void QYEdit::guiDrawCell( YCursor pos , const YDrawCell& cell, QPainter* p )
{
    //dbg() << "QYEdit::guiDrawCell(" << x << "," << y <<",'" << cell.content() << "')" << endl;
    p->save();
    bool has_bg = false;
	if ( cell.hasSelection(yzis::SelectionVisual) ) {
        has_bg = true;
        p->setBackground( QColor(181, 24, 181) ); //XXX setting
        p->setPen( Qt::white );
	} else if ( cell.hasSelection(yzis::SelectionAny) ) {
        has_bg = true;
        p->setBackground( cell.foregroundColor().isValid() ? QColor(cell.foregroundColor().rgb()) : palette().color( QPalette::WindowText ) );
        p->setPen( cell.backgroundColor().isValid() ? QColor(cell.backgroundColor().rgb()) : palette().color( QPalette::Window ) );
	} else {
        if ( cell.foregroundColor().isValid() )
            p->setPen( cell.foregroundColor().rgb() );
        if ( cell.backgroundColor().isValid() ) {
            has_bg = true;
            p->setBackground( QColor(cell.backgroundColor().rgb()) );
        }
    }
    QRect r( pos.x()*fontMetrics().maxWidth(), pos.y()*fontMetrics().lineSpacing(), cell.content().length()*fontMetrics().maxWidth(), fontMetrics().lineSpacing() );

    //dbg() << "guiDrawCell: r=" << r.topLeft() << "," << r.bottomRight() << " has_bg=" << has_bg << endl;
    //dbg() << "guiDrawCell: fg=" << p->pen().color().name() << endl;
    if ( has_bg )
        p->eraseRect( r );
    p->drawText( r, cell.content() );
    p->restore();
}

void QYEdit::guiDrawClearToEOL( YCursor pos , const YDrawCell& clearCell, QPainter* p )
{
	/* TODO 
    //dbg() << "QYEdit::guiDrawClearToEOL("<< x << "," << y <<"," << clearChar << ")" << endl;
    if ( clearCell.c.isSpace() ) {
        // not needed as we called qt for repainting this widget, and autoFillBackground = True
        return ;
    } else {
        QRect r;
        r.setTopLeft( translatePositionToReal( YCursor(pos) ) );
        r.setRight( width() );
        r.setHeight( fontMetrics().lineSpacing() );
        int nb_char = mView->getColumnsVisible() - pos.x();
        p->drawText( r, QString(nb_char, clearChar) );
    }
	*/
}

QString QYEdit::keysToShortcut( const QString& keys )
{
    QString ret = keys;
    ret = ret.replace( "<CTRL>", "CTRL+" );
    ret = ret.replace( "<SHIFT>", "SHIFT+" );
    ret = ret.replace( "<ALT>", "ALT+" );
    dbg().SPrintf("keysToShortcut( %s ) --> %s", qp(keys), qp(ret) );
    return ret;
}

void QYEdit::registerModifierKeys( const QString& keys )
{
    Q_UNUSED(keys);
    /*
    KAction* k = new KAction( "", KShortcut( keysToShortcut( keys ) ), &signalMapper, SLOT( map() ), actionCollection, keys.ascii() );
    signalMapper.setMapping( k, keys );
    */
}
void QYEdit::unregisterModifierKeys( const QString& keys )
{
    Q_UNUSED(keys);
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
    signalMapper.removeMappings( k );
    delete k;
    */
}

void QYEdit::sendMappedKey( const QString & keys )
{
    dbg().SPrintf("sendMappedKey( keys=%s )", qp(keys) );
    YKeySequence inputs(keys);
    YSession::self()->sendMultipleKeys( static_cast<YView *>( mView ), inputs);
}

void QYEdit::inputMethodEvent ( QInputMethodEvent * )
{
    //TODO
}

QVariant QYEdit::inputMethodQuery ( Qt::InputMethodQuery query ) const
{
    return QWidget::inputMethodQuery( query );
}

// for InputMethod (OnTheSpot)
/*
void QYEdit::imStartEvent( QIMEvent *e )
{
 if ( mView->modePool()->current()->supportsInputMethod() ) {
  mView->modePool()->current()->imBegin( mView );
 }
 e->accept();
}*/

// for InputMethod (OnTheSpot)
/*
void QYEdit::imComposeEvent( QIMEvent *e ) {
 //dbg() << "QYEdit::imComposeEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
 if ( mView->modePool()->current()->supportsInputMethod() ) {
  mView->modePool()->current()->imCompose( mView, e->text() );
  e->accept();
 } else {
  e->ignore();
 }
}*/

// for InputMethod (OnTheSpot)
/*
void QYEdit::imEndEvent( QIMEvent *e ) {
// dbg() << "QYEdit::imEndEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
 if ( mView->modePool()->current()->supportsInputMethod() ) {
  mView->modePool()->current()->imEnd( mView, e->text() );
 } else {
  YSession::self()->sendKey( static_cast<YView *>( mView ), e->text() );
 }
 e->accept();
}*/

#include "qyedit.moc"

