/*  This file is part of the Yzis libraries
*  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
*  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
*  Copyright (C) 2003-2004 Philippe Fremy <phil@freehackers.org>
*  Copyright (C) 2003-2004 Pascal "Poizon" Maillard <poizon@gmx.at>
*  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
*  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
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

#ifndef YZ_MODE_COMMAND_H
#define YZ_MODE_COMMAND_H

#include <QList>
#include <QStringList>

#include "mode.h"
#include "yzismacros.h"

class YCursor;
class YCommand;
class YInterval;
class YModeCommand;
class YView;

/** @file mode_command.h
  * Some documentation
  */

/** holds the arguments a command needs in order to execute */
struct YCommandArgs
{
    /// the command that is executed
    const YCommand *cmd;
    /// the origin of inputs
    YView *view;
    /// the registers to operate upon
    QList<QChar> regs;
    /// exec this number of times the command
    int count;
    /// was the count gave by the user
    bool usercount;
    /// the argument
    QString arg;

    YCommandArgs(const YCommand *_cmd, YView *v, const QList<QChar> &r, int c, bool user, QString a)
    {
        cmd = _cmd;
        view = v;
        regs = r;
        count = c;
        arg = a;
        usercount = user;
    }
    YCommandArgs(const YCommand *_cmd, YView *v, const QList<QChar> &r, int c, bool user)
    {
        cmd = _cmd;
        view = v;
        regs = r;
        count = c;
        usercount = user;
    }
};

class YModeCommand;
typedef CmdState (YModeCommand::*PoolMethod) (const YCommandArgs&);

enum CmdArg {
    ArgNone,
    ArgMotion,
    ArgChar,
    ArgMark,
    ArgReg,
};

/** Contains all the necessary information that makes up a normal command. @ref YModeCommand
 * creates a list of them at startup. Note that the members of the command cannot be changed
 * after initialization. */
class YZIS_EXPORT YCommand
{
public:
    YCommand( const QString &keySeq, PoolMethod pm, CmdArg a = ArgNone)
    {
        mKeySeq = keySeq;
        mPoolMethod = pm;
        mArg = a;
    }
    virtual ~YCommand()
    {}

    QString keySeq() const
    {
        return mKeySeq;
    }
    const PoolMethod &poolMethod() const
    {
        return mPoolMethod;
    }
    CmdArg arg() const
    {
        return mArg;
    }

    static bool isMark(const QChar &c)
    {
        return c >= 'a' && c <= 'z';
    }
protected:
    /** the key sequence the command "listens to" */
    QString mKeySeq;
    /** the method of @ref YModeCommand which will be called in order to execute the command */
    PoolMethod mPoolMethod;
    /** indicates what sort of argument this command takes */
    CmdArg mArg;
};


/**
 * Arguments for a motion command
 */
class YZIS_EXPORT YMotionArgs
{
public:
    explicit YMotionArgs(YView *v, int cnt = 1, QString a = QString(), QString c = QString(), bool uc = false, bool s = false)
    {
        cmd = c;
        view = v;
        count = cnt;
        arg = a;
        standalone = s;
        usercount = uc;
    }
    YView *view;
    int count;
    QString arg;
    bool standalone;
    bool usercount;
    QString cmd;
};

typedef YCursor (YModeCommand::*MotionMethod) (const YMotionArgs&, bool *);

/**
 * Command mode (The default mode of Yzis)
 *
 * Commands in command mode are implemented as methods of this class.
 */
class YZIS_EXPORT YModeCommand : public YMode
{

    friend class YMotion;

public:
    YModeCommand();
    virtual ~YModeCommand();

    virtual void init();
    /** This function is the entry point to execute any normal command in Yzis */
    virtual CmdState execCommand(YView *view, const QString& inputs);

    virtual void initPool();
    virtual void initMotionPool();
    virtual void initCommandPool();
    virtual void initModifierKeys();

    /** Parses the string inputs, which must be a valid motion + argument,
     * and executes the corresponding motion function. */
    YCursor move(YView *view, const QString &inputs, int count, bool usercount, bool *stopped );

    // methods implementing motions
    YCursor moveLeft(const YMotionArgs &args, bool *stopped);
    YCursor moveRight(const YMotionArgs &args, bool *stopped);
    YCursor moveLeftWrap(const YMotionArgs &args, bool *stopped);
    YCursor moveRightWrap(const YMotionArgs &args, bool *stopped);
    YCursor moveDown(const YMotionArgs &args, bool *stopped);
    YCursor moveUp(const YMotionArgs &args, bool *stopped);
    YCursor moveWordForward(const YMotionArgs &args, bool *stopped);
    YCursor moveSWordForward(const YMotionArgs &args, bool *stopped);
    YCursor moveWordBackward(const YMotionArgs &args, bool *stopped);
    YCursor moveSWordBackward(const YMotionArgs &args, bool *stopped);
    YCursor gotoSOL(const YMotionArgs &args, bool *stopped);
    YCursor gotoEOL(const YMotionArgs &args, bool *stopped);
    YCursor gotoStartOfDocument(const YMotionArgs &args, bool *stopped);
    YCursor gotoEndOfDocument(const YMotionArgs &args, bool *stopped);
    //YCursor find(const YMotionArgs &args);
    YCursor findNext(const YMotionArgs &args, bool *stopped);
    YCursor findBeforeNext(const YMotionArgs &args, bool *stopped);
    YCursor findPrevious(const YMotionArgs &args, bool *stopped);
    YCursor findAfterPrevious(const YMotionArgs &args, bool *stopped);
    YCursor repeatFind(const YMotionArgs &args, bool *stopped);
    YCursor matchPair(const YMotionArgs &args, bool *stopped);
    YCursor firstNonBlank(const YMotionArgs &args, bool *stopped);
    YCursor gotoMark(const YMotionArgs &args, bool *stopped);
    YCursor firstNonBlankNextLine(const YMotionArgs &args, bool *stopped);
    YCursor gotoLine(const YMotionArgs &args, bool *stopped);
    YCursor searchWord(const YMotionArgs &args, bool *stopped);
    YCursor searchNext(const YMotionArgs &args, bool *stopped);
    YCursor searchPrev(const YMotionArgs &args, bool *stopped);
    YCursor nextEmptyLine(const YMotionArgs &args, bool *stopped);
    YCursor previousEmptyLine(const YMotionArgs &args, bool *stopped);

