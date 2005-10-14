/* This file is part of the Yzis Project
 *  Copyright (C) 2004 Mickael Marchand <marchand@kde.org>
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

/**
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gyzis.h"
#include <qtgtk/qtgtk.h>

int
main(int argc, char* argv[])
{
  Gtk::Main mainDoc(argc, argv);

  Bakery::init();
  qtgtk_init( 0 );

  GYzis* pApp = new GYzis("test");
  GYzis::set_command_line_args(argc, argv);
  pApp->init(); //Sets it up and shows it.

  mainDoc.run();

  return 0;
}
