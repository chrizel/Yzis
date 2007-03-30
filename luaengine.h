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

#include "yzismacros.h"

extern "C" {
#include <lua.h>
}

#include <QString>

class YZView;

/** This class is the main interface for the lua engine.
  *
  * YZLuaEngine is the central point of interaction with the lua engine. It is
  * responsible for initialising the lua engine, and provides all the
  * facilities needed to deal with lua.
  *
  * The class is a singleton and initialises the lua engine upon
  * instancing. The instance pointer is available via self().
  *
  * All code related to the lua engine and lua stack should go in this class.
  * Implementation of actual lua functions (the binding) is done in \ref
  * YZLuaFuncs. The functions are registered to lua by the call to 
  * YZLuaFuncs::registerLuaFuncs(),
  * performed during the class instancing.
  *
  * The function that are useful when dealing with lua in the libyzis are:
  * - source(): source a lua file
  * - lua(): execute some lua code
  * - exe(): calls a lua function
  *
  * When implementing a lua function, the interesting functions are:
  * - yzpcall(): calls lua with arguments already on the stack, raises a popup
  * in case of error.
  * - checkFunctionArguments(): check that the right number of arguments are
  * provided to a lua function
  * - print_lua_stack(): print the content of the lua stack on the debug
  * interface
  *
  */
class YZIS_EXPORT YZLuaEngine {
	public:
		/** Get the pointer to the singleton YZLuaEngine */
		static YZLuaEngine * self();

		~YZLuaEngine();

		/**
		 * Source a lua file.
         *
         * The file is looked for in:
         * - the current path
         * - ~/.yzis/scripts
         * - ~/.yzis/scripts/indent
         * - [install_dir]/share/yzis/scripts
         * - [install_dir]/share/yzis/scripts/indent
         *
         * @param filename the name of the lua file, with or without .lua
         * extension.
         * \return 1 if file isn't found, 0 otherwise
         *
		 */
		int source( const QString& filename );

		/**
		 * Execute some lua code.
         *
         * This method is an easy entry point, usable directly
         * by the Ex function binder.
         *
         * The method will call execInLua().
		 */
		QString lua(YZView *view, const QString& args);

        /** Calls a lua function.
          *
          * This function uses yzpcall() to call the function.
          *
          * XXX The arguments are supposed to be already on the stack, which
          * is a bit strange since there is no function to put them.
          *
          * The function is used by the event stuff, which actually does not
          * put any stuff on the stack.
          */
		void execute(const QString& function, int nbArgs, int nbResults);

		/**
		 * Return the results of the last Lua method invoked.
         *
         * If you execute some lua code with execInLua or exe or execute, the
         * results of the function call will remain on the lua stack.
         *
         * This function will pop @param nb results and put them in a string
         * list. If the result is neither a string or a number, an empty
         * string is pushed in the list.
         *
         * The list is constructed so that the last element of the list is the
         * topmost element of the stack.
         *
         * @param nb number of items to pop from the stack
         * @return a string list containing all @param nb elements.
         *
         * XXX the code of the function is inconsistent if nb != number of
         * elements of the stack. It will pop the topmost element but put the
         * bottom element in the string list.
         *
         * XXX What happens if there is not enough entries to pop ?
         *
		 */
		QStringList getLastResult(int nb) const;

        /** Calls a lua function.
          *
          * The function allows to call any lua function with any number
          * argument and to get the results.
          *
          * The arguments being passed and expected in the result should be \b
          * exactly as described in the function signature
          * else - quoting from the man page of stdarg -
          * <i>random things may happen</i>.
          *
          * The signature of the function is described in \p sig:
          * - d: argument passed is double
          * - i: argument passed is a number
          * - s: argument passed is char *
          * - >: now describing the return values of the function
          *
          * For return values, the meaning is the same:
          * - d: double return value expected 
          * - i: int return value expected
          * - s: char * return value expected
          *
          * The return values are fetched by passing pointers to the expected
          * type.
          *
          * Example:
          * Calling a function that takes as argument two ints, one string and
          * one double, and returns a double and a string.
          * \code
          * int a1, a2;
          * char * s3 = "toto";
          * double d4 = 3.1415927;
          * double ret1d;
          * char * ret2s;
          *
          * exe( "some_lua_function", "iisd>ds", a1, a2, s3, d4, &ret1d, &ret2s );
          *
          * printf("returns %f %s\n", ret1d, ret2s );
          *
          * \endcode
          *
          *
          * @param function function to call
          * @param sig signature of the function
          */		
		void exe(const QString& function, const char* sig, ...);
	
