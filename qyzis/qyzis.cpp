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
#include "qyzis.h"
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


#define dbg()    yzDebug("QYzis")
#define err()    yzError("QYzis")

QYzis *QYzis::me = NULL;

QYzis::QYzis(QWidget *w)
        : QMainWindow(w),
        mBuffers( 0 ), mViews( 0 )
{
    resize( 800, 600 );

    mTabWidget = new QTabWidget();
    setCentralWidget( mTabWidget );

    setupActions();
    //createShellGUI( true );

    // call it as last thing, must be sure everything is already set up ;)
    //setAutoSaveSettings ("MainWindow Settings");

    me = this;
}

QYzis::~QYzis()
{}

void QYzis::load(const QString& url)
{
    dbg() << "load " << url << endl;
    /// TODO : do something here, like creating a buffer and such..
    // KParts::ReadWritePart *p = getCurrentPart();
    // if ( p ) p->openURL(url);
    //XXX else
}

void QYzis::setupActions()
{
    QAction *a;
    QMenu *m;

    QMenuBar *mb = menuBar();
    m = mb->addMenu( _( "&File" ) );

    a = new QAction( QIcon( ":/images/new.png" ), _( "&New..." ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( fileNew() ) );
    m->addAction(a);

    a = new QAction( QIcon( ":/images/open.png" ), _( "&Open..." ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( fileOpen() ) );
    m->addAction(a);

    a = new QAction( QIcon( ":/images/quit.png" ), _( "&Quit..." ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( fileQuit() ) );
    m->addAction(a);

    m = mb->addMenu( _( "&Settings" ) );
    a = new QAction( _( "&Preferences..." ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( preferences() ) );
    m->addAction(a);

    m = mb->addMenu( _( "&Help" ) );

    a = new QAction( _( "&About QYzis" ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( about() ) );
    m->addAction(a);

    /*
    m_openRecentAction = KStdAction::openRecent(this, SLOT(openURL(const QString&)), actionCollection() );
    m_openRecentAction->setWhatsThis( i18n("Opens recently opened file.") );
    m_openRecentAction->loadEntries( kapp->config(), "RecentFiles" );
    */
}

void QYzis::fileNew()
{
    // this slot is called whenever the File->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

    // About this function, the style guide (
    // http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
    // says that it should open a new window if the document is _not_
    // in its initial state.  This is what we do here..
    // if ( ! m_currentPart->url().isEmpty() || m_currentPart->isModified() ) {
    //   KTempFile *tmp = new KTempFile(locateLocal("tmp", "kyzis"));
    QYSession::self()->createBufferAndView();
    // };
}

void QYzis::fileQuit()
{
    close();
}

void QYzis::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    QString url = QFileDialog::getOpenFileName(this, "Open a file" );
    if (url.isEmpty())
        return ;
    openURL(url);
}

void QYzis::openURL(const QString &url)
{
    if (url.isEmpty()) {
        fileOpen();
        return ;
    }

    /*
    m_openRecentAction->addURL(url);
    m_openRecentAction->saveEntries( kapp->config(), "RecentFiles" );
    */

    QYSession::self()->createBufferAndView( url );
}

bool QYzis::queryClose()
{
    const YBufferList &buffers = QYSession::self()->buffers();

    for ( YBufferList::const_iterator it = buffers.begin(); it != buffers.end(); ++it ) {
        YBuffer *buf = *it;

        if ( buf->fileIsModified() ) {
            int msg = QMessageBox::warning(
                          this,
                          "Close Document",
                          QString("The file '%1' has been modified but not saved, do you want to save it ?" )
                          .arg( buf->fileName() ),
                          _("save"),
                          _("cancel"),
                          QString(),  // button 2
                          0,  // default to "ok"
                          2 ); // escape is "cancel"
            if ( msg == 2 ) {
                return false;
            }
            if ( msg == 1 ) {
                buf->save(); //automatically popups saveAs dialog if needed
            }
        }
    }

    return true;
}

void QYzis::preferences()
{
    QYConfigureDialog* w = new QYConfigureDialog(this);
    w->exec();
}

void QYzis::about()
{
    QMessageBox::about(this, _("About QYzis"),
                       _("Qt frontend for the yzis text editor\n\n"\
                         "http://www.yzis.org"));
}

void QYzis::embedPartView(QWidget *view, const QString & title, const QString& tooltip )
{
    Q_UNUSED( tooltip );
    if (!view)
        return ;
    mTabWidget->addTab( view, title );
    // addWidget(view, QString());
    //setCentralWidget(view);
    view->show();
}


#if 0 
// removed by orzel, those are newui stuff not even used in Kyzis
void QYzis::removeView(QWidget *view)
{
    if (!view)
        return ;

    //try to remove it from all parts of main window
    //@fixme This method needs to be divided in two - one for docks and one for part views
    // TODO : orzel : something is probably really wrong in commenting
    // that :


    /*
    if (m_docks.contains(view))
     toolWindow(m_docks[view])->removeWidget(view);
    else
    */
    removeWidget(view);
}

/*
void QYzis::setViewAvailable(QWidget *pView, bool bEnabled) {
    Q3DockWindow *dock;
    if (m_docks.contains(pView))
        dock = toolWindow(m_docks[pView]);
    else
        return;
 
    bEnabled ? dock->showWidget(pView) : dock->hideWidget(pView);
}
*/

void QYzis::raiseView(QWidget *view)
{
    //adymo: a workaround to make editor wrappers work:
    //editor view is passed to this function but the ui library knows only
    //of its parent which is an editor wrapper, simply replacing the view
    //by its wrapper helps here
    if (view->parent() && view->parent()->isA("EditorWrapper")) {
        //         dbg() << "parent is editor wrapper: " <<
        //             static_cast<EditorWrapper*>(view->parent()) << endl;
        view = (QWidget*)view->parent();
    }

    // TODO : orzel: commenting that out is probably really wrong
    /*
    if (m_docks.contains(view))
    {
        Q3DockWindow *dock = toolWindow(m_docks[view]);
        dock->raiseWidget(view);
    }
    else*/ if  (m_widgets.contains(view) &&  m_widgetTabs.contains(view))
        m_widgetTabs[view]->showPage(view);
}

void QYzis::lowerView(QWidget * /*view*/)
{
    //nothing to do
}

void QYzis::gotoNextWindow()
{
    if ((m_activeTabWidget->currentPageIndex() + 1) < m_activeTabWidget->count())
        m_activeTabWidget->setCurrentPage(m_activeTabWidget->currentPageIndex() + 1);
    else
        m_activeTabWidget->setCurrentPage(0);
}

void QYzis::gotoPreviousWindow()
{
    if ((m_activeTabWidget->currentPageIndex() - 1) >= 0)
        m_activeTabWidget->setCurrentPage(m_activeTabWidget->currentPageIndex() - 1);
    else
        m_activeTabWidget->setCurrentPage(m_activeTabWidget->count() - 1);
}

void QYzis::gotoLastWindow()
{
    //@todo implement
}
#endif

void QYzis::closeTab()
{
    // TODO : do something smart
    //    actionCollection()->action("file_close")->activate();
}


void QYzis::closeTab(QWidget *)
{
    /*    const Q3PtrList<KParts::Part> *partlist = PartController::getInstance()->parts();
        Q3PtrListIterator<KParts::Part> it(*partlist);
        while (KParts::Part* part = it.current())
        {
            QWidget *widget = EditorProxy::getInstance()->topWidgetForPart(part);
            if (widget && widget == w)
            {
                PartController::getInstance()->closePart(part);
                return;
            }
            ++it;
        }*/
}

QMainWindow *QYzis::main()
{
    return this;
}

YDebugStream& operator<<( YDebugStream& out, Qt::KeyboardModifiers v )
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
