/* This file is part of the KDE libraries
   Copyright (C) 2003, 2004 Anders Lund <anders@alweb.dk>
   Copyright (C) 2003, 2005 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2001,2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kateextendedattribute.h"
#include <QString>

KateExtendedAttribute::KateExtendedAttribute(const QString& name, int defaultStyleIndex)
{
  setName(name);
  setDefaultStyleIndex(defaultStyleIndex);
}

int KateExtendedAttribute::indexForStyleName( const QString & name )
{
  if (name=="dsNormal") return KateExtendedAttribute::dsNormal;
  else if (name=="dsKeyword") return KateExtendedAttribute::dsKeyword;
  else if (name=="dsDataType") return KateExtendedAttribute::dsDataType;
  else if (name=="dsDecVal") return KateExtendedAttribute::dsDecVal;
  else if (name=="dsBaseN") return KateExtendedAttribute::dsBaseN;
  else if (name=="dsFloat") return KateExtendedAttribute::dsFloat;
  else if (name=="dsChar") return KateExtendedAttribute::dsChar;
  else if (name=="dsString") return KateExtendedAttribute::dsString;
  else if (name=="dsComment") return KateExtendedAttribute::dsComment;
  else if (name=="dsOthers")  return KateExtendedAttribute::dsOthers;
  else if (name=="dsAlert") return KateExtendedAttribute::dsAlert;
  else if (name=="dsFunction") return KateExtendedAttribute::dsFunction;
  else if (name=="dsRegionMarker") return KateExtendedAttribute::dsRegionMarker;
  else if (name=="dsError") return KateExtendedAttribute::dsError;

  return KateExtendedAttribute::dsNormal;
}

QString KateExtendedAttribute::name( ) const
{
  return stringProperty(AttributeName);
}

void KateExtendedAttribute::setName( const QString & name )
{
  setProperty(AttributeName, name);
}

bool KateExtendedAttribute::isDefaultStyle( ) const
{
  return hasProperty(AttributeDefaultStyleIndex);
}

int KateExtendedAttribute::defaultStyleIndex( ) const
{
  return intProperty(AttributeDefaultStyleIndex);
}

void KateExtendedAttribute::setDefaultStyleIndex( int index )
{
  setProperty(AttributeDefaultStyleIndex, QVariant(index));
}

//added for Yzis compatibility, extracted from Qt's qtextformat.cpp
void KateExtendedAttribute::setProperty(int propertyId, const QVariant &value)
{
	if (!value.isValid())
		clearProperty(propertyId);
	else
		insertProperty(propertyId, value);
}
bool KateExtendedAttribute::hasProperty(int propertyId) const
{
	return propertyIndex(propertyId) != -1;
}
int KateExtendedAttribute::propertyIndex(qint32 key) const
{
	for (int i = 0; i < props.count(); ++i)
		if (props.at(i).key == key)
			return i;
	return -1;
}
void KateExtendedAttribute::clearProperty(qint32 key)
{
	for (int i = 0; i < props.count(); ++i)
		if (props.at(i).key == key) {
			props.remove(i);
			return;
		}
}
void KateExtendedAttribute::insertProperty(qint32 key, const QVariant &value)
{
	for (int i = 0; i < props.count(); ++i)
		if (props.at(i).key == key) {
			props[i].value = value;
			return;
		}
	props.append(Property(key, value));
}
QString KateExtendedAttribute::stringProperty(int propertyId) const
{
	const QVariant prop = property(propertyId);
	if (prop.type() != QVariant::String)
		return QString();
	return prop.toString();
}
QVariant KateExtendedAttribute::property(qint32 key) const
{
	const int idx = propertyIndex(key);
	if (idx < 0)
		return QVariant();
	return props.at(idx).value;
}
int KateExtendedAttribute::intProperty(int propertyId) const
{
	const QVariant prop = property(propertyId);
	if (prop.type() != QVariant::Int)
		return 0;
	return prop.toInt();
}
//end of Yzis compatibility
