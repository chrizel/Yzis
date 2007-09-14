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
class YMotion;
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
    /// the input being parsed
    const YKeySequence *inputs;
    // the position in input
    YKeySequence::const_iterator *parsePos;

    YCommandArgs(const YCommand *_cmd, YView *v, const QList<QChar> &r, int c, bool user, 
                 const YKeySequence *in, YKeySequence::const_iterator *pP)
    {
        cmd = _cmd;
        view = v;
        regs = r;
        inputs = in;
        parsePos = pP;
        count = c;
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
    YCommand( const YKeySequence &keySeq, PoolMethod pm, CmdArg a = ArgNone)
    {
        mKeySeq = keySeq;
        mPoolMethod = pm;
        mArg = a;
    }
    virtual ~YCommand()
    {}

    const YKeySequence &keySeq() const
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
    YKeySequence mKeySeq;
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
    explicit YMotionArgs(YView *v, int cnt = 1, const YKeySequence *in=NULL, 
                         YKeySequence::const_iterator *pP = NULL, QString c="", bool uc = false, bool s = false)
    {
        cmd = c;
        view = v;
        count = cnt;
        inputs = in;
        parsePos = pP;
        standalone = s;
        usercount = uc;
    }
    QString cmd;
    YView *view;
    int count;
    const YKeySequence *inputs;
    YKeySequence::const_iterator *parsePos;
    bool standalone;
    bool usercount;
};

typedef YCursor (YModeCommand::*MotionMethod) (const YMotionArgs&, CmdState *);

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
    virtual CmdState execCommand(YView *view, const YKeySequence & inputs, YKeySequence::const_iterator &parsePos);

    virtual void initPool();
    void initGenericMotionPool();
    virtual void initMotionPool();
    virtual void initCommandPool();
    virtual void initModifierKeys();

    /** Parses the string inputs, which must be a valid motion + argument,
     * and executes the corresponding motion function. */
    
    YCommand *parseCommand( const YKeySequence &inputs, YKeySequence::const_iterator &parsePos );
    YMotion *parseMotion( const YKeySequence &inputs, YKeySequence::const_iterator &parsePos, int &count );

    YCursor execMotion(YView *view, const QString &inputs, int count, bool usercount, bool *stopped );

    // methods implementing motions
    YCursor moveLeft(const YMotionArgs &args, CmdState *state);
    YCursor moveRight(const YMotionArgs &args, CmdState *state);
    YCursor moveLeftWrap(const YMotionArgs &args, CmdState *state);
    YCursor moveRightWrap(const YMotionArgs &args, CmdState *state);
    YCursor moveDown(const YMotionArgs &args, CmdState *state);
    YCursor moveUp(const YMotionArgs &args, CmdState *state);
    YCursor moveWordForward(const YMotionArgs &args, CmdState *state);
    YCursor moveSWordForward(const YMotionArgs &args, CmdState *state);
    YCursor moveWordBackward(const YMotionArgs &args, CmdState *state);
    YCursor moveSWordBackward(const YMotionArgs &args, CmdState *state);
    YCursor gotoSOL(const YMotionArgs &args, CmdState *state);
    YCursor gotoEOL(const YMotionArgs &args, CmdState *state);
    YCursor gotoStartOfDocument(const YMotionArgs &args, CmdState *state);
    YCursor gotoEndOfDocument(const YMotionArgs &args, CmdState *state);
    //YCursor find(const YMotionArgs &args);
    YCursor findNext(const YMotionArgs &args, CmdState *state);
    YCursor findBeforeNext(const YMotionArgs &args, CmdState *state);
    YCursor findPrevious(const YMotionArgs &args, CmdState *state);
    YCursor findAfterPrevious(const YMotionArgs &args, CmdState *state);
    YCursor repeatFind(const YMotionArgs &args, CmdState *state);
    YCursor matchPair(const YMotionArgs &args, CmdState *state);
    YCursor firstNonBlank(const YMotionArgs &args, CmdState *state);
    YCursor gotoMark(const YMotionArgs &args, CmdState *state);
    YCursor firstNonBlankNextLine(const YMotionArgs &args, CmdState *state);
    YCursor gotoLine(const YMotionArgs &args, CmdState *state);
    YCursor searchWord(const YMotionArgs &args, CmdState *state);
    YCursor searchNext(const YMotionArgs &args, CmdState *state);
    YCursor searchPrev(const YMotionArgs &args, CmdState *state);
    YCursor nextEmptyLine(const YMotionArgs &args, CmdState *state);
    YCursor previousEmptyLine(const YMotionArgs &args, CmdState *state);
    YCursor scrollPageUp( const YMotionArgs &args, CmdState *state );
    YCursor scrollPageDown( const YMotionArgs &args, CmdState *state );
    YCursor scrollLineUp( const YMotionArgs &args, CmdState *state );
    YCursor scrollLineDown( const YMotionArgs &args, CmdState *state );

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
    CmdState redoLastCommand( const YCommandArgs & args );
    CmdState tagNext( const YCommandArgs & args );
    CmdState tagPrev( const YCommandArgs & args );
    CmdState undoJump( const YCommandArgs & args );
    CmdState incrementNumber( const YCommandArgs& args );
    CmdState decrementNumber( const YCommandArgs& args );

    QList<YCommand*> commands;
    QList<YMotion *> motions;

    // this is not a QValueList because there is no constructor with no arguments for YCommands
    QStringList textObjects;

    virtual YInterval interval(const YCommandArgs &args, CmdState *state);

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
    YMotion(const YKeySequence &keySeq, MotionMethod mm, CmdArg a = ArgNone)
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
    bool argsPresent( const YKeySequence &inputs, YKeySequence::const_iterator &parsePos ) const {
        if ( mArg == ArgNone )
            return true;
        else if ( parsePos == inputs.end() )
            return false;
        else
            return true;
    }
    
    /** @return true if s is a valid key sequence + argument */
    bool matches(const QString &s, bool fully = true) const;
protected:
    MotionMethod mMotionMethod;
};


#endif

