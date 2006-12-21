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

#ifndef YZLUASCRIPTING
#define YZLUASCRIPTING

extern "C" {
#include <lua.h>
}

/** All lua commands available in Yzis
  *
  * This class contains all the static functions that provide a lua
  * command.
  *
  * XXX describe here how to write a lua command
  *
  */
class YZLuaFuncs {

public:

	/** Register all lua functions defined in this class to lua.
	  *
	  * To be called during initialisation.
	  */
	static void registerLuaFuncs(lua_State *L);

	/**
	  * Replacement of lua print, allows to control the output of print
	  * Just take one string argument
	  */
	static int yzprint(lua_State *L);

	/**
	 * Get one line of text:
	 * Arguments:
	 * - the line number
	 *
	 * Returns a string
	 */
	static int line(lua_State *L);

	/**
	 * Get one line of text:
	 * Arguments:
	 * - the line number
	 * - line content
	 *
	 * Returns a string
	 */
	static int setline(lua_State *L);

	/**
	 * Insert text inside a line:
	 * Arguments:
	 * startCol,startLine,text
	 *
	 * Returns nothing.
	 */
	static int insert(lua_State *L);

	/**
	 * Remove the given number of caracters
	 * Arguments :
	 * col, line, number
	 */
	static int remove(lua_State *L);

	/**
	 * Insert a new line
	 * Arguments:
	 * line,text
	 *
	 * Returns nothing.
	 */
	static int insertline(lua_State *L);

	/**
	 * Append line at the end of the buffer
	 * Arguments:
	 * text
	 *
	 * Returns nothing.
	 */
	static int appendline(lua_State *L);

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
	 * Returns the current cursor position: col, line
	 */
	static int winpos(lua_State *L);

	/**
	 * Moves the cursor to the given position
	 * Arguments: col, line
	 *
	 * Note: the underscore is necessary because the name is already
	 * reserved in C++
	 */
	static int _goto(lua_State *L);

	/**
	 * Returns the current column position on screen
	 */
	static int scrcol(lua_State *L);

	/**
	 * Returns the current line position on screen
	 */
	static int scrline(lua_State *L);

	/**
	 * Moves the cursor to the given position on screen
	 * Arguments: col, line
	 */
	static int scrgoto(lua_State *L);

	/**
	 * Deletes the given line.
	 * Returns nothing
	 */
	static int deleteline(lua_State *L);

	/**
	 * Return the current lua filename
	 */
	static int filename(lua_State *L);

	/**
	 * Return current's syntax highlighting color for given column,line
	 * Arguments: col, line
	 * Returns a color string
	 */
	static int color(lua_State *L);

	/**
	 * Returns the number of lines of the current buffer.
	 */
	static int linecount(lua_State *L);

	/**
	 * Returns the yzis version string
	 */
	static int version(lua_State *L);

	/**
	 * Send a set of keys contained in a string asif they were typed
	 * by the user.
	 *
	 * Arguments: string
	 * Returns nothing
	 */
	static int sendkeys(lua_State *L);

	/**
	* Command to customize syntax highlighting settings
	* Arguments : style, type, ...
	* Returns nothing
	*/
	static int highlight(lua_State *L);

	/**
	 * Plugins main registration point
	 * Arguments : the event name, the lua function to call
	 * Returns nothing
	 */
	static int connect(lua_State *L);

	/**
	 * Find a file in standard yzis plugin directories
	 * and source it
	 */
	static int source(lua_State *L);

	/**
	 * Sends a string to debug output
	 */
	static int yzdebug(lua_State *L);

	/**
	 * Set local options
	 */
	static int setlocal(lua_State *L);

	/**
	 * Set global options
	 */
	static int set(lua_State *L);

	/**
	 * Create a new option
	 */
	static int newoption(lua_State *L);

	/**
	 * Adds a new global mapping
	 */
	static int map(lua_State *L);
	static int unmap(lua_State *L);

	/**
	 * Adds new insert mappings
	 */
	static int imap(lua_State *L);
	static int iunmap(lua_State *L);

	/**
	 * Adds a new visual mapping
	 */
	static int vmap(lua_State *L);
	static int vunmap(lua_State *L);

	/**
	 * Adds a new cmdline mapping
	 */
	static int cmap(lua_State *L);
	static int cunmap(lua_State *L);

	/**
	 * Adds a new pending op mapping
	 */
	static int omap(lua_State *L);
	static int ounmap(lua_State *L);

	/**
	 * Adds a new normal mapping
	 */
	static int nmap(lua_State *L);
	static int nunmap(lua_State *L);

	/**
	 * Adds a new not remappable global mapping
	 */
	static int noremap(lua_State *L);

	/**
	 * Adds a new not remappable normal mapping
	 */
	static int nnoremap(lua_State *L);

	/**
	 * Adds a new not remappable visual mapping
	 */
	static int vnoremap(lua_State *L);
	
	/**
	 * Adds a new not remappable pending op mapping
	 */
	static int onoremap(lua_State *L);

	/**
	 * Adds a new not remappable insert mapping
	 */
	static int inoremap(lua_State *L);

	/**
	 * Adds a new not remappable cmd line mapping
	 */
	static int cnoremap(lua_State *L);
	
	static int matchpair(lua_State *L);

	/**
	 * Returns the current view mode
	 */
	static int mode(lua_State *L);
	
	/**
	 * Create a new buffer/view for the given file
	 */
	 static int edit(lua_State *L);
};

#endif // YZLUASCRIPTING

