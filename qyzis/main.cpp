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

/* St */
#ifndef YZIS_WIN32_GCC
#include <libintl.h>
#endif
#include <locale.h>

/* Qt */
#include <QApplication>
#include <QTimer>
#include <QDateTime>

/* Yzis */
#include "libyzis/translator.h"
#include "libyzis/session.h"
#include "libyzis/view.h"
#include "debug.h"
#include "yzis.h"
#include "qyzis.h"
#include "qsession.h"
#include "buffer.h"

int main(int argc, char **argv)
{
    YZSession::initDebug( argc, argv );

    // ==============[ Create application ]=============
    QApplication app(argc, argv);
    app.setOrganizationName("Yzis");
    app.setOrganizationDomain("yzis.org");
    app.setApplicationName("QYzis");

    // ==============[ create session ]=============
    QYSession::createInstance();
    QYzis* mw = new QYzis();
    mw->show();

    YZSession::self()->parseCommandLine( argc, argv );
    //YZSession::self()->frontendGuiReady();
    QTimer::singleShot(0, static_cast<QYSession*>( YZSession::self() ), SLOT(frontendGuiReady()) );

    // ==============[ let's rock ]=============

    return app.exec();
}

