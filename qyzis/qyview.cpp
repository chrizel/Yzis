/* This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
*  Copyright (C) 2005 Erlend Hamberg <hamberg@stud.ntnu.no>
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


/* QYzis */
#include "qyview.h"
#include "qyedit.h"
#include "qycursor.h"
#include "qysession.h"
#include "qyzis.h"
#include "qylinenumbers.h"
#include "qystatusbar.h"

/* Yzis */
#include "portability.h"
#include "yzis.h"
#include "mode_visual.h"
#include "debug.h"
#include "buffer.h"

/* Qt */
#include <QEvent>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QSettings>
#include <qapplication.h>

#define dbg() yzDebug("QYView")
#define err() yzError("QYView")

QYView::QYView ( YBuffer *_buffer, QWidget *, const char *)
        : YView( _buffer, QYSession::self(), 0, 0 ), buffer( _buffer ), m_popup( 0 )

{
    m_editor = new QYEdit( this );
    status = new QYStatusBar(this);
    command = new QYCommandLine (this);
    mVScroll = new QScrollBar( this);
    connect( mVScroll, SIGNAL(sliderMoved(int)), this, SLOT(scrollView(int)) );
    //connect( mVScroll, SIGNAL(prevLine()), this, SLOT(scrollLineUp()) );
    //connect( mVScroll, SIGNAL(nextLine()), this, SLOT(scrollLineDown()) );

    status->setFocusProxy( command );
    status->setFocusPolicy( Qt::ClickFocus );

    m_lineNumbers = new QYLineNumbers(this);

    QHBoxLayout* editorLayout = new QHBoxLayout();
    editorLayout->setMargin(0);
    editorLayout->setSpacing(0);
    editorLayout->addWidget( m_lineNumbers );
    editorLayout->addWidget( m_editor );
    editorLayout->addWidget( mVScroll );

    QVBoxLayout* viewLayout = new QVBoxLayout( this );
    viewLayout->setMargin(0);
    viewLayout->setSpacing(0);
    viewLayout->addLayout( editorLayout );
    viewLayout->addWidget( command );
    viewLayout->addWidget( status );

    // setupActions();
    setupKeys();

    m_editor->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

    QSettings settings;
    applyConfig( settings ); // XXX factory role

    m_editor->show();
    status->show();
    m_editor->setFocus();
    setFocusProxy( m_editor );
    mVScroll->setMaximum( buffer->lineCount() - 1 );

    // setupCodeCompletion();
}

QYView::~QYView ()
{
    dbg() << "~QYView()" << endl;
    // if ( buffer ) buffer->removeView(this);
}

void QYView::guiSetCommandLineText( const QString& text )
{
    command->setText( text );
}

QString QYView::guiGetCommandLineText() const
{
    return command->text();
}

void QYView::guiSetFocusMainWindow()
{
    dbg() << "setFocusMainWindow()" << endl;
    m_editor->setFocus();
}

void QYView::guiSetFocusCommandLine()
{
    dbg() << "setFocusCommandLine()" << endl;
    command->setFocus();
}

void QYView::guiScroll( int dx, int dy )
{
    m_editor->scroll( dx, dy );
    m_lineNumbers->scroll( dy );
}

void QYView::setVisibleArea( int columns, int lines )
{
    m_lineNumbers->setLineCount( lines );
    YView::setVisibleArea( columns, lines );
}

void QYView::refreshScreen()
{
    bool o_number = getLocalBooleanOption("number");
    if ( o_number != m_lineNumbers->isVisible() ) {
        m_lineNumbers->setVisible(o_number);
    }
    YView::refreshScreen();
}

void QYView::guiNotifyContentChanged( const YSelection& s )
{
    // content has changed, ask qt to repaint changed parts

    YSelectionMap m = s.map();
    // convert each interval to QWidget coordinates and update
    for ( int i = 0; i < m.size(); ++i ) {
        YInterval interval = m[i] - getScreenPosition();
        QRect r;
        if ( interval.fromPos().y() == interval.toPos().y() ) {
            r = interval.boundingRect();
            r.setBottom( r.bottom() + 1 );
            r.setRight( r.right() + 1 );
        } else {
            // XXX optimise : split into multiple qrect
            r.setTop( interval.fromPos().y() );
            r.setBottom( interval.toPos().y() + 1 );
            r.setLeft( 0 );
            r.setRight( getColumnsVisible() );
        }
        //  dbg() << "notifiyContentChanged: interval=" << interval.fromPos() << "," << interval.toPos()
        //     << ", r=" << r.topLeft() << "," << r.bottomRight();
        r.setBottomRight( m_editor->translatePositionToReal( r.bottomRight() ) );
        r.setTopLeft( m_editor->translatePositionToReal( r.topLeft() ) );
        //  dbg() << " => " << r.topLeft() << "," << r.bottomRight() << endl;
        m_editor->update( r );
    }
}

