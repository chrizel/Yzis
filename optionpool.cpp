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

#include <assert.h>
#include <qmap.h>
#include <qregexp.h>
#include "option.h"
#include "optionpool.h"

namespace YZOptionPool {
	//	void initConfFiles();

	// contains the information for all the options.
	QMap<QString, const YZOption*> mOptions;

	// the name to the maps below, contains the option name and the id of the
	// entity the value refers to
	struct MapName {
		QString name;
		unsigned int id;
		bool operator<(const MapName &b) const {
			return name != b.name ? name < b.name : id < b.id;
		}
	};
	// the map storing the option values
	QMap<MapName, QString> mOptionValues;

	/*
	void loadFrom(const QString& file ) {
		QFile f( file );

		if ( !f.exists() ) return;

		if ( f.open( IO_ReadOnly ) ) {
			QTextStream stream( &f );
			QRegExp rx("\\[(.*)\\]");
			QRegExp rx2( "(.*)=(.*)" );
			uint idx = 0;
			while ( !stream.atEnd() ) {
				QString line(stream.readLine() ); // line of text excluding '\n'
				if ( line.simplifyWhiteSpace().startsWith( "#" ) || line.isEmpty() ) continue; //skip comment and empty lines
				if ( rx.exactMatch( line ) )
					setGroup(rx.cap(1).simplifyWhiteSpace());
				else {
					if ( rx2.exactMatch( line ) ) {
						//we got an option there
						if ( rx2.cap( 2 ).simplifyWhiteSpace() == "true" ) {
							setBoolOption(rx2.cap( 1 ).simplifyWhiteSpace(), true);
						} else if ( rx2.cap( 2 ).simplifyWhiteSpace() == "false" ) {
							setBoolOption(rx2.cap( 1 ).simplifyWhiteSpace(), false);
						} else
							setQStringOption(rx2.cap( 1 ).simplifyWhiteSpace(), rx2.cap( 2 ).simplifyWhiteSpace());
					} else
						yzDebug( "YZOptionPool" ) << "Error parsing line " << idx << " of " << file << endl;
				}
				idx++;
			}
			f.close();
		}
	}

	void saveTo(const QString& file, const QString& what, bool force ) {
		QFile f( file );

		if ( f.exists() && !force ) return;

		if ( f.open( IO_WriteOnly ) ) {
			QTextStream stream( &f );
			QValueList<QString> names = mOptions.names();
			qHeapSort( names );
			QValueList<QString>::iterator it;
			QString cGroup = "";
			for (it = names.begin(); it != names.end() ; ++it) {
				QString myGroup = QStringList::split( "\\", ( *it ) )[ 0 ];
				if ( !myGroup.startsWith( what ) ) continue; //filter !

				if ( myGroup != cGroup ) { // changing group
					stream << "[" << myGroup << "]\n";
					cGroup = myGroup;
				}
				//dump the option + value
				stream << QStringList::split( "\\", ( *it ) )[ 1 ] << "=" << mOptions[ ( *it ) ]->getValue() << "\n";
			}
			f.close();
		}
	}

	void initConfFiles() {
		//first, do we have a config directory ?
		QDir homeConf( QDir::homeDirPath()+"/.yzis/" );
		if ( !homeConf.exists( QDir::homeDirPath()+"/.yzis/" ) )
			if ( !homeConf.mkdir(QDir::homeDirPath()+"/.yzis/", true) ) return;

		loadFrom(QDir::rootDirPath()+"/etc/yzis.conf");
		loadFrom(QDir::rootDirPath()+"/etc/yzis/yzis.conf");
		loadFrom(QDir::homeDirPath()+"/.yzis/yzis.conf");

		//load cache files
		loadFrom(QDir::homeDirPath()+"/.yzis/hl.conf");
	}
	*/

	void init() {
		addOption(new YZIntOption("tabstop", CXT_VIEW, 8, 1));
		addOption(new YZBoolOption("number", CXT_VIEW, false));
		addOption(new YZBoolOption("wrap", CXT_VIEW, false));
		addOption(new YZStringOption("backspace", CXT_VIEW, "eol"));
		addOption(new YZIntOption("updatecount", CXT_BUFFER, 200, 0));
		addOption(new YZStringOption("matchpairs", CXT_BUFFER, "(){}[]", QRegExp("(..)*")));
		addOption(new YZBoolOption("cindent", CXT_VIEW, true));
		addOption(new YZBoolOption("vicursor", CXT_VIEW, true));

		//read config files now
		//	initConfFiles();
	}

