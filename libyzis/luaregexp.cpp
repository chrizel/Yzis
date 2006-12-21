/* This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <mikmak@yzis.org>
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

#include <QRegExp>

#include "portability.h"
#include "luaengine.h"
#include "luaregexp.h"
#include "debug.h"


/*
 * TODO:
 * - invert line/col arguments
 * - test every argument of the functions
 * - find how to add file:line info to error messages
 * - override print() in lua for yzis
 * - clear the lua stack properly
 * - arguments to :source must be passed as argv
 * - add missing function from vim
 * - clean up all the lua calling functions
 * - yzpcall does not need to take nresult argument
 * - use correct printf api
 * - yzpcall should display the type of lua error when error occurs
 * - understand and document the highlight command
 *
 */

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}


// ========================================================
//
//                     Regexp for lua
//
// ========================================================

void YZLuaRegexp::registerLuaRegexp(lua_State * L)
{
	lua_register(L,"Regexp_create", Regexp_create);
	lua_register(L,"Regexp_matchIndex", Regexp_matchIndex);
	lua_register(L,"Regexp_match", Regexp_match);
	lua_register(L,"Regexp_setCaseSensitive", Regexp_setCaseSensitive);
	lua_register(L,"Regexp_setMinimal", Regexp_setMinimal);
	lua_register(L,"Regexp_pos", Regexp_pos);
	lua_register(L,"Regexp_numCaptures", Regexp_numCaptures);
	lua_register(L,"Regexp_captured", Regexp_captured);
	lua_register(L,"Regexp_replace", Regexp_replace);
	lua_register(L,"Regexp_pattern", Regexp_pattern);
	lua_register(L,"Regexp_userdata_finalize", Regexp_userdata_finalize);

	QString regexpLuaCode = ""
"Regexp = { 					\n"
"    setCaseSensitive = Regexp_setCaseSensitive		\n"
"    ,setMinimal = Regexp_setMinimal		\n"
"    ,match = Regexp_match		\n"
"    ,replace = Regexp_replace		\n"
"    ,pattern = Regexp_pattern		\n"
"    ,matchIndex = Regexp_matchIndex		\n"
"	 ,pos = Regexp_pos			\n"
"	 ,numCaptures = Regexp_numCaptures	\n"
"	 ,captured = Regexp_captured			\n"
"}								\n"

"Regexp_Class_mt = { 			\n"
"    __call  = Regexp_create	\n"
"}								\n"
" 								\n"
"setmetatable( Regexp, Regexp_Class_mt )	\n"
" 								\n"
"Regexp_Object_mt = { 			\n"
"    __index = Regexp,			\n"
"}								\n"
" 								\n"
" 								\n"
" 								\n"
;
	YZLuaEngine::self()->execInLua( regexpLuaCode );
}

int YZLuaRegexp::Regexp_create(lua_State *L)
{
	if (! YZLuaEngine::checkFunctionArguments(L, 2, 2, "Regexp.create", "Regexp table, pattern")) return 0;
	// discard the Regexp table
	lua_remove(L, 1);
	QString re = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	lua_pop(L,1);

	// create table
	lua_newtable( L );
	// stack: table

		// store ud
	lua_pushstring( L, "qregexp*" );
	// stack: table, "qregexp*"
	QRegExp **pRegExp = (QRegExp **) lua_newuserdata(L, sizeof( QRegExp * ) ); // store the pointer as userdata
	*pRegExp = new QRegExp(re);
	// stack: table, "qregexp*", userdata

	// create userdata metatable and fill it
	lua_newtable( L ); 
	// stack: table, "qregexp*", userdata, table
	lua_pushstring( L, "__gc" ); 
	// stack: table, "qregexp*", ud, table, "__gc"
	lua_pushstring(L,"Regexp_userdata_finalize"); 
	// stack: table, "qregexp*",ud, table, "__gc", "Regexp_userdata_finalize"
	lua_gettable(L, LUA_GLOBALSINDEX);
	// stack: table, "qregexp*",ud, table, "__gc", function
	lua_rawset(L, -3 );
	// stack: table, "qregexp*",ud, table
	lua_setmetatable(L, -2);
	// stack: table, "qregexp*",ud
	lua_rawset(L, -3 );
	// stack: table

	// set Regexp_mt as metatable
	lua_pushstring(L, "Regexp_Object_mt" );
	// stack: table, "Regexp_mt"
	lua_gettable( L, LUA_GLOBALSINDEX );
	// stack: table, table Regexp_mt
	lua_setmetatable(L, -2);
	// stack: table

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return 1;
}

