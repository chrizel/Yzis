/*
   Copyright (c) 2003-2005 Mickael Marchand <mikmak@yzis.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of version 2 of the GNU General Public
   License as published by the Free Software Foundation

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


/* QYzis */
#include "qysession.h"
#include "qyview.h"
#include "qyconfiguredialog.h"

/* Yzis */
#include "portability.h"
#include "debug.h"
#include "buffer.h"

/* Qt */
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QTabWidget>
#include <QClipboard>
#include <QSettings>
#include <QResizeEvent>
#ifdef Q_WS_X11
#include <QX11Info>
#endif


#define dbg()    yzDebug("QYSession")
#define err()    yzError("QYSession")

// ===================[ Qt stuff for the main windows ]==================

QYSession::QYSession(QWidget *w)
        : QMainWindow(w),
        YSession()
{
    dbg() << "QYSession()" << endl;
    resize( 800, 600 );

    mTabWidget = new QTabWidget();
    setCentralWidget( mTabWidget );

    setupActions();

    YSession::setInstance( this );
}

QYSession::~QYSession()
{
    dbg() << "~QYSession()" << endl;
}

void QYSession::slotFrontendGuiReady()
{
    dbg() << "slotFrontendGuiReady()" << endl;
    YSession::self()->frontendGuiReady();
}

