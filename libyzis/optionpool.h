/* This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
 *  Copyright (C) 2004 Pascal "Poizon" Maillard <poizon@gmx.at>
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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/
 
 
/**
 * Class to handle the options that can be set by the user, either via :set or
 * :setlocal or via yzis.conf.
 *
 * YZOptionPool manages the descriptions to the options and the options'
 * values, the global as well as the local ones.
 *
 * 
 */
class YZOptionPool {
	public:
		/**
		 * Default constructor
		 */
		YZOptionPool();
		YZOptionPool(const YZOptionPool&);
		virtual ~YZOptionPool();

		/**
		 * Load settings from @param file
		 */
		void loadFrom(const QString& file);

		/**
		 * Save settings to @param file
		 */
		void saveTo(const QString& file, bool force=false);
		
		/**
		 * Returns the value of a string option
		 */
		const QString& readStringValue( const QString& option,
				const YZOptionContext& context = YZOptionContext::currentSession() );

		/**
		 * Sets the value of a string option
		 */
		void setStringValue( const QString& option, const QString& value );

		/**
		 * Returns the value of an int option
		 */
		int readIntValue( const QString& option,
				  const YZOptionContext& context = YZOptionContext::currentSession() );

		/**
		 * Sets the value of an int option
		 */
		void setIntValue( const QString& option, int value );

		/**
		 * Returns the value of a bool option
		 */
		bool readBoolValue( const QString& option , 
				    const YZOptionContext& context = YZOptionContext::currentSession() );

		/**
		 * Sets the value of a bool option
		 */
		void setBoolValue( const QString& option, bool value );

#if 0
		/**
		 * Returns the value of a list option
		 */
		QStringList readQStringListValue( const QString& option,
				const YZOptionContext& context = YZOptionContext::currentSession() );// QStringList::split("","") );

		/**
		 * Sets the value of a qstringlist option
		 */
		void setQStringListValue( const QString& option, const QStringList& value );
#endif

		/**
		 * Returns the value of a color option
		 */
		QColor readColorValue( const QString& option,
					const YZOptionContext& context = YZOptionContext::currentSession() );

		/**
		 * Sets the value of a color option
		 */
		void setColorValue( const QString& option, const QColor& value );

		/**
		 * Returns a pointer to an option
		 */
		YZOption* queryOption( const QString& option );

		/**
		 * Register a new option for yzis
		 */
		void registerOption(YZOption *option);

	private:
		void init();
		void initConfFiles();
		/**
		 * Clean memory
		 */
		void cleanup();

		// The registered options, indexed by their names
		QMap<QString, YZOption*> mOptions;
		// The options' values, indexed hierarchically by name (1. level)
		// and context (2. level)
		QMap<QString, QMap<YZOptionContext, QString> > mValues;
};
