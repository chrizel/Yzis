/*
 * $Id$
 * Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
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
#include "document.h"

Kyzis::Kyzis(QDomElement& dockConfig, KMdi::MdiMode mode)
	: KMdiMainFrm(0L,"mdiApp",mode), DCOPObject( "Kyzis" ),
	m_dockConfig( dockConfig ) 
{
	setIDEAlModeStyle( 1 );
	dockManager->setReadDockConfigMode(KDockManager::RestoreAllDockwidgets);

	setupActions();
	
	if ( m_dockConfig.hasChildNodes() ) {
		readDockConfig(m_dockConfig);
	}

	KLibFactory *factory = KLibLoader::self()->factory("libkyzispart");
	KParts::ReadWritePart *m_part;
	if (factory) {
		m_part = static_cast<KParts::ReadWritePart *>(factory->create(this, "kyzis_part", "KParts::ReadWritePart" ));
		if (m_part) {
			kdDebug() << "Yzis part successfully loaded" << endl;
			m_currentPart = m_part;
			KMdiChildView *view = createWrapper( m_part->widget(), "new buffer" , "buffer1" );
			addWindow( view );
			createGUI(m_part);
		}
	} else {
		KMessageBox::error(this, "Could not find our Part!");
		kapp->quit();
		return;
	}

	dockManager->finishReadDockConfig();
	setMenuForSDIModeSysButtons( menuBar() );
	
}
	
#if 0
		setXMLFile("kyzis_shell.rc");

		// then, setup our actions
		setupActions();


		// this routine will find and load our Part.  it finds the Part by name
		// which is a bad idea usually.. but it's alright in this case since our
		// Part is made for this Shell
		KLibFactory *factory = KLibLoader::self()->factory("libkyzispart");
		KParts::ReadWritePart *m_part;
		dock = createDockWidget("mainDock", 0L);
		if (factory)
		{
			// now that the Part is loaded, we cast it to a Part to get
			// our hands on it
			m_part = static_cast<KParts::ReadWritePart *>(factory->create(this, "kyzis_part", "KParts::ReadWritePart" ));
			if (m_part)
			{
				partsList.append(m_part);
				m_currentPart = m_part;

				// tell the KParts::MainWindow that this is indeed the main widget
				//setCentralWidget(m_part->widget());
				dock->setWidget(m_part->widget());
				//dock->setToolTipString();

				// and integrate the part's GUI with the shell's
				createGUI(m_part);
			}
		}
		else
		{
			// if we couldn't find our Part, we exit since the Shell by
			// itself can't do anything useful
			KMessageBox::error(this, "Could not find our Part!");
			kapp->quit();
			// we return here, cause kapp->quit() only means "exit the
			// next time we enter the event loop...
			return;
		}
		setMainDockWidget(dock);

		// apply the saved mainwindow settings, if any, and ask the mainwindow
		// to automatically save settings if changed: window size, toolbar
		// position, icon size, etc.
		setAutoSaveSettings();
#endif

Kyzis::~Kyzis() {
	writeDockConfig(m_dockConfig);
	delete m_toolbarAction;
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

	m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());

	KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
}

void Kyzis::fileNew() {
	// this slot is called whenever the File->New menu is selected,
	// the New shortcut is pressed (usually CTRL+N) or the New toolbar
	// button is clicked

	// About this function, the style guide (
	// http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
	// says that it should open a new window if the document is _not_
	// in its initial state.  This is what we do here..
	//XXX NOPE
	if ( ! m_currentPart->url().isEmpty() || m_currentPart->isModified() ) {
//		FIXME
//		(new Kyzis)->show();
	};
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
			//XXX NOPE
			// we open the file in a new window...
			/*Kyzis* newWin = new Kyzis;
			newWin->load( url );
			newWin->show();*/
		}
	}
}

void Kyzis::createBuffer(const QString& path) {
		kdDebug() << "Kyzis::createBuffer " << path << endl;
		KLibFactory *factory = KLibLoader::self()->factory("libkyzispart");
		if (factory)
		{
			// now that the Part is loaded, we cast it to a Part to get
			// our hands on it
			KParts::ReadWritePart * m_part = static_cast<KParts::ReadWritePart *>(factory->create(this, "kyzis_part", "KParts::ReadWritePart" ));

			if (m_part)
			{
				kdDebug() << "Yzis part successfully loaded" << endl;
				m_currentPart = m_part;
				KMdiChildView *view = createWrapper( m_part->widget(), "buffer-" + path , path );
				addWindow( view );
				createGUI(m_part);
				load( KURL( path ) );
			}
		}
}