		/** Execute the given lua code.
          *
          * This function will use the lua function \e loadstring() to execute
          * the lua code that is passed in argument.
          *
          * The lua code could contain multiple statements.
          *
          * The stack is not cleaned after this call, so any return value
          * will stay on the stack.
          *
          * Internally, this will call yzpcall(), which in other things
          * can popup with an error message.
          *
          * @param luacode lua code to execute
          * @return 0 in case of success, 1 in case of error
          */
		int execInLua( const QString & luacode );

		/** Called when lua wants to print */
		void yzisprint(const QString & text);

		/**
		 * Empty the lua stack. Normally, you should not need
         * this function because you should pop the lua stack items by items
         * and you should know exactly how many items were put on the stack.
		 */
		static void cleanLuaStack( lua_State * L );

		/** Wrapper around lua_pcall()
		  *
		  * Takes the same arguments as lua_pcall(). Returns true if the lua
		  * call is successful.
		  *
		  * In case of error, the functions displays a popup with the error
		  * message.
		  *
		  *  @param nbArg number of arguments for the function call
		  *  @param nbReturn number of expected results from the function call
		  *  @param context string describing the context of the call,
		  *  displayed when an error occurs.
		  *  @return true if the call is without error 
		  */
		bool yzpcall( int nbArg, int nbReturn, const QString & context=QString::null );

		/** 
		  * Check that the lua stack contains the number of expected argument.
          *
		  * Calls lua_error() if the stack does not contain the right
          * arguments.
		  *
		  * @param L the lua state
		  * @param argNbMin minimum number of expected argument
		  * @param argNbMax maximum number of expected argument
		  * @param functionName name of the function being checked (used when
		  * 				reporting an error)
          * @param functionArgDesc description of the function arguments (used
          * when reporting an error)
          *
          * @return true if everything is correct.
		  */
		static bool checkFunctionArguments(lua_State*L, 
					int argNbMin,
					int argNbMax,
					const char * functionName, 
					const char * functionArgDesc );

        /** Print one stack element on the debug interface.
          *
          * Print the lua stack item \p index on the debug interface.
          * The index is passed directly to the lua function, so can be
          * positive or negative.
          *
          * @param L the lua state
          * @param index the position of the element on the stack
          * @param type_only whether to output table content (false) or just the type name (true)
          */
		static void print_lua_stack_value(lua_State*L, int index, bool type_only=false);

        /** Print the content of the stack on the debug interface.
          *
          * Print the content of the lua stack on the debug interface. When
          * \p msg is specified, it is displayed as well. Use \p msg to
          * hint the context of the call.
          *
          * @param L the lua state
          * @param msg a message describing the context in which this function
          * is called. 
          * @param type_only whether to output table content (false) or just the type name (true)
          */
		static void print_lua_stack(lua_State *L, const char * msg, bool type_only=false);

        /** Convert an element of the stack into a string.
          *
          * The method is used by print_lua_stack_value().
          *
          * @param L the lua state
          * @param depth indentation added to the content of the element
          * @param type_only whether to output table content (false) or just the type name (true)
          * @param index the position of the element on the stack
          */
        static QString lua_value_to_string(lua_State*L, int index, int depth=0, bool type_only=false);

        /** Convert a lua table on the stack into a string.
          *
          * The method is used by lua_value_to_string() when type_only is set
          * to false. If type_only is set to true, this method is not called.
          *
          * @param L the lua state
          * @param index the position of the element on the stack
          * @param depth indentation added to the content of the element
          */
        static QString lua_table_to_string(lua_State*L, int index, int depth);

	protected:
        /** Lua state.
          *
          * The lua state is created in init() and destroyed in the destructor
          */
		lua_State *L;


		/**
		 * Init core lua stuff (functions, regexps...)
		 */
		void init();

    private:
		/** Private constructor for a singleton */
		YZLuaEngine();
		static YZLuaEngine * me; //!< Singleton instance holder


};

#endif // YZ_LUA_ENGINE
