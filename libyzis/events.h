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

#ifndef YZ_EVENTS
#define YZ_EVENTS

#include "view.h"
#if QT_VERSION < 0x040000
#include <qstringlist.h>
#include <qmap.h>
#include <qobject.h>
#else
#include <QObject>
#include <QMap>
#include <QStringList>
#endif

class YZEvents : public QObject {
	Q_OBJECT
	
	public:
		/**
		 * Default constructor
		 */
		YZEvents();
		virtual ~YZEvents();

		/**
		 * Connect a Lua function to a specific event
		 * @param function the Lua function to be called by the event
		 * @param event the event to listen for
		 */
		void connect(const QString& event, const QString& function);

		/**
		 * Call plugins for event
		 * @param event the event to execute plugins for
		 */
		 QStringList exec(const QString& event, YZView *view=NULL);

	private:
		QMap<QString,QStringList> mEvents;
};

#endif