void QYView::guiPreparePaintEvent( int /*min_y*/, int /*max_y*/ )
{
    // dbg() << "QYView::guiPreparePaintEvent" << endl;
    m_painter = new QPainter( m_editor );
    m_drawBuffer.setCallbackArgument( m_painter );
    //m_editor->drawMarginLeft( min_y, max_y, m_painter );
}
void QYView::guiEndPaintEvent()
{
    dbg() << "guiEndPaintEvent()" << endl;
    delete m_painter;
}

void QYView::guiPaintEvent( const YSelection& s )
{
    YView::guiPaintEvent( s );
}

/*
 * View painting methods
 */
void QYView::guiDrawCell( QPoint pos, const YDrawCell& cell, void* arg )
{
    m_editor->guiDrawCell( pos, cell, (QPainter*)arg );
}
void QYView::guiDrawClearToEOL( QPoint pos, const QChar& clearChar )
{
    m_editor->guiDrawClearToEOL( pos, clearChar, m_painter );
}
void QYView::guiDrawSetMaxLineNumber( int max )
{
    mVScroll->setMaximum( max );
    m_lineNumbers->setMaxLineNumber( max );
}
void QYView::guiDrawSetLineNumber( int y, int n, int h )
{
    m_lineNumbers->setLineNumber( y, h, n );
}
QChar QYView::currentChar() const
{
    return myBuffer()->textline( viewCursor().bufferY() ).at( viewCursor().bufferX() );
}

void QYView::wheelEvent( QWheelEvent * e )
{
    if ( e->orientation() == Qt::Vertical ) {
        int n = - ( e->delta() * mVScroll->singleStep() ) / 40; // WHEEL_DELTA(120) / 3 XXX
        scrollView( getCurrentTop() + n );
    } else {
        // TODO : scroll horizontally
    }
    e->accept();
}

/*
void QYView::setupActions() {
 KStdAction::save(this, SLOT(fileSave()), actionCollection());
 KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
}
*/

void QYView::registerModifierKeys( const QString& keys )
{
    m_editor->registerModifierKeys( keys );
}
void QYView::unregisterModifierKeys( const QString& keys )
{
    m_editor->unregisterModifierKeys( keys );
}

void QYView::applyConfig( const QSettings& settings, bool refresh )
{

    QFont default_font;
    default_font.setStyleHint(QFont::TypeWriter);
    default_font.setFamily("Courier");
    QFont user_font = settings.value("appearance/font", default_font).value<QFont>();
    // TODO: support non-fixed fonts
    if ( !user_font.fixedPitch() ) {
        user_font = default_font;
    }
    m_editor->setFont( user_font );
    m_lineNumbers->setFont( user_font );

    QPalette default_palette;
    default_palette.setColor( QPalette::Window, Qt::black );
    default_palette.setColor( QPalette::WindowText, Qt::white );
    QPalette my_palette = settings.value("appearance/palette", default_palette).value<QPalette>();
    qreal opacity = settings.value("appearance/opacity", 1.).value<qreal>();
    m_editor->setPalette( my_palette, opacity );

    if ( refresh ) {
        m_editor->updateArea( );
    }
}

void QYView::fileSave()
{
    myBuffer()->save();
}

void QYView::fileSaveAs()
{
    if ( guiPopupFileSaveAs() )
        myBuffer()->save();
}

void QYView::guiUpdateFileName()
{
    if (QYzis::me) {
        //QYzis::me->setCaption(getId(), myBuffer()->fileName());
        QYzis::me->setWindowTitle( myBuffer()->fileName());
    } else
        err() << "guiUpdateFileName() : couldn't find QYzis::me.. is that ok ?";

}

void QYView::guiUpdateCursor()
{
    m_editor->setCursor(viewCursor().screenX(), viewCursor().screenY());
}

void QYView::guiUpdateMode()
{
    m_editor->updateCursor();
}

void QYView::guiHighlightingChanged()
{
    sendRefreshEvent();
}

bool QYView::guiPopupFileSaveAs()
{
    QString url = QFileDialog::getSaveFileName();
    if ( url.isEmpty() ) return false; //canceled

    if ( ! url.isEmpty() ) {
        myBuffer()->setPath( url );
        return true;
    }
    return false;
}

YStatusBarIface* QYView::guiStatusBar()
{
    return status;
}

// scrolls the _view_ on a buffer and moves the cursor it scrolls off the screen
void QYView::scrollView( int value )
{
    if ( value < 0 ) value = 0;
    else if ( value > buffer->lineCount() - 1 )
        value = buffer->lineCount() - 1;

    // only redraw if the view actually moves
    if (value != getCurrentTop()) {
        alignViewBufferVertically( value );

        if (!mVScroll->isSliderDown())
            mVScroll->setValue( value );
    }
}