int YZLuaRegexp::Regexp_userdata_finalize(lua_State *L)
{
	if (! YZLuaEngine::checkFunctionArguments(L, 1, 1, "Regexp.finalize", "Regexp object")) return 0;

	QRegExp ** pRegexp = (QRegExp **) lua_touserdata(L, -1);
	QRegExp * regexp = *pRegexp;
	lua_pop(L, 1);

	//	printf("Finalizing %s\n", pRegexp->pattern().latin1() );

	delete regexp;
	*pRegexp = NULL;
	
	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaRegexp::Regexp_match(lua_State *L)
{
	if (! YZLuaEngine::checkFunctionArguments(L, 2, 2, "Regexp.match", "Regexp object, string")) return 0;

	QString s = QString::fromUtf8( (  char * ) lua_tostring (  L, 2 ) );

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));

	lua_pop(L, 3);

	lua_pushboolean( L, regexp->indexIn( s ) != -1);
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ;
}


int YZLuaRegexp::Regexp_matchIndex(lua_State *L)
{
	if (! YZLuaEngine::checkFunctionArguments(L, 2, 2, "Regexp.matchIndex", "Regexp object, string")) return 0;

	QString s = QString::fromUtf8( (  char * ) lua_tostring (  L, 2 ) );
	lua_pop(L, 2 );

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 1 );

	lua_pushnumber( L, regexp->indexIn( s ) );
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ;
}


int YZLuaRegexp::Regexp_setMinimal(lua_State *L)
{
	if (! YZLuaEngine::checkFunctionArguments(L, 2, 2, "Regexp.setMinimal", "Regexp object, boolean")) return 0;

	bool b = lua_toboolean ( L, 2 );
	lua_pop(L, 2);

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 1);

	regexp->setMinimal( b );
	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}


int YZLuaRegexp::Regexp_setCaseSensitive(lua_State *L)
{
	if (! YZLuaEngine::checkFunctionArguments(L, 2, 2, "Regexp.setCaseSensitive", "Regexp object, boolean")) return 0;

	bool b = lua_toboolean ( L, 2 );
	lua_pop(L, 2);

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 1);

	regexp->setCaseSensitivity( b ? Qt::CaseSensitive : Qt::CaseInsensitive );
	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}


int YZLuaRegexp::Regexp_pos(lua_State *L)
{
	if (! YZLuaEngine::checkFunctionArguments(L, 2, 2, "Regexp.pos", "Regexp object, index")) return 0;

	int index = ( int )lua_tonumber( L, 2 );
	lua_pop(L, 2);

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 1);

	lua_pushnumber( L, regexp->pos( index ) );
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ;
}

int YZLuaRegexp::Regexp_numCaptures(lua_State *L)
{
	if (! YZLuaEngine::checkFunctionArguments(L, 1, 1, "Regexp.numCaptures", "Regexp object")) return 0;
	lua_pop(L, 1);

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 1);

	lua_pushnumber( L, regexp->numCaptures() );
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ;
}

int YZLuaRegexp::Regexp_captured(lua_State *L)
{
	if (! YZLuaEngine::checkFunctionArguments(L, 2, 2, "Regexp.captured", "Regexp object, index")) return 0;

	int index = ( int )lua_tonumber( L, 2 );
	lua_pop(L, 2);

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 1);

	lua_pushstring( L, regexp->cap( index ).toUtf8().data() );
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ;
}

int YZLuaRegexp::Regexp_replace(lua_State *L)
{
	if (! YZLuaEngine::checkFunctionArguments(L, 3, 4, "Regexp.replace", "Regexp object, string, string, number (optional)")) return 0;

	QString s  = lua_tostring( L, 2 );
	QString replacement  = lua_tostring( L, 3 );

	int nb = -1;
	if ( lua_gettop(L) >= 4) {
		nb = ( int )lua_tonumber(L, 4);
		lua_pop(L, 1);
	}
	
	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 4);


	QString sRet = "", matched;
	int matchLen=0, matchIdx, idx=0;

	while( nb == -1 || nb-- > 0) {
		// look for regexp inside s starting at index idx
		if (regexp->indexIn(s, idx) == -1) break;
		// if regexp match, we have an index and a length
		matched = regexp->cap( 0 );
		matchIdx = regexp->pos( 0 );
		matchLen = matched.length();
		sRet += s.mid( idx, matchIdx-idx );
		sRet += matched.replace( *regexp, replacement );
		idx = matchIdx + matchLen;
	}

	sRet += s.mid( idx );

	lua_pushstring( L, sRet.toUtf8().data() );
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ;
}

int YZLuaRegexp::Regexp_pattern(lua_State *L)
{
	if (! YZLuaEngine::checkFunctionArguments(L, 1, 1, "Regexp.pattern", "Regexp object")) return 0;
	lua_pop(L, 1);

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 1);

	lua_pushstring( L, regexp->pattern().toUtf8().data() );
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ;
}

