/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Thomas Capricelli <orzel@freehackers.org>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/
#ifndef  YZIS_EVENT_LOOP
#define  YZIS_EVENT_LOOP

#if QT_VERSION < 0x040000
#include <qeventloop.h> 
#else
#include <QAbstractEventDispatcher> 
#endif


class NYZEventLoop : 
#if QT_VERSION < 0x040000
	public QEventLoop
#else
	public QAbstractEventDispatcher
#endif
{
public:
	inline NYZEventLoop () {}

	virtual bool processEvents(  QEventLoop::ProcessEventsFlags flags );
#if QT_VERSION >= 0x040000
	virtual void flush () {}
	virtual bool hasPendingEvents () { return true; }
	virtual void interrupt () {}
	virtual void registerSocketNotifier (  QSocketNotifier * notifier ) {}
	virtual void registerTimer (  int timerId, int interval, QObject * object ) {}
	virtual QList<TimerInfo> registeredTimers (  QObject * object ) const { return QList<TimerInfo>(); }
	virtual void unregisterSocketNotifier (  QSocketNotifier * notifier ) { }
	virtual bool unregisterTimer (  int timerId ) { return true; }
	virtual bool unregisterTimers (  QObject * object ) { return true; }
	virtual void wakeUp () { }
#endif
};


#endif  // YZIS_EVENT_LOOP
