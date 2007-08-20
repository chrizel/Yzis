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

extern "C"
{
#include <lua.h>
}

/** \brief Regexp class for lua.
 *
 * Lua does not feature a builtin regexp support, so this class provides one.
 * The support is entirely based on Qt <a
 * href="http://doc.trolltech.com/4.2/qregexp.html">QRegexp</a>. We use lua
 * syntaxic sugar to build an object like interface on top of a table.
 *
 * See <a href="http://doc.trolltech.com/4.2/qregexp.html">QRegexp</a> for the
 * regexp syntax.
 *
 * Internally, we have a table with a metadata for creating the regexp
 * (overloading call), a userdata that stores a pointer to a QRegexp. When the
 * Regexp is garbage-collected, Regexp_userdata_finalize() is called and the
 * pointer is deleted.
 *
 * All the methods of this class extract the arguments from the stack, extract
 * the QRegexp pointer, calls the appropriate method on QRegexp and return the
 * result.
 *
 * The function registerLuaRegexp() registers all the other function to the
 * lua engine. All the functions but this one are called directly by Lua.
 */
class YLuaRegexp
{

public:

    /** \brief Register the regexp functions to lua
     *
     * This functions register all the other methods listed in this
     * class as function callable from lua. This function is called by
     * YZLuaEngine::init() upon initialisation.
     */
    static void registerLuaRegexp(lua_State *L);

    /** \brief Create a regexp (based on QRegexp)
           *
     * \b Arguments: 
           * - string: the regexp expression
           *
     * \b Returns: a table that can be used as a regexp object.
           *
           * <b>Lua code example</b>
           * \code
           * re = Regexp( 'my_re' )
           * assertEquals( (re:match('XXmy_re')), true )
           * assertEquals( (re:match('no match')), false )
           * \endcode
     */
    static int Regexp_create(lua_State *L);

    /** \brief Match a regexp with a string.
           *
     * \b Argument: 
           * - string: the string to match
           *
     * \b Returns: true if matched, false else
           *
           * <b>Lua code example</b>
           * \code
           * re = Regexp( 'my_re' )
           * assertEquals( (re:match('XXmy_re')), true )
           * assertEquals( (re:match('no match')), false )
           * \endcode
     */
    static int Regexp_match(lua_State *L);

    /** \brief Match a regexp with a string, return index
           *
     * \b Arguments: 
           * - string: the string to match
           *
           * \b Returns: the index of the match if any or -1 if there was no
           * matches.
           *
           * <b>Lua code example</b>
           * \code
           * re = Regexp( 'my_re' )
           * assertEquals( (re:matchIndex('XXmy_re')), 2 )
           * assertEquals( (re:matchIndex('no match')), -1 )
           * \endcode
     */
    static int Regexp_matchIndex(lua_State *L);

    /** \brief Return positions of the matches in the string.
           *
     * \b Arguments: 
           * - number: index of the match for which we want the position (0 is
           * for the whole Regexp).
           *
     * \b Returns: The position of the regexp being matched, or -1 
           * if nothing matched at that position.
           *
           * <b>Lua code example</b>
           * \code
           * re = Regexp( '(aa) (bb)' )
           * assertEquals( (re:match('  aa bb  ')), true )
           * assertEquals( (re:pos(0)), 2 )
           * assertEquals( (re:pos(1)), 2 )
           * assertEquals( (re:pos(2)), 5 )
           * assertEquals( (re:pos(3)), -1)
           * \endcode
     */
    static int Regexp_pos(lua_State *L);

    /** \brief Returns the number of captured in the regular expression itself.
           *
           * \b Arguments: none
           *
           * \b Returns: number of captures
           *
           * <b>Lua code example</b>
           * \code
           * re = Regexp( '(a+)(b*)(c+)' )
           * assertEquals( re:numCaptures(),  3 )
           * \endcode
     */
    static int Regexp_numCaptures(lua_State *L);

    /** \brief Returns the text captured by the regexp
           *
     * \b Arguments: 
           * - number: index of match (0 for the whole regexp)
           *
           * \b Returns: string, text matched at the index or empty string.
           *
           * <b>Lua code example</b>
           * \code
           * re = Regexp( '(a+)(b*)(c+)' )
           * assertEquals( re:match( '   aabbcc  ' ), true )
           * assertEquals( re:captured(0),  'aabbcc' )
           * assertEquals( re:captured(1),  'aa' )
           * assertEquals( re:captured(2),  'bb' )
           * assertEquals( re:captured(3),  'cc' )
           * assertEquals( re:captured(4),  '' )
           * 
           * assertEquals( re:match( '  aacc  ' ), true )
           * assertEquals( re:captured(0),  'aacc' )
           * assertEquals( re:captured(1),  'aa' )
           * assertEquals( re:captured(2),  '' )
           * assertEquals( re:captured(3),  'cc' )
           * assertEquals( re:captured(4),  '' )
           *
           * assertEquals( re:match( '  XXX  ' ), false )
           * assertEquals( re:captured(0),  '' )
           * \endcode
     */
    static int Regexp_captured(lua_State *L);

    /** \brief Returns a string where the occurrences of Regexp have been
     * replaced by the provided string.
           *
     * \b Arguments: 
     * - string: text to be replaced
     * - string: a replacement string (which can use \\1)
           *
           * \b Returns: the replaced string
           *
           * <b>Lua code example</b>
           * \code
           * re = Regexp('(aaa) (bbb) (ccc)')
           * s = "aaa bbb ccc"
           * result = re:replace( s, "\\3 \\2 \\1" )
           * expected = 'ccc bbb aaa'
           * assertEquals( result, expected )
           * \endcode
     */
    static int Regexp_replace(lua_State *L);

    /** \brief Return the pattern of the regexp as a string
           *
           * \b Arugments: nothing
           *
           * \b Returns: string, the text of the pattern
           *
           * <b>Lua code example</b>
           * \code
           * pat = '(aaa) (bbb) (ccc)'
           * re = Regexp(pat)
           * assertEquals( re:pattern(), pat )
           * \endcode
           */
    static int Regexp_pattern(lua_State *L);

    /** \brief Set regexp greediness
           *
     * \b Arguments: 
           * - bool: true will choose minimum greediness, false will choose
           * maximum greediness
           *
           * \b Returns: nothing
           *
           * <b>Lua code example</b>
           * \code
           * re = Regexp( 'a+' )
           * assertEquals( re:matchIndex( '   aaa' ), 3 )
           * assertEquals( re:captured(0), 'aaa' )
           * re:setMinimal( true )
           * assertEquals( re:matchIndex( '   aaa' ), 3 )
           * assertEquals( re:captured(0), 'a' )
           * \endcode
     */
    static int Regexp_setMinimal(lua_State *L);

    /** \brief Set regexp case sensitiveness
           *
     * \b Arguments: 
           * - bool: true for case sensitive regexp, false for non case
           * sensitive.
           *
           * Default is to have case sensitive regexp.
           *
           * \b Returns: nothing
           *
           * <b>Lua code example</b>
           * \code
           * re = Regexp( 'a+' )
           * assertEquals( re:matchIndex( 'xxxAAA' ), -1 )
           * re:setCaseSensitive( false )
           * assertEquals( re:matchIndex( 'xxxAAA' ), 3 )
           * \endcode
     */
    static int Regexp_setCaseSensitive(lua_State *L);

    /** \brief
      * Called by lua when the regexp is about to be deleted
     */
    static int Regexp_userdata_finalize(lua_State *L);

};

#endif // YZ_LUA_REGEXP
