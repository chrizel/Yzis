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

/* System */

/* Qt */

/* yzis */
#include "yzismacros.h"

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

#ifdef SAFE_MODE
#define YZIS_SAFE_MODE
#else
#define YZIS_SAFE_MODE if(0)
#endif

/** Standard namespace for yzis special types */
namespace yzis {

/** Possible scope of an option */
enum scope_t {
	default_scope, //!< XXX ???
	global_scope,  //!< global to all buffers
	local_scope,   //!< local to a buffer or a view
};

/** visibility of an option */
enum context_t {
	ctx_none,   //!< no visibility
	ctx_session,//!< session visibility (global)
	ctx_buffer, //!< visbility to the buffer
	ctx_view    //!< visiblity to the view
};

/** kind of value stored by an option */
enum value_t {
	invalid_t,  //!< No value yet
	integer_t,  //!< Integer
	string_t,   //!< String
	list_t,     //!< List of string
	boolean_t,  //!< Boolean
	map_t,      //!< Dictionary of string to string
	color_t,    //!< Color
};

/** Different modes available for a mapping */
enum mapping_t {
	normal      = 1, //!< in normal mode
	cmdline     = 2, //!< on command line
	visual      = 4, //!< in visual mode
	pendingop   = 8, //!< waiting for an operator
	insert      = 16,//!< insert mode
};

};

#endif /* YZIS_H */

