/* This file is part of the Yzis libraries
*  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
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
*  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
*  Boston, MA 02110-1301, USA.
**/


#ifndef NOGUI_SESSION_H
#define NOGUI_SESSION_H

#include <QObject>

class NoGuiView;

#include "libyzis/session.h"
#include "libyzis/buffer.h"

class NoGuiSession : public QObject, public YZSession
{
    Q_OBJECT

public:
    static void createInstance();


    virtual YZView* createView ( YZBuffer* buf);

    virtual YZBuffer *createBuffer(const QString& path = QString());

    virtual void guiPopupMessage( const QString& message);

    virtual void guiQuit(bool /*savePopup=true */);

    virtual void guiDeleteBuffer ( YZBuffer * );
    virtual void guiChangeCurrentView( YZView* );
    virtual void guiSetFocusCommandLine( );
    virtual void guiSetFocusMainWindow( );
    virtual bool guiQuit(int);
    virtual bool guiPromptYesNo(const QString&, const QString&);
    virtual int guiPromptYesNoCancel( const QString&, const QString& );
    virtual void guiSplitHorizontally(YZView*);
    virtual YZView *guiCreateView(YZBuffer*b);
    virtual void guiDeleteView(YZView*v);
    virtual YZBuffer* guiCreateBuffer();
    virtual void guiSetClipboardText(const QString&, Clipboard::Mode);

public slots:
    /** To be called by single shot timer, when the gui is ready
      * and the Qt event loop is running.
      */
    void frontendGuiReady();

private:
    NoGuiSession();

};

#endif // NOGUI_YZ_SESSION_H
