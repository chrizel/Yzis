/*
   Copyright (c) 2003-2005 Mickael Marchand <marchand@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of version 2 of the GNU General Public
   License as published by the Free Software Foundation

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* Std */
#ifndef YZIS_WIN32_GCC
#include <libintl.h>
#endif
#include <locale.h>

/* Qt */
#include <QApplication>
#include <QTimer>
#include <QDateTime>

/* Yzis */
#include "translator.h"
#include "session.h"
#include "view.h"
#include "debug.h"
#include "buffer.h"
#include "yzis.h"

/* QYzis */
#include "qyzis.h"
#include "qysession.h"

int main(int argc, char **argv)
{
    YSession::initDebug( argc, argv );

    // ==============[ Create application ]=============
    QApplication app(argc, argv);
    app.setOrganizationName("Yzis");
    app.setOrganizationDomain("yzis.org");
    app.setApplicationName("QYzis");

    // ==============[ create session ]=============
    QYzis* qyzis = new QYzis();
    qyzis->show();

    YSession::self()->parseCommandLine( argc, argv );
    //YSession::self()->frontendGuiReady();
    QTimer::singleShot(0, qyzis, SLOT(slotFrontendGuiReady()) );

    // ==============[ let's rock ]=============

    return app.exec();
}

