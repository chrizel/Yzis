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
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef KYZIS_H
#define KYZIS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <dcopobject.h>
#include <qmap.h>

#include "viewid.h"
#include "dmainwindow.h"

class KYZTextEditorIface;

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author Yzis Team <yzis-dev@yzis.org>
 */
class KRecentFilesAction;

class Kyzis : public DMainWindow, public DCOPObject
{
	K_DCOP
    Q_OBJECT
public:
    /**
	 * Constructs a Kyzis widget
	 * @param dockConfig the configuration of dock widgets
	 * @param mode the startup MDI mode
     */
    Kyzis(QWidget *w, const QString& initialKeys);

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

	void createKPartGUI( KParts::ReadWritePart *part ) { createGUI( part ); }

	virtual void embedPartView(QWidget *view, const QString &title, const QString& toolTip = QString());
    virtual void embedSelectView(QWidget *view, const QString &title, const QString &toolTip);
    virtual void embedOutputView(QWidget *view, const QString &title, const QString &toolTip);
    virtual void embedSelectViewRight(QWidget* view, const QString& title, const QString &toolTip);

    virtual void removeView(QWidget *view);
    virtual void setViewAvailable(QWidget *pView, bool bEnabled);
    virtual void raiseView(QWidget *view);
    virtual void lowerView(QWidget *view);
    virtual KMainWindow *main();


k_dcop:
	/**
	 * Sets the caption of the tab
	 */
	void setCaption( const YZViewId &id, const QString& caption );

public slots:
	void init();
	void closeTab(QWidget *);
    void closeTab();

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
    void gotoNextWindow();
    void gotoPreviousWindow();
//    void gotoFirstWindow();
    void gotoLastWindow();
//    void raiseEditor();

private:
    void setupWindowMenu();


private:
    void setupActions();
	virtual bool queryClose();

    KToggleAction *m_toolbarAction;
    KRecentFilesAction *m_openRecentAction;
	int mBuffers;
	unsigned int mViews;

	QString m_initialCommand;
	QMap<QWidget*, DDockWindow::Position> m_docks;
	KPopupMenu *m_windowMenu;
	typedef QPair<int, KURL> WinInfo;
	QList<WinInfo> m_windowList;

};

#endif // KYZIS_H
