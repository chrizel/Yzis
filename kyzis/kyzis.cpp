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
#include "kyzis.moc"

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
#include "document.h"
#include "debug.h"

Kyzis::Kyzis(QDomElement& dockConfig, KMdi::MdiMode mode)
	: KMdiMainFrm(0L,"mdiApp",mode), DCOPObject( "Kyzis" ),
	m_dockConfig( dockConfig ), mBuffers( 0 ), mViews( 0 )
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
	m_currentPart->openURL(url);
	//m_part->openURL( url );
}

void Kyzis::setupActions() {
	KStdAction::openNew(this, SLOT(fileNew()), actionCollection());
	KStdAction::open(this, SLOT(fileOpen()), actionCollection());

	KStdAction::quit(kapp, SLOT(quit()), actionCollection());
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
	KKeyDialog::configureKeys(actionCollection(), "kyzis_shell.rc");
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
		if ( m_currentPart->url().isEmpty() && ! m_currentPart->isModified() ) {
			// we open the file in this window...
			load( url );
		} else {
			//FIXME (download)
			createBuffer( url.url() );
		}
	}
}

void Kyzis::createBuffer(const QString& path) {
		kdDebug() << "Kyzis::createBuffer " << path << endl;
		KLibFactory *factory = KLibLoader::self()->factory("libkyzispart");
		if (!factory) {
			yzDebug() << "Kyzis::createBuffer() called with no factory, discarding" << endl;
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
			m_currentPart = m_part;
			KMdiChildView *view = createWrapper( m_part->widget(), QString::number( mViews ), path );
			m_part->widget()->setFocus();
			addWindow( view );
			viewList[mViews-1] = view;
			createGUI(m_part);
			load( KURL( path ) );
		}
}

void Kyzis::closeView() {
	closeActiveView();	
}

void Kyzis::setCaption( int tab, const QString& caption ) {
	kdDebug() << "setCaption : " << caption << endl;
	KMdiChildView *v = viewList[tab];
	if ( v ) {
		v->setCaption(caption);
		v->setTabCaption(caption);
	}
}

