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
		/** Get the pointer to the singleton YZExLua */
		static YZExLua * instance();

		~YZExLua();

		/**
		 * Source a lua file
		 */
		QString source(YZView *view, const QString& args);
		QString source(YZView *view, const QString& args,bool canPopup);

		/**
		 * Execute a lua command
		 */
		QString lua(YZView *view, const QString& args);

		/** Execute the given lua code */
		int execInLua( const QString & luacode );

		/** Called when lua wants to print */
		void yzisprint(const QString & text);

		/**
		 * Return the results of the last Lua method invoked
		 */
		QStringList getLastResult(int nb);

		void execute(const QString& function, int nbArgs, int nbResults);
		
		void exe(const QString& function, const char* sig, ...);
	
	protected:
		/** Protected call to lua, popups an error window in case of
		  * failure. Return true if the call is without error */
		bool pcall( int nbArg, int nbReturn, int errLevel, const QString & errorMsg );

		// ========================================================
		//
		//                     Lua Commands
		//
		// ========================================================

		/**
		  * Replacement of lua print, allows to control the output of print
		  * Just take one string argument
		  */
		static int myprint(lua_State *L);

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
		* Arguments : 3 strings
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
		static int debug(lua_State *L);

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
		 * Adds new insert mappings
		 */
		static int imap(lua_State *L);

	protected:
		lua_State *L;

		/** Private constructor for a singleton */
		YZExLua();
		static YZExLua * _instance;
		static bool checkFunctionArguments(lua_State*L, 
					int argNb,
					const char * functionName, 
					const char * functionArgDesc );
};

#endif
