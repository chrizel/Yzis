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

extern "C"
{
#include <lua.h>
}

/* This class contains all the static functions that provide a lua
 * command.
 */

/** \brief All lua commands available in Yzis
  *
  * The arguments described are for lua scripting.
  *
  * See also \ref YZLuaRegexp
  *
  */
class YZLuaFuncs
{

public:

    /** \brief Register all lua functions defined in this class to lua.
      *
      * To be called during initialisation.
      */
    static void registerLuaFuncs(lua_State *L);

    /** \brief Replacement of lua print.
         *
         * Override print to catch print statements in yzis.
      * 
         * \b Arguments:
         * - string, text to print
      */
    static int yzprint(lua_State *L);

    /** \brief
     * Get one line of text.
        *
     * \b Arguments:
     * - int, the line number
     *
        * \b Returns:
        * - string: the content of the line 
     */
    static int line(lua_State *L);

    /** \brief
     * Set one line of text.
        *
     * \b Arguments:
     * - int, the line number
        * - string, the text to set
        * 
        * \b Returns: nothing
     */
    static int setline(lua_State *L);

    /** \brief
     * Insert text inside a line:
        *
     * \b Arguments:
     * - int, the starting column
        * - int, the starting line
        * - string, the text to be inserted
     *
        * \b Returns: nothing
     */
    static int insert(lua_State *L);

    /** \brief
     * Remove the given number of characters
        *
     * \b Arguments :
     * - int, the starting column
        * - int, the starting line
        * - int, the number of characters to remove
        *
        * XXX check if can remove endline ?
        * 
        * \b Returns: nothing
     */
    static int remove(lua_State *L);

    /** \brief
     * Insert a new line
        *
     * \b Arguments:
     * - int, line number before which to insert
        * - string, text of the new line
     *
        * \b Returns: nothing
     */
    static int insertline(lua_State *L);

    /** \brief
     * Append line at the end of the buffer
        *
     * \b Arguments:
     * - string, text to append
     *
        * \b Returns: nothing
     */
    static int appendline(lua_State *L);

    /** \brief
     * Replace text on a given line.
        *
        * Does not accept multi-line strings.
        *
     * \b Arguments:
     * - int, starting column
        * - int, starting line 
        * - string, new text of the line
     *
        * \b Returns: nothing
     */
    static int replace(lua_State *L);

    /** \brief
     * Returns the current column position in buffer
        *
     * \b Arguments: None
     *
        * \b Returns: int, current column of the cursor of the view
     */
    static int wincol(lua_State *L);

    /** \brief
     * Returns the current line position in buffer
        *
     * \b Arguments: None
     *
        * \b Returns: int, current line of the cursor of the view
     */
    static int winline(lua_State *L);

    /** \brief
     * Returns the current cursor position: col, line
        *
     * \b Arguments: None
     *
        * \b Returns: 
        * - int, current column of the cursor of the view
        * - int, current line of the cursor of the view
     */
    static int winpos(lua_State *L);

    /** \brief
     * Moves the cursor to the given position
        *
     * \b Arguments: 
        * - int, destination column
        * - int, destination line 
     *
     * Note: the underscore is necessary because the name "goto" is already
     * reserved in C/C++ but the lua function is really named "goto".
     */
    static int _goto(lua_State *L);

    /** \brief
     * Returns the current column position on screen
        *
     * \b Arguments: None
     *
        * \b Returns: int, current column of the cursor of the screen
        *
        * XXX what is the difference between screen and buffer ?
     */
    static int scrcol(lua_State *L);

    /** \brief
     * Returns the current line position on screen
        *
     * \b Arguments: None
     *
        * \b Returns: int, current line of the cursor of the view
        */
    static int scrline(lua_State *L);

    /** \brief
     * Moves the cursor to the given position on screen
        *
     * \b Arguments: 
        * - int, destination column
        * - int, destination line 
        *
        * \b Returns: nothing
     */
    static int scrgoto(lua_State *L);

    /** \brief
     * Deletes the given line.
        *
     * \b Arguments:
        * - int, line to delete
     *
        * \b Returns: nothing
     */
    static int deleteline(lua_State *L);

