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

#include <libintl.h>

/* Qt */

/* yzis */
#include "yzismacros.h"

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 0

//let's use the patch number since it exists ...
#define VERSION_CHAR "1.0-alpha1"
#define VERSION_CHAR_LONG "Yzis "VERSION_CHAR" for Qt4" 

// either 

//#define VERSION_CHAR_ST "(stable release)"
//#define VERSION_CHAR_ST "(development release - Use for testing only)"
#define VERSION_CHAR_ST "(preview release - Use for testing only)" 

// if defined, will be displayed in the "Intro" Mode
#define VERSION_CHAR_DATE "mercurial>2008-06-25"

#define _(a) QString::fromUtf8(gettext(a))

#ifdef SAFE_MODE
#define YZIS_SAFE_MODE
#else
#define YZIS_SAFE_MODE if(0)
#endif

/** Standard namespace for yzis special types */
namespace yzis
{

/** Possible scope of an option */
enum OptScope {
    ScopeDefault,    //!< XXX ???
    ScopeGlobal,     //!< global to all buffers
    ScopeLocal,      //!< local to a buffer or a view
};

/** visibility of an option */
enum OptContext {
    ContextNone,     //!< no visibility
    ContextSession,  //!< session visibility (global)
    ContextBuffer,   //!< visbility to the buffer
    ContextView     //!< visiblity to the view
};

/** kind of value stored by an option */
enum OptType {
    TypeInvalid,   //!< No value yet
    TypeInt,       //!< Integer
    TypeString,    //!< String
    TypeList,      //!< List of string
    TypeBool,      //!< Boolean
    TypeMap,       //!< Dictionary of string to string
    TypeColor,     //!< Color
};

/** Different modes available for a mapping */
enum MapMode {
    MapNormal = 1,   //!< in normal mode
    MapCmdline = 2,   //!< on command line
    MapVisual = 4,   //!< in visual mode
    MapPendingOp = 8,   //!< waiting for an operator
    MapInsert = 16,  //!< insert mode
};

};

namespace Clipboard
{
enum Mode {
    Clipboard,
    Selection
};
};

#endif /* YZIS_H */

