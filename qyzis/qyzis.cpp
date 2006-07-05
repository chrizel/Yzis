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

/*
 * $Id: kyzis.cpp 2072 2005-09-01 11:01:40Z mikmak $
 */


#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <qmenubar.h>

#include "session.h"
#include "viewid.h"
#include "viewwidget.h"
#include "qyzis.h"
#include "debug.h"

#include "configuredialog.h"

Qyzis *Qyzis::me = NULL;

Qyzis::Qyzis(QWidget *w, const QString& keys)
	: QMainWindow(w),
	mBuffers( 0 ), mViews( 0 )
{
	m_initialCommand = keys;
	resize( 800, 600 );

	//setXMLFile( "kyzis_shell.rc" );
	setupActions();
	//createShellGUI( true );

	// call it as last thing, must be sure everything is already set up ;)
	//setAutoSaveSettings ("MainWindow Settings");

	me = this;
}

Qyzis::~Qyzis() {
}

void Qyzis::init () {
	if (m_initialCommand.length()) {
		YZSession::me->sendMultipleKeys(m_initialCommand);
	}
}

void Qyzis::load(const QString& url) {
	yzDebug() << "load " << url << endl;
	/// TODO : do something here, like creating a buffer and such..
//	KParts::ReadWritePart *p = getCurrentPart();
//	if ( p ) p->openURL(url);
	//XXX else
}

void Qyzis::setupActions() {
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
        a = new QAction(  _( "&Preferences..." ), this );
	connect( a, SIGNAL( triggered() ), this, SLOT( preferences() ) );
	m->addAction(a);

	mb->addMenu( _( "&Help" ) );

	/*
	m_openRecentAction = KStdAction::openRecent(this, SLOT(openURL(const QString&)), actionCollection() );
	m_openRecentAction->setWhatsThis( i18n("Opens recently opened file.") );
	m_openRecentAction->loadEntries( kapp->config(), "RecentFiles" );
	*/
}

void Qyzis::fileNew() {
	// this slot is called whenever the File->New menu is selected,
	// the New shortcut is pressed (usually CTRL+N) or the New toolbar
	// button is clicked

	// About this function, the style guide (
	// http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
	// says that it should open a new window if the document is _not_
	// in its initial state.  This is what we do here..
//	if ( ! m_currentPart->url().isEmpty() || m_currentPart->isModified() ) {
//			KTempFile *tmp = new KTempFile(locateLocal("tmp", "kyzis"));
	YZSession::me->createBufferAndView();
//	};
}

void Qyzis::fileQuit() {
    close();
}

void Qyzis::fileOpen() {
	// this slot is called whenever the File->Open menu is selected,
	// the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
	// button is clicked
	QString url = QFileDialog::getOpenFileName(this, "Open a file" );
	if (url.isEmpty())
		return;
	openURL(url);
}

void Qyzis::openURL(const QString &url) {
	if (url.isEmpty())
	{
		fileOpen();
		return;
	}

	/*
	m_openRecentAction->addURL(url);
	m_openRecentAction->saveEntries( kapp->config(), "RecentFiles" );
	*/

	YZSession::me->createBufferAndView( url );
}

/* dcop stuff from Kyzis, removed by orzel, we dont care in qyzis
void Qyzis::setCaption( const YZViewId &id, const QString& caption ) {
	// Parse out the filename.
	QString filename = caption.section("/", -1);
	
	QYZisView *view = dynamic_cast<QYZisView*>(YZSession::me->findView( id ));
	
}
*/

bool Qyzis::queryClose() {
	const YZBufferList &buffers = YZSession::me->buffers();
	
	for ( YZBufferList::const_iterator it = buffers.begin(); it != buffers.end(); ++it ) {
		YZBuffer *buf = *it;
		
		if ( buf->fileIsModified() ) {
			int msg = QMessageBox::warning( 
						this,
						"Close Document",
						QString("The file '%1' has been modified but not saved, do you want to save it ?" )
								.arg( buf->fileName() ),
						_("save"),
						_("cancel"),
						QString(), // button 2
						0, // default to "ok"
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

void Qyzis::preferences() {
	QYZConfigureDialog* w = new QYZConfigureDialog(this);
	w->exec();
}

void Qyzis::embedPartView(QWidget *view, const QString &, const QString& ) {
	if (!view)
			return;
//	addWidget(view, QString());
	setCentralWidget(view);
	view->show();
}


#if 0
// removed by orzel, those are newui stuff not even used in Kyzis
void Qyzis::removeView(QWidget *view) {
	if (!view)
		return;

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
void Qyzis::setViewAvailable(QWidget *pView, bool bEnabled) {
    Q3DockWindow *dock;
    if (m_docks.contains(pView))
        dock = toolWindow(m_docks[pView]);
    else
        return;

    bEnabled ? dock->showWidget(pView) : dock->hideWidget(pView);
}
*/

void Qyzis::raiseView(QWidget *view) {
    //adymo: a workaround to make editor wrappers work:
    //editor view is passed to this function but the ui library knows only
    //of its parent which is an editor wrapper, simply replacing the view
    //by its wrapper helps here
    if (view->parent() && view->parent()->isA("EditorWrapper"))
    {
//         yzDebug() << "parent is editor wrapper: " <<
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
    else*/ if (m_widgets.contains(view) && m_widgetTabs.contains(view))
        m_widgetTabs[view]->showPage(view);
}

void Qyzis::lowerView(QWidget * /*view*/) {
    //nothing to do
}

void Qyzis::gotoNextWindow() {
    if ((m_activeTabWidget->currentPageIndex() + 1) < m_activeTabWidget->count())
        m_activeTabWidget->setCurrentPage(m_activeTabWidget->currentPageIndex() + 1);
    else
        m_activeTabWidget->setCurrentPage(0);
}

void Qyzis::gotoPreviousWindow() {
    if ((m_activeTabWidget->currentPageIndex() - 1) >= 0)
        m_activeTabWidget->setCurrentPage(m_activeTabWidget->currentPageIndex() - 1);
    else
        m_activeTabWidget->setCurrentPage(m_activeTabWidget->count() - 1);
}

void Qyzis::gotoLastWindow()
{
    //@todo implement
}
#endif

void Qyzis::closeTab()
{
	// TODO : do something smart
//    actionCollection()->action("file_close")->activate();
}


void Qyzis::closeTab(QWidget *)
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

QMainWindow *Qyzis::main()
{
    return this;
}

