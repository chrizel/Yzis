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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
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

#define VERSION_MAJOR	0
#define VERSION_MINOR	0
#define VERSION_PATCH	0

//let's use the patch number since it exists ...
#define VERSION_CHAR	"M3--"
#if QT_VERSION < 0x040000
#define VERSION_CHAR_LONG "Yzis "VERSION_CHAR" for Qt3"
#else
#define VERSION_CHAR_LONG "Yzis "VERSION_CHAR" for Qt4"
#endif
//dont change these 2 and dont use them in the code
#define VERSION_CHAR_STATE1 "(stable release)"
#define VERSION_CHAR_STATE2 "(development release - Use for testing only)"
#define VERSION_CHAR_STATE3 "(preview release - Use for testing only)"
//and change/use this one in the code
#define VERSION_CHAR_ST VERSION_CHAR_STATE2
#define VERSION_CHAR_DATE "SVN>2005-01-02"

#if QT_VERSION < 0x040000
#define qMax QMAX
#define qMin QMIN
#endif

//visibility of the option
enum option_t {
	global_opt,
	buffer_opt,
	view_opt
};

//visibility of the option
enum context_t {
    CXT_SESSION,
    CXT_BUFFER,
    CXT_VIEW,
};

//kind of value stored by the option
enum value_t {
    int_t, //is an integer
    string_t, // is a string
	stringlist_t, // a , separated list of strings (for options like "listchars=space:.,tab:>,trail:-"
    //	enum_t, // is an enumeration
    bool_t, // is a boolean
    color_t, // is a color
};

enum mapping_t {
	normal = 1, // in normal mode
	cmdline = 2, // on command line
	visual = 4, // in visual mode
	pendingop = 8, // waiting for an operator
	insert = 16, // insert mode
};

#endif /* YZIS_H */

