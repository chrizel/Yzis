/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Mickael Marchand <marchand@kde.org>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "events.h"
#include "debug.h"
#include "ex_lua.h"
#include "buffer.h"

class YZView;

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
	yzDebug() << "Executing event " << event << endl;
	QMap<QString,QStringList>::Iterator it = mEvents.begin(), end = mEvents.end();
	QStringList results;
	QString hlName;
	if ( view->myBuffer()->highlight() )
		hlName = view->myBuffer()->highlight()->name();
#if QT_VERSION < 0x040000
	hlName = hlName.lower();
#else
	hlName = hlName.toLower();
#endif
	hlName.replace("+","p");
	for ( ; it != end; ++it ) {
		yzDebug() << "Comparing " << it.key() << " to " << event << endl;
		if ( QString::compare(it.key(), event) == 0 ) {
#if QT_VERSION < 0x040000
			QStringList list = it.data();
#else
			QStringList list = it.value();
#endif
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
					const char *inputs = view->getInputBuffer();
					QRegExp rx("^(\\s*).*$"); //regexp to get all tabs and spaces
					QString curLine = view->myBuffer()->textline(view->getBufferCursor()->getY());
					rx.exactMatch(curLine);
#if QT_VERSION < 0x040000
					int nbCurTabs = rx.cap(1).contains("\t");
					int nbCurSpaces = rx.cap(1).contains(" ");
#else
					int nbCurTabs = rx.cap(1).count("\t");
					int nbCurSpaces = rx.cap(1).count(" ");
#endif
					QString nextLine = view->myBuffer()->textline(view->getBufferCursor()->getY()+1);
					rx.exactMatch(nextLine);
#if QT_VERSION < 0x040000
					int nbNextTabs = rx.cap(1).contains("\t");
					int nbNextSpaces = rx.cap(1).contains(" ");
#else
					int nbNextTabs = rx.cap(1).count("\t");
					int nbNextSpaces = rx.cap(1).count(" ");
#endif
					QString prevLine = view->myBuffer()->textline(view->getBufferCursor()->getY()-1);
					rx.exactMatch(prevLine);
#if QT_VERSION < 0x040000
					int nbPrevTabs = rx.cap(1).contains("\t");
					int nbPrevSpaces = rx.cap(1).contains(" ");
#else
					int nbPrevTabs = rx.cap(1).count("\t");
					int nbPrevSpaces = rx.cap(1).count(" ");
#endif
					YZExLua::instance()->exe(*it2, "siiiiiisss", inputs,nbPrevTabs,nbPrevSpaces,nbCurTabs,nbCurSpaces,nbNextTabs,nbNextSpaces,(const char*)curLine,(const char*)prevLine,(const char*)nextLine);
				} else if ( QString::compare(event, "INDENT_ON_ENTER") == 0 ) {
					QRegExp rx("^(\\s*).*$"); //regexp to get all tabs and spaces
					QString nextLine = view->myBuffer()->textline(view->getBufferCursor()->getY());
					rx.exactMatch(nextLine);
#if QT_VERSION < 0x040000
					int nbNextTabs = rx.cap(1).contains("\t");
					int nbNextSpaces = rx.cap(1).contains(" ");
#else
					int nbNextTabs = rx.cap(1).count("\t");
					int nbNextSpaces = rx.cap(1).count(" ");
#endif
					QString prevLine = view->myBuffer()->textline(view->getBufferCursor()->getY()-1);
					rx.exactMatch(prevLine);
#if QT_VERSION < 0x040000
					int nbPrevTabs = rx.cap(1).contains("\t");
					int nbPrevSpaces = rx.cap(1).contains(" ");
#else
					int nbPrevTabs = rx.cap(1).count("\t");
					int nbPrevSpaces = rx.cap(1).count(" ");
#endif
					char *result;
					YZExLua::instance()->exe(*it2, "iiiiss>s",nbNextTabs,nbNextSpaces,nbPrevTabs,nbPrevSpaces, (const char*)prevLine, (const char*)nextLine, &result);
					yzDebug() << "Got INDENT_ON_ENTER response : (" << result << ")" << endl;
					results << QString(result);
				} else {
					yzDebug() << "Executing plugin " << *it2 << " with " << nbArgs << " arguments and " << nbResults << " results" << endl;
					YZExLua::instance()->execute(*it2,nbArgs,nbResults);
					results += YZExLua::instance()->getLastResult(1);
				}
			}
		}
	}
	return results;
}
