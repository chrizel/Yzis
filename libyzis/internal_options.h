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

#ifndef YZ_OPTIONS
#define YZ_OPTIONS

#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qcolor.h>
#include "yzis.h"

/**
 * Class to handle an internal option.
 * It includes : a name , a group, a default value and the current value
 */
class YZInternalOption {
	public:
		YZInternalOption( const QString& key, const QString& group, const QString& value, const QString& defaultValue, option_t type, value_t vtype);
		YZInternalOption( const QString& key, const QString& group, const QStringList& value, const QStringList& defaultValue, option_t type, value_t vtype);
		YZInternalOption( const QString& key, const QString& group, int value, int defaultValue, option_t type, value_t vtype);
		YZInternalOption( const QString& key, const QString& group, bool value, bool defaultValue, option_t type, value_t vtype);
		~YZInternalOption() {}

		const QString& getGroup() { return mGroup; }
		const QString& getKey() { return mKey; }
		const QString& getDefault() { return mDefaultValue; }
		const QString& getValue() { return mValue; }
		option_t getType() { return mType; }
		value_t getValueType() { return mValueType; }
		//this is valid only for stringlist_t options
		QString getValueForKey( const QString& key );

		void setValue( const QString& value ) { mValue = value; }

	private:
		QString mKey;
		QString mGroup;
		QString mDefaultValue;
		QString mValue;
		option_t mType;
		value_t mValueType;
};

/**
 * Class to handle the internal options.
 *
 * every setOption and readOption has a key parameter.
 * The key is composed of two strings, one is for the "group" of the option
 * the other is the actual key name
 */
class YZInternalOptionPool {
	public:
		/**
		 * Default constructor
		 */
		YZInternalOptionPool();
		YZInternalOptionPool(const YZInternalOptionPool&);
		virtual ~YZInternalOptionPool();

		/**
		 * Load settings from @param file
		 */
		void loadFrom(const QString& file);

		/**
		 * Save settings to @param file
		 */
		void saveTo(const QString& file, const QString& what=QString::null, bool force=false);

		/**
		 * return a QString option
		 */
		const QString& readQStringEntry( const QString& key , const QString& def = QString::null );

		/**
		 * Sets a qstring option
		 */
		void setQStringOption( const QString& key, const QString& value );

		/**
		 * return an int option
		 */
		int readIntEntry( const QString& key, int def = 0 );

		/**
		 * Sets an int option
		 */
		void setIntOption( const QString& key, int value );

		/**
		 * return a bool option
		 */
		bool readBoolEntry( const QString& key , bool def = false );

		/**
		 * Sets a bool option
		 */
		void setBoolOption( const QString& key, bool value );

		/**
		 * return a list option
		 */
		QStringList readQStringListEntry( const QString& key, const QStringList& def = QStringList() );// QStringList::split("","") );

		/**
		 * Sets a qstringlist option
		 */
		void setQStringListOption( const QString& key, const QStringList& value );

		/**
		 * return a QColor option
		 */
		const QColor& readQColorEntry( const QString& key, const QColor& def );

		/**
		 * Sets a qcolor option
		 */
		void setQColorOption( const QString& key, const QColor& value );

		/**
		 * Changes the current group of options
		 */
		void setGroup( const QString& group );

		/**
		 * Does this group already exists ?
		 */
		bool hasGroup ( const QString& group );

		/**
		 * Check the existence of an option
		 */
		bool hasOption ( const QString& key );

		/**
		 * Return a pointer on a specific option
		 */
		YZInternalOption* getOption( const QString& option );

	private:
		void init();
		void initConfFiles();
		/**
		 * Clean memory
		 */
		void cleanup();

		//QString here is == group/key
		QMap<QString, YZInternalOption*> mOptions;
		QString currentGroup;
};

#endif