    // methods implementing commands
    CmdState execMotion(const YCommandArgs &args);
    CmdState moveWordForward(const YCommandArgs &args);
    CmdState appendAtEOL(const YCommandArgs &args);
    CmdState append(const YCommandArgs &args);
    CmdState substitute(const YCommandArgs &args);
    CmdState changeLine(const YCommandArgs &args);
    CmdState changeToEOL(const YCommandArgs &args);
    CmdState deleteLine(const YCommandArgs &args);
    CmdState deleteToEndOfLastLine(const YCommandArgs &args);
    CmdState deleteToEOL(const YCommandArgs &args);
    CmdState gotoExMode(const YCommandArgs &args);
    CmdState gotoLineAtTop(const YCommandArgs &args);
    CmdState gotoLineAtCenter(const YCommandArgs &args);
    CmdState gotoLineAtBottom(const YCommandArgs &args);
    CmdState insertAtSOL(const YCommandArgs &args);
    CmdState insertAtCol1(const YCommandArgs &args);
    CmdState gotoInsertMode(const YCommandArgs &args);
    CmdState gotoCommandMode(const YCommandArgs &args);
    CmdState gotoReplaceMode(const YCommandArgs &args);
    CmdState gotoVisualLineMode(const YCommandArgs &args);
    CmdState gotoVisualBlockMode(const YCommandArgs &args);
    CmdState gotoVisualMode(const YCommandArgs &args);
    CmdState insertLineAfter(const YCommandArgs &args);
    CmdState insertLineBefore(const YCommandArgs &args);
    CmdState joinLine(const YCommandArgs &args);
    CmdState joinLineWithoutSpace(const YCommandArgs& args);
    CmdState pasteAfter(const YCommandArgs &args);
    CmdState pasteBefore(const YCommandArgs &args);
    CmdState yankLine(const YCommandArgs &args);
    CmdState yankToEOL(const YCommandArgs &args);
    CmdState closeWithoutSaving(const YCommandArgs &args);
    CmdState saveAndClose(const YCommandArgs &args);
    CmdState searchBackwards(const YCommandArgs &args);
    CmdState searchForwards(const YCommandArgs &args);
    CmdState change(const YCommandArgs &args);
    CmdState del(const YCommandArgs &args);
    CmdState yank(const YCommandArgs &args);
    CmdState mark(const YCommandArgs &args);
    CmdState undo(const YCommandArgs &args);
    CmdState redo(const YCommandArgs &args);
    CmdState macro(const YCommandArgs &args);
    CmdState replayMacro(const YCommandArgs &args);
    CmdState deleteChar(const YCommandArgs &args);
    CmdState deleteCharBackwards(const YCommandArgs &args);
    CmdState redisplay(const YCommandArgs &args);
    CmdState changeCase(const YCommandArgs &args);
    CmdState lineToUpperCase(const YCommandArgs &args);
    CmdState lineToLowerCase(const YCommandArgs &args);
    CmdState replace(const YCommandArgs &args);
    CmdState abort(const YCommandArgs &args);
    CmdState delkey(const YCommandArgs &args);
    CmdState indent( const YCommandArgs& args );
    CmdState scrollPageUp( const YCommandArgs &args );
    CmdState scrollPageDown( const YCommandArgs &args );
    CmdState scrollLineUp( const YCommandArgs &args );
    CmdState scrollLineDown( const YCommandArgs &args );
    CmdState redoLastCommand( const YCommandArgs & args );
    CmdState tagNext( const YCommandArgs & args );
    CmdState tagPrev( const YCommandArgs & args );
    CmdState undoJump( const YCommandArgs & args );
    CmdState incrementNumber( const YCommandArgs& args );
    CmdState decrementNumber( const YCommandArgs& args );

    QList<YCommand*> commands;
    // this is not a QValueList because there is no constructor with no arguments for YCommands
    QStringList textObjects;

    virtual YInterval interval(const YCommandArgs &args, bool *stopped);

private:
    CmdState adjustNumber( const YCommandArgs& args, int change );
};

/** This class represents a command that is also a motion. Its new member is
 * mMotionMethod, which is also a pointer to a member function of
 * @ref YModeCommand, but which does nothing but calculate the new position
 * of the cursor. This way, other commands can easily "call" this motion by executing
 * the function whose pointer they can get with motionMethod().
 * When this motion is executed as a command, the function
 * YModeCommand::execMotion() is called which itself calls the function pointed
 * to by mMotionMethod.
 */
class YZIS_EXPORT YMotion : public YCommand
{
public:
    YMotion(const QString &keySeq, MotionMethod mm, CmdArg a = ArgNone)
            : YCommand(keySeq, &YModeCommand::execMotion, a)
    {
        mMotionMethod = mm;
    }
    virtual ~YMotion()
    {}
    const MotionMethod &motionMethod() const
    {
        return mMotionMethod;
    }
    /** @return true if s is a valid key sequence + argument */
    bool matches(const QString &s, bool fully = true) const;
protected:
    MotionMethod mMotionMethod;
};


#endif

