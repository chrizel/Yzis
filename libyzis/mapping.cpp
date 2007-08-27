/* This file is part of the Yzis libraries
*  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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

#include "mapping.h"
#include "yzis.h"
#include "debug.h"
#include "luaengine.h"
#include "session.h"

#define dbg()    yzDebug("YZMapping")
#define err()    yzError("YZMapping")

using namespace yzis;

YZMapping *YZMapping::me = 0L;

YZMapping *YZMapping::self()
{
    if (YZMapping::me == 0L) YZMapping::me = new YZMapping();
    return YZMapping::me;
}

YZMapping::YZMapping()
{
    mNoremap = false;
}

YZMapping::~YZMapping()
{}

bool YZMapping::applyMappings( QString& text, QMap<QString, QString>& mappings )
{
    bool pendingMapp = false;
    QString old = text;
    QMap<QString, QString>::Iterator it = mappings.begin(), end = mappings.end();
    bool match = false;
    for (; it != end && !match ; ++it) {
        match = text.contains( it.key() );
        if ( it.value().startsWith("<Script>") && match ) {
            char *result;
            QByteArray t = it.key().toUtf8();
            YLuaEngine::self()->exe( it.value().mid(8, it.value().length() - 10), "s>s", t.data(), &result);
            text.replace(it.key(), result);
        } else if ( it.value().startsWith("<Noremap>") && match ) {
            text.replace(it.key(), it.value().right(it.value().length() - 9));
            mNoremap = true;
        } else if ( match ) {
                        if ( it.key() == "<BTAB>" ) //backtab comes with shift modifier in front of it, so remove it too
                                text.replace("<SHIFT>"+it.key(), it.value());
                        else
                                text.replace(it.key(), it.value());
        } else {
            pendingMapp = pendingMapp || it.key().startsWith(text);
        }
    }
    return pendingMapp;
}

bool YZMapping::applyMappings( QString& text, int modes, bool *mapped )
{
    // dbg() << "Text1: " << text << endl;
    bool pendingMapp = false;
    QString old = text;
    if (mNoremap) {
        mNoremap = false;
        return pendingMapp;
    }
    if ( modes & MapNormal )
        pendingMapp = pendingMapp || applyMappings(text, mNormalMappings);
    if ( modes & MapPendingOp )
        pendingMapp = pendingMapp || applyMappings(text, mPendingOpMappings);
    if ( modes & MapVisual )
        pendingMapp = pendingMapp || applyMappings(text, mVisualMappings);
    if ( modes & MapInsert )
        pendingMapp = pendingMapp || applyMappings(text, mInsertMappings);
    if ( modes & MapCmdline )
        pendingMapp = pendingMapp || applyMappings(text, mCmdLineMappings);
    // dbg() << "Text2: " << text << endl << "Pending mapping : " << pendingMapp << endl;
    *mapped = old != text;
    return pendingMapp;
}

void YZMapping::registerModifier(const QString& map)
{
    YSession::self()->registerModifier(map);
}

void YZMapping::unregisterModifier(const QString& map)
{
    YSession::self()->unregisterModifier(map);
}