	void cleanup() {
		QMap<QString, const YZOption*>::iterator it;
		for ( it = mOptions.begin(); it != mOptions.end(); ++it ) {
			delete it.data();
		}
	}

	void addOption(const YZOption *opt) {
		QString name = opt->getName();
		if(mOptions.contains(name)) return;

		mOptions[name] = opt;
		MapName mk={name, GLOBAL_ID};
		mOptionValues[mk] = opt->getStringDefault();
	}

	// checks if an option of type type exists and return it
	const bool checkOption( const QString &name, value_t type ) {
		if(!mOptions.contains(name)) return false;
		const YZOption *opt=mOptions[name];
		if(opt->getValueType() != type) return false;
		return true;
	}

	// stores the value of an option in the map
	void storeValue( const QString &name, unsigned int id, const QString &value ) {
		MapName mk={name, id};
		mOptionValues[mk] = value;
	}

	bool setOption( const QString &name, const QString &value, unsigned int id ) {
		const YZOption *opt = getOption(name);
		if(!opt || !opt->isValid(value)) return false;
		storeValue(name, id, value);
		return true;

		// TODO: global id -> all the other ids are changed
	}

	bool setStringOption( const QString &name, const QString& value, unsigned int id ) {
		if(!checkOption(name, string_t)) return false;
		return setOption(name, value, id);
	}

	bool setIntOption( const QString &name, int value, unsigned int id ) {
		if(!checkOption(name, int_t)) return false;
		return setOption(name, QString::number(value), id);
	}

	bool setBoolOption( const QString &name, bool value, unsigned int id ) {
		if(!checkOption(name, bool_t)) return false;
		return setOption(name, value ? "yes" : "no", id);
	}

	bool setColorOption( const QString &name, const QColor& value, unsigned int id ) {
		if(!checkOption(name, color_t)) return false;
		return setOption(name, value.name(), id);
	}

	// get the value of the option name in respect to the ID id
	const QString &getOptionValue( const QString &name, unsigned int id ) {
		assert(checkOption(name, string_t));
		MapName mk={name, id};
		// if there is no local value, we take the global one
		if(!mOptionValues.contains(mk))
			mk.id=GLOBAL_ID;
		return mOptionValues[mk];
	}

	QString getStringOption( const QString &name, unsigned int id ) {
		return getOptionValue(name, id);
	}

	int getIntOption( const QString &name, unsigned int id ) {
		return getOptionValue(name, id).toInt();
	}

	bool getBoolOption( const QString &name, unsigned int id ) {
		QString v = getOptionValue(name, id);
		if(v == "yes" || v == "on" || v == "true")
			return true;
		else
			return false;
	}

	QColor getColorOption( const QString &name, unsigned int id ) {
		return QColor(getOptionValue(name, id));
	}

	bool hasOption ( const QString& name ) {
		return mOptions.contains(name);
	}

	const YZOption* getOption( const QString& name ) {
		if(hasOption(name))
			return mOptions[name];
		else
			return 0;
	}
} // end namespace YZOptionPool

QString YZOptionInterface::getStringOption( const QString & name ) {
	assert(YZOptionPool::hasOption(name));
	return YZOptionPool::getStringOption(name, getEffId(name));
}

int YZOptionInterface::getIntOption( const QString & name ) {
	assert(YZOptionPool::hasOption(name));
	return YZOptionPool::getIntOption(name, getEffId(name));
}

bool YZOptionInterface::getBoolOption( const QString & name ) {
	assert(YZOptionPool::hasOption(name));
	return YZOptionPool::getBoolOption(name, getEffId(name));
}

QColor YZOptionInterface::getColorOption( const QString & name ) {
	assert(YZOptionPool::hasOption(name));
	return YZOptionPool::getColorOption(name, getEffId(name));
}

unsigned int YZOptionInterface::getEffId( const QString & option ) {
	assert(YZOptionPool::hasOption(option));
	if(getContext() == YZOptionPool::getOption(option)->getContext())
		return getId();
	else
		return GLOBAL_ID;
}
