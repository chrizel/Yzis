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

#ifndef YZ_MODE_INSERT_H
#define YZ_MODE_INSERT_H

#include "mode.h"
#include "yzismacros.h"

class YMode;
class YView;

/**
 * Insert mode
 *
 * The mode that text is inserted in.
 */
class YZIS_EXPORT YModeInsert : public YMode
{
public:
    YModeInsert();
    virtual ~YModeInsert()
    {}

    virtual void enter( YView* mView );
    virtual void leave( YView* mView );
    virtual void initModifierKeys();
    virtual CmdState execCommand( YView* mView, const YKeySequence& key,
                                  YKeySequence::const_iterator &parsePos);

    virtual CmdState commandDefault( YView* mView, const QString& key );
    virtual void commandHome( YView* mView, const QString& key );
    virtual void commandEnd( YView* mView, const QString& key );
    virtual void commandDocumentHome( YView* mView, const QString& key );
    virtual void commandDocumentEnd( YView* mView, const QString& key );
    virtual void commandEscape( YView* mView, const QString& key );
    virtual void commandInsert( YView* mView, const QString& key );
    virtual void commandEx( YView* mView, const QString& key );
    virtual void commandVisual( YView* mView, const QString& key );
    virtual void commandInsertFromBelow( YView* mView, const QString& key );
    virtual void commandInsertFromAbove( YView* mView, const QString& key );
    virtual void commandCompletion( YView* mView, const QString& key );
    virtual void commandCompletionPrevious( YView* mView, const QString& key );
    virtual void commandCompletionNext( YView* mView, const QString& key );
    virtual void commandDown( YView* mView, const QString& key );
    virtual void commandUp( YView* mView, const QString& key );
    virtual void commandLeft( YView* mView, const QString& key );
    virtual void commandRight( YView* mView, const QString& key );
    virtual void commandPageUp( YView* mView, const QString& key );
    virtual void commandPageDown( YView* mView, const QString& key );
    virtual void commandBackspace( YView* mView, const QString& key );
    virtual void commandDeleteWordBefore( YView* mView, const QString& key );
    virtual void commandDel( YView* mView, const QString& key );
    virtual void commandEnter( YView* mView, const QString& key );

    virtual void imBegin( YView* mView );
    virtual void imCompose( YView* mView, const QString& entry );
    virtual void imEnd( YView* mView, const QString& entry );

protected :
    QString m_imPreedit;
};

/**
 * Replace mode
 */
class YZIS_EXPORT YModeReplace : public YModeInsert
{
public :
    YModeReplace();
    virtual ~YModeReplace()
    {}

    virtual CmdState commandDefault( YView* mView, const QString& key );
    virtual void commandInsert( YView* mView, const QString& key );
    virtual void commandBackspace( YView* mView, const QString& key );
};

#endif

