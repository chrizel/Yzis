/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2004 Mickael Marchand <marchand@kde.org>
 *  Thomas Capricelli <orzel@freehackers.org>
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

/**
 * $Id$
 */

/**
 * This file contains the list of mappings between keystrokes and commands
 */


#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>
#include "view.h"
#include "buffer.h"
#include "session.h"
#include "plugin.h"
#include "ex_executor.h"
#include "ex_lua.h"

#ifndef YZ_COMMANDS_H
#define YZ_COMMANDS_H

//Macros to add new commands to the pool
//The basic supposition is that you know on which kind of object your command will act. I guess you know right ? :)
//anyway if you don't know then default to Plugin
//note this overwrites previous commands with the same name
#define NEW_POOL_COMMAND( x,y,z,a,b,c ) { YZCommand cmd; cmd.immutable=z; cmd.obj=POOL; cmd.poolFunc=y; cmd.hasCounter=a; cmd.hasMotion=b; cmd.hasRegister=c; globalCommands[ x ] = cmd; }
#define NEW_VIEW_COMMAND( x,y,z,a,b,c ) { YZCommand cmd; cmd.immutable=z; cmd.obj=VIEW; cmd.viewFunc=y; cmd.hasCounter=a; cmd.hasMotion=b; cmd.hasRegister=c; globalCommands[ x ] = cmd; }
#define NEW_BUFF_COMMAND( x,y,z,a,b,c ) { YZCommand cmd; cmd.immutable=z; cmd.obj=BUFF; cmd.buffFunc=y; cmd.hasCounter=a; cmd.hasMotion=b; cmd.hasRegister=c; globalCommands[ x ] = cmd; }
#define NEW_SESS_COMMAND( x,y,z,a,b,c ) { YZCommand cmd; cmd.immutable=z; cmd.obj=SESS; cmd.sessFunc=y; cmd.hasCounter=a; cmd.hasMotion=b; cmd.hasRegister=c; globalCommands[ x ] = cmd; }
//to be used from plugin constructors
#define NEW_PLUG_COMMAND( x,y,z,a,b,c ) { YZCommand cmd; cmd.immutable=z; cmd.obj=PLUG; cmd.plugFunc=y; cmd.hasCounter=a; cmd.hasMotion=b; cmd.hasRegister=c; globalCommands[ x ] = cmd; }

//to be used only for the Ex Pool
#define NEW_EX_COMMAND( x,y,z,a ) { YZCommand cmd; cmd.priority=a; cmd.immutable=z; cmd.obj=EX; cmd.exFunc=y; globalExCommands[ x ] = cmd; }
#define NEW_LUA_COMMAND( x,y,z,a ) { YZCommand cmd; cmd.priority=a; cmd.immutable=z; cmd.obj=LUA; cmd.luaFunc=y; globalExCommands[ x ] = cmd; }

class YZSession;
class YZExExecutor;
class YZExLua;
class YZBuffer;
class YZView;

struct args {
	QChar registr;
	//exec this number of times the command 3j (3 times j), 2d3w (delete 3 words 2 times) ...
	unsigned int count;
	unsigned int motionStartX;
	unsigned int motionEndX;
	unsigned int motionStartY;
	unsigned int motionEndY;
	QString command;
	QString motion;
	//the origin of inputs
	YZView *view;

	//constructor
	args() {
		registr = QChar();
		count = 1;
		motionStartX = 0;
		motionStartY = 0;
		motionEndX = 0;
		motionEndY = 0;
		command = "";
		view = NULL;
	}
};

typedef struct args YZCommandArgs;


//oh please don't instanciate me twice !
class YZCommandPool {
	//object types definition
	enum type {
		POOL,
		VIEW,
		BUFF,
		SESS,
		EX,
		LUA
	};

	struct cmd {
		type obj; //object type 
		bool immutable; //is this command overwritable ?//FIXME
		bool hasCounter;
		bool hasMotion;
		bool hasRegister;
		//priorities are used to determine which functions must be used by default (for eg, if we :s -> execute :substitute instead of :set)
		//internal EX commands should use values between 1-100, plugins any values >100 (a plugin should not override an internal command IMHO)
		int priority;
		
		//with function pointers we are limited by class and by prototypes so ...
		QString ( YZCommandPool::*poolFunc ) (const QString& inputsBuff, YZCommandArgs args);
		QString ( YZView::*viewFunc ) (const QString& inputsBuff, YZCommandArgs args);
		QString ( YZBuffer::*buffFunc ) (const QString& inputsBuff, YZCommandArgs args);
		QString ( YZSession::*sessFunc ) (const QString& inputsBuff, YZCommandArgs args);
		QString ( YZPlugin::*plugFunc ) (const QString& inputsBuff, YZCommandArgs args);
		QString ( YZExExecutor::*exFunc ) (YZView *view, const QString& inputsBuff);
		QString ( YZExLua::*luaFunc ) (YZView *view, const QString& inputsBuff);
		// TODO : shouldn't that be an union ?
	};

	typedef struct cmd YZCommand;

	public:
		YZCommandPool();
		~YZCommandPool();
		
		QMap<QString, YZCommand> globalCommands;
		QMap<QString, YZCommand> globalExCommands;

		void initPool();
		void initExPool();

		/**
		 * This function is the entry point to execute ANY command in Yzis
		 */
		void execCommand(YZView *view, const QString& inputs, int *error);

		/**
		 * Entry point for ex functions ( scripting )
		 */
		void execExCommand(YZView *view, const QString& inputs);
		
	private:
		YZExExecutor *executor;
		YZExLua *lua_executor;

};

#endif
