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

/* This file was taken from the Kate editor which is part of KDE
   Copyright (C) 2001-2003 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
*/

#ifndef __YZIS_SCHEMA_H__
#define __YZIS_SCHEMA_H__

#include "syntaxhighlight.h"

#include <qstringlist.h>
#include <qintdict.h>

class YzisSchemaManager
{
  public:
    YzisSchemaManager ();
    ~YzisSchemaManager ();

    /**
     * Schema Config changed, update all renderers
     */
    void update (bool readfromfile = true);

    /**
     * return kconfig with right group set or set to Normal if not there
     */
//    KConfig *schema (uint number);

    void addSchema (const QString &t);

    void removeSchema (uint number);

    /**
     * is this schema valid ? (does it exist ?)
     */
    bool validSchema (uint number);

    /**
     * if not found, defaults to 0
     */
    uint number (const QString &name);

    /**
     * group names in the end, no i18n involved
     */
    QString name (uint number);

    /**
     * Don't modify, list with the names of the schemas (i18n name for the default ones)
     */
    const QStringList &list () { return m_schemas; }

    static QString normalSchema ();
    static QString printingSchema ();

  private:
    QStringList m_schemas;
};

#endif