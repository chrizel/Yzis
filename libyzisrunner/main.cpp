/*
   Copyright (c) 2004 Philippe Fremy <phil@freehackers.org>

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

#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QTimer>
#include <qtextcodec.h>

#include "libyzis/portability.h"
#include "libyzis/session.h"
#include "libyzis/view.h"
#include "libyzis/debug.h"
#include "libyzis/internal_options.h"

#include "NoGuiSession.h"

int main(int argc, char **argv)
{
    YZSession::initDebug( argc, argv );

    QCoreApplication *app = new QCoreApplication( argc, argv );

    NoGuiSession::createInstance();

    YZSession::self()->parseCommandLine( argc, argv );
    QTimer::singleShot(0, static_cast<NoGuiSession*>( YZSession::self() ), SLOT(frontendGuiReady()) );

    app->exec();

    yzDebug("liyzisrunner::main() - app finished!" );

    return 0;
}

