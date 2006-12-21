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

#ifndef YZ_LUA_ENGINE
#define YZ_LUA_ENGINE

extern "C" {
#include <lua.h>
}

#include <QString>

class YZView;


class YZLuaEngine {
	public:
		/** Get the pointer to the singleton YZLuaEngine */
		static YZLuaEngine * self();

		~YZLuaEngine();

		/**
		 * Source a lua file.
         *
         * The file is looked into:
         * - the current path
         * - ~/.yzis/scripts
         * - ~/.yzis/scripts/indent
         * - <install_dir>/share/yzis/scripts
         * - <install_dir>/share/yzis/scripts/indent
         *
         * @param filename the name of the lua file, with or without .lua
         * extension.
         *
		 */
		QString source( const QString& filename );

		/**
		 * Execute a lua command
		 */
		QString lua(YZView *view, const QString& args);

		/**
		 * Cleanup the stack
		 */
		static void cleanLuaStack( lua_State * L );

		/** Execute the given lua code */
		int execInLua( const QString & luacode );

		/** Called when lua wants to print */
		void yzisprint(const QString & text);

		/**
		 * Return the results of the last Lua method invoked
		 */
		QStringList getLastResult(int nb) const;

		void execute(const QString& function, int nbArgs, int nbResults);
		
		void exe(const QString& function, const char* sig, ...);
	
		/** Wrapper around lua_pcall()
		  *
		  * Takes the same arguments as lua_pcall(). Returns true if the lua
		  * call is successful.
		  *
		  * In case of error, the functions displays a popup with the error
		  * content.
		  *
		  *  @param nbArg number of arguments for the function call
		  *  @param nbReturn number of expected results from the function call
		  *  @param context string describing the context of the call,
		  *  displayed when an error occurs.
		  * Return true if the call is without error 
		  */
		bool yzpcall( int nbArg, int nbReturn, const QString & context=QString::null );

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

		static bool checkFunctionArguments(lua_State*L, 
					int argNb,
					const char * functionName, 
					const char * functionArgDesc );

		static void print_lua_stack_value(lua_State*L, int index);

		static void print_lua_stack(lua_State *L, const char * msg="");

	protected:
		lua_State *L;

		/** Private constructor for a singleton */
		YZLuaEngine();
		static YZLuaEngine * me;
};

#endif // YZ_LUA_ENGINE
