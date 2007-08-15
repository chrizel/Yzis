/* This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
*  Copyright (C) 2007 Lothar Braun <lothar@lobraun.de>
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
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
**/

#ifndef _KYZIS_SESSION_H_
#define _KYZIS_SESSION_H_

#include <libyzis/session.h>

#include <QObject>


class KYZisSession : public YZSession, public QObject
{
public:
    static void createInstance();

    virtual void guiDeleteBuffer(YZBuffer*);
    virtual void guiSplitHorizontally(YZView*);
    virtual bool guiQuit(int);
    virtual void guiPopupMessage(const QString&);
    virtual bool guiPromptYesNo(const QString&, const QString&);
    virtual int guiPromptYesNoCancel(const QString&, const QString&);
    virtual void guiSetFocusCommandLine();
    virtual void guiSetFocusMainWindow();
    virtual void guiSetClipboardText(const QString&, Clipboard::Mode);
    virtual YZView* guiCreateView(YZBuffer*);
    virtual void guiDeleteView(YZView*);
    virtual YZBuffer* guiCreateBuffer();
    virtual void guiChangeCurrentView(YZView*);

private:
    virtual ~KYZisSession();
    KYZisSession();
    static KYZisSession* me;
};

#endif
