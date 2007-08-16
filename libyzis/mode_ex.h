/*  This file is part of the Yzis libraries
*  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
*  Copyright (C) 2004 Thomas Capricelli <orzel@freehackers.org>,
*  Copyright (C) 2004 Philippe Fremy <phil@freehackers.org>,
*  Copyright (C) 2004 Pascal "Poizon" Maillard <poizon@gmx.at>,
*  Copyright (C) 2004-2005 Loic Pauleve <panard@inzenet.org>
*  Copyright (C) 2005 Erlend Hamberg <hamberg@stud.ntnu.no>
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

#ifndef YZ_MODE_EX_H
#define YZ_MODE_EX_H

/* Qt */
#include <QRegExp>
#include <QList>

/* yzis */
#include "mode.h"
#include "yzismacros.h"

class YView;
class YExCommand;
class YExRange;
class YModeEx;

/**
 * Range argument for an Ex command
 */
struct YExRangeArgs
{
    const YExRange* cmd;
    YView* view;
    QString arg;

    YExRangeArgs( const YExRange* _cmd, YView* _view, const QString& a )
    {
        cmd = _cmd;
        view = _view;
        arg = a;
    }
};

/**
 * Command arguments for an Ex command
 */
struct YExCommandArgs
{
    // caller view
    YView* view;
    // all input
    QString input;
    // command to execute
    QString cmd;
    // arguments
    QString arg;
    // range
    unsigned int fromLine;
    unsigned int toLine;
    // !
    bool force;

    YExCommandArgs( YView* _view, const QString& _input, const QString& _cmd, const QString& _arg, unsigned int _fromLine, unsigned int _toLine, bool _force );
    QString toString() const;
};

typedef CmdState (YModeEx::*ExPoolMethod) (const YExCommandArgs&);
typedef int (YModeEx::*ExRangeMethod) (const YExRangeArgs&);

/**
 * Ex range
 */
class YZIS_EXPORT YExRange
{

public :
    YExRange( const QString& regexp, ExRangeMethod pm );
    virtual ~YExRange()
    { }

    QString keySeq() const
    {
        return mKeySeq;
    }
    const ExRangeMethod& poolMethod() const
    {
        return mPoolMethod;
    }
    const QRegExp & regexp() const
    {
        return mRegexp;
    }

private :
    QRegExp mRegexp;
    QString mKeySeq;
    ExRangeMethod mPoolMethod;

};

/**
  * Command in exution mode ( as ":w" or ":q")
  */
class YZIS_EXPORT YExCommand
{
public :
    /**
      * Constructor. It creates a commands
      * @arg input is the string that is to be matched to recognise this command
      * @arg pm is the ( static ) function that gets executed when this command is entered.
      * @arg longName is a list of names for this command. (to be displayed to the user )
      * @arg word is.. ?? ( used in some regexp thinguy )
     */
    YExCommand( const QString& input, ExPoolMethod pm, const QStringList& longName = QStringList(), bool word = true );
    virtual ~YExCommand()
    { }

    const QString & keySeq() const
    {
        return mKeySeq;
    }
    const QRegExp & regexp() const
    {
        return mRegexp;
    }
    const ExPoolMethod& poolMethod() const
    {
        return mPoolMethod;
    }
    const QStringList & longName() const
    {
        return mLongName;
    }

private :
    QRegExp mRegexp;
    QString mKeySeq;
    QStringList mLongName;
    ExPoolMethod mPoolMethod;

};

class YZHistory;

/**
  * @short Execution mode ( "command ':').
  */
class YZIS_EXPORT YModeEx : public YMode
{

public :
    YModeEx();
    virtual ~YModeEx();

    void init();
    void enter( YView* mView );
    void leave( YView* mView );

    void initPool();
    CmdState execCommand( YView* mView, const QString& key );
    CmdState execExCommand( YView* view, const QString& inputs );

    YZHistory *getHistory();

private :
    QList<const YExCommand*> commands;
    QList<const YExRange*> ranges;
    YZHistory *mHistory;
    //completion stuff
    QStringList mCompletePossibilities;
    int mCurrentCompletionProposal;
    QString mCompletionCurrentSearch;