    /** \brief
     * Return the current buffer filename
        *
     * \b Arguments: nothing
     *
        * \b Returns: string, the filename
     */
    static int filename(lua_State *L);

    /** \brief
     * Return current syntax highlighting color for given column,line
        *
     * \b Arguments: 
        * - int, target column
        * - int, target line
        *
        * \b Returns: string, the color name
     */
    static int color(lua_State *L);

    /** \brief
     * Returns the number of lines of the current buffer.
        *
     * Note that empty buffer always have one empty line.
        *
     * \b Arguments: nothing
     *
        * \b Returns: int, current number of lines
     */
    static int linecount(lua_State *L);

    /** \brief
     * Returns the yzis version string
        *
     * \b Arguments: nothing
     *
        * \b Returns: string, the current version.
     */
    static int version(lua_State *L);

    /** \brief
     * Send a set of keys contained in a string asif they were typed
     * by the user.
        *
        * This function is the most important one for testing. You can simulate
        * just any user behavior with this.
        *
        * Special keys like &lt;ESC&gt; and &lt;ENTER&gt; are recognised.
     *
     * \b Arguments: string, list of key stroke
        *
        * \b Returns: nothing
     */
    static int sendkeys(lua_State *L);

    /** \brief
     * Command to customize syntax highlighting settings
        *
     * \b Arguments : 
        * - string, style
        * - string, type
        * - more strings
        * 
        * \b Returns: nothing
     */
    static int highlight(lua_State *L);

    /** \brief
     * Register a lua function to call when a specific event occurs.
        *
        * This is the main entry point for plugins.
        *
     * \b Arguments : 
        * - string, the event name, 
        * - string, the name of the lua function to call
        *
        * \b Returns: nothing
     */
    static int connect(lua_State *L);

    /** \brief
     * Find a file in standard yzis plugin directories
     * and source it.
        *
     * \b Arguments:
        * - string, filename to source, with or without ".lua" extension.
     *
        * \b Returns: nothing
        *
     */
    static int source(lua_State *L);

    /** \brief
     * Sends a string to debug output from lua.
        *
        * See \ref YZDebugBackend for more information about debugging.
        *
        * The debug is in the area "Lua.exec".
        *
     * \b Arguments:
        * - string, the text to add to debug output
     *
        * \b Returns: nothing
     */
    static int yzdebug(lua_State *L);

    /** \brief
     * Set an option as local
        *
     * \b Arguments:
        * - string, option name
        *
        * XXX need better documentation
     *
        * \b Returns: nothing
     */
    static int setlocal(lua_State *L);

    /** \brief
     * Set global options
        *
     * \b Arguments:
        * - string, the option to set
     *
        * XXX need better documentation
        *
        * \b Returns: nothing
     */
    static int set(lua_State *L);

    /** \brief
     * Create a new option
        *
     * \b Arguments:
        * - string, option name, 
        * - string, group name, 
        * - string, default value, 
        * - string, value, 
        * - int, visibility
        * - int, type
     *
        * \b Returns: nothing
     */
    static int newoption(lua_State *L);

    /** \brief
     * Adds a new global mapping
        *
     * \b Arguments:
        * - string, key
        * - string, mapping
     *
        * \b Returns: nothing
     */
    static int map(lua_State *L);

    /** \brief
     * Removes a global mapping
        *
     * \b Arguments:
        * - string, key
     *
        * \b Returns: nothing
     */
    static int unmap(lua_State *L);

    /** \brief
     * Adds new insert mappings
        *
     * \b Arguments:
        * - string, key
        * - string, mapping
     *
        * \b Returns: nothing
     */
    static int imap(lua_State *L);
    /** \brief
     * Remove an insert mappings
        *
     * \b Arguments:
        * - string, key
     *
        * \b Returns: nothing
     */
    static int iunmap(lua_State *L);

    /** \brief
     * Adds a new visual mapping
        *
     * \b Arguments:
        * - string, key
        * - string, mapping
     *
        * \b Returns: nothing
     */
    static int vmap(lua_State *L);
    /** \brief
     * Remove a visual mapping
        *
     * \b Arguments:
        * - string, key
     *
        * \b Returns: nothing
     */
    static int vunmap(lua_State *L);

