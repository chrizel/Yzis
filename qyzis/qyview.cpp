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

QYView::QYView ( YBuffer * buffer, QYSession * qysession)
        : QWidget( ),
          YView(  buffer, qysession, 0, 0 ),
          mSession( qysession )
{
    mEdit = new QYEdit( this );
    mStatusBar = new QYStatusBar(this);
    mCommandLine = new QYCommandLine (this);
    mVScroll = new QScrollBar( this);
    connect( mVScroll, SIGNAL(sliderMoved(int)), this, SLOT(scrollView(int)) );
    //connect( mVScroll, SIGNAL(prevLine()), this, SLOT(scrollLineUp()) );
    //connect( mVScroll, SIGNAL(nextLine()), this, SLOT(scrollLineDown()) );

    mStatusBar->setFocusProxy( mCommandLine );
    mStatusBar->setFocusPolicy( Qt::ClickFocus );

    mLineNumbers = new QYLineNumbers(this);

    QHBoxLayout* editorLayout = new QHBoxLayout();
    editorLayout->setMargin(0);
    editorLayout->setSpacing(0);
    editorLayout->addWidget( mLineNumbers );
    editorLayout->addWidget( mEdit );
    editorLayout->addWidget( mVScroll );

    QVBoxLayout* viewLayout = new QVBoxLayout( this );
    viewLayout->setMargin(0);
    viewLayout->setSpacing(0);
    viewLayout->addLayout( editorLayout );
    viewLayout->addWidget( mCommandLine );
    viewLayout->addWidget( mStatusBar );

    setupKeys();

    mEdit->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

    QSettings settings;
    applyConfig( settings ); // XXX factory role

    mEdit->show();
    mStatusBar->show();
    mEdit->setFocus();
    setFocusProxy( mEdit );
    mVScroll->setMaximum( myBuffer()->lineCount() - 1 );
	refreshScreen();

}

QYView::~QYView ()
{
    dbg() << "~QYView() done" << endl;
}

void QYView::guiSetCommandLineText( const QString& text )
{
    mCommandLine->setText( text );
}

QString QYView::guiGetCommandLineText() const
{
    return mCommandLine->text();
}

void QYView::focusInEvent( QFocusEvent * e )
{
    dbg() << "focusInEvent() for " << myBuffer()->fileNameShort() << endl;
}

void QYView::resizeEvent( QResizeEvent * e )
{
    dbg() << "resizeEvent() for " << myBuffer()->fileNameShort() << endl;
}

void QYView::guiSetFocusMainWindow()
{
    dbg() << "setFocusMainWindow() for " << myBuffer()->fileNameShort() << endl;
    mEdit->setFocus();
    mCommandLine->setEnabled( false );
}

void QYView::guiSetFocusCommandLine()
{
    dbg() << "setFocusCommandLine()" << endl;
    mCommandLine->setFocus();
    mCommandLine->setEnabled( true );
}

void QYView::guiScroll( int dx, int dy )
{
    mEdit->scroll( dx, dy );
    mLineNumbers->scroll( dy );
    // TODO scroll QScrollBar
}

void QYView::setVisibleArea( int columns, int lines )
{
    mLineNumbers->setLineCount( lines );
    YView::setVisibleArea( columns, lines );
}

void QYView::refreshScreen()
{
    bool o_number = getLocalBooleanOption("number");
    if ( o_number != mLineNumbers->isVisible() ) {
        mLineNumbers->setVisible(o_number);
    }
    YView::refreshScreen();
}

void QYView::guiNotifyContentChanged( const YSelection& s )
{
    dbg() << "guiNotifyContentChanged()" << endl;
    // content has changed, ask qt to repaint changed parts

    YSelectionMap m = s.map();
    // convert each interval to QWidget coordinates and update
    for ( int i = 0; i < m.size(); ++i ) {
        YInterval interval = m[i];
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
        r.setBottomRight( mEdit->translatePositionToReal( r.bottomRight() ) );
        r.setTopLeft( mEdit->translatePositionToReal( r.topLeft() ) );
        //  dbg() << " => " << r.topLeft() << "," << r.bottomRight() << endl;
        mEdit->update( r );
    }
}

void QYView::guiPreparePaintEvent()
{
    mPainter = new QPainter( mEdit );
}
void QYView::guiEndPaintEvent()
{
    delete mPainter;
    mPainter = NULL;
}

void QYView::guiPaintEvent( const YSelection& s )
{
    YView::guiPaintEvent( s );
}

/*
 * View painting methods
 */
void QYView::guiDrawCell( YCursor pos, const YDrawCell& cell )
{
    mEdit->guiDrawCell( pos, cell, mPainter );
}
void QYView::guiDrawClearToEOL( YCursor pos, const YDrawCell& clearCell )
{
    mEdit->guiDrawClearToEOL( pos, clearCell, mPainter );
}
void QYView::guiDrawSetMaxLineNumber( int max )
{
    mVScroll->setMaximum( max );
    mLineNumbers->setMaxLineNumber( max );
}
void QYView::guiDrawSetLineNumber( int y, int n, int h )
{
    mLineNumbers->setLineNumber( y, h, n );
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

void QYView::registerModifierKeys( const QString& keys )
{
    mEdit->registerModifierKeys( keys );
}
void QYView::unregisterModifierKeys( const QString& keys )
{
    mEdit->unregisterModifierKeys( keys );
}

void QYView::applyConfig( const QSettings& settings, bool refresh )
{

    QFont default_font;
    default_font.setStyleHint(QFont::TypeWriter);
    default_font.setFamily("Courier");
    QFont user_font = settings.value("appearance/font", default_font).value<QFont>();
    if ( !user_font.fixedPitch() ) {
        user_font = default_font;
    }
    mEdit->setFont( user_font );
    mLineNumbers->setFont( user_font );

    QPalette default_palette;
    default_palette.setColor( QPalette::Window, Qt::black );
    default_palette.setColor( QPalette::WindowText, Qt::white );
    QPalette my_palette = settings.value("appearance/palette", default_palette).value<QPalette>();
    mEdit->setPalette( my_palette );

    if ( refresh ) {
        mEdit->updateArea( );
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
    mSession->viewFilenameChanged( this, myBuffer()->fileNameShort() );
}

void QYView::guiUpdateCursor()
{
    mEdit->setCursor(viewCursor().screenX(), viewCursor().screenY());
}

void QYView::guiUpdateMode()
{
    mEdit->updateCursor();
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
    return mStatusBar;
}

// scrolls the _view_ on a buffer and moves the cursor it scrolls off the screen
void QYView::scrollView( int value )
{
    if ( value < 0 ) value = 0;
    else if ( value > myBuffer()->lineCount() - 1 )
        value = myBuffer()->lineCount() - 1;

    // only redraw if the view actually moves
    if (value != getCurrentTop()) {
        alignViewBufferVertically( value );

        if (!mVScroll->isSliderDown())
            mVScroll->setValue( value );
    }
}