void QYSession::setupActions()
{
    QAction *a;
    QMenu *m;

    QMenuBar *mb = menuBar();
    m = mb->addMenu( _( "&File" ) );

    a = new QAction( QIcon( ":/images/new.png" ), _( "&New..." ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( slotFileNew() ) );
    m->addAction(a);

    a = new QAction( QIcon( ":/images/open.png" ), _( "&Open..." ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( slotFileOpen() ) );
    m->addAction(a);

    a = new QAction( QIcon( ":/images/quit.png" ), _( "&Quit..." ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( slotFileQuit() ) );
    m->addAction(a);

    m = mb->addMenu( _( "&Settings" ) );
    a = new QAction( _( "&Preferences..." ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( slotPreferences() ) );
    m->addAction(a);

    m = mb->addMenu( _( "&Help" ) );

    a = new QAction( _( "&About QYzis" ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( slotAbout() ) );
    m->addAction(a);
}

void QYSession::slotFileNew()
{
    dbg() << "slotFileNew()" << endl;
    createBufferAndView();
}

void QYSession::slotFileQuit()
{
    dbg() << "slotFileQuit()" << endl;
    close();
}

void QYSession::slotFileOpen()
{
    dbg() << "slotFileOpen()" << endl;

    QString url = QFileDialog::getOpenFileName(this, "Open a file" );
    if (url.isEmpty())
        return ;
    openURL(url);
}

void QYSession::openURL(const QString &url)
{
    dbg() << "openURL( " << url << ")" << endl;
    if (url.isEmpty()) {
        return ;
    }

    createBufferAndView( url );
}

void QYSession::slotPreferences()
{
    dbg() << "slotPreferences()" << endl;
    QYConfigureDialog* w = new QYConfigureDialog(this);
    w->exec();
}

void QYSession::slotAbout()
{
    dbg() << "slotAbout()" << endl;
    QMessageBox::about(this, _("About QYzis"),
                       _("Qt frontend for the yzis text editor\n\n"\
                         "http://www.yzis.org"));
}


// ===================[ Yzis stuff ]==================
//
void QYSession::guiSetClipboardText( const QString& text, Clipboard::Mode mode )
{
    dbg() << QString("guiSetClipboardText( %1, %2 )").arg(text).arg((int)mode) << endl;
    bool hasDisplay = true;
#ifdef Q_WS_X11
    hasDisplay = QX11Info::display();
#endif

    if (hasDisplay) {
        QApplication::clipboard()->setText( text, mode == Clipboard::Clipboard ? QClipboard::Clipboard : QClipboard::Selection );
    }
}

bool QYSession::guiQuit( int errorCode )
{
    dbg() << "guiQuit(" << errorCode << ")" << endl;
    qApp->quit();
    return true;
}

void QYSession::applyConfig()
{
    dbg() << "applyConfig()" << endl;
    QSettings settings;
    qreal opacity = settings.value("appearance/opacity", 1.).value<qreal>();
    setWindowOpacity(opacity);
    // apply new configuration to all views
    // TODO
}

void QYSession::guiChangeCurrentView( YView* view )
{
    dbg() << "guiChangeCurrentView(" << view->toString() << ")" << endl;
    QYView *v = static_cast<QYView*>(view);
    Q_ASSERT( v );
    mTabWidget->setCurrentWidget( v );
}

YView* QYSession::guiCreateView( YBuffer *buffer )
{
    dbg() << "guiCreateView(" << buffer->toString() << ")" << endl;
    QYView *view;
    view = new QYView( buffer, this );
    dbg().sprintf("guiCreateView(): new view=%p\n", view );
    mTabWidget->addTab( view, buffer->fileNameShort() );
    dbg().sprintf("guiCreateView(): tabIdx=%d\n", mTabWidget->indexOf( view ) );
    view->show();
    return view;
}

void QYSession::viewFilenameChanged( QYView * view, const QString & filename )
{
    Q_ASSERT( view );
    int tabIdx = mTabWidget->indexOf( view );
    mTabWidget->setTabText( tabIdx, filename );
    setWindowTitle( filename );
}


void QYSession::guiDeleteView( YView *view )
{
    dbg() << "guiDeleteView(" << view->toString() << ")" << endl;
    QYView * v = dynamic_cast<QYView*>(view);
    Q_ASSERT( v );
    dbg() << "guiDeleteView(): delete v;" << endl;
    delete v;
    // dumpObjectTree();
    dbg() << "guiDeleteView(): done" << endl;
}


void QYSession::guiPopupMessage( const QString& message )
{
    dbg() << "popupMessage(" << message << ")" << endl;
    QMessageBox::information( this , _( "Error" ), message);
}

bool QYSession::guiPromptYesNo(const QString& title, const QString& message)
{
    dbg() << "guiPromptYesNo(" << title << "," << message << ")" << endl;
    int v = QMessageBox::question( this , title, message, _("Yes"), _("No"));
    if (v == 0) return true;
    else return false;
}

int QYSession::guiPromptYesNoCancel(const QString& title, const QString& message)
{
    dbg() << "guiPromptYesNoCancel(" << title << "," << message << ")" << endl;
    return QMessageBox::question( this, title, message, _("Yes"), _("No"), _("Cancel"));
}

void QYSession::guiSplitHorizontally(YView *view)
{
    dbg() << "guiSplitHorizontally(" << view->toString() << ")" << endl;
}

YDebugStream& operator<<( YDebugStream& out, const Qt::KeyboardModifiers & v )
{
    QString s;
    switch (v) {
    case Qt::NoModifier: s += "NoModifier |"; break;
    case Qt::ShiftModifier: s += "ShiftModifier |"; break;
    case Qt::ControlModifier: s += "ControlModifier |"; break;
    case Qt::AltModifier: s += "AltModifier |"; break;
    case Qt::MetaModifier: s += "MetaModifier |"; break;
    case Qt::KeypadModifier: s += "KeypadModifier |"; break;
    case Qt::GroupSwitchModifier: s += "GroupSwitchModifier |"; break;
    }
    s = s.left( s.length() - 2 );
    out << s;
    return out;
}

YDebugStream& operator<<( YDebugStream& out, const QSize & sz )
{
    out << sz.width() << "," << sz.height();
    return out;
}

YDebugStream& operator<<( YDebugStream& out, const QResizeEvent & e )
{
    out << "(oldsize=" << e.oldSize() << ",newSize=" << e.size() << ")" << endl;
    return out;
}

YDebugStream& operator<<( YDebugStream& out, const Qt::FocusReason & e )
{
    switch( e ) {
    case Qt::NoFocusReason: out << "NoFocusReason"; break;
    case Qt::MouseFocusReason: out << "MouseFocusReason"; break;
    case Qt::TabFocusReason: out << "TabFocusReason"; break;
    case Qt::BacktabFocusReason: out << "BacktabFocusReason"; break;
    case Qt::ActiveWindowFocusReason: out << "ActiveWindowFocusReason"; break;
    case Qt::PopupFocusReason: out << "PopupFocusReason"; break;
    case Qt::ShortcutFocusReason: out << "ShortcutFocusReason"; break;
    case Qt::MenuBarFocusReason: out << "MenuBarFocusReason"; break;
    case Qt::OtherFocusReason: out << "OtherFocusReason"; break;
    }
    return out;
}

