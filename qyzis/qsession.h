/* This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
*  Boston, MA 02111-1307, USA.
**/


#ifndef QYZIS_SESSION_H
#define QYZIS_SESSION_H

#include <QMap>

#include "session.h"

class YZBuffer;
class QYView;

class QYSession : public QObject, public YZSession
{
    Q_OBJECT

public:
    /**
     *  Creates one and the only session instance.
     *  Should be called from main() before any other yzis object
     *  construction.
     */
    static void createInstance();

    //GUI interface
    bool guiQuit(int errorCode);
    void guiDeleteBuffer ( YZBuffer *b );
    void guiSetFocusCommandLine();
    void guiSetFocusMainWindow();
    void guiSplitHorizontally(YZView *view);
    void guiPopupMessage( const QString& message );
    bool guiPromptYesNo(const QString& title, const QString& message);
    int guiPromptYesNoCancel(const QString& title, const QString& message);
    virtual void guiSetClipboardText( const QString& text, Clipboard::Mode mode );

    /**
     * Sets the parent widget for all created QYView
     * This is an ugly hack to get around the deep calls into
     * YZSession in order to create Views.  These calls cannot
     * have parent information passed around.  createPartObject
     * can look in this field to get a parent
     */
    void setViewParentWidget( QWidget *viewParent )
    {
        m_viewParent = viewParent;
    }
    //Editor Interface
    YZView *guiCreateView( YZBuffer* buffer );
    void guiDeleteView( YZView *view );

    /**
     * Opens a new buffer
     * @return the newly created buffer
     */
    YZBuffer* guiCreateBuffer();

public slots:
    /** To be called by single shot timer, when the gui is ready
      * and the Qt event loop is running.
      */
    void frontendGuiReady();

    void applyConfig();
    void closeView();

private:
    QYSession();
    QYSession(const QYSession&); // disable copy
    QYSession& operator=(const QYSession&); // disable copy
    virtual ~QYSession();

    void guiChangeCurrentView( YZView* );
public:
    QYView *lastView;
    QWidget *m_viewParent;
};

#endif // QYZIS_SESSION_H