    /** \brief
     * Adds a new cmdline mapping
        *
     * \b Arguments:
        * - string, key
        * - string, mapping
     *
        * \b Returns: nothing
     */
    static int cmap(lua_State *L);
    /** \brief
     * Removes a cmdline mapping
        *
     * \b Arguments:
        * - string, key
     *
        * \b Returns: nothing
     */
    static int cunmap(lua_State *L);

    /** \brief
     * Adds a new pending op mapping
        *
     * \b Arguments:
        * - string, key
        * - string, mapping
     *
        * \b Returns: nothing
     */
    static int omap(lua_State *L);
    /** \brief
     * Removes a new pending op mapping
        *
     * \b Arguments:
        * - string, key
     *
        * \b Returns: nothing
     */
    static int ounmap(lua_State *L);

    /** \brief
     * Adds a new normal mapping
        *
     * \b Arguments:
        * - string, key
        * - string, mapping
     *
        * \b Returns: nothing
     */
    static int nmap(lua_State *L);

    /** \brief
     * Removes a normal mapping
        *
        *
     * \b Arguments:
        * - string, key
     *
        * \b Returns: nothing
     */
    static int nunmap(lua_State *L);


    /** \brief
     * Adds a new not remappable global mapping
        *
     * \b Arguments:
        * - string, key
        * - string, mapping
     *
        * \b Returns: nothing
     */
    static int noremap(lua_State *L);

    /** \brief
     * Adds a new not remappable normal mapping
        *
     * \b Arguments:
        * - string, key
        * - string, mapping
     *
        * \b Returns: nothing
     */
    static int nnoremap(lua_State *L);

    /** \brief
     * Adds a new not remappable visual mapping
        *
     * \b Arguments:
        * - string, key
        * - string, mapping
     *
        * \b Returns: nothing
     */
    static int vnoremap(lua_State *L);

    /** \brief
     * Adds a new not remappable pending op mapping
        *
     * \b Arguments:
        * - string, key
        * - string, mapping
     *
        * \b Returns: nothing
     */
    static int onoremap(lua_State *L);

    /** \brief
     * Adds a new not remappable insert mapping
        *
     * \b Arguments:
        * - string, key
        * - string, mapping
     *
        * \b Returns: nothing
     */
    static int inoremap(lua_State *L);

    /** \brief
     * Adds a new not remappable cmd line mapping
        *
     * \b Arguments:
        * - string, key
        * - string, mapping
     *
        * \b Returns: nothing
     */
    static int cnoremap(lua_State *L);

    /** \brief Find matching pair (parenthesis, brackets, ...)
    *
    * If a matching pair is found, the cursor is moved to this new position.
    * The pairs that can be matched are described in the option 'matchpairs'
    *
    * \b Arguments: nothing
    *
    * \b Returns: nothing
    * - boolean: whether the pair was found or not
    * - int: current column of the cursor (new position if the pair was found)
    * - int: current line of the cursor (new position if the pair was found)
    */
    static int matchpair(lua_State *L);

    /** \brief
     * Returns the current view mode
        *
     * \b Arguments: nothing
     *
        * \b Returns: 
        * - string,  a string describing the current view.
        *
        * XXX needs more doc
     */
    static int mode(lua_State *L);

    /** \brief
     * Create a new buffer/view for the given file
        *
     * \b Arguments:
        * - string, name of the file to open
     *
        * \b Returns: nothing
     */
    static int edit(lua_State *L);

    /** \brief
     * Load a plugin using the resource manager
         *
     * \b Arguments:
         * - string, name of the plugin
         *
         * \b Returns: nothing
     */
    static int loadplugin(lua_State *L );

    /* \brief
     * Set the script return value for Yzis.
         *
         * The return value is used as the exit code of Yzis when running a
         * script with <i>yzis -s scriptname.lua</i>.
         *
         * The function does not stop the execution, it justs informs the yzis
         * core of the return value and returns to lua.
         *
         * The recommendation is to put the function as the last one of the lua
         * script.
         *
     * \b Arguments:
         * - string, the return value for Yzis
         *
         * \b Returns: nothing
     */
    static int setLuaReturnValue(lua_State *L );
};

#endif // YZLUASCRIPTING

