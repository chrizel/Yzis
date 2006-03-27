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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*
 * $Id$
 */
#include "kyzis.h"

#include <kkeydialog.h>
#include <kshortcut.h>
#include <kconfig.h>
#include <kurl.h>

#include <ktoolbar.h>
#include <kedittoolbar.h>

#include <kaction.h>
#include <kstdaction.h>
#include <kiconloader.h>

#include <klibloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <kmultitabbar.h>
#include <kmenu.h>
#include <kmenubar.h>
#include <ktoolbar.h>

#include "ktexteditoriface.h"
#include "session.h"
#include "viewid.h"
#include "viewwidget.h"
#include "dtabwidget.h"

// #include "configdialog.h"

//#include "ktexteditor/ktexteditoriface.h"
//class KTextEditor::View;

Kyzis *Kyzis::me = NULL;

Kyzis::Kyzis(QWidget *w, const QString& keys)
	: DMainWindow(w), DCOPObject( "Kyzis" ),
	mBuffers( 0 ), mViews( 0 )
{
	m_initialCommand = keys;
	resize( 800, 600 );

	setXMLFile( "kyzis_shell.rc" );
	setupActions();
	createShellGUI( true );

	// call it as last thing, must be sure everything is already set up ;)
	setAutoSaveSettings ("MainWindow Settings");

    me = this;
}

Kyzis::~Kyzis() {
}

void Kyzis::init () {
	if (m_initialCommand.length()) {
		YZSession::me->sendMultipleKeys(m_initialCommand);
	}
}

void Kyzis::load(const KUrl& url) {
	kDebug() << "load " << url << endl;
	KParts::ReadWritePart *p = getCurrentPart();
	if ( p ) p->openURL(url);
	//XXX else
}

void Kyzis::setupActions() {
	KStdAction::openNew(this, SLOT(fileNew()), actionCollection())->setShortcut(KShortcut::null());
	KStdAction::open(this, SLOT(fileOpen()), actionCollection());

	KStdAction::quit(this, SLOT(fileQuit()), actionCollection());

	KStdAction::preferences(this, SLOT(preferences()), actionCollection());

	m_openRecentAction = KStdAction::openRecent(this, SLOT(openURL(const KUrl&)),
		actionCollection() );
	m_openRecentAction->setWhatsThis( i18n("Opens recently opened file.") );
	m_openRecentAction->loadEntries( kapp->sessionConfig(), "RecentFiles" );
}

void Kyzis::fileNew() {
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

void Kyzis::optionsShowToolbar() {
	// this is all very cut and paste code for showing/hiding the
	// toolbar
	if (m_toolbarAction->isChecked())
		toolBar()->show();
	else
		toolBar()->hide();
}

void Kyzis::optionsConfigureKeys() {
	//KKeyDialog::configureKeys(actionCollection(), "kyzis_shell.rc");
}

void Kyzis::optionsConfigureToolbars() {
	saveMainWindowSettings(KGlobal::config(), autoSaveGroup());

	// use the standard toolbar editor
	KEditToolbar dlg(factory());
	connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(applyNewToolbarConfig()));
	dlg.exec();
}

void Kyzis::applyNewToolbarConfig() {
	applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
}

void Kyzis::fileQuit() {
    close();
}

void Kyzis::fileOpen() {
	// this slot is called whenever the File->Open menu is selected,
	// the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
	// button is clicked
	KUrl url = KFileDialog::getOpenURL( QString::null, QString::null, this );
	if (url.isEmpty())
		return;
	openURL(url);
}

void Kyzis::openURL(const KUrl &url) {
	if (url.isEmpty())
	{
		fileOpen();
		return;
	}

	m_openRecentAction->addUrl(url);
	m_openRecentAction->saveEntries( kapp->sessionConfig(), "RecentFiles" );

	// About this function, the style guide (
	// http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
	// says that it should open a new window if the document is _not_
	// in its initial state.  This is what we do here..
	YZSession::me->createBufferAndView( url.url() );
}

void Kyzis::setCaption( const YZViewId &id, const QString& caption ) {
	// Parse out the filename.
	QString filename = caption.section("/", -1);
	
	QWidget *view = dynamic_cast<QWidget*>(YZSession::me->findView( id ));
    if (view && m_widgetTabs.contains(view)) {
        DTabWidget *tab = m_widgetTabs[view];
		if (tab)
			tab->setTabLabel(view,filename);
    }
	
	KMainWindow::setCaption( caption );
}

KParts::ReadWritePart* Kyzis::getCurrentPart() {
	kDebug() << "getCurrentPart" << endl;
	
	KYZisView *view = dynamic_cast<KYZisView*>(YZSession::me->currentView());
	return view->getKPart();
}

