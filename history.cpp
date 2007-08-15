/* This file is part of the Yzis libraries
*  C++ Implementation: history
*  Author: Craig Howard <craig@choward.ca>, (C) 2005
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


#include "history.h"

#include <QStringList>
#include <QTextStream>

#define dbg()    yzDebug("YZHistory")
#define err()    yzError("YZHistory")

static QString Null = QString();

struct YZHistory::Private
{
    QStringList entries;
    QStringList::Iterator current;
};

YZHistory::YZHistory()
        : d(new Private)
{
    d->current = d->entries.end();
}

YZHistory::~YZHistory()
{
    delete d;
}

void YZHistory::addEntry( const QString &entry )
{
    // erase from the current entry until the end of the container
    // because we want to forget about that part of the stack
    d->current = d->entries.erase( d->current, d->entries.end() );

    // add the new entry and put the current pointer after it
    d->current = ++d->entries.insert( d->current, entry );
}

void YZHistory::goBackInTime()
{
    if ( !atBeginning() ) {
        --d->current;
    }
}

void YZHistory::goForwardInTime()
{
    if ( !atEnd() ) {
        ++d->current;
    }
}

bool YZHistory::atBeginning() const
{
    return d->current == d->entries.begin();
}

bool YZHistory::atEnd() const
{
    return d->current == d->entries.end();
}

const QString YZHistory::getEntry() const
{
    if ( d->current != d->entries.end() ) {
        return *d->current;
    } else {
        return Null;
    }
}

unsigned int YZHistory::getNumEntries() const
{
    return d->entries.size();
}

QString &YZHistory::getEntryByIdx( unsigned int idx )
{
    return const_cast<QString&>(static_cast<const YZHistory*>(this)->getEntryByIdx( idx ));
}

const QString &YZHistory::getEntryByIdx( unsigned int idx ) const
{
    return d->entries[ idx ];
}

bool YZHistory::isEmpty() const
{
    return d->entries.empty();
}

QTextStream &YZHistory::writeToStream( QTextStream &stream ) const
{
    static const unsigned int MAX_ENTRIES_TO_WRITE = 50;

    QStringList::iterator start = d->entries.begin();
    QStringList::iterator end = d->entries.end();

    // cap the write out at MAX_ENTRIES_TO_WRITE
    if ( getNumEntries() > MAX_ENTRIES_TO_WRITE ) {
        start += getNumEntries() - MAX_ENTRIES_TO_WRITE;
    }

    for ( QStringList::iterator i = start; i != end; ++i ) {
        stream << ":";
        stream << *i;
        stream << endl;
    }

    return stream;
}
