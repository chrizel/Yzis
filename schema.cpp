/* This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation
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

/* This file was taken from the Kate editor which is part of KDE
   Kate's code is published under the LGPL version 2 (and 2 only not any later 
   version)
   Copyright (C) 2001-2003 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
*/

#include "schema.h"

QString YzisSchemaManager::normalSchema ()
{
  return QString ("Yzis - Normal");
}

QString YzisSchemaManager::printingSchema ()
{
  return QString ("Yzis - Printing");
}

YzisSchemaManager::YzisSchemaManager ()
{
  update ();
}

YzisSchemaManager::~YzisSchemaManager ()
{
}

//
// read the types from config file and update the internal list
//
void YzisSchemaManager::update (bool /*readfromfile*/)
{
//  if (readfromfile)
//    m_config.reparseConfiguration ();

 // m_schemas = m_config.groupList();
//  m_schemas.sort ();

  m_schemas.removeAll (printingSchema());
  m_schemas.removeAll (normalSchema());
  m_schemas.prepend (printingSchema());
  m_schemas.prepend (normalSchema());
}

//
// get the right group
// special handling of the default schemas ;)
//
QString YzisSchemaManager::schema (uint number)
{
  if ((number>1) && (number < ( uint )m_schemas.count()))
    return m_schemas[number];
  else if (number == 1)
    return printingSchema();

    return normalSchema();
}

void YzisSchemaManager::addSchema (const QString &/*t*/)
{
/*  m_config.setGroup (t);
  m_config.writeEntry("Color Background", KGlobalSettings::baseColor());
*/
  update (false);
}

void YzisSchemaManager::removeSchema (uint number)
{
  if (number >= ( uint )m_schemas.count())
    return;

  if (number < 2)
    return;

 // m_config.deleteGroup (name (number));

  update (false);
}

bool YzisSchemaManager::validSchema (uint number)
{
  if (number < ( uint )m_schemas.count())
    return true;

  return false;
}

uint YzisSchemaManager::number (const QString &name)
{
  if (name == normalSchema())
    return 0;

  if (name == printingSchema())
    return 1;

  int i;
  if ((i = m_schemas.indexOf(name)) > -1)
    return i;

  return 0;
}

QString YzisSchemaManager::name (uint number)
{
  if ((number>1) && (number < ( uint )m_schemas.count()))
    return m_schemas[number];
  else if (number == 1)
    return printingSchema();

  return normalSchema();
}

