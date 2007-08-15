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

/* Yzis */
#include "qsession.h"
#include "portability.h"
#include "viewwidget.h"
#include "debug.h"
#include "yzis.h"
#include "qyzis.h"
#include "view.h"
#include "buffer.h"

/* Std */
#include <assert.h>
#include <unistd.h>

/* Qt */
#include <qapplication.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <qtextcodec.h>
#include <QClipboard>
#include <QSettings>
#include <QList>
#ifdef Q_WS_X11
#include <QX11Info>
#endif

#define dbg() yzDebug("QYZisSession")
#define err() yzError("QYZisSession")

void QYZisSession::createInstance()
{
    dbg() << "createInstance()" << endl;
    // such allocation (i.e. not "new QYZisSession") will ensure that
    // "instance" object will be properly and automatically deleted
    // when program exits
    static QYZisSession instance;
    setInstance(&instance);
}

QYZisSession::QYZisSession() :
        YZSession(),
        lastView(0),
        m_viewParent(0)
{
    dbg() << "QYZisSession()" << endl;
}

QYZisSession::~QYZisSession()
{
    dbg() << "~QYZisSession()" << endl;
}

void QYZisSession::frontendGuiReady()
{
    dbg() << "frontendGuiReady()" << endl;
    YZSession::self()->frontendGuiReady();
}


void QYZisSession::guiSetClipboardText( const QString& text, Clipboard::Mode mode )
{
    dbg() << QString("guiSetClipboardText( %1, %2 )").arg(text).arg((int)mode) << endl;
#ifdef Q_WS_X11
    if ( QX11Info::display() )
#endif
        QApplication::clipboard()->setText( text, mode == Clipboard::Clipboard ? QClipboard::Clipboard : QClipboard::Selection );
}

bool QYZisSession::guiQuit( int errorCode )
{
    dbg() << "guiQuit(" << errorCode << ")" << endl;
    if (Qyzis::me) {
        qApp->quit();
    } else if ( currentView() && currentView()->modePool()->currentType() == YZMode::ModeEx
                && !currentView()->guiGetCommandLineText().isEmpty() ) {
        dbg() << "guiQuit(" << errorCode << ") done." << endl;
        return false;
    }


    dbg() << "guiQuit(" << errorCode << ") done." << endl;
    return true;
}

void QYZisSession::applyConfig()
{
    dbg() << "applyConfig()" << endl;
    QSettings settings;
    // apply new configuration to all views
    YZBufferList::ConstIterator it = buffers().begin();
    YZBufferList::ConstIterator end = buffers().end();
    for ( ; it != end; ++it ) {
        YZBuffer *b = *it;
        QList<YZView*> l = b->views();
        for ( QList<YZView*>::Iterator itr = l.begin(); itr != l.end(); ++itr ) {
            QYZisView* yv = static_cast<QYZisView*>( *itr );
            yv->applyConfig( settings );
        }
    }
}

void QYZisSession::guiChangeCurrentView( YZView* view )
{
    dbg() << "guiChangeCurrentView(" << view->toString() << ")" << endl;
    QYZisView *v = static_cast<QYZisView*>(view);
    v->activateWindow();
    v->setFocus();
}

YZView* QYZisSession::guiCreateView( YZBuffer *buffer )
{
    dbg() << "guiCreateView(" << buffer->toString() << ")" << endl;
    QYZisView *view;

    view = new QYZisView( buffer, m_viewParent, QString(buffer->fileName() + "-view").toUtf8().constData() );

    if ( Qyzis::me ) {
        Qyzis::me->embedPartView( view, buffer->fileName(), buffer->fileName() );
    } else {
        //?   view->setMdiChildView( 0 );
        err() << "unhandled case in YZView* QYZisSession::guiCreateView( YZBuffer *buffer ) " ;
    }

    view->setFocus();
    view->show();

    return view;
}

YZBuffer *QYZisSession::guiCreateBuffer()
{
    dbg() << "guiCreateBuffer()" << endl;
    return new YZBuffer;
}

void QYZisSession::guiPopupMessage( const QString& message )
{
    dbg() << "popupMessage(" << message << ")" << endl;
    QYZisView *v = static_cast<QYZisView*>(currentView());
    QMessageBox::information(v, _( "Error" ), message);
}

void QYZisSession::closeView()
{
    dbg() << "closeView()" << endl;
    if (Qyzis::me) {
        Qyzis::me->closeTab();
    } else {
        lastView->close();
    }
    lastView = 0;
}

void QYZisSession::guiDeleteView( YZView *view )
{
    dbg() << "guiDeleteView(" << view->toString() << ")" << endl;
    QYZisView *kview = dynamic_cast<QYZisView*>(view);
    if ( kview ) {
        lastView = kview;
        QTimer::singleShot(0, this, SLOT( closeView() ));
    }
}

void QYZisSession::guiDeleteBuffer(YZBuffer* b)
{
    dbg() << "guiDeleteBuffer(" << b->toString() << ")" << endl;
    delete b;
}

void QYZisSession::guiSetFocusMainWindow()
{
    dbg() << "guiSetFocusMainWindow()" << endl;
    QYZisView *yv = static_cast<QYZisView*>( currentView() );
    yv->guiSetFocusMainWindow();
}

void QYZisSession::guiSetFocusCommandLine()
{
    dbg() << "guiSetFocusCommandLine()" << endl;
    QYZisView *yv = static_cast<QYZisView*>( currentView() );
    yv->guiSetFocusCommandLine();
}

bool QYZisSession::guiPromptYesNo(const QString& title, const QString& message)
{
    dbg() << "guiPromptYesNo(" << title << "," << message << ")" << endl;
    int v = QMessageBox::question(static_cast<QYZisView*>(currentView()), title, message, _("Yes"), _("No"));
    if (v == 0) return true;
    else return false;
}

int QYZisSession::guiPromptYesNoCancel(const QString& title, const QString& message)
{
    dbg() << "guiPromptYesNoCancel(" << title << "," << message << ")" << endl;
    return QMessageBox::question(static_cast<QYZisView*>(currentView()), title, message, _("Yes"), _("No"), _("Cancel"));
}

void QYZisSession::guiSplitHorizontally(YZView* view)
{
    dbg() << "guiSplitHorizontally(" << view->toString() << ")" << endl;
}


