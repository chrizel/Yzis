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

#include "kyview.h"
#include "kysession.h"
#include "kyeditor.h"
#include "kycommand.h"
#include "kyinfobar.h"
#include "kteview.h"

#include <QSignalMapper>
#include <QGridLayout>
#include <QPainter>
#include <QScrollBar>

#include <kactioncollection.h>
#include <kaction.h>

#include <libyzis/view.h>
#include <libyzis/buffer.h>
#include <libyzis/debug.h>
#include <libyzis/keys.h>

KYView::KYView(YBuffer* buffer, QWidget* parent)
        : QWidget(parent), YView(buffer, KYSession::self(), 0, 0),
        actionCollection(0), signalMapper(0), m_painter(0)
{
    m_editor = new KYEditor( this );
    m_editor->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    // TODO: remove this, as soon as we can configure the edior component
    m_editor->setPalette( Qt::white, Qt::black, 0 );

    m_command = new KYCommand( this );

    mVScroll = new QScrollBar( this );
    connect( mVScroll, SIGNAL( sliderMoved(int) ), this, SLOT( scrollView( int ) ) );
    connect( mVScroll, SIGNAL( prevLine() ), this, SLOT( scrollLineUp() ) );
    connect( mVScroll, SIGNAL( nextLine() ), this, SLOT( scrollLineDown() ) );
    mVScroll->setMaximum( buffer->lineCount() - 1 );

    m_infoBar = new KYInfoBar( this );

    QGridLayout* g = new QGridLayout( this );
    g->setMargin( 0 );
    g->setSpacing( 0 );
    g->addWidget( m_editor, 0, 0 );
    g->addWidget( mVScroll, 0, 1 );
    g->addWidget( m_command, 1, 0 );
    g->addWidget( m_infoBar, 2, 0 );


    initKeys();

    m_editor->setFocus();
    setFocusProxy( m_editor );
}

KYView::~KYView()
{
    delete signalMapper;
    for ( int i = actionCollection->count() - 1; i >= 0; --i )
        delete actionCollection->takeAction( actionCollection->action(i) );
    delete actionCollection;
}

void KYView::setFocusMainWindow()
{
    m_editor->setFocus();
}

void KYView::setFocusCommandLine()
{
    m_command->setFocus();
}

void KYView::guiScroll(int dx, int dy)
{
    m_editor->scroll( dx, dy );
}

QString KYView::guiGetCommandLineText() const
{
    return m_command->text();
}

void KYView::guiSetCommandLineText( const QString& text )
{
    m_command->setText( text );
}

YStatusBarIface* KYView::guiStatusBar()
{
    return m_infoBar;
}

void KYView::guiUpdateCursor()
{
    m_editor->setCursor( viewCursor().screenX(), viewCursor().screenY() );
}

void KYView::guiUpdateMode()
{
    m_editor->updateCursor();
}

bool KYView::guiPopupFileSaveAs()
{
    return false;
}

void KYView::guiUpdateFileName()
{
}

void KYView::guiHighlightingChanged()
{
}

void KYView::guiNotifyContentChanged(const YSelection& s)
{
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
        //              dbg() << "notifiyContentChanged: interval=" << interval.fromPos() << "," << interval.toPos()
        //                                      << ", r=" << r.topLeft() << "," << r.bottomRight();
        r.setBottomRight( m_editor->translatePositionToReal( r.bottomRight() ) );
        r.setTopLeft( m_editor->translatePositionToReal( r.topLeft() ) );
        //              dbg() << " => " << r.topLeft() << "," << r.bottomRight() << endl;
        m_editor->update( r );
    }
}

void KYView::guiPreparePaintEvent(int /*min_y*/, int /*max_y*/)
{
    yzDebug() << "KYView::guiPreparePaintEvent" << endl;
    m_painter = new QPainter( m_editor );
    m_drawBuffer.setCallbackArgument( m_painter );
}

void KYView::guiPaintEvent( const YSelection& drawMap )
{
    YView::guiPaintEvent( drawMap );
}

