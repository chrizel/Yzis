/*  This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <mikmak@yzis.org>,
*  Copyright (C) 2003-2004 Lucijan Busch <lucijan@kde.org>
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
#include "qysession.h"
#include "qyview.h"
#include "qyzis.h"

/* Yzis */
#include "portability.h"
#include "debug.h"
#include "yzis.h"
#include "view.h"
#include "buffer.h"

/* Std */
#include <assert.h>
#include <unistd.h>

/* Qt */
#include <QApplication>
#include <QString>
#include <QMessageBox>
#include <QTimer>
#include <QTextCodec>
#include <QClipboard>
#include <QSettings>
#include <QList>
#include <QTabWidget>
#ifdef Q_WS_X11
#include <QX11Info>
#endif

#define dbg() yzDebug("QYSession")
#define err() yzError("QYSession")

void QYSession::createInstance()
{
    dbg() << "createInstance()" << endl;
    // such allocation (i.e. not "new QYSession") will ensure that
    // "instance" object will be properly and automatically deleted
    // when program exits
    static QYSession instance;
    setInstance(&instance);
}

QYSession::QYSession() :
        YSession(),
        mLastView(0),
        mViewParent(0)
{
    dbg() << "QYSession()" << endl;
}

QYSession::~QYSession()
{
    dbg() << "~QYSession()" << endl;
}

void QYSession::frontendGuiReady()
{
    dbg() << "frontendGuiReady()" << endl;
    YSession::self()->frontendGuiReady();
}


void QYSession::guiSetClipboardText( const QString& text, Clipboard::Mode mode )
{
    dbg() << QString("guiSetClipboardText( %1, %2 )").arg(text).arg((int)mode) << endl;
#ifdef Q_WS_X11
    if ( QX11Info::display() )
#endif
        QApplication::clipboard()->setText( text, mode == Clipboard::Clipboard ? QClipboard::Clipboard : QClipboard::Selection );
}

bool QYSession::guiQuit( int errorCode )
{
    dbg() << "guiQuit(" << errorCode << ")" << endl;
    if (QYzis::me) {
        qApp->quit();
    } else if ( currentView() && currentView()->modePool()->currentType() == YMode::ModeEx
                && !currentView()->guiGetCommandLineText().isEmpty() ) {
        dbg() << "guiQuit(" << errorCode << ") done." << endl;
        return false;
    }


    dbg() << "guiQuit(" << errorCode << ") done." << endl;
    return true;
}

void QYSession::applyConfig()
{
    dbg() << "applyConfig()" << endl;
    QSettings settings;
    // apply new configuration to all views
    YBufferList::ConstIterator it = buffers().begin();
    YBufferList::ConstIterator end = buffers().end();
    for ( ; it != end; ++it ) {
        YBuffer *b = *it;
        QList<YView*> l = b->views();
        for ( QList<YView*>::Iterator itr = l.begin(); itr != l.end(); ++itr ) {
            QYView* yv = static_cast<QYView*>( *itr );
            yv->applyConfig( settings );
        }
    }
}

void QYSession::guiChangeCurrentView( YView* view )
{
    dbg() << "guiChangeCurrentView(" << view->toString() << ")" << endl;
    QYView *v = static_cast<QYView*>(view);
    QYzis::me->mTabWidget->setCurrentWidget( v );
}

YView* QYSession::guiCreateView( YBuffer *buffer )
{
    dbg() << "guiCreateView(" << buffer->toString() << ")" << endl;
    QYView *view;

    view = new QYView( buffer, mViewParent, QString(buffer->fileName() + "-view").toUtf8().constData() );

    if ( QYzis::me ) {
        QYzis::me->embedPartView( view, buffer->fileName(), buffer->fileName() );
    } else {
        //?   view->setMdiChildView( 0 );
        err() << "unhandled case in YView* QYSession::guiCreateView( YBuffer *buffer ) " ;
    }

    view->setFocus();
    view->show();

    return view;
}

YBuffer *QYSession::guiCreateBuffer()
{
    dbg() << "guiCreateBuffer()" << endl;
    return new YBuffer;
}

void QYSession::guiPopupMessage( const QString& message )
{
    dbg() << "popupMessage(" << message << ")" << endl;
    QYView *v = static_cast<QYView*>(currentView());
    QMessageBox::information(v, _( "Error" ), message);
}

void QYSession::closeView()
{
    dbg() << "closeView()" << endl;
    if (QYzis::me) {
        QYzis::me->closeTab();
    } else {
        mLastView->close();
    }
    mLastView = 0;
}

void QYSession::guiDeleteView( YView *view )
{
    dbg() << "guiDeleteView(" << view->toString() << ")" << endl;
    QYView *kview = dynamic_cast<QYView*>(view);
    if ( kview ) {
        mLastView = kview;
        QTimer::singleShot(0, this, SLOT( closeView() ));
    }
}

void QYSession::guiDeleteBuffer(YBuffer* b)
{
    dbg() << "guiDeleteBuffer(" << b->toString() << ")" << endl;
    delete b;
}

void QYSession::guiSetFocusMainWindow()
{
    dbg() << "guiSetFocusMainWindow()" << endl;
    QYView *yv = static_cast<QYView*>( currentView() );
    yv->guiSetFocusMainWindow();
}

void QYSession::guiSetFocusCommandLine()
{
    dbg() << "guiSetFocusCommandLine()" << endl;
    QYView *yv = static_cast<QYView*>( currentView() );
    yv->guiSetFocusCommandLine();
}

bool QYSession::guiPromptYesNo(const QString& title, const QString& message)
{
    dbg() << "guiPromptYesNo(" << title << "," << message << ")" << endl;
    int v = QMessageBox::question(static_cast<QYView*>(currentView()), title, message, _("Yes"), _("No"));
    if (v == 0) return true;
    else return false;
}

int QYSession::guiPromptYesNoCancel(const QString& title, const QString& message)
{
    dbg() << "guiPromptYesNoCancel(" << title << "," << message << ")" << endl;
    return QMessageBox::question(static_cast<QYView*>(currentView()), title, message, _("Yes"), _("No"), _("Cancel"));
}

void QYSession::guiSplitHorizontally(YView* view)
{
    dbg() << "guiSplitHorizontally(" << view->toString() << ")" << endl;
}


