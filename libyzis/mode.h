/*  This file is part of the Yzis libraries
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
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
**/

#ifndef YZ_MODE_H
#define YZ_MODE_H

/* Qt */
#include <QMap>
#include <QStringList>

/* Yzis */
#include "yzis.h"
#include "keys.h"


class YDebugStream;
class YView;
class YModePool;

/** @file mode.h
  * Some documentation
  *
  */

/** Command mode state machine */
enum CmdState {
    /** The command does not exist */
    CmdError,
    /** The user hasn't entered a valid, non-ambigous command yet. */
    CmdNotYetValid,
    /** Waiting for a motion/text object. */
    CmdOperatorPending,
    /** The command has been successfully executed. */
    CmdOk,
    /** The command has been executed, but stopped due to text constraints*/
    CmdStopped,
    /** It is time to leave the event loop */
    CmdQuit,
};

YZIS_EXPORT YDebugStream& operator<<( YDebugStream& out, const CmdState & state );

/**
 * Abstract class for modes
 */
class YZIS_EXPORT YMode
{
public:
    enum ModeType {
        ModeCommand,        //!< default mode, named normal mode in vim
        ModeInsert,         //!< 'i' : entering text (insert)
        ModeReplace,        //!< 'R' : entering text (replace)
        ModeEx,             //!< ':' : execute some yzis command
        ModeSearch,         //!< '/' : search text
        ModeSearchBackward, //!< '?' : search backward
        ModeIntro,          //!< display intro text, and move to command mode on first key entered
        ModeCompletion,     //!< used from within insert mode for completion
        ModeVisual,         //!< 'v' : visual mode with characters and lines
        ModeVisualLine,     //!< 'V' : visual mode lines by lines
        ModeVisualBlock,    //!< C-V : visual mode, by blocks
    };

    YMode();
    virtual ~YMode() {}

    virtual void init();
    virtual void initModifierKeys();
    virtual void enter( YView* mView );
    virtual void leave( YView* mView );
    virtual CmdState execCommand( YView* mView, const YKeySequence & keys, 
                                  YKeySequence::const_iterator &parsePos ) = 0;

    virtual void cursorMoved( YView* mView );

    ModeType modeType() const;
    const QString& toString() const;
    yzis::MapMode mapMode() const;
    bool registered() const;
    void setRegistered( bool registered );
    QStringList modifierKeys() const;


    /** Returns true if the command line has focus in this mode.
     *
     * List of the modes for which this is true:
     *  \li ModeEx
     *  \li ModeSearch
     *  \li ModeSearchBackward
     */
    virtual bool isCommandLineMode() const;

    /** Returns if the editor widget has focus in this mode.
     *
     * List of the modes for which this is true:
     *  \li ModeInsert
     *  \li ModeReplace
     *  \li ModeIntro
     *  \li ModeCompletion
     *  \li ModeVisual
     *  \li ModeVisualLine
     *  \li ModeVisualBlock
     */
    virtual bool isEditMode() const;

    /**
     * returns true if we can select text using this mode
     */
    virtual bool isSelMode() const;

    /**
     * returns true if we can use input method in this mode
     */
    virtual bool supportsInputMethod() const;

    /**
     * Input Method
     */
    virtual void imBegin( YView* mView );
    virtual void imCompose( YView* mView, const QString& entry );
    virtual void imEnd( YView* mView, const QString& entry );

protected:
    ModeType mType;
    QString mString;
    bool mIsEditMode;
    bool mIsSelMode;
    bool mIsCmdLineMode;
    bool mIM;
    yzis::MapMode mMapMode;
    QStringList mModifierKeys;
    bool mRegistered;
};

YZIS_EXPORT YDebugStream& operator<<( YDebugStream& out, const YMode::ModeType & type );


/**
 * Mode with introductory text that is entered at startup
 */
class YModeIntro : public YMode
{
public:
    YModeIntro();
    virtual ~YModeIntro() {}

    void enter( YView* mView );
    void leave( YView* mView );
    CmdState execCommand( YView* mView, const YKeySequence& key, YKeySequence::const_iterator &parsePos );

};

typedef YMode::ModeType ModeType;

typedef QMap<ModeType, YMode*> YModeMap;
typedef QList<YMode*> YModeStack;

#endif // YZ_MODE_H
