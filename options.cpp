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

KOption::KOption( QString key, QString group, QString value, QString defaultValue) {
	mKey = key;
	mGroup = group;
	mValue = value;
	mDefaultValue = defaultValue;
}

KOption::KOption( QString key, QString group, QStringList value, QStringList defaultValue) {
	mKey = key;
	mGroup = group;
	mValue = value.join( "/YZ/" );
	mDefaultValue = defaultValue.join( "/YZ/" );
}

KOption::KOption( QString key, QString group, int value, int defaultValue) {
	mKey = key;
	mGroup = group;
	mValue = QString::number( value );
	mDefaultValue = QString::number( defaultValue );
}

KOption::KOption( QString key, QString group, bool value, bool defaultValue) {
	mKey = key;
	mGroup = group;
	mValue = value ? QString("true") : QString("false");
	mDefaultValue = defaultValue ? QString("true") : QString("false");
}



YZOption::YZOption() {
	init();
}

YZOption::~YZOption() {
	mOptions.clear();
}

YZOption::YZOption(const YZOption&) {
}

void YZOption::loadFrom(const QString& /* file */ ) {
	
}

void YZOption::saveTo(const QString& /* file */ ) {

}

void YZOption::init() {
	KOption *tabwidth = new KOption("tabwidth", "general", 4, 8 );
	KOption *number = new KOption("number","general", false, false);

	mOptions[ "general/tabwidth" ] = tabwidth;
	mOptions[ "general/number" ] = number;
	setGroup("general");
}

const QString& YZOption::readQStringEntry( const QString& key ) {
	const QString& s = mOptions[ currentGroup + '/' + key ]->getValue();
	return s;
}

void YZOption::setQStringOption( const QString& key, const QString& option ) {
	KOption *opt = mOptions[ currentGroup + '/' + key ];
	if ( opt ) {
		opt->setValue(option);
		mOptions[ currentGroup + '/' + key ] = opt;
	} else {
		QMap<QString,KOption*>::Iterator it;
		for ( it = mOptions.begin(); it!=mOptions.end() ; it++ ) yzDebug() << "LOOP : " << it.key() << endl;
	}
}

int YZOption::readIntEntry( const QString& key ) {
	QString s = mOptions[ currentGroup + '/' + key ]->getValue();
	return s.toInt();
}

void YZOption::setIntOption( const QString& key, int option ) {
	KOption *opt = mOptions[ currentGroup + '/' + key ];
	if ( opt ) {
		opt->setValue(QString::number( option ));
		mOptions[ currentGroup + '/' + key ] = opt;
	}
}

bool YZOption::readBoolEntry( const QString& key ) {
	const QString& s = mOptions[ currentGroup + '/' + key ]->getValue();
	return s.toInt();
}

void YZOption::setBoolOption( const QString& key, bool option ) {
	KOption *opt = mOptions[ currentGroup + '/' + key ];
	if ( opt )  {
		opt->setValue(option ? "true" : "false");
		mOptions[ currentGroup + '/' + key ] = opt;
	}
}

const QStringList& YZOption::readQStringListEntry( const QString& key ) {
	const QString& s = mOptions[ currentGroup + '/' + key ]->getValue();
	const QStringList& list ( QStringList::split("/YZ/",s) );
	return list;
}

void YZOption::setQStringListOption( const QString& key, const QStringList& option ) {
	KOption *opt = mOptions[ currentGroup + '/' + key ];
	if ( opt ) {
		opt->setValue(option.join("/YZ/"));
		mOptions[ currentGroup + '/' + key ] = opt;
	}
}

const QColor& YZOption::readQColorEntry( const QString& key ) {
	const QString& s = mOptions[ currentGroup + '/' + key ]->getValue();
	const QColor& col( s );
	return col;
}

void YZOption::setQColorOption( const QString& key, const QColor& option ) {
	KOption *opt = mOptions[ currentGroup + '/' + key ];
	if ( opt ) {
		opt->setValue(option.name());
		mOptions[ currentGroup + '/' + key ] = opt;
	}
}

void YZOption::setGroup( const QString& group ) {
	currentGroup = group;
}