void KYView::guiEndPaintEvent()
{
    delete m_painter;
    yzDebug() << "KYView::endPaintEvent" << endl;
}

void KYView::guiDrawCell(QPoint pos, const YDrawCell& cell, void* arg)
{
    m_editor->guiDrawCell( pos, cell, (QPainter*)arg );
}

void KYView::guiDrawClearToEOL(QPoint pos, const QChar& clearChar)
{
    m_editor->guiDrawClearToEOL( pos, clearChar, m_painter );
}

void KYView::guiDrawSetLineNumber(int, int, int)
{
}

void KYView::guiDrawSetMaxLineNumber( int /*max*/ )
{}

const YKey& KYView::convertKey( int key )
{
    return keys[ key ];
}

void KYView::initKeys()
{
    if ( keys.empty() ) { 
        /* initialize keys */ 
        keys[ Qt::Key_Escape ] = YKey::Key_Esc;
        keys[ Qt::Key_Tab ] = YKey::Key_Tab ;
        keys[ Qt::Key_Backtab ] = YKey::Key_BTab ;
        keys[ Qt::Key_Backspace ] = YKey::Key_BackSpace ;
        keys[ Qt::Key_Return ] = YKey::Key_Enter ;
        keys[ Qt::Key_Enter ] = YKey::Key_Enter ;
        keys[ Qt::Key_Insert ] = YKey::Key_Insert ;
        keys[ Qt::Key_Delete ] = YKey::Key_Delete ;
        keys[ Qt::Key_Pause ] = YKey::Key_Pause ;
        keys[ Qt::Key_Print ] = YKey::Key_PrintScreen ;
        keys[ Qt::Key_SysReq ] = YKey::Key_SysReq ;
        keys[ Qt::Key_Home ] = YKey::Key_Home;
        keys[ Qt::Key_End ] = YKey::Key_End;
        keys[ Qt::Key_Left ] = YKey::Key_Left ;
        keys[ Qt::Key_Up ] = YKey::Key_Up ; 
        keys[ Qt::Key_Right ] = YKey::Key_Right ;
        keys[ Qt::Key_Down ] = YKey::Key_Down ;
        keys[ Qt::Key_PageUp ] = YKey::Key_PageUp ;
        keys[ Qt::Key_PageDown ] = YKey::Key_PageDown ;
        keys[ Qt::Key_Shift ] = YKey::Key_Shift ;
        keys[ Qt::Key_Control ] = YKey::Key_Ctrl ;
        keys[ Qt::Key_Meta ] = YKey::Key_Meta ;
        keys[ Qt::Key_Alt ] = YKey::Key_Alt ;
        //hmm ignore it keys[ Qt::Key_CapsLock ] = "<CAPSLOCK>" ;
        //hmm ignore it keys[ Qt::Key_NumLock ] = "<NUMLOCK>" ;
        //hmm ignore it keys[ Qt::Key_ScrollLock ] = "<SCROLLLOCK>" ;
        keys[ Qt::Key_Clear ] = YKey::Key_Clear ;
        keys[ Qt::Key_F1 ] = YKey::Key_F1 ;
        keys[ Qt::Key_F2 ] = YKey::Key_F2 ;
        keys[ Qt::Key_F3 ] = YKey::Key_F3 ;
        keys[ Qt::Key_F4 ] = YKey::Key_F4 ;
        keys[ Qt::Key_F5 ] = YKey::Key_F5 ;
        keys[ Qt::Key_F6 ] = YKey::Key_F6 ;
        keys[ Qt::Key_F7 ] = YKey::Key_F7 ;
        keys[ Qt::Key_F8 ] = YKey::Key_F8 ;
        keys[ Qt::Key_F9 ] = YKey::Key_F9 ; 
        keys[ Qt::Key_F10 ] = YKey::Key_F10 ;
        keys[ Qt::Key_F11 ] = YKey::Key_F11 ;
        keys[ Qt::Key_F12 ] = YKey::Key_F12 ;
        keys[ Qt::Key_F13 ] = YKey::Key_F13 ;
        keys[ Qt::Key_F14 ] = YKey::Key_F14 ;
        keys[ Qt::Key_F15 ] = YKey::Key_F15 ;
        keys[ Qt::Key_F16 ] = YKey::Key_F16 ;
        keys[ Qt::Key_F17 ] = YKey::Key_F17 ;
        keys[ Qt::Key_F18 ] = YKey::Key_F18 ;
        keys[ Qt::Key_F19 ] = YKey::Key_F19 ;
        keys[ Qt::Key_F20 ] = YKey::Key_F20 ;
        keys[ Qt::Key_F21 ] = YKey::Key_F21 ;
        keys[ Qt::Key_F22 ] = YKey::Key_F22 ;
        keys[ Qt::Key_F23 ] = YKey::Key_F23 ;
        keys[ Qt::Key_F24 ] = YKey::Key_F24 ;
        keys[ Qt::Key_F25 ] = YKey::Key_F25 ;
        keys[ Qt::Key_F26 ] = YKey::Key_F26 ;
        keys[ Qt::Key_F27 ] = YKey::Key_F27 ;
        keys[ Qt::Key_F28 ] = YKey::Key_F28 ;
        keys[ Qt::Key_F29 ] = YKey::Key_F29 ;
        keys[ Qt::Key_F30 ] = YKey::Key_F30 ;
        keys[ Qt::Key_F31 ] = YKey::Key_F31 ;
        keys[ Qt::Key_F32 ] = YKey::Key_F32 ;
        keys[ Qt::Key_F33 ] = YKey::Key_F33 ;
        keys[ Qt::Key_F34 ] = YKey::Key_F34 ;
        keys[ Qt::Key_F35 ] = YKey::Key_F35 ;
        keys[ Qt::Key_BracketLeft ] = YKey::Key_LeftSBracket;
        keys[ Qt::Key_BracketRight ] = YKey::Key_RightSBracket;
    }

    actionCollection = new KActionCollection( this );
    signalMapper = new QSignalMapper( this );
    connect( signalMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( sendMultipleKeys( const QString& ) ) );
}