    /*
     * Init the completion for the command line
     * it will search for command names or file names
     */
    void completeCommandLine(YView *view);
    /*
     * Get the list of completion items available
     * @return a QStringList containing the completion possibilities
     */
    const QStringList& completionList();
    /*
     * Reset all completion system
     */
    void resetCompletion();
    /*
     * Get the current completion proposal
     */
    int completionIndex();
    /*
     * Get the completion proposal at position @arg idx in the internal list
     * @param idx the index position
     * @return a QString containing the completion proposal
     */
    const QString& completionItem(int idx);

    QString parseRange( const QString& inputs, YView* view, int* range, bool* matched );

    // ranges
    int rangeLine( const YExRangeArgs& args );
    int rangeCurrentLine( const YExRangeArgs& args );
    int rangeLastLine( const YExRangeArgs& args );
    int rangeMark( const YExRangeArgs& args );
    int rangeVisual( const YExRangeArgs& args );
    int rangeSearch( const YExRangeArgs& args );

public:
    // list all full command names
    const QStringList extractCommandNames();

    // commands
    CmdState write( const YExCommandArgs& args );
    CmdState quit( const YExCommandArgs& args );
    CmdState bufferfirst( const YExCommandArgs& args );
    CmdState bufferlast( const YExCommandArgs& args );
    CmdState buffernext( const YExCommandArgs& args );
    CmdState bufferprevious( const YExCommandArgs& args );
    CmdState bufferdelete( const YExCommandArgs& args );
    CmdState edit( const YExCommandArgs& args );
    CmdState mkyzisrc( const YExCommandArgs& args );
    CmdState set( const YExCommandArgs& args );
    CmdState substitute( const YExCommandArgs& args );
    CmdState hardcopy( const YExCommandArgs& args );
    CmdState gotoOpenMode( const YExCommandArgs& args );
    CmdState gotoCommandMode( const YExCommandArgs& args );
    CmdState preserve( const YExCommandArgs& args );
    CmdState lua( const YExCommandArgs& args );
    CmdState source( const YExCommandArgs& args );
    CmdState map( const YExCommandArgs& args );
    CmdState unmap( const YExCommandArgs& args );
    CmdState imap( const YExCommandArgs& args );
    CmdState iunmap( const YExCommandArgs& args );
    CmdState omap( const YExCommandArgs& args );
    CmdState ounmap( const YExCommandArgs& args );
    CmdState vmap( const YExCommandArgs& args );
    CmdState vunmap( const YExCommandArgs& args );
    CmdState cmap( const YExCommandArgs& args );
    CmdState cunmap( const YExCommandArgs& args );
    CmdState nmap( const YExCommandArgs& args );
    CmdState nunmap( const YExCommandArgs& args );
    CmdState noremap( const YExCommandArgs& args );
    CmdState onoremap( const YExCommandArgs& args );
    CmdState vnoremap( const YExCommandArgs& args );
    CmdState inoremap( const YExCommandArgs& args );
    CmdState cnoremap( const YExCommandArgs& args );
    CmdState nnoremap( const YExCommandArgs& args );
    CmdState indent( const YExCommandArgs& args );
    CmdState enew( const YExCommandArgs& args );
    CmdState syntax( const YExCommandArgs& args );
    CmdState highlight( const YExCommandArgs& args );
    CmdState registers( const YExCommandArgs& args );
    CmdState split( const YExCommandArgs& args );
    CmdState retab( const YExCommandArgs& args );
    CmdState genericMap( const YExCommandArgs& args, int );
    CmdState genericUnmap( const YExCommandArgs& args, int );
    CmdState genericNoremap( const YExCommandArgs& args, int );
    CmdState foldCreate( const YExCommandArgs& args );
    CmdState cd( const YExCommandArgs& args );
    CmdState pwd( const YExCommandArgs& args );
    CmdState tag( const YExCommandArgs& args );
    CmdState pop( const YExCommandArgs& args );
    CmdState tagnext( const YExCommandArgs& args );
    CmdState tagprevious( const YExCommandArgs& args );
};


#endif

