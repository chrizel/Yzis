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

#include "options.h"
#include "debug.h"
#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>

KOption::KOption( QString key, QString group, QString value, QString defaultValue, option_t type) {
	mKey = key;
	mGroup = group;
	mType = type;
	mValue = value;
	mDefaultValue = defaultValue;
}

KOption::KOption( QString key, QString group, QStringList value, QStringList defaultValue, option_t type) {
	mKey = key;
	mGroup = group;
	mType = type;
	mValue = value.join( "/YZ/" );
	mDefaultValue = defaultValue.join( "/YZ/" );
}

KOption::KOption( QString key, QString group, int value, int defaultValue, option_t type) {
	mKey = key;
	mGroup = group;
	mType = type;
	mValue = QString::number( value );
	mDefaultValue = QString::number( defaultValue );
}

KOption::KOption( QString key, QString group, bool value, bool defaultValue, option_t type) {
	mKey = key;
	mGroup = group;
	mType = type;
	mValue = value ? QString::fromLatin1( "true" ) : QString::fromLatin1( "false" );
	mDefaultValue = defaultValue ? QString::fromLatin1( "true" ) : QString::fromLatin1( "false" );
}

YZOption::YZOption() {
	init();
	setGroup("General");
}

YZOption::~YZOption() {
	cleanup();
	mOptions.clear();
}

YZOption::YZOption(const YZOption&) {
}

void YZOption::loadFrom(const QString& file ) {
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
					yzDebug( "YZOption" ) << "Error parsing line " << idx << " of " << file << endl;
			}
			idx++;
		}
		f.close();
	}
}

void YZOption::saveTo(const QString& file, const QString& what, bool force ) {
	QFile f( file );

	if ( f.exists() && !force ) return;

	if ( f.open( IO_WriteOnly ) ) {
		QTextStream stream( &f );
		QValueList<QString> keys = mOptions.keys();
		qHeapSort( keys );
		QValueList<QString>::iterator it;
		QString cGroup = "";
		for (it = keys.begin(); it != keys.end() ; ++it) {
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

void YZOption::init() {
	KOption *tabwidth = new KOption("tabwidth", "General", 8, 8, view_opt );
	KOption *number = new KOption("number","General", false, false, view_opt );
	KOption *wrap = new KOption( "wrap", "General", false, false, view_opt );
	KOption *backspace = new KOption( "backspace", "General", "eol", "eol", view_opt );

	mOptions[ "General\\tabwidth" ] = tabwidth;
	mOptions[ "General\\number" ] = number;
	mOptions[ "General\\wrap" ] = wrap;
	mOptions[ "General\\backspace" ] = backspace;
	setGroup("General");

	//read config files now
	initConfFiles();
}

const QString& YZOption::readQStringEntry( const QString& _key, const QString& def ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		const QString& s = mOptions[ key ]->getValue();
		return s;
	} else return def;
}

void YZOption::setQStringOption( const QString& key, const QString& option ) {
	KOption *opt = mOptions[ currentGroup + '\\' + key ];
	if ( opt ) {
		opt->setValue(option);
		mOptions[ currentGroup + '\\' + key ] = opt;
	} else {
		opt = new KOption( currentGroup, key, option , option, getOption( key ) ? getOption( key )->getType() : global_opt );
		mOptions[ currentGroup + '\\' + key ] = opt;
//		yzDebug( "YZOption" ) << "New option " << currentGroup + '\\' + key << " added !" << endl;
	}
}

int YZOption::readIntEntry( const QString& _key, int def ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		const QString& s = mOptions[ key ]->getValue();
		return s.toInt();
	} else return def;
}

void YZOption::setIntOption( const QString& key, int option ) {
	KOption *opt = mOptions[ currentGroup + '\\' + key ];
	if ( opt ) {
		opt->setValue(QString::number( option ));
		mOptions[ currentGroup + '\\' + key ] = opt;
	} else {
		opt = new KOption( currentGroup, key, option , option, getOption( key ) ? getOption( key )->getType() : global_opt);
		mOptions[ currentGroup + '\\' + key ] = opt;
//		yzDebug( "YZOption" ) << "New option " << currentGroup + '\\' + key << " added !" << endl;
	}
}

bool YZOption::readBoolEntry( const QString& _key, bool def ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains(  key ) ) {
		KOption *opt = mOptions[ key ];
		if ( opt ) {
			QString s = opt->getValue();
			return s == QString::fromLatin1( "true" ) ? true : false;
		} else {
			yzDebug( "YZOption" ) << "Option " << key << " does not exist !" << endl;
		}
	}
	return def;
}

