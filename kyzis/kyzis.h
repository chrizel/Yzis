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

#ifndef KYZIS_H
#define KYZIS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kmdimainfrm.h>
#include <dcopobject.h>

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author Yzis Team <yzis-dev@yzis.org>
 */
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
    Kyzis(QDomElement& dockConfig, KMdi::MdiMode mode);

    /**
     * Default Destructor
     */
    virtual ~Kyzis();

    /**
     * Use this method to load whatever file/URL you have
	 * @param url the url to open
     */
    void load(const KURL& url);

k_dcop:
	/**
	 * Opens a new buffer
	 * @param path file to which the buffer is linked
	 */
    void createBuffer(const QString& path);

	/**
	 * Closes the activated view
	 */
	void closeView();

public slots:

protected slots:
	
protected:
	virtual void resizeEvent( QResizeEvent *e );

private slots:
    void fileNew();
    void fileOpen();
    void optionsShowToolbar();
    void optionsConfigureKeys();
    void optionsConfigureToolbars();
    void applyNewToolbarConfig();

private:
    void setupActions();
    KToggleAction *m_toolbarAction;
	QDomElement m_dockConfig;
	KParts::ReadWritePart *m_currentPart;
};

#endif // KYZIS_H
