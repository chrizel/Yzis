/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Mickael Marchand <marchand@kde.org>
 *  Pascal "Poizon" Maillard <poizon@gmx.at>
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
 
#ifndef YZ_OPTIONPOOL
#define YZ_OPTIONPOOL

#include <qstring.h>
class YZOption;

/** the id for the global option values */
#define GLOBAL_ID 0xFFFFFFFF

/**
 * Contains the interface to manage the options set by the user. You can set and read the
 * values of the options and load and save them from/to a file.
 * 
 * @ref YZOptionInterface provides an easy and secure way for sessions, views and buffers
 * to read the values of their options.
 */
namespace YZOptionPool {
	/** This must be called at the beginning of the program. */
	void init();
	/** This must be called at the end of the program. */
	void cleanup();
	// Used to add an option to the pool.
	void addOption(const YZOption *opt);

	/** Load settings from @param file. */
	//void loadFrom(const QString& file);
	/** Save settings to @param file. */
	//void saveTo(const QString& file, const QString& what=QString::null, bool force=false);
	
	/** Sets the value of an option whose type is unknown or not important. */
	bool setOption( const QString &name, const QString &value, unsigned int id=GLOBAL_ID );

	/** Sets the value of a string option in respect to the object ID provided by @arg id */
	bool setStringOption( const QString &name, const QString &value, unsigned int id=GLOBAL_ID );
	/** Sets the value of an int option. */
	bool setIntOption( const QString &name, int value, unsigned int id=GLOBAL_ID );
	/** Sets the value of a bool option. */
	bool setBoolOption( const QString &name, bool value, unsigned int id=GLOBAL_ID );
	/** Sets the value of a color option. */
	bool setColorOption( const QString &name, const QColor& value, unsigned int id=GLOBAL_ID );
	
	/** Returns the value of a string option in respect to the object ID provided by @arg id */
	QString getStringOption( const QString &name, unsigned int id=GLOBAL_ID );
	/** Returns the value of an int option */
	int getIntOption( const QString &name, unsigned int id=GLOBAL_ID );
	/** Returns the value of a bool option */
	bool getBoolOption( const QString &name, unsigned int id=GLOBAL_ID );
	/** Returns the value of a color option */
	QColor getColorOption( const QString &name, unsigned int id=GLOBAL_ID );

	/** Check the existence of an option */
	bool hasOption( const QString& name );
	/** Return a pointer on a specific option. */
	const YZOption* getOption( const QString& name );
}

/** Provides facilities for objects that have local options to read them
 * easily. All they have to do is inherit this class and implement
 * @ref getContext() and @ref getId().
 */
class YZOptionInterface {
public:
	virtual ~YZOptionInterface() {}
	
	/** Returns the local value of a string option. If it is not set locally,
	 * or if the option has another context, its global value is returned. */
	QString getStringOption( const QString &name );
	/** Returns the value of an int option */
	int getIntOption( const QString &name );
	/** Returns the value of a bool option */
	bool getBoolOption( const QString &name );
	/** Returns the value of a color option */
	QColor getColorOption( const QString &name );
	
	/** Returns the context the options must have if local values are to be returned. */
	virtual context getContext() = 0;
	/** Returns the ID of the object. Objects of the same type must all have different IDs. */
	virtual unsigned int getId() = 0;
private:
	// Looks if the context of the option is the same as getContext returns
	// and returns getId() if yes or GLOBAL_ID if otherwise
	unsigned int getEffId( const QString &option );
};
	
	

#endif

