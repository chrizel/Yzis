/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
 *  Copyright (C) 2003-2004 Philippe Fremy <phil@freehackers.org>
 *  Copyright (C) 2003-2004 Pascal "Poizon" Maillard <poizon@gmx.at>
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
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

#ifndef YZ_MODE_COMMAND_H
#define YZ_MODE_COMMAND_H

#include "mode.h"
#include "view.h"
#include "cursor.h"
#if QT_VERSION < 0x040000
#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>
#else
#include <QList>
#include <QStringList>
#endif

class YZBuffer;
class YZView;
class YZCursor;
class YZCommand;
class YZMode;
class YZModeCommand;

/** holds the arguments a command needs in order to execute */
struct YZCommandArgs {
	//the command that is executed
	const YZCommand *cmd;
	//the origin of inputs
	YZView *view;
	//the registers to operate upon
#if QT_VERSION < 0x040000
	QValueList<QChar> regs;
#else
	QList<QChar> regs;
#endif
	//exec this number of times the command
	unsigned int count;
	//was the count gave by the user
	bool usercount;
	//the argument
	QString arg;

#if QT_VERSION < 0x040000
	YZCommandArgs(const YZCommand *_cmd, YZView *v, const QValueList<QChar> &r, unsigned int c, bool user, QString a) {
#else
	YZCommandArgs(const YZCommand *_cmd, YZView *v, const QList<QChar> &r, unsigned int c, bool user, QString a) {
#endif
		cmd=_cmd;
		view=v;
		regs=r;
		count=c;
		arg=a;
		usercount=user;
	}
#if QT_VERSION < 0x040000
	YZCommandArgs(const YZCommand *_cmd, YZView *v, const QValueList<QChar> &r, unsigned int c, bool user) {
#else
	YZCommandArgs(const YZCommand *_cmd, YZView *v, const QList<QChar> &r, unsigned int c, bool user) {
#endif
		cmd=_cmd;
		view=v;
		regs=r;
		count=c;
		usercount=user;
	}
};

class YZModeCommand;
typedef void (YZModeCommand::*PoolMethod) (const YZCommandArgs&);

enum cmd_arg {
	ARG_NONE,
	ARG_MOTION,
	ARG_CHAR,
	ARG_MARK,
	ARG_REG,
};

/** Contains all the necessary information that makes up a normal command. @ref YZModeCommand
 * creates a list of them at startup. Note that the members of the command cannot be changed
 * after initialization. */
class YZCommand {
public:
	YZCommand( const QString &keySeq, PoolMethod pm, cmd_arg a=ARG_NONE) {
		mKeySeq=keySeq;
		mPoolMethod=pm;
		mArg=a;
	}
	virtual ~YZCommand() {}

	QString keySeq() const { return mKeySeq; }
	const PoolMethod &poolMethod() const { return mPoolMethod; }
	cmd_arg arg() const { return mArg; }

	static bool isMark(const QChar &c) {
		return c >= 'a' && c <= 'z';
	}
protected:
	/** the key sequence the command "listens to" */
	QString mKeySeq;
	/** the method of @ref YZModeCommand which will be called in order to execute the command */
	PoolMethod mPoolMethod;
	/** indicates what sort of argument this command takes */
	cmd_arg mArg;
};

class YZMotionArgs;


class YZMotionArgs {
	public:
		YZMotionArgs(YZView *v, unsigned int cnt=1, QString a=QString::null,QString c=QString::null, bool uc = false, bool s=false) {
			cmd = c;
			view=v;
			count=cnt;
			arg=a;
			standalone=s;
			usercount = uc;
		}

		YZView *view;
		unsigned int count;
		QString arg;
		bool standalone;
		bool usercount;
		QString cmd;
};

typedef YZCursor (YZModeCommand::*MotionMethod) (const YZMotionArgs&);

class YZModeCommand : public YZMode {

	friend class YZMotion;

	public:
		YZModeCommand();
		virtual ~YZModeCommand();

		virtual void init();
		/** This function is the entry point to execute any normal command in Yzis */
		virtual cmd_state execCommand(YZView *view, const QString& inputs);

		virtual void initPool();
		virtual void initMotionPool();
		virtual void initCommandPool();
		virtual void initModifierKeys();

		/** Parses the string inputs, which must be a valid motion + argument,
		 * and executes the corresponding motion function. */
		YZCursor move(YZView *view, const QString &inputs, unsigned int count, bool usercount );

		// methods implementing motions
		YZCursor moveLeft(const YZMotionArgs &args);
		YZCursor moveRight(const YZMotionArgs &args);
		YZCursor moveLeftWrap(const YZMotionArgs &args);
		YZCursor moveRightWrap(const YZMotionArgs &args);
		YZCursor moveDown(const YZMotionArgs &args);
		YZCursor moveUp(const YZMotionArgs &args);
		YZCursor moveWordForward(const YZMotionArgs &args);
		YZCursor moveSWordForward(const YZMotionArgs &args);
		YZCursor moveWordBackward(const YZMotionArgs &args);
		YZCursor moveSWordBackward(const YZMotionArgs &args);
		YZCursor gotoSOL(const YZMotionArgs &args);
		YZCursor gotoEOL(const YZMotionArgs &args);
		//YZCursor find(const YZMotionArgs &args);
		YZCursor findNext(const YZMotionArgs &args);
		YZCursor findBeforeNext(const YZMotionArgs &args);
		YZCursor findPrevious(const YZMotionArgs &args);
		YZCursor findAfterPrevious(const YZMotionArgs &args);
		YZCursor repeatFind(const YZMotionArgs &args);
		YZCursor matchPair(const YZMotionArgs &args);
		YZCursor firstNonBlank(const YZMotionArgs &args);
		YZCursor gotoMark(const YZMotionArgs &args);
		YZCursor firstNonBlankNextLine(const YZMotionArgs &args);
		YZCursor gotoLine(const YZMotionArgs &args);
		YZCursor searchWord(const YZMotionArgs &args);
		YZCursor searchNext(const YZMotionArgs &args);
		YZCursor searchPrev(const YZMotionArgs &args);
		YZCursor nextEmptyLine(const YZMotionArgs &args);
		YZCursor previousEmptyLine(const YZMotionArgs &args);

		// methods implementing commands
		void execMotion(const YZCommandArgs &args);
		void moveWordForward(const YZCommandArgs &args);
		void appendAtEOL(const YZCommandArgs &args);
		void append(const YZCommandArgs &args);
		void substitute(const YZCommandArgs &args);
		void changeLine(const YZCommandArgs &args);
		void changeToEOL(const YZCommandArgs &args);
		void deleteLine(const YZCommandArgs &args);
		void deleteToEOL(const YZCommandArgs &args);
		void gotoExMode(const YZCommandArgs &args);
		void gotoLineAtTop(const YZCommandArgs &args);
		void gotoLineAtCenter(const YZCommandArgs &args);
		void gotoLineAtBottom(const YZCommandArgs &args);
		void insertAtSOL(const YZCommandArgs &args);
		void gotoInsertMode(const YZCommandArgs &args);
		void gotoCommandMode(const YZCommandArgs &args);
		void gotoReplaceMode(const YZCommandArgs &args);
		void gotoVisualLineMode(const YZCommandArgs &args);
		void gotoVisualMode(const YZCommandArgs &args);
		void insertLineAfter(const YZCommandArgs &args);
		void insertLineBefore(const YZCommandArgs &args);
		void joinLine(const YZCommandArgs &args);
		void pasteAfter(const YZCommandArgs &args);
		void pasteBefore(const YZCommandArgs &args);
		void yankLine(const YZCommandArgs &args);
		void yankToEOL(const YZCommandArgs &args);
		void closeWithoutSaving(const YZCommandArgs &args);
		void saveAndClose(const YZCommandArgs &args);
		void searchBackwards(const YZCommandArgs &args);
		void searchForwards(const YZCommandArgs &args);
		void change(const YZCommandArgs &args);
		void del(const YZCommandArgs &args);
		void yank(const YZCommandArgs &args);
		void mark(const YZCommandArgs &args);
		void undo(const YZCommandArgs &args);
		void redo(const YZCommandArgs &args);
		void macro(const YZCommandArgs &args);
		void replayMacro(const YZCommandArgs &args);
		void deleteChar(const YZCommandArgs &args);
		void deleteCharBackwards(const YZCommandArgs &args);
		void redisplay(const YZCommandArgs &args);
		void changeCase(const YZCommandArgs &args);
		void lineToUpperCase(const YZCommandArgs &args);
		void lineToLowerCase(const YZCommandArgs &args);
		void replace(const YZCommandArgs &args);
		void abort(const YZCommandArgs &args);
		void delkey(const YZCommandArgs &args);
		void indent( const YZCommandArgs& args );
		void scrollPageUp( const YZCommandArgs &args );
		void scrollPageDown( const YZCommandArgs &args );
#if QT_VERSION < 0x040000
		QPtrList<const YZCommand> commands;
#else
		QList<YZCommand*> commands;
#endif
		// this is not a QValueList because there is no constructor with no arguments for YZCommands
		QStringList textObjects;

		virtual YZInterval interval(const YZCommandArgs &args);

};

/** This class represents a command that is also a motion. Its new member is
 * mMotionMethod, which is also a pointer to a member function of
 * @ref YZModeCommand, but which does nothing but calculate the new position
 * of the cursor. This way, other commands can easily "call" this motion by executing
 * the function whose pointer they can get with @ref motionMethod().
 * When this motion is executed as a command, the function
 * YZModeCommand::execMotion() is called which itself calls the function pointed
 * to by mMotionMethod.
 */
class YZMotion : public YZCommand {
public:
	YZMotion(const QString &keySeq, MotionMethod mm, cmd_arg a=ARG_NONE)
	: YZCommand(keySeq, &YZModeCommand::execMotion, a) {
		mMotionMethod=mm;
	}
	virtual ~YZMotion() {}
	const MotionMethod &motionMethod() const { return mMotionMethod; }
	/** @return true if s is a valid key sequence + argument */
	bool matches(const QString &s, bool fully=true) const;
protected:
	MotionMethod mMotionMethod;
};


#endif

