/**
 * $Id$
 */

/**
 * This file contains the list of mappings between keystrokes and commands
 */

#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>
#include "yz_buffer.h"
#include "yz_view.h"
#include "yz_session.h"
#include "yz_plugin.h"

#ifndef YZ_COMMANDS_H
#define YZ_COMMANDS_H

//Macros to add new commands to the pool
//The basic supposition is that you know on which kind of object your command will act. I guess you know right ? :)
//anyway if you don't know then default to Plugin
//note this overwrites previous commands with the same name
#define NEW_POOL_COMMAND( x,y,z ) { YZCommand cmd; cmd.immutable=z; cmd.obj=POOL; cmd.poolFunc=y; globalCommands[ x ] = cmd; }
#define NEW_VIEW_COMMAND( x,y,z ) { YZCommand cmd; cmd.immutable=z; cmd.obj=VIEW; cmd.viewFunc=y; globalCommands[ x ] = cmd; }
#define NEW_BUFF_COMMAND( x,y,z ) { YZCommand cmd; cmd.immutable=z; cmd.obj=BUFF; cmd.buffFunc=y; globalCommands[ x ] = cmd; }
#define NEW_SESS_COMMAND( x,y,z ) { YZCommand cmd; cmd.immutable=z; cmd.obj=SESS; cmd.sessFunc=y; globalCommands[ x ] = cmd; }
//to be used from plugin constructors
#define NEW_PLUG_COMMAND( x,y,z ) { YZCommand cmd; cmd.immutable=z; cmd.obj=PLUG; cmd.plugFunc=y; globalCommands[ x ] = cmd; }

//to be used only for the Ex Pool
#define NEW_EX_COMMAND( x,y,z ) { YZCommand cmd; cmd.immutable=z; cmd.obj=EX; cmd.exFunc=y; globalExCommands[ x ] = cmd; }

class YZSession;

//oh please don't instanciate me twice !
class YZCommandPool {
	//object types definition
	enum type {
		POOL,
		VIEW,
		BUFF,
		SESS,
		EX
	};
	struct cmd {
		type obj; //object type 
		bool immutable; //is this command overwritable ?//FIXME
		//with function pointers we are limited by class and by prototypes so ...
		QString ( YZCommandPool::*poolFunc ) (const QString& inputsBuff);
		QString ( YZView::*viewFunc ) (const QString& inputsBuff);
		QString ( YZBuffer::*buffFunc ) (const QString& inputsBuff);
		QString ( YZSession::*sessFunc ) (const QString& inputsBuff);
		QString ( YZPlugin::*plugFunc ) (const QString& inputsBuff);
		QString ( YZPlugin::*exFunc ) (const QString& inputsBuff);
	};

	public:
		YZCommandPool();
		~YZCommandPool();
		
		//that's a keystroke/function pointer mapping
		//this will allow our plugin (Yes there will be some) to add their own functions
		//
		//QMap does not like having a function pointer as a parameter so i use a struct to wrap it
		//it's quite weird ...
		typedef struct cmd YZCommand;
		QMap<QString, YZCommand> globalCommands;
		QMap<QString, YZCommand> globalExCommands;

		void initPool();
		void initExPool();

		//just an example method
		QString test(const QString&);

		/**
		 * This function is the entry point to execute ANY command in Yzis
		 */
		void execCommand(YZView *view, const QString& inputs);

		/**
		 * Entry point for ex functions ( scripting )
		 */
		void execExCommand(YZView *view, const QString& inputs);
};

#endif
