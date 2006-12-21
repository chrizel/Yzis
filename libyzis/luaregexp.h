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

#ifndef YZ_LUA_REGEXP
#define YZ_LUA_REGEXP

extern "C" {
#include <lua.h>
}

/** Regexp class for lua.
 *
 * Lua does not feature a builtin regexp support, so this class provides one.
 * The support is entirely based on Qt QRegexp. We use lua syntaxic sugar to
 * build an object like interface on top of a table.
 *
 * See \ref QRegexp for the regexp syntax.
 *
 * Lua code example:
 * re = Regexp:create( 'my_re' )
 * print(re:match('XXmy_re')) --> true
 * print(re:match('no match')) --> false
 * print(re:matchIndex('XXmy_re')) --> 2
 * print(re:matchIndex('no match')) --> -1
 */
class YZLuaRegexp {

	public:

		/** Register the regexp functions to lua */
		static void registerLuaRegexp(lua_State *L);

		/** 
		 * Create a regexp (based on QRegexp)
		 * Argument 1 : a string for the regexp
		 * Returns: a table that can be used as a regexp object:
		 *
		 */
		static int Regexp_create(lua_State *L);

		/** 
		 * Match a regexp with a string.
		 * Argument 1: string to match
		 * Returns: true if matched, false else
		 */
		static int Regexp_match(lua_State *L);

		/** 
		 * Match a regexp with a string.
		 * Argument 1: string to match
		 * Returns: index of matched, -1 else
		 */
		static int Regexp_matchIndex(lua_State *L);

		/** 
		  * Called by lua when the regexp is about to be deleted
		 */
		static int Regexp_userdata_finalize(lua_State *L);

		/**
		 * Set regexp greediness
		 */
		static int Regexp_setMinimal(lua_State *L);

		/**
		 * Set regexp case sensitiveness
		 */
		static int Regexp_setCaseSensitive(lua_State *L);

		/**
		 * Return positions of the matches
		 * Arguments: a number, referring to the inside regexp match
		 * 		for 0, returns the position of the whole regexp
		 * Returns: the position of the regexp being matched, or -1 if nothing
		 * matched at that position
		 */
		static int Regexp_pos(lua_State *L);

		/**
		 * Returns the number of captured strings (0 when no match)
		 */
		static int Regexp_numCaptures(lua_State *L);

		/**
		 * Returns the text captured by the regexp
		 * Argument: the index of the regexp (0 for the whole regexp)
		 */
		static int Regexp_captured(lua_State *L);

		/**
		 * Returns a string where the occurences of Regexp have been
		 * replaced by the provided string.
		 * Argument: 
		 * - a string to be replaced
		 * - a replacement string (which can use \\1)
		 * - optional: number of replacement that should occur
		 */
		static int Regexp_replace(lua_State *L);

		/** Return the pattern of the regexp as a string */
		static int Regexp_pattern(lua_State *L);

};

#endif // YZ_LUA_REGEXP
