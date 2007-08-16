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

#include "events.h"
#include "debug.h"
#include "luaengine.h"
#include "buffer.h"
#include "kate4/katehighlight.h"
#include "view.h"

#define dbg()    yzDebug("YEvents")
#define err()    yzError("YEvents")

YEvents::YEvents()
{}

YEvents::~YEvents()
{
    mEvents.clear();
}

void YEvents::connect(const QString& event, const QString& function)
{
    dbg() << "Events : connecting event " << event << " to " << function << endl;
    if ( mEvents.contains(event) ) {
        QStringList list = mEvents[event];
        if ( !list.contains(function) ) list += function;
        mEvents[event] = list;
    } else {
        QStringList list;
        list << function;
        mEvents[event] = list;
    }
}


QStringList YEvents::exec(const QString& event, YView *view)
{
    /* XXX when view is NULL, what shall we do ? */
    dbg() << "Executing event " << event << endl;
    QMap<QString, QStringList>::Iterator it = mEvents.begin(), end = mEvents.end();
    QStringList results;
    QString hlName;
    if ( view && view->myBuffer()->highlight() )
        hlName = view->myBuffer()->highlight()->name();
    hlName = hlName.toLower();
    hlName.replace("+", "p");
    for ( ; it != end; ++it ) {
        dbg() << "Comparing " << it.key() << " to " << event << endl;
        if ( QString::compare(it.key(), event) == 0 ) {
            QStringList list = it.value();
            dbg() << "Matched " << list << endl;
            foreach( QString action, list ) {
                int nbArgs = 0, nbResults = 0;
                if ( event == "INDENT_ON_ENTER" && action != "Indent_" + hlName )
                    continue; //skip it (it's not the right plugin for indent according to the current highlight name)
                else if ( event == "INDENT_ON_KEY" && action != "Indent_OnKey_" + hlName )
                    continue; //skip it (it's not the right plugin for indent according to the current highlight name)

                //special handling for indent
                if ( QString::compare(event, "INDENT_ON_KEY") == 0 ) {
                    QByteArray b = view->getInputBuffer().toUtf8();
                    const char *inputs = b.data();
                    QRegExp rx("^(\\s*).*$"); //regexp to get all tabs and spaces
                    QString curLine = view->myBuffer()->textline(view->getBufferCursor().y());
                    rx.exactMatch(curLine);
                    int nbCurTabs = rx.cap(1).count("\t");
                    int nbCurSpaces = rx.cap(1).count(" ");
                    QString nextLine;
                    if (view->getBufferCursor().y() + 1 < view->myBuffer()->lineCount())
                        nextLine = view->myBuffer()->textline(view->getBufferCursor().y() + 1);
                    rx.exactMatch(nextLine);
                    int nbNextTabs = rx.cap(1).count("\t");
                    int nbNextSpaces = rx.cap(1).count(" ");
                    QString prevLine = view->myBuffer()->textline(view->getBufferCursor().y() - 1);
                    rx.exactMatch(prevLine);
                    int nbPrevTabs = rx.cap(1).count("\t");
                    int nbPrevSpaces = rx.cap(1).count(" ");
                    QByteArray cur = curLine.toUtf8();
                    QByteArray prev = prevLine.toUtf8();
                    QByteArray next = nextLine.toUtf8();
                    YLuaEngine::self()->exe(action, "siiiiiisss", inputs, nbPrevTabs, nbPrevSpaces, nbCurTabs, nbCurSpaces, nbNextTabs, nbNextSpaces, cur.data(), prev.data(), next.data());
                } else if ( QString::compare(event, "INDENT_ON_ENTER") == 0 ) {
                    QRegExp rx("^(\\s*).*$"); //regexp to get all tabs and spaces
                    QString nextLine = view->myBuffer()->textline(view->getBufferCursor().y());
                    rx.exactMatch(nextLine);
                    int nbNextTabs = rx.cap(1).count("\t");
                    int nbNextSpaces = rx.cap(1).count(" ");
                    QString prevLine = view->myBuffer()->textline(view->getBufferCursor().y() - 1);
                    rx.exactMatch(prevLine);
                    int nbPrevTabs = rx.cap(1).count("\t");
                    int nbPrevSpaces = rx.cap(1).count(" ");
                    char *result;
                    QByteArray prev = prevLine.toUtf8();
                    QByteArray next = nextLine.toUtf8();
                    YLuaEngine::self()->exe(action, "iiiiss>s", nbNextTabs, nbNextSpaces, nbPrevTabs, nbPrevSpaces, prev.data(), next.data(), &result);
                    dbg() << "Got INDENT_ON_ENTER response : (" << result << ")" << endl;
                    results << QString(result);
                } else {
                    dbg() << "Executing plugin " << action << " with " << nbArgs << " arguments and " << nbResults << " results" << endl;
                    YLuaEngine::self()->execute(action, nbArgs, nbResults);
                    results += YLuaEngine::self()->getLastResult(1);
                }
            }
        }
    }
    return results;
}
