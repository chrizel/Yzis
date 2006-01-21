/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>
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

#ifndef YZIS_H
#define YZIS_H
/**
 * yzis.h
 *
 * Main include file for the yzis project
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <qglobal.h>
#include <yzismacros.h>
#include "portability.h"

#define VERSION_MAJOR	0
#define VERSION_MINOR	0
#define VERSION_PATCH	0

//let's use the patch number since it exists ...
#define VERSION_CHAR	"M3++"
#define VERSION_CHAR_LONG "Yzis "VERSION_CHAR" for Qt4"
//dont change these 2 and dont use them in the code
#define VERSION_CHAR_STATE1 "(stable release)"
#define VERSION_CHAR_STATE2 "(development release - Use for testing only)"
#define VERSION_CHAR_STATE3 "(preview release - Use for testing only)"
//and change/use this one in the code
#define VERSION_CHAR_ST VERSION_CHAR_STATE2
#define VERSION_CHAR_DATE "SVN>2005-12-22"

#define _(a) QString::fromUtf8(gettext(a))

enum scope_t {
	default_scope,
	global_scope,
	local_scope,
};

//visibility of the option
enum context_t {
    CXT_CONFIG, // simple entry
    CXT_SESSION,
    CXT_BUFFER,
    CXT_VIEW,
};

//kind of value stored by the option
enum value_t {
	invalid_t,
	integer_t,
	string_t,
	list_t,
	boolean_t,
	map_t,
	color_t,
};

enum mapping_t {
	normal = 1, // in normal mode
	cmdline = 2, // on command line
	visual = 4, // in visual mode
	pendingop = 8, // waiting for an operator
	insert = 16, // insert mode
};

#endif /* YZIS_H */

