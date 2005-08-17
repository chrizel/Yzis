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

#ifndef KYZIS_H
#define KYZIS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kmdimainfrm.h>
#include <dcopobject.h>
#include <qmap.h>

#include "konsole.h"
#include "viewid.h"

class KYZTextEditorIface;

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author Yzis Team <yzis-dev@yzis.org>
 */
class KRecentFilesAction;
class Kyzis : public KMdiMainFrm, public DCOPObject
{
	K_DCOP
    Q_OBJECT
public:
    /**
	 * Constructs a Kyzis widget
	 * @param dockConfig the configuration of dock widgets
	 * @param mode the startup MDI mode
     */
    Kyzis(QDomElement& dockConfig, KMdi::MdiMode mode, const QString& initialKeys);

    /**
     * Default Destructor
     */
    virtual ~Kyzis();

    static Kyzis *me;

    /**
     * Use this method to load whatever file/URL you have
	 * @param url the url to open
     */
    void load(const KURL& url);

	KParts::ReadWritePart* getCurrentPart();

	KMdiToolViewAccessor *addToolView(KDockWidget::DockPosition position, QWidget *widget, const QPixmap& icon, const QString& sname, const QString& tabToolTip = 0, const QString& tabCaption = 0);
	
	void createKPartGUI( KParts::ReadWritePart *part ) { createGUI( part ); }

k_dcop:
	/**
	 * Sets the caption of the tab
	 */
	void setCaption( const YZViewId &id, const QString& caption );

public slots:
	/**
	 * Enables/disables the konsole
	 */
	void showKonsole();

	void init();


protected slots:

protected:
	virtual void resizeEvent( QResizeEvent *e );
	void setWindowMenu();

private slots:
    void fileNew();
    void fileOpen();
    void openURL( const KURL& );
    void fileQuit();
    void optionsShowToolbar();
    void optionsConfigureKeys();
    void optionsConfigureToolbars();
    void applyNewToolbarConfig();
    void preferences();

private:
    void setupActions();
	virtual bool queryClose();

    KToggleAction *m_toolbarAction;
    KToggleAction *m_konsoleAction;
    KRecentFilesAction *m_openRecentAction;
	QDomElement m_dockConfig;
	int mBuffers;
	unsigned int mViews;

	Konsole *mConsole;
	QString m_initialCommand;

};

#endif // KYZIS_H
