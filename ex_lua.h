/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Mickael Marchand <mikmak@yzis.org>
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

#ifndef YZ_EX_LUA
#define YZ_EX_LUA

#include <qobject.h>
extern "C" {
#include <lua.h>
}

class YZView;

class YZExLua : public QObject {
	Q_OBJECT

	public:
		YZExLua();
		~YZExLua();

		/**
		 * Lua test
		 */
		QString lua(YZView *view, const QString& inputs);

		/**
		 * Source a lua file
		 */
		QString loadFile(YZView *view, const QString& inputs);

		/**
		 * Extract some text from view.
		 * Arguments:
		 * startCol, startLine, endCol, endLine
		 *
		 * Returns a string
		 */
		static int text(lua_State *L);

		/**
		 * Get one line of text:
		 * Arguments:
		 * - the line number
		 *
		 * Returns a string
		 */
		static int line(lua_State *L);

		/**
		 * Insert text inside a line:
		 * Arguments:
		 * startCol,startLine,text
		 *
		 * Returns nothing.
		 */
		static int insert(lua_State *L);

		/**
		 * Insert a new line
		 * Arguments:
		 * line,text
		 *
		 * Returns nothing.
		 */
		static int insertline(lua_State *L);

		

		/**
		 * Replace text on view.
		 * Arguments:
		 * startCol,startLine, text to replace
		 *
		 * Returns nothing.
		 */
		static int replace(lua_State *L);

		/**
		 * Returns the current column position in buffer
		 */
		static int wincol(lua_State *L);

		/**
		 * Returns the current line position in buffer
		 */
		static int winline(lua_State *L);

		/**
		 * Moves the cursor to the given position
		 * Arguments: col, line
		 *
		 * Note: the underscore is necessary because the name is already
		 * reserved in C++
		 */
		static int _goto(lua_State *L);

		/**
		 * Deletes the given line.
		 * Returns nothing
		 *
		 * Note: the underscore is necessary because the name is already
		 * reserved in C++
		 */
		static int YZExLua::deleteline(lua_State *L);

		/**
		 * Return current's filename
		 */
		static int YZExLua::filename(lua_State *L);

		/**
		 * Return current's syntax highlighting color for given column,line
		 * Arguments: col, line
		 * Returns a color string
		 */
		static int YZExLua::getcolor(lua_State *L);

		/**
		 * Returns the number of lines of the current buffer.
		 */
		static int YZExLua::linecount(lua_State *L);

		/**
		 * Returns the yzis version string
		 */
		static int YZExLua::version(lua_State *L);

		/**
		 * Send a set of keys contained in a string asif they were typed
		 * by the user.
		 *
		 * Arguments: string
		 * Returns nothing
		 */
		static int YZExLua::sendkeys(lua_State *L);

	private:
		lua_State *st;
};

#endif
