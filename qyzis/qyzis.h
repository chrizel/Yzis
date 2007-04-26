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

#ifndef QYZIS_H
#define QYZIS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qapplication.h>
#include <qmainwindow.h>
#include <qmap.h>

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author Yzis Team <yzis-dev@yzis.org>
 */

class Qyzis : public QMainWindow
{
    Q_OBJECT
public:
    /**
	 * Constructs a Qyzis widget
	 * @param w parent widget
     */
    Qyzis(QWidget *w = 0);

    /**
     * Default Destructor
     */
    virtual ~Qyzis();

    static Qyzis *me;

    /**
     * Use this method to load whatever file/URL you have
	 * @param url the url to open
     */
    void load(const QString& url);

    virtual void embedPartView(QWidget *view, const QString &title, const QString& toolTip = QString());
    /*
    virtual void embedSelectView(QWidget *view, const QString &title, const QString &toolTip);
    virtual void embedOutputView(QWidget *view, const QString &title, const QString &toolTip);
    virtual void embedSelectViewRight(QWidget* view, const QString& title, const QString &toolTip);

    virtual void removeView(QWidget *view);
    virtual void setViewAvailable(QWidget *pView, bool bEnabled);
    virtual void raiseView(QWidget *view);
    virtual void lowerView(QWidget *view);
    */
    virtual QMainWindow *main();

public slots:
	void closeTab(QWidget *);
    void closeTab();

private slots:
    void fileNew();
    void fileOpen();
    void openURL( const QString & );
    void fileQuit();
    void preferences();
	void about();
//    void gotoNextWindow();
//    void gotoPreviousWindow();
//    void gotoFirstWindow();
//    void gotoLastWindow();
//    void raiseEditor();

private:
    void setupActions();
	virtual bool queryClose();

	int mBuffers;
	unsigned int mViews;

//	QMap<QWidget*, Q3DockWindow::Position> m_docks;
//	Q3PopupMenu *m_windowMenu;
	typedef QPair<int, QString> WinInfo;
	QList<WinInfo> m_windowList;

};

#endif // QYZIS_H
