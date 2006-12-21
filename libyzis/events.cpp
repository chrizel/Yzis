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
#include "syntaxhighlight.h"
#include "view.h"

YZEvents::YZEvents() {
}

YZEvents::~YZEvents() {
	mEvents.clear();
}

void YZEvents::connect(const QString& event, const QString& function) {
	yzDebug() << "Events : connecting event " << event << " to " << function << endl;
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


QStringList YZEvents::exec(const QString& event, YZView *view) {
	/* XXX when view is NULL, what shall we do ? */
	yzDebug() << "Executing event " << event << endl;
	QMap<QString,QStringList>::Iterator it = mEvents.begin(), end = mEvents.end();
	QStringList results;
	QString hlName;
	if ( view && view->myBuffer()->highlight() )
		hlName = view->myBuffer()->highlight()->name();
	hlName = hlName.toLower();
	hlName.replace("+","p");
	for ( ; it != end; ++it ) {
		yzDebug() << "Comparing " << it.key() << " to " << event << endl;
		if ( QString::compare(it.key(), event) == 0 ) {
			QStringList list = it.value();
			yzDebug() << "Matched " << list << endl;
			QStringList::Iterator it2 = list.begin(), end2 = list.end();
			for ( ; it2 != end2; ++it2 ) { 
				int nbArgs = 0, nbResults = 0;
				if ( event == "INDENT_ON_ENTER" && *it2 != "Indent_" + hlName ) 
					continue; //skip it (it's not the right plugin for indent according to the current highlight name)
				else if ( event == "INDENT_ON_KEY" && *it2 != "Indent_OnKey_" + hlName )
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
					if (view->getBufferCursor().y()+1 < view->myBuffer()->lineCount())
						nextLine = view->myBuffer()->textline(view->getBufferCursor().y()+1);
					rx.exactMatch(nextLine);
					int nbNextTabs = rx.cap(1).count("\t");
					int nbNextSpaces = rx.cap(1).count(" ");
					QString prevLine = view->myBuffer()->textline(view->getBufferCursor().y()-1);
					rx.exactMatch(prevLine);
					int nbPrevTabs = rx.cap(1).count("\t");
					int nbPrevSpaces = rx.cap(1).count(" ");
					QByteArray cur = curLine.toUtf8();
					QByteArray prev = prevLine.toUtf8();
					QByteArray next = nextLine.toUtf8();
					YZLuaEngine::self()->exe(*it2, "siiiiiisss", inputs,nbPrevTabs,nbPrevSpaces,nbCurTabs,nbCurSpaces,nbNextTabs,nbNextSpaces,cur.data(),prev.data(),next.data());
				} else if ( QString::compare(event, "INDENT_ON_ENTER") == 0 ) {
					QRegExp rx("^(\\s*).*$"); //regexp to get all tabs and spaces
					QString nextLine = view->myBuffer()->textline(view->getBufferCursor().y());
					rx.exactMatch(nextLine);
					int nbNextTabs = rx.cap(1).count("\t");
					int nbNextSpaces = rx.cap(1).count(" ");
					QString prevLine = view->myBuffer()->textline(view->getBufferCursor().y()-1);
					rx.exactMatch(prevLine);
					int nbPrevTabs = rx.cap(1).count("\t");
					int nbPrevSpaces = rx.cap(1).count(" ");
					char *result;
					QByteArray prev = prevLine.toUtf8();
					QByteArray next = nextLine.toUtf8();
					YZLuaEngine::self()->exe(*it2, "iiiiss>s",nbNextTabs,nbNextSpaces,nbPrevTabs,nbPrevSpaces, prev.data(), next.data(), &result);
					yzDebug() << "Got INDENT_ON_ENTER response : (" << result << ")" << endl;
					results << QString(result);
				} else {
					yzDebug() << "Executing plugin " << *it2 << " with " << nbArgs << " arguments and " << nbResults << " results" << endl;
					YZLuaEngine::self()->execute(*it2,nbArgs,nbResults);
					results += YZLuaEngine::self()->getLastResult(1);
				}
			}
		}
	}
	return results;
}
