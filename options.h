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

/**
 * Class to handle a single option
 * It includes : a name , a group, a default value and the current value
 */
class KOption {
	public:
		KOption( QString key, QString group, QString value, QString defaultValue);
		KOption( QString key, QString group, QStringList value, QStringList defaultValue);
		KOption( QString key, QString group, int value, int defaultValue);
		KOption( QString key, QString group, bool value, bool defaultValue);
		~KOption();

		const QString& getGroup() { return mGroup; }
		const QString& getKey() { return mKey; }
		const QString& getDefault() { return mDefaultValue; }
		const QString& getValue() { return mValue; }

	private: 
		QString mKey;
		QString mGroup;
		QString mDefaultValue;
		QString mValue;
};

/**
 * Class to handle all common options of yzis
 * 
 * every setOption and readOption has a key parameter. 
 * The key is composed of two strings, one is for the "group" of the option
 * the other is the actual key name
 */
class YZOption {
	public:
		/**
		 * Default constructor
		 */
		YZOption();
		YZOption(const YZOption&);
		~YZOption();

		/**
		 * Load settings from @param file
		 */
		static void loadFrom(const QString& file);

		/**
		 * Save settings to @param file
		 */
		static void saveTo(const QString& file);

		/**
		 * return a QString option
		 */
		const QString& readQStringEntry( const QString& key );

		/**
		 * return an int option
		 */
		int readIntEntry( const QString& key );
		
		/**
		 * return a bool option
		 */
		bool readBoolEntry( const QString& key );

		/**
		 * return a list option
		 */
		const QStringList& readQStringListEntry( const QString& key );

		/**
		 * return a QColor option
		 */
		const QColor& readQColorEntry( const QString& key );

		/**
		 * Changes the current group of options
		 */
		void setGroup( const QString& group );
		
	private:
		void init();
		
		//QString here is == group/key
		QMap<QString, KOption*> mOptions;
		QString currentGroup;
};

#endif
