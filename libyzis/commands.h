/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2004 Mickael Marchand <marchand@kde.org>
 *  Thomas Capricelli <orzel@freehackers.org>
 *  Pascal "Poizon" Maillard <poizon@gmx.at>
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


#ifndef YZ_COMMANDS_H
#define YZ_COMMANDS_H

#include <qstring.h>
#include <qmap.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include "selection.h"
#include "cursor.h"

//to be used only for the Ex Pool
#define NEW_EX_COMMAND( x,y,z,a ) { YZExCommand cmd; cmd.priority=a; cmd.immutable=z; cmd.obj=EX; cmd.exFunc=y; globalExCommands[ x ] = cmd; }
#define NEW_LUA_COMMAND( x,y,z,a ) { YZExCommand cmd; cmd.priority=a; cmd.immutable=z; cmd.obj=LUA; cmd.luaFunc=y; globalExCommands[ x ] = cmd; }

class YZExExecutor;
class YZExLua;
class YZBuffer;
class YZView;
class YZCursor;

/** holds the arguments a command needs in order to execute */
struct YZCommandArgs {
	//the origin of inputs
	YZView *view;
	//the registers to operate upon
	QValueList<QChar> regs;
	//exec this number of times the command
	unsigned int count;
	//the argument
	QString arg;
	//the visual mode selection
	const YZSelectionMap *selection;

	YZCommandArgs(YZView *v, const QValueList<QChar> &r, unsigned int c, QString a) {
		view=v;
		regs=r;
		count=c;
		arg=a;
		selection=0;
	}
	YZCommandArgs(YZView *v, const QValueList<QChar> &r, unsigned int c, const YZSelectionMap *const s) {
		view=v;
		regs=r;
		count=c;
		selection=s;
	}
};

class YZCommandPool;
typedef QString (YZCommandPool::*PoolMethod) (const YZCommandArgs&);

enum cmd_arg {
	ARG_NONE,
	ARG_MOTION,
	ARG_CHAR,
	ARG_MARK,
	ARG_REG,
};

enum cmd_state {
	/** The command does not exist */
	CMD_ERROR,
	/** The user hasn't entered a valid, non-ambigous command yet. */
	NO_COMMAND_YET,
	/** Waiting for a motion/text object. */
	OPERATOR_PENDING,
	/** The command has been successfully executed. */
	CMD_OK,
};

/** Contains all the necessary information that makes up a normal command. @ref YZCommandPool
 * creates a list of them at startup. Note that the members of the command cannot be changed
 * after initialization. */
class YZCommand {
public:
	YZCommand( const QString &keySeq, PoolMethod pm, cmd_arg a=ARG_NONE, bool m=false) {
		mKeySeq=keySeq;
		mPoolMethod=pm;
		mArg=a;
		mMotion=m;
	}
	QString keySeq() const { return mKeySeq; }
	const PoolMethod &poolMethod() const { return mPoolMethod; }
	bool isMotion() const { return mMotion; }
	cmd_arg arg() const { return mArg; }

	/** @return true if this is a motion and s is a valid key sequence + argument */
	bool matches(const QString &s, bool fully=true) const;

	static bool isMark(const QChar &c) {
		return c >= 'a' && c <= 'z';
	}
private:
	/** the key sequence the command "listens to" */
	QString mKeySeq;
	/** the method of @ref YZCommandPool which will be called in order to execute the command */
	PoolMethod mPoolMethod;
	bool mMotion;
	/** indicates what sort of argument this command takes */
	cmd_arg mArg;
};

//oh please don't instanciate me twice !
class YZCommandPool {
	//object types definition
	enum type {
	    EX,
	    LUA
	};

	struct YZExCommand {
		type obj; //object type
		bool immutable; //is this command overwritable ?//FIXME
		bool hasCounter;
		bool hasMotion;
		bool hasRegister;
		//priorities are used to determine which functions must be used by default (for eg, if we :s -> execute :substitute instead of :set)
		//internal EX commands should use values between 1-100, plugins any values >100 (a plugin should not override an internal command IMHO)
		int priority;

		//with function pointers we are limited by class and by prototypes so ...
		QString ( YZExExecutor::*exFunc ) (YZView *view, const QString& inputsBuff);
		QString ( YZExLua::*luaFunc ) (YZView *view, const QString& inputsBuff);
		// TODO : shouldn't that be an union ?
	};
	
public:
	YZCommandPool();
	~YZCommandPool();

	// this is not a QValueList because there is no constructor with no arguments for YZCommands
	QPtrList<const YZCommand> commands;
	QStringList textObjects;

	QMap<QString, YZExCommand> globalExCommands;

	void initPool();
	void initExPool();

	/**
	 * This function is the entry point to execute any normal command in Yzis
	 */
	cmd_state execCommand(YZView *view, const QString& inputs);

	/**
	 * Entry point for ex functions ( scripting )
	 */
	void execExCommand(YZView *view, const QString& inputs);
	
	// methods implementing commands
	QString moveLeft(const YZCommandArgs &args);
	QString moveRight(const YZCommandArgs &args);
	QString moveDown(const YZCommandArgs &args);
	QString moveUp(const YZCommandArgs &args);
	QString appendAtEOL(const YZCommandArgs &args);
	QString append(const YZCommandArgs &args);
	QString changeLine(const YZCommandArgs &args);
	QString changeToEOL(const YZCommandArgs &args);
	QString deleteLine(const YZCommandArgs &args);
	QString deleteToEOL(const YZCommandArgs &args);
	QString gotoExMode(const YZCommandArgs &args);
	QString gotoFirstLine(const YZCommandArgs &args);
	QString gotoInsertMode(const YZCommandArgs &args);
	QString gotoLastLine(const YZCommandArgs &args);
	QString gotoReplaceMode(const YZCommandArgs &args);
	QString gotoVisualLineMode(const YZCommandArgs &args);
	QString gotoVisualMode(const YZCommandArgs &args);
	QString insertLineAfter(const YZCommandArgs &args);
	QString insertLineBefore(const YZCommandArgs &args);
	QString joinLine(const YZCommandArgs &args);
	QString pasteAfter(const YZCommandArgs &args);
	QString pasteBefore(const YZCommandArgs &args);
	QString yankLine(const YZCommandArgs &args);
	QString yankToEOL(const YZCommandArgs &args);
	QString closeWithoutSaving(const YZCommandArgs &args);
	QString saveAndClose(const YZCommandArgs &args);
	QString searchBackwards(const YZCommandArgs &args);
	QString searchForwards(const YZCommandArgs &args);
	QString searchNext(const YZCommandArgs &args);
	QString searchPrev(const YZCommandArgs &args);
	QString change(const YZCommandArgs &args);
	QString del(const YZCommandArgs &args);
	QString find(const YZCommandArgs &args);
	QString yank(const YZCommandArgs &args);
	QString mark(const YZCommandArgs &args);
	QString gotoMark(const YZCommandArgs &args);
	QString undo(const YZCommandArgs &args);
	QString gotoSOL(const YZCommandArgs &args);
	QString gotoEOL(const YZCommandArgs &args);
	QString moveWordForward(const YZCommandArgs &args);
};

#endif
