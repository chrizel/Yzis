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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef YZ_INTERNAL_OPTIONS
#define YZ_INTERNAL_OPTIONS

#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qcolor.h>
#include "yzis.h"

#include "option.h"

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
		 * set an option value from a QString
		 * *matched is set to true if there is an option which mached the entry
		 * returns true if we succed to set the value
		 *  => matched && returns false => option found, but the value given in the entry is bad.
		 */
		bool setOptionFromString( bool* matched, const QString& entry, scope_t user_scope = default_scope, YZBuffer* b = NULL, YZView* v = NULL );
		bool setOptionFromString( const QString& entry, scope_t user_scope = default_scope, YZBuffer* b = NULL, YZView* v = NULL );

		
		/**
		 * Load settings from @param file
		 */
		void loadFrom(const QString& file);

		/**
		 * Save settings to @param file
		 */
		void saveTo(const QString& file, const QString& what=QString::null, const QString& except=QString::null, bool force=false);


		/**
		 * There is two type of options :
		 * 	- options, They cannot be dynamically created (except with createOption), 
		 * 		they are attached to type, a context, a scope, and may 
		 * 		run command after their value has been changed.
		 * 	- settings, simple entries in the config files.
		 *
		 * 	Methods ending with Option are designed to manipulate options
		 * 	Methods ending with Entry are designed to manipulate settings
		 */

		/**
		 * return a QString option
		 */
		const QString& readStringOption( const QString& key , const QString& def = QString::null );

		/**
		 * return an int option
		 */
		int readIntegerOption( const QString& key, int def = 0 );

		/**
		 * return a bool option
		 */
		bool readBooleanOption( const QString& key , bool def = false );

		/**
		 * return a list option
		 */
		QStringList readListOption( const QString& key, const QStringList& def = QStringList() );

		/**
		 * returns a map option
		 */
		MapOption readMapOption( const QString& key );

		/**
		 * return a color option
		 */
		QColor readColorOption( const QString& key, const QColor& def = QColor() );

		
		/**
		 * return a QString option
		 */
		const QString& readQStringEntry( const QString& key , const QString& def = QString::null );

		/**
		 * Sets a qstring option
		 */
		void setQStringEntry( const QString& key, const QString& value );

		/**
		 * return an int option
		 */
		int readIntEntry( const QString& key, int def = 0 );

		/**
		 * Sets an int option
		 */
		void setIntEntry( const QString& key, int value );

		/**
		 * return a bool option
		 */
		bool readBoolEntry( const QString& key , bool def = false );

		/**
		 * Sets a bool option
		 */
		void setBoolEntry( const QString& key, bool value );

		/**
		 * return a list option
		 */
		QStringList readQStringListEntry( const QString& key, const QStringList& def = QStringList() );// QStringList::split("","") );

		/**
		 * Sets a qstringlist option
		 */
		void setQStringListEntry( const QString& key, const QStringList& value );

		/**
		 * return a QColor option
		 */
		QColor readQColorEntry( const QString& key, const QColor& def );

		/**
		 * Sets a qcolor option
		 */
		void setQColorEntry( const QString& key, const QColor& value );


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
		YZOptionValue* getOption( const QString& option );

		/**
		 * Dynamically creates a new option for yzis
		 */
		void createOption(const QString& optionName, const QString& group, const QString& defaultValue, const QString& value, context_t ctx, value_t type );

		/**
		 * Update the keys depending on buffers file name when a buffer change his name
		 */
		void updateOptions(const QString& oldPath, const QString& newPath);

#if QT_VERSION < 0x040000
		QValueList<YZOption*> options;
#else
		QList<YZOption*> options;
#endif

	private:
		void init();
		void initConfFiles();
		void applyOption( YZOption* option, context_t ctx, scope_t scope, YZBuffer* b, YZView* v );

		bool fillOptionFromString( YZOption* opt, const QString& entry );

		/**
		 * Clean memory
		 */
		void cleanup();

		QMap<QString, YZOptionValue*> mOptions;
		QString currentGroup;
};

#endif
