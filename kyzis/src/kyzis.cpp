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
 * $Id$
 */
#include "kyzis.h"

#include <kkeydialog.h>
#include <kshortcut.h>
#include <kconfig.h>
#include <kurl.h>

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

#include <ktexteditor/configinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <kmultitabbar.h>

#include "document.h"
#include "viewid.h"

// #include "configdialog.h"

//#include "kyzis/document.h"
//#include "ktexteditor/document.h"
//class KTextEditor::View;

Kyzis *Kyzis::me = NULL;

Kyzis::Kyzis(QDomElement& dockConfig, KMdi::MdiMode mode, const QString& keys)
	: KMdiMainFrm(0L,"mdiApp",mode), DCOPObject( "Kyzis" ),
	m_dockConfig( dockConfig ),
	mBuffers( 0 ), mViews( 0 )
{
	m_initialCommand = keys;
	resize( 700, 480 );
//	setMinimumSize( 200, 200 );
	mConsole = NULL;
//	setToolviewStyle(KMultiTabBar::KDEV3ICON);
	dockManager->setReadDockConfigMode(KDockManager::RestoreAllDockwidgets);

	if ( m_dockConfig.hasChildNodes() ) {
		readDockConfig(m_dockConfig);
	}

	dockManager->finishReadDockConfig();
	setMenuForSDIModeSysButtons( menuBar() );
	setManagedDockPositionModeEnabled(true);

	setXMLFile( "kyzis_shell.rc" );
	setupActions();
	createShellGUI( true );
	setWindowMenu();

	//setAutoSaveSettings();
	// call it as last thing, must be sure everything is already set up ;)
	setAutoSaveSettings ("MainWindow Settings");

	// init with more usefull size, stolen from konq :)
//	if ( !initialGeometrySet() && !kapp->config()->hasGroup("MainWindow Settings"))

    me = this;
}

Kyzis::~Kyzis() {
	writeDockConfig(m_dockConfig);
	//delete m_toolbarAction;
	delete m_konsoleAction;
	delete mConsole;
}

void Kyzis::init () {
	if (m_initialCommand.length()) {
		YZSession::me->sendMultipleKeys(m_initialCommand);
	}
}

void Kyzis::resizeEvent( QResizeEvent *e) {
   KMdiMainFrm::resizeEvent( e );
   setSysButtonsAtMenuPosition();
}

void Kyzis::load(const KURL& url) {
	kdDebug() << "load " << url << endl;
	KParts::ReadWritePart *p = getCurrentPart();
	if ( p ) p->openURL(url);
	//XXX else
}

void Kyzis::setupActions() {
	KStdAction::openNew(this, SLOT(fileNew()), actionCollection())->setShortcut(KShortcut::null());
	KStdAction::open(this, SLOT(fileOpen()), actionCollection());

	KStdAction::quit(this, SLOT(fileQuit()), actionCollection());

	KStdAction::preferences(this, SLOT(preferences()), actionCollection());

	m_openRecentAction = KStdAction::openRecent(this, SLOT(openURL(const KURL&)),
		actionCollection() );
	m_openRecentAction->setWhatsThis( i18n("Opens recently opened file.") );
	m_openRecentAction->loadEntries( kapp->config(), "RecentFiles" );

	if ( !isFakingSDIApplication() ) {
//		menuBar()->insertItem(i18n( "&Window" ), windowMenu(), -1, menuBar()->count()-2);
//		menuBar()->insertItem("&Docking", dockHideShowMenu() );//, -1 , menuBar()->count()-2);
	}

	//m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
	m_konsoleAction = new KToggleAction( i18n("Show &Konsole"), SmallIcon( "konsole" ), 0, this, SLOT(showKonsole()), actionCollection(), "show_konsole" );

	//KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	//KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
}

void Kyzis::setWindowMenu() {
	QPopupMenu *menu = ( QPopupMenu* ) menuBar()->child(  "window", "KPopupMenu" );
	if ( menu )
		QObject::connect( menu , SIGNAL( aboutToShow() ), this, SLOT( fillWindowMenu() ) );
	else menuBar()->insertItem(i18n( "&Window" ), windowMenu(), -1, menuBar()->count()-1);
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
			createBuffer();
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
	KURL url = KFileDialog::getOpenURL( QString::null, QString::null, this );
	if (url.isEmpty())
		return;
	openURL(url);
}

void Kyzis::openURL(const KURL &url)
{
	if (url.isEmpty())
	{
		fileOpen();
		return;
	}

	m_openRecentAction->addURL(url);
	m_openRecentAction->saveEntries( kapp->config(), "RecentFiles" );

	// About this function, the style guide (
	// http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
	// says that it should open a new window if the document is _not_
	// in its initial state.  This is what we do here..
	if ( getCurrentPart() && getCurrentPart()->url().isEmpty() && ! getCurrentPart()->isModified() )
	{
		// we open the file in this window...
		load( url );
	} else {
		createBuffer( url.url() );
	}
}