YDrawCell KYView::getCursorDrawCell( )
{
    return m_drawBuffer.at( getCursor() - YCursor(getDrawCurrentLeft(), getDrawCurrentTop( )) );
}

void KYView::registerModifierKeys( const QString& keys )
{
    KAction* k = new KAction( actionCollection );
    k->setShortcut( KShortcut( keysToShortcut( keys ) ) );
    connect( k, SIGNAL( triggered() ), signalMapper, SLOT( map() ) );

    signalMapper->setMapping( k, keys );
}
void KYView::unregisterModifierKeys( const QString& keys )
{
    QByteArray ke = keys.toUtf8();
    QAction* q = actionCollection->action( ke.data() );
    if ( q == NULL ) {
        yzDebug() << "No KAction for " << keys << endl;
        return ;
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

QString KYView::keysToShortcut( const QString& keys )
{
    QString ret = keys;
    ret = ret.replace( "<CTRL>", "CTRL+" );
    ret = ret.replace( "<SHIFT>", "SHIFT+" );
    ret = ret.replace( "<ALT>", "ALT+" );
    return ret;
}

void KYView::scrollLineUp()
{
    scrollView( getCurrentTop() - 1 );
}

void KYView::scrollLineDown()
{
    scrollView( getCurrentTop() + 1 );
}


// scrolls the _view_ on a buffer and moves the cursor it scrolls off the screen
void KYView::scrollView( int value )
{
    if ( value < 0 )
        value = 0;
    else if ( value > myBuffer()->lineCount() - 1 )
        value = myBuffer()->lineCount() - 1;

    // only redraw if the view actually moves
    if (value != getCurrentTop()) {
        alignViewBufferVertically( value );

        if ( !mVScroll->isSliderDown() )
            mVScroll->setValue( value );
    }
}

void KYView::sendMultipleKeys( const QString& k )
{
    YKeySequence input(k);
    YKeySequence::const_iterator parsePos = input.begin();
    KYSession::self()->sendMultipleKeys( this, input, parsePos );
}

#include "kyview.moc"
