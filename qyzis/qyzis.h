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

#include <QApplication>
#include <QMainWindow>
#include <QMap>
class QTabWidget;
class QResizeEvent;

#include "debug.h"
#include "session.h"

class QYView;

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author Yzis Team <yzis-dev@yzis.org>
 */

class QYzis : public QMainWindow, public YSession
{
    Q_OBJECT
public:
    /**
    * Constructs a QYzis widget and initialise the YSession instance.
    * @param w parent widget
     */
    QYzis(QWidget *w = 0);

    /**
     * Default Destructor
     */
    virtual ~QYzis();

    // ===============[ GUI interface of Yzis ]===================
    bool guiQuit(int errorCode);
    void guiSplitHorizontally(YView *view);
    void guiPopupMessage( const QString& message );
    bool guiPromptYesNo(const QString& title, const QString& message);
    int guiPromptYesNoCancel(const QString& title, const QString& message);
    virtual void guiSetClipboardText( const QString& text, Clipboard::Mode mode );
    YView *guiCreateView( YBuffer* buffer );
    void guiDeleteView( YView *view );
    void guiChangeCurrentView( YView* );

    void viewFilenameChanged( QYView * view, const QString & filename );


private slots:
    void slotFileNew();
    void slotFileOpen();
    void slotFileQuit();
    void slotPreferences();
    void slotAbout();

    /** To be called by single shot timer, when the gui is ready
      * and the Qt event loop is running.
    */
    void slotFrontendGuiReady();

protected: 
    void openURL( const QString & );
    void setupActions();
    void applyConfig();
    void closeView();

    QTabWidget * mTabWidget;


};

YDebugStream& operator<<( YDebugStream& out, const Qt::KeyboardModifiers & v );
YDebugStream& operator<<( YDebugStream& out, const QSize & sz );
YDebugStream& operator<<( YDebugStream& out, const QResizeEvent & e );
YDebugStream& operator<<( YDebugStream& out, const Qt::FocusReason & e );


#endif // QYZIS_H