bool Kyzis::queryClose() {
	const YZBufferList &buffers = YZSession::me->buffers();
	
	for ( YZBufferList::const_iterator it = buffers.begin(); it != buffers.end(); ++it ) {
		YZBuffer *buf = *it;
		
		if ( buf->fileIsModified() ) {
			int msg = KMessageBox::warningYesNoCancel
					(
						this, 
						QString("The file '%1' has been modified but not saved, do you want to save it ?" )
								.arg( buf->fileName() ), 
						"Close Document", 
						KStdGuiItem::save(), 
						KStdGuiItem::discard()
					);
			if ( msg == KMessageBox::Cancel ) {
				return false;
			}
			if ( msg == KMessageBox::Yes ) {
				buf->save(); //automatically popups saveAs dialog if needed
			}
		}
	}

	return true;
}

void Kyzis::preferences() {
    KTextEditor::Editor *edit = dynamic_cast<KTextEditor::Document*>( getCurrentPart() )->editor();
    if (!edit)
        return;
    edit->configDialog(this);
    edit->writeConfig();
}

void Kyzis::embedPartView(QWidget *view, const QString &title, const QString&/*toolTip*/) {
	if (!view)
		return;

	addWidget(view, title);
	view->show();
//	splitVertical();
}

void Kyzis::embedSelectView(QWidget *view, const QString &title, const QString &/*toolTip*/) {
	toolWindow(DDockWindow::Left)->addWidget(title, view);
	m_docks[view] = DDockWindow::Left;
}

void Kyzis::embedOutputView(QWidget *view, const QString &title, const QString &/*toolTip*/) {
	toolWindow(DDockWindow::Bottom)->addWidget(title, view);
	m_docks[view] = DDockWindow::Bottom;
	toolWindow(DDockWindow::Bottom)->show();
}

void Kyzis::embedSelectViewRight(QWidget *view, const QString &title, const QString &/*toolTip*/) {
	toolWindow(DDockWindow::Right)->addWidget(title, view);
	m_docks[view] = DDockWindow::Right;
}

void Kyzis::removeView(QWidget *view) {
	if (!view)
		return;

	//try to remove it from all parts of main window
	//@fixme This method needs to be divided in two - one for docks and one for part views
	if (m_docks.contains(view))
		toolWindow(m_docks[view])->removeWidget(view);
	else
		removeWidget(view);
}

void Kyzis::setViewAvailable(QWidget *pView, bool bEnabled) {
    DDockWindow *dock;
    if (m_docks.contains(pView))
        dock = toolWindow(m_docks[pView]);
    else
        return;

    bEnabled ? dock->showWidget(pView) : dock->hideWidget(pView);
}

void Kyzis::raiseView(QWidget *view) {
    //adymo: a workaround to make editor wrappers work:
    //editor view is passed to this function but the ui library knows only
    //of its parent which is an editor wrapper, simply replacing the view
    //by its wrapper helps here
    if (view->parent() && view->parent()->isA("EditorWrapper"))
    {
//         kDebug() << "parent is editor wrapper: " <<
//             static_cast<EditorWrapper*>(view->parent()) << endl;
        view = (QWidget*)view->parent();
    }

    if (m_docks.contains(view))
    {
        DDockWindow *dock = toolWindow(m_docks[view]);
        dock->raiseWidget(view);
    }
    else if (m_widgets.contains(view) && m_widgetTabs.contains(view))
        m_widgetTabs[view]->showPage(view);
}

void Kyzis::lowerView(QWidget * /*view*/) {
    //nothing to do
}

void Kyzis::gotoNextWindow() {
    if ((m_activeTabWidget->currentPageIndex() + 1) < m_activeTabWidget->count())
        m_activeTabWidget->setCurrentPage(m_activeTabWidget->currentPageIndex() + 1);
    else
        m_activeTabWidget->setCurrentPage(0);
}

void Kyzis::gotoPreviousWindow() {
    if ((m_activeTabWidget->currentPageIndex() - 1) >= 0)
        m_activeTabWidget->setCurrentPage(m_activeTabWidget->currentPageIndex() - 1);
    else
        m_activeTabWidget->setCurrentPage(m_activeTabWidget->count() - 1);
}

void Kyzis::gotoLastWindow() {
    //@todo implement
}

void Kyzis::closeTab() {
    actionCollection()->action("file_close")->trigger();
}

void Kyzis::setupWindowMenu() {
    // get the xmlgui created one instead
    m_windowMenu = qFindChild<KMenu *>(main(), QLatin1String("window"));

    if (!m_windowMenu)
    {
        kDebug() << "Couldn't find the XMLGUI window menu. Creating new." << endl;

        m_windowMenu = new KMenu(main());
        menuBar()->insertItem(i18n("&Window"), m_windowMenu);
    }

    actionCollection()->action("file_close")->plug(m_windowMenu);
    actionCollection()->action("file_close_all")->plug(m_windowMenu);
    actionCollection()->action("file_closeother")->plug(m_windowMenu);

    QObject::connect(m_windowMenu, SIGNAL(activated(int)), this, SLOT(openURL(int)));
    QObject::connect(m_windowMenu, SIGNAL(aboutToShow()), this, SLOT(fillWindowMenu()));
}

void Kyzis::closeTab(QWidget* /*w*/) {
	//TODO
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

KMainWindow *Kyzis::main() {
    return this;
}

#include "kyzis.moc"
