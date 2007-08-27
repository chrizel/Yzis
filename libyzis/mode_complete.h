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

#ifndef YZ_MODE_COMPLETE_H
#define YZ_MODE_COMPLETE_H

/* Qt */
#include <QList>

/* yzis */
#include "mode.h"
#include "cursor.h"
#include "yzismacros.h"


class YBuffer;

/**
  * @short Completion mode.
  */
class YZIS_EXPORT YModeCompletion : public YMode
{
public :
    YModeCompletion();
    virtual ~YModeCompletion();

    virtual void leave( YView* mView );
    virtual CmdState execCommand( YView* mView, const YKeySequence &keys, YKeySequence::const_iterator &parsePos);

protected :
    void doComplete( YView* mView, bool forward );
    bool initCompletion( YView* mView, bool forward );

private :
    void completeFromBuffer( YBuffer *buffer, QStringList &proposed, bool elimDups = true, QList<YCursor> *cursors = NULL );
    void completeFromCurrentBuffer( const YCursor cursor, bool forward, QStringList &proposed );
    void completeFromOtherBuffers( YBuffer *skip, QStringList &proposed );
    void completeFromIncludes( QStringList &proposed );
    void completeFromTags( QStringList &proposed );
    void completeFromDictionary( QStringList &proposed );
    void completeFromFileNames( QStringList &proposed );
    QString mPrefix;
    YCursor mCompletionStart;
    YCursor mCompletionEnd;

    QStringList mProposedCompletions;
    unsigned int mCurrentProposal;
    QString mLastMatch;

    YKey mLastKey;
    bool mForward;
};

#endif

