/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Thomas Capricelli <orzel@freehackers.org>,
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

#include <qglobal.h>
#if QT_VERSION < 0x040000
#include <qapplication.h>
#else
#include <QApplication>
#include <QAbstractEventDispatcher>
#endif

#include "debug.h"
#include "factory.h"
#include "eventloop.h"


bool NYZEventLoop::processEvents( QEventLoop::ProcessEventsFlags /*flags*/ )
{
	// from Qt Doc :
	// If the WaitForMore flag is set in flags, the behavior of this function is as follows: 
	//   If events are available, this function returns after processing them. 
	//   If no events are available, this function will wait until more are available and return after processing newly available events.
	// If the WaitForMore flag is not set in flags, and no events are available, this function will return immediately. 
	//
	// NOTE: This function will not process events continuously; it returns after all available events are processed. 

//	bool nyzis_had_some  = false;
//	if ( ! flags&QEventLoop::ExcludeUserInput ) { // doesn't work (?)
//	if (  flags&QEventLoop::AllEvents ) { // neither..
		// flush our events, only if ExcludeUserInput was not set
//		nyzis_had_some  = NYZFactory::self->process_one_event();
//		while ( NYZFactory::self->process_one_event() )
//			;
//	}

	// flush Qt ones
	bool qt_had_some = false;
//	yzDebug () << "Flags " << flags << " has events " << QEventLoop::hasPendingEvents() << endl;
	qt_had_some =  NYZFactory::self->process_one_event();
#if QT_VERSION < 0x040000
	if ( !qt_had_some && qApp->type() != QApplication::Tty && QEventLoop::hasPendingEvents() )
		qt_had_some = QEventLoop::processEvents(QEventLoop::AllEvents/*flags*/);
#else
	if ( !qt_had_some && qApp->type() != QApplication::Tty && QAbstractEventDispatcher::instance()->hasPendingEvents() )
		qt_had_some = QAbstractEventDispatcher::instance()->processEvents(QEventLoop::AllEvents/*flags*/);
#endif

	return /*true*/qt_had_some;
}

