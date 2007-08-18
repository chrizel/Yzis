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
typedef void (YModeCommand::*PoolMethod) (const YCommandArgs&);

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

typedef YCursor (YModeCommand::*MotionMethod) (const YMotionArgs&);

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
    YCursor move(YView *view, const QString &inputs, int count, bool usercount );

    // methods implementing motions
    YCursor moveLeft(const YMotionArgs &args);
    YCursor moveRight(const YMotionArgs &args);
    YCursor moveLeftWrap(const YMotionArgs &args);
    YCursor moveRightWrap(const YMotionArgs &args);
    YCursor moveDown(const YMotionArgs &args);
    YCursor moveUp(const YMotionArgs &args);
    YCursor moveWordForward(const YMotionArgs &args);
    YCursor moveSWordForward(const YMotionArgs &args);
    YCursor moveWordBackward(const YMotionArgs &args);
    YCursor moveSWordBackward(const YMotionArgs &args);
    YCursor gotoSOL(const YMotionArgs &args);
    YCursor gotoEOL(const YMotionArgs &args);
    YCursor gotoStartOfDocument(const YMotionArgs &args);
    YCursor gotoEndOfDocument(const YMotionArgs &args);
    //YCursor find(const YMotionArgs &args);
    YCursor findNext(const YMotionArgs &args);
    YCursor findBeforeNext(const YMotionArgs &args);
    YCursor findPrevious(const YMotionArgs &args);
    YCursor findAfterPrevious(const YMotionArgs &args);
    YCursor repeatFind(const YMotionArgs &args);
    YCursor matchPair(const YMotionArgs &args);
    YCursor firstNonBlank(const YMotionArgs &args);
    YCursor gotoMark(const YMotionArgs &args);
    YCursor firstNonBlankNextLine(const YMotionArgs &args);
    YCursor gotoLine(const YMotionArgs &args);
    YCursor searchWord(const YMotionArgs &args);
    YCursor searchNext(const YMotionArgs &args);
    YCursor searchPrev(const YMotionArgs &args);
    YCursor nextEmptyLine(const YMotionArgs &args);
    YCursor previousEmptyLine(const YMotionArgs &args);

    // methods implementing commands
    void execMotion(const YCommandArgs &args);
    void moveWordForward(const YCommandArgs &args);
    void appendAtEOL(const YCommandArgs &args);
    void append(const YCommandArgs &args);
    void substitute(const YCommandArgs &args);
    void changeLine(const YCommandArgs &args);
    void changeToEOL(const YCommandArgs &args);
    void deleteLine(const YCommandArgs &args);
    void deleteToEndOfLastLine(const YCommandArgs &args);
    void deleteToEOL(const YCommandArgs &args);
    void gotoExMode(const YCommandArgs &args);
    void gotoLineAtTop(const YCommandArgs &args);
    void gotoLineAtCenter(const YCommandArgs &args);
    void gotoLineAtBottom(const YCommandArgs &args);
    void insertAtSOL(const YCommandArgs &args);
    void insertAtCol1(const YCommandArgs &args);
    void gotoInsertMode(const YCommandArgs &args);
    void gotoCommandMode(const YCommandArgs &args);
    void gotoReplaceMode(const YCommandArgs &args);
    void gotoVisualLineMode(const YCommandArgs &args);
    void gotoVisualBlockMode(const YCommandArgs &args);
    void gotoVisualMode(const YCommandArgs &args);
    void insertLineAfter(const YCommandArgs &args);
    void insertLineBefore(const YCommandArgs &args);
    void joinLine(const YCommandArgs &args);
    void joinLineWithoutSpace(const YCommandArgs& args);
    void pasteAfter(const YCommandArgs &args);
    void pasteBefore(const YCommandArgs &args);
    void yankLine(const YCommandArgs &args);
    void yankToEOL(const YCommandArgs &args);
    void closeWithoutSaving(const YCommandArgs &args);
    void saveAndClose(const YCommandArgs &args);
    void searchBackwards(const YCommandArgs &args);
    void searchForwards(const YCommandArgs &args);
    void change(const YCommandArgs &args);
    void del(const YCommandArgs &args);
    void yank(const YCommandArgs &args);
    void mark(const YCommandArgs &args);
    void undo(const YCommandArgs &args);
    void redo(const YCommandArgs &args);
    void macro(const YCommandArgs &args);
    void replayMacro(const YCommandArgs &args);
    void deleteChar(const YCommandArgs &args);
    void deleteCharBackwards(const YCommandArgs &args);
    void redisplay(const YCommandArgs &args);
    void changeCase(const YCommandArgs &args);
    void lineToUpperCase(const YCommandArgs &args);
    void lineToLowerCase(const YCommandArgs &args);
    void replace(const YCommandArgs &args);
    void abort(const YCommandArgs &args);
    void delkey(const YCommandArgs &args);
    void indent( const YCommandArgs& args );
    void scrollPageUp( const YCommandArgs &args );
    void scrollPageDown( const YCommandArgs &args );
    void scrollLineUp( const YCommandArgs &args );
    void scrollLineDown( const YCommandArgs &args );
    void redoLastCommand( const YCommandArgs & args );
    void tagNext( const YCommandArgs & args );
    void tagPrev( const YCommandArgs & args );
    void undoJump( const YCommandArgs & args );
    void incrementNumber( const YCommandArgs& args );
    void decrementNumber( const YCommandArgs& args );

    QList<YCommand*> commands;
    // this is not a QValueList because there is no constructor with no arguments for YCommands
    QStringList textObjects;

    virtual YInterval interval(const YCommandArgs &args);

private:
    void adjustNumber( const YCommandArgs& args, int change );
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

