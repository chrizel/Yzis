/* This file is part of the KDE libraries
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

#ifndef KATEEXTENDEDATTRIBUTE_H
#define KATEEXTENDEDATTRIBUTE_H

//#include <ktexteditor/attribute.h>
#include <QList>
#include <QVariant>
#include <QVector>
#include "ksharedptr_yzis.h"

class KateExtendedAttribute;

typedef QList<KateExtendedAttribute *> KateAttributeList;

/**
 * An extension of the KTextEditor::Attribute class, with convenience functions
 * for access to extra kate-specific information, and a parent heirachy system
 * for display in the config
 */
class KateExtendedAttribute : public KShared
{
  public:
    typedef KSharedPtr<KateExtendedAttribute> Ptr;

    explicit KateExtendedAttribute(const QString& name, int defaultStyleIndex = -1);
	
	//from ktexteditor/attribute.h
	enum CustomProperties {
		/// Draws an outline around the text
#warning port me
//		Outline = QTextFormat::UserProperty,
		/// Changes the brush used to paint the text when it is selected
		SelectedForeground,
		/// Changes the brush used to paint the background when it is selected
		SelectedBackground,
		/// Determines whether background color is drawn over whitespace. Defaults to true.
		BackgroundFillWhitespace,
		/// Defined to allow storage of dynamic effect information
		AttributeDynamicEffect = 0x10A00,
		/// Defined for internal usage of KTextEditor implementations
		AttributeInternalProperty = 0x10E00,
		/// Defined to allow 3rd party code to create their own custom attributes - you may use values at or above this property.
		AttributeUserProperty = 0x110000
	};

    enum DefaultStyle {
      dsNormal,
      dsKeyword,
      dsDataType,
      dsDecVal,
      dsBaseN,
      dsFloat,
      dsChar,
      dsString,
      dsComment,
      dsOthers,
      dsAlert,
      dsFunction,
      dsRegionMarker,
      dsError
    };

    enum InternalProperties {
      AttributeName = AttributeInternalProperty,
      AttributeDefaultStyleIndex
    };

    static int indexForStyleName(const QString& name);

    QString name() const;
    void setName(const QString& name);

    bool isDefaultStyle() const;
    int defaultStyleIndex() const;
    void setDefaultStyleIndex(int index);

	//added for Yzis compatibility
	struct Property
	{
		inline Property(qint32 k, const QVariant &v) : key(k), value(v) {}
		inline Property() {}

		qint32 key;
		QVariant value;

		inline bool operator==(const Property &other) const
		{ return key == other.key && value == other.value; }
		inline bool operator!=(const Property &other) const
		{ return key != other.key || value != other.value; }
	};	
	void setProperty (int propertyId, const QVariant &value);
	bool hasProperty(int propertyId) const;
	int propertyIndex(qint32 key) const;
	void clearProperty(qint32 key);
	void insertProperty(qint32 key, const QVariant &value);
	QVariant property(qint32 key) const;
	QString stringProperty(int propertyId) const;
	int intProperty(int propertyId) const;

	QVector<Property> props;

	//end of Yzis compat
};

#endif
