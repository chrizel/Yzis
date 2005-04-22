/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Philippe Fremy <phil@freehackers.org>,
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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

#ifndef PORTABILITY_H
#define PORTABILITY_H

#ifdef YZIS_WIN32_MSVC

// boah, we are on windows
#include <windows.h>
#define PREFIX ""

// XXX Phil: I'll fix that later 
static const char * gettext( const char * s) { return s; }
#define chmod( fname , flag )
#define S_IRUSR 0 
#define S_IWUSR 0

#else 
// ahh, we are on unix
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include "config.h"
#include "translator.h"
#include "libintl.h"
#endif

#endif // PORTABILITY_H