void YZOption::setBoolOption( const QString& key, bool option ) {
	KOption *opt = mOptions[ currentGroup + '\\' + key ];
	if ( opt )  {
		opt->setValue(option ? QString::fromLatin1( "true" ) : QString::fromLatin1( "false" ));
		mOptions[ currentGroup + '\\' + key ] = opt;
	} else {
		opt = new KOption( currentGroup, key, option , option, getOption( key ) ? getOption( key )->getType() : global_opt);
		mOptions[ currentGroup + '\\' + key ] = opt;
//		yzDebug( "YZOption" ) << "New option " << currentGroup + '\\' + key << " added !" << endl;
	}
}

const QStringList& YZOption::readQStringListEntry( const QString& _key, const QStringList& def ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		const QString& s = mOptions[ key ]->getValue();
		const QStringList& list ( QStringList::split("/YZ/",s) );
		return list;
	} else return def;
}

void YZOption::setQStringListOption( const QString& key, const QStringList& option ) {
	KOption *opt = mOptions[ currentGroup + '\\' + key ];
	if ( opt ) {
		opt->setValue(option.join("/YZ/"));
		mOptions[ currentGroup + '\\' + key ] = opt;
	} else {
		opt = new KOption( currentGroup, key, option , option, getOption( key ) ? getOption( key )->getType() : global_opt);
		mOptions[ currentGroup + '\\' + key ] = opt;
//		yzDebug( "YZOption" ) << "New option " << currentGroup + '\\' + key << " added !" << endl;
	}
}

const QColor& YZOption::readQColorEntry( const QString& _key, const QColor& def ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		const QString& s = mOptions[ key ]->getValue();
		const QColor& col( s );
		return col;
	} else return def;
}

void YZOption::setQColorOption( const QString& key, const QColor& option ) {
	KOption *opt = mOptions[ currentGroup + '\\' + key ];
	if ( opt ) {
		opt->setValue(option.name());
		mOptions[ currentGroup + '\\' + key ] = opt;
	} else {
		opt = new KOption( currentGroup, key, option.name() , option.name(), getOption( key ) ? getOption( key )->getType() : global_opt );
		mOptions[ currentGroup + '\\' + key ] = opt;
//		yzDebug( "YZOption" ) << "New option " << currentGroup + '\\' + key << " added !" << endl;
	}
}

void YZOption::setGroup( const QString& group ) {
	currentGroup = group;
}

void YZOption::initConfFiles() {
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

bool YZOption::hasGroup( const QString& group ) {
	QValueList<QString> keys = mOptions.keys();
//	qHeapSort( keys );
	QValueList<QString>::iterator it;
	for (it = keys.begin(); it != keys.end() ; ++it) {
		if ( QStringList::split( "\\", ( *it ) )[ 0 ] == group ) return true;
	}
	return false;
}

void YZOption::cleanup() {
	QMap<QString,KOption*>::Iterator it;
	for ( it = mOptions.begin(); it != mOptions.end(); ++it ) {
		delete it.data();
	}
}

bool YZOption::hasOption( const QString& _key ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	return mOptions.contains( key );
}

KOption *YZOption::getOption( const QString& option ) {
	QString key = option;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	KOption *opt = mOptions[ key ];
	return opt; //may be NULL
}