QString Kyzis::createBuffer(const QString& path) {
		kdDebug() << "Kyzis::createBuffer " << path << endl;
		KLibFactory *factory = KLibLoader::self()->factory("libkyzispart");
		if (!factory) {
			kdDebug() << "Kyzis::createBuffer() called with no factory, discarding" << endl;
			return QString::null;
		}
		// now that the Part is loaded, we cast it to a Part to get
		// our hands on it
		QStringList list;
		//buffer number , view number
		list << QString::number( mBuffers++ ) << QString::number( mViews++ );

		KParts::ReadWritePart * part = static_cast<KParts::ReadWritePart *>(factory->create(this, "kyzispart", "KParts::ReadWritePart", list ));
		KYZisDoc *doc = static_cast<KYZisDoc*>(part);

		if (part)
		{
			if ( path == QString::null ) {
				doc->openNewFile();
			} else {
				doc->load( path );
			}
			
			kdDebug() << "Yzis part successfully loaded" << endl;
			KMdiChildView *mdi = createWrapper( part->widget(), QString::number( mViews ), doc->fileName() );
			part->widget()->setFocus();
			addWindow( mdi );

            // doc already had a view associated with it in factory->create()
            YZView *view = doc->firstView();
			KView v = { mdi , part };
			viewList[view->getId()] = v;
			createGUI(part);
		}
		
		if ( doc ) {
			return doc->fileName();
		} else {
			return QString::null;
		}
}

void Kyzis::createView( /*const KTextEditor::Document &doc*/ ) {
		KParts::ReadWritePart *part = getCurrentPart();
		KTextEditor::Document *doc = static_cast<KTextEditor::Document*>(part);
		KYZisDoc *kdoc = static_cast<KYZisDoc*>(doc);
		KTextEditor::View *kv = doc->createView(this,"view"+QString::number(mViews++));
		QString filename = kdoc->fileName().section("/", -1);
		KMdiChildView *view = createWrapper( kv, QString::number( mViews - 1 ), filename );
		kv->setFocus();
		addWindow( view );
		KView v = { view , part };
		kdDebug() << "Adding new view " << QString::number(mViews - 1) << endl;
		viewList[YZViewId(mViews-1)] = v;
		createGUI(part);
}

void Kyzis::setCaption( unsigned int tab, const QString& caption ) {
	// Parse out the filename.
	QString filename = caption.section("/", -1);
	YZViewId id( tab );
	
	if ( viewList.contains( id ) ) {
		viewList[ id ].v->setCaption(filename);
		viewList[ id ].v->setTabCaption(filename);
	}
	KMainWindow::setCaption( caption );
}

void Kyzis::closeView(const YZViewId &id) {
//	closeActiveView();
	kdDebug() << "Main : Close view " << id << endl;
	if ( viewList.contains( id ) ) {
		kdDebug() << "Closing view from main app " << id << endl;
		closeWindow(viewList[id].v);
        viewList.remove( id );
	}
}

KParts::ReadWritePart* Kyzis::getCurrentPart() {
	kdDebug() << "getCurrentPart" << endl;
	QMap<YZViewId, KView>::Iterator it = viewList.begin(), end = viewList.end();
	for ( ; it != end; ++it ) {
		if ( it.data().v == activeWindow() ) {
			kdDebug() << "Found part at index " << it.key() << endl;
			return it.data().p;
		}
	}
	if ( viewList.size() > 0 ) {
		return (*viewList.begin()).p;
	} else {
		return NULL;
	}
}

void Kyzis::childWindowCloseRequest( KMdiChildView *v ) {
	kdDebug() << "childWindowCloseRequest" << endl;
	QMap<YZViewId, KView>::Iterator it = viewList.begin(), end = viewList.end();
	for ( ; it != end; ++it ) {
		if ( it.data().v == v ) {
			kdDebug() << "Found view at index " << it.key() << endl;
			viewList.remove( it ); //remove the corresponding part (no delete here)
		}
	}
	KMdiMainFrm::childWindowCloseRequest( v );
}

bool Kyzis::queryClose() {
	QMap<YZViewId, KView>::Iterator it = viewList.begin(), end = viewList.end();
	for ( ; it != end; ++it ) {
		if ( it.data().p->isModified() ) {
			int msg = KMessageBox::warningYesNoCancel(this, QString("The file '%1' has been modified but not saved, do you want to save it ?" ).arg( it.data().p->url().prettyURL() ), "Close Document", KStdGuiItem::save(), KStdGuiItem::discard() );
			if ( msg == KMessageBox::Cancel ) return false;
			if ( msg == KMessageBox::Yes )
				it.data().p->save(); //automatically popups saveAs dialog if needed
		}
	}

	return true;
}

void Kyzis::showKonsole() {
	if ( m_konsoleAction->isChecked() ) {
		if ( mConsole ) mConsole->parentWidget()->show();
		else {
			mConsole = new Konsole(this, "konsole");
			addToolView(KDockWidget::DockBottom, mConsole, SmallIcon("konsole"), i18n("Terminal"));
		}
	} else {
		if ( mConsole ) {
			deleteToolWindow(mConsole);
			mConsole=0;
		}
	}
}

KMdiToolViewAccessor * Kyzis::addToolView(KDockWidget::DockPosition position, QWidget *widget, const QPixmap& icon, const QString& sname, const QString& tabToolTip, const QString& tabCaption) {
	widget->setIcon(icon);
	widget->setCaption(sname);
	return addToolWindow(widget,position,getMainDockWidget(), 25, tabToolTip, tabCaption);
}

void Kyzis::preferences() {
    KTextEditor::ConfigInterface *conf = dynamic_cast<KTextEditor::ConfigInterface*>(getCurrentPart());
    if (!conf)
        return;
    conf->configDialog();
    conf->writeConfig();
}

#include "kyzis.moc"
