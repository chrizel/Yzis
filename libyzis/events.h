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

#ifndef YZ_EVENTS
#define YZ_EVENTS

#include <QMap>
#include <QStringList>

class YView;

/**
  * Each View has one YEvents, and this is the only place where this is
  * used.
  */
class YEvents
{
public:
    /**
     * Default constructor
     */
    YEvents();
    virtual ~YEvents();

    /**
     * Connect a Lua function to a specific event
     * @param event the event to listen for
     * @param function the Lua function to be called by the event
     */
    void connect(const QString& event, const QString& function);

    /**
     * Call plugins for event
     * @param event the event to execute plugins for
     * @param view view is the YView in which to execute the event string
     * (can be different from the current view)
     */
    QStringList exec(const QString& event, YView *view = NULL);

private:
    QMap<QString, QStringList> mEvents;
};

#endif
