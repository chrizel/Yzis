/*
    Copyright (c) 2003-2004 Mickael Marchand <mikmak@yzis.org>

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
#include <kconfig.h>
#include <kurl.h>

#include <kedittoolbar.h>

#include <kaction.h>
#include <kstdaction.h>

#include <klibloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include <ktexteditor/configinterface.h>

// #include "configdialog.h"

//#include "kyzis/document.h"
//#include "ktexteditor/document.h"
//class KTextEditor::View;

Kyzis::Kyzis(QDomElement& dockConfig, KMdi::MdiMode mode)
	: KMdiMainFrm(0L,"mdiApp",mode), DCOPObject( "Kyzis" ),
	m_dockConfig( dockConfig ),
	mBuffers( 0 ), mViews( 0 )
{
	setIDEAlModeStyle( 1 );
	dockManager->setReadDockConfigMode(KDockManager::RestoreAllDockwidgets);
	
	if ( m_dockConfig.hasChildNodes() ) {
		readDockConfig(m_dockConfig);
	}

	dockManager->finishReadDockConfig();
	setMenuForSDIModeSysButtons( menuBar() );

	setXMLFile( "kyzis_shell.rc" );
	setupActions();
	createShellGUI( true );
	setWindowMenu();

	setAutoSaveSettings();
	
}
	
Kyzis::~Kyzis() {
	writeDockConfig(m_dockConfig);
	//delete m_toolbarAction;
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
	KStdAction::openNew(this, SLOT(fileNew()), actionCollection());
	KStdAction::open(this, SLOT(fileOpen()), actionCollection());

	KStdAction::quit(kapp, SLOT(quit()), actionCollection());
	
	KStdAction::preferences(this, SLOT(preferences()), actionCollection());

	
	if ( !isFakingSDIApplication() ) {
//		menuBar()->insertItem(i18n( "&Window" ), windowMenu(), -1, menuBar()->count()-2);
//		menuBar()->insertItem("&Docking", dockHideShowMenu() );//, -1 , menuBar()->count()-2);
	}

	//m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());

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

void Kyzis::fileOpen() {
	// this slot is called whenever the File->Open menu is selected,
	// the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
	// button is clicked
	KURL url = KFileDialog::getOpenURL( QString::null, QString::null, this );

	if (url.isEmpty() == false) {
		// About this function, the style guide (
		// http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
		// says that it should open a new window if the document is _not_
		// in its initial state.  This is what we do here..
		if ( getCurrentPart() && getCurrentPart()->url().isEmpty() && ! getCurrentPart()->isModified() ) {
			// we open the file in this window...
			load( url );
		} else {
			createBuffer( url.url() );
		}
	}
}

void Kyzis::createBuffer(const QString& path) {
		kdDebug() << "Kyzis::createBuffer " << path << endl;
		KLibFactory *factory = KLibLoader::self()->factory("libkyzispart");
		if (!factory) {
			kdDebug() << "Kyzis::createBuffer() called with no factory, discarding" << endl;
			return;
		}
		// now that the Part is loaded, we cast it to a Part to get
		// our hands on it
		QStringList list;
		//buffer number , view number 
		list << QString::number( mBuffers++ ) << QString::number( mViews++ );

		KParts::ReadWritePart * m_part = static_cast<KParts::ReadWritePart *>(factory->create(this, "kyzispart", "KParts::ReadWritePart", list ));

		if (m_part)
		{
			kdDebug() << "Yzis part successfully loaded" << endl;
			KMdiChildView *view = createWrapper( m_part->widget(), QString::number( mViews ), path );
			m_part->widget()->setFocus();
			addWindow( view );
			KView v = { view , m_part };
			viewList[mViews-1] = v;
			createGUI(m_part);
			load( KURL( path ) );
		}
}

void Kyzis::setCaption( int tab, const QString& caption ) {
	kdDebug() << "setCaption : " << caption << endl;
	if ( viewList.contains( tab ) ) {
		viewList[ tab ].v->setCaption(caption);
		viewList[ tab ].v->setTabCaption(caption);
	}
}

void Kyzis::closeView(int Id) {
//	closeActiveView();
	if ( viewList.contains( Id ) ) {
		kdDebug() << "Closing view from main app " << Id << endl;
		closeWindow(viewList[Id].v);
	}
}

KParts::ReadWritePart* Kyzis::getCurrentPart() {
	kdDebug() << "getCurrentPart" << endl;
#if 0
	QMap<int,KMdiChildView*>::Iterator it;
	for ( it = viewList.begin(); it != viewList.end(); it++ ) {
		if ( it.data() == activeWindow() ) {
			kdDebug() << "Found part at index " << it.key() << endl;
			return partList[it.key()];
		}
	}
	return partList[ 0] ;
#endif
	QMap<int,KView>::Iterator it;
	for ( it = viewList.begin(); it != viewList.end(); it++ ) {
		if ( it.data().v == activeWindow() ) {
			kdDebug() << "Found part at index " << it.key() << endl;
			return it.data().p;
		}
	}
	return viewList[ 0 ].p;
}

void Kyzis::childWindowCloseRequest( KMdiChildView *v ) {
	kdDebug() << "childWindowCloseRequest" << endl;
#if 0
	QMap<int,KMdiChildView*>::Iterator it;
	for ( it = viewList.begin(); it != viewList.end(); it++ ) {
		if ( it.data() == v ) {
			kdDebug() << "Found view at index " << it.key() << endl;
			partList.remove( it.key() ); //remove the corresponding part (no delete here)
		}
	}
#endif
	QMap<int,KView>::Iterator it;
	for ( it = viewList.begin(); it != viewList.end(); it++ ) {
		if ( it.data().v == v ) {
			kdDebug() << "Found view at index " << it.key() << endl;
			viewList.remove( it ); //remove the corresponding part (no delete here)
		}
	}
	KMdiMainFrm::childWindowCloseRequest( v );
}

bool Kyzis::queryClose() {
	QMap<int,KView>::Iterator it;
	for ( it = viewList.begin(); it != viewList.end(); it++ ) {
		if ( it.data().p->isModified() ) {
			int msg = KMessageBox::warningYesNoCancel(this, QString("The file '%1' has been modified but not saved, do you want to save it ?" ).arg( it.data().p->url().prettyURL() ), "Close Document", KStdGuiItem::save(), KStdGuiItem::discard() );
			if ( msg == KMessageBox::Cancel ) return false;
			if ( msg == KMessageBox::Yes ) 
				it.data().p->save(); //automatically popups saveAs dialog if needed
		}
	}

	return true;
}

void Kyzis::preferences() {
    KTextEditor::ConfigInterface *conf = dynamic_cast<KTextEditor::ConfigInterface*>(getCurrentPart());
    if (!conf)
        return;

    conf->configDialog();
    conf->writeConfig();
}


#include "kyzis.moc"
