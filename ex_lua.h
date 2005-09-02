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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef YZ_EX_LUA
#define YZ_EX_LUA

#include <QString>
extern "C" {
#include <lua.h>
}

class YZView;

class YZExLua {
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

		/**
		 * Cleanup the stack
		 */
		void cleanup();

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
		  * failure. 
		  *
		  * Return true if the call is without error 
		  */
		bool yzpcall( int nbArg, int nbReturn, int errLevel, const QString & errorMsg );

		// ========================================================
		//
		//                     Lua Commands
		//
		// ========================================================

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

		/** Register the regexp functions to lua */
		void registerRegexp(lua_State *L);

		/** 
		 * Create a regexp (based on QRegexp)
		 * Argument 1 : a string for the regexp
		 * Returns: a table that can be used as a regexp object:
		 *
		 * Lua code example:
		 * re = Regexp:create( 'my_re' )
		 * print(re:match('XXmy_re')) -- true
		 * print(re:match('no match')) -- false
		 * print(re:matchIndex('XXmy_re')) -- 2
		 * print(re:matchIndex('no match')) -- -1
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
		static int YZExLua::Regexp_pattern(lua_State *L);

		/** 
		  * Check that the lua stack contains the number of expected argument.
		  * Calls lua_error() is that is not the case with a description of
		  * what the argument should be
		  *
		  * argNbMin: minimum number of expected argument
		  * argNbMax: maximum number of expected argument
		  * functionName: name of the function being checked (used when
		  * 				reporting an error)
		  * functionArgDesc: description of the function arguments (used when
		  * 				reporting an error)
		  */
		static bool checkFunctionArguments(lua_State*L, 
					int argNbMin,
					int argNbMax,
					const char * functionName, 
					const char * functionArgDesc );

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
