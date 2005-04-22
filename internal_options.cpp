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

#include "portability.h"
#include "internal_options.h"
#include "debug.h"
#include "session.h"
#if QT_VERSION < 0x040000
#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>
#else
#include <QTextStream>
#include <QDir>
#endif

YZInternalOptionPool::YZInternalOptionPool() {
	init();
	setGroup("Global");
}

YZInternalOptionPool::~YZInternalOptionPool() {
	cleanup();
	options.clear();
}

YZInternalOptionPool::YZInternalOptionPool(const YZInternalOptionPool&) {
}

void YZInternalOptionPool::loadFrom(const QString& file ) {
	QFile f( file );

	if ( !f.exists() ) return;

#if QT_VERSION < 0x040000
	if ( f.open( IO_ReadOnly ) ) {
#else
	if ( f.open( QIODevice::ReadOnly ) ) {
#endif
		QTextStream stream( &f );
		QRegExp rx("\\[(.*)\\]");
		QRegExp rx2( "(.*)=(.*)" );
		uint idx = 0;
#if QT_VERSION < 0x040000
		while ( !stream.atEnd() ) {
			QString line(stream.readLine() ); // line of text excluding '\n'
			if ( line.simplifyWhiteSpace().startsWith( "#" ) || line.isEmpty() ) continue; //skip comment and empty lines
			if ( rx.exactMatch( line ) )
				setGroup(rx.cap(1).simplifyWhiteSpace());
			else {
				if ( rx2.exactMatch( line ) ) {
					bool matched;
					setOptionFromString( &matched, rx2.cap(1).simplifyWhiteSpace()+'='+rx2.cap(2).simplifyWhiteSpace() );
					if ( ! matched ) { // this option is not known, probably a setting
						setQStringEntry( rx2.cap(1).simplifyWhiteSpace(), rx2.cap(2).simplifyWhiteSpace() );
					}
				} else
					yzDebug( "YZInternalOptionPool" ) << "Error parsing line " << idx << " of " << file << endl;
			}
			idx++;
		}
		f.close();
#else
		while ( !stream.atEnd() ) {
			QString line(stream.readLine() ); // line of text excluding '\n'
			if ( line.trimmed().startsWith( "#" ) || line.isEmpty() ) continue; //skip comment and empty lines
			if ( rx.exactMatch( line ) )
				setGroup(rx.cap(1).trimmed());
			else {
				if ( rx2.exactMatch( line ) ) {
					setOptionFromString( &matched, rx2.cap(1).trimmed()+'='+rx2.cap(2).trimmed() );
					if ( ! matched ) { // this option is not known, probably a setting
						setQStringEntry( rx2.cap(1).trimmed(), rx2.cap(2).trimmed() );
					}
					setOptionFromString( rx2.cap( 1 ).trimmed() );
				} else
					yzDebug( "YZInternalOptionPool" ) << "Error parsing line " << idx << " of " << file << endl;
			}
			idx++;
		}
		f.close();
#endif
	}
}

void YZInternalOptionPool::saveTo(const QString& file, const QString& what, const QString& except, bool force ) {
	QFile f( file );

	if ( f.exists() && !force ) return;

#if QT_VERSION < 0x040000
	if ( f.open( IO_WriteOnly ) ) {
#else
	if ( f.open( QIODevice::WriteOnly ) ) {
#endif
		QTextStream stream( &f );
		QValueList<QString> keys = mOptions.keys();
		qHeapSort( keys );
		QString cGroup = "";
		for ( unsigned int i = 0; i < keys.size(); ++i ) {
			QString myGroup = keys[i].section( "\\", 0, -2 );
			if ( !what.isEmpty() && !myGroup.startsWith( what ) ) continue; //filter !
			if ( !except.isEmpty() && myGroup.startsWith( except ) ) continue; //filter

			if ( myGroup != cGroup ) { // changing group
				stream << "\n[" << myGroup << "]\n";
				cGroup = myGroup;
			}
			YZOptionValue* ov = mOptions[ keys[i] ];
			stream << ov->parent()->name() << "=" << ov->toString() << "\n";
		}
		f.close();
	}
}

/**
 * Apply options functions
 */

void doNothing( YZBuffer*, YZView* ) {
}
void changeEncoding( YZBuffer* b, YZView* v ) {
	if ( b == NULL && v )
		b = v->myBuffer();
	if ( b ) {
		QString enc = b->getLocalStringOption("encoding");
		if ( enc != b->encoding() ) {
			if (b->fileIsModified() && YZSession::me->promptYesNo(_("File modified"), _("This file has been modified, do you want to save it ?"))){
				b->save();
			}
			b->setEncoding( enc );
		}
	}
}
void changeCursor( YZBuffer*, YZView* v ) {
	if ( v )
		v->modeChanged();
}
void refreshView( YZBuffer*, YZView* v ) {
	if ( v )
		v->refreshScreen();
}
void recalcView( YZBuffer*, YZView* v ) {
	if ( v )
		v->recalcScreen();
}
void viewUpdateListChars( YZBuffer*, YZView* v ) {
	if ( v && v->getLocalBooleanOption("list") )
		v->refreshScreen();
}
void setSyntax( YZBuffer* b, YZView* v ) {
	if ( b == NULL && v )
		b = v->myBuffer();
	if ( b )
		b->setHighLight( b->getLocalStringOption("syntax") );
}
void updateHLSearch( YZBuffer*, YZView* ) {
	YZSession::me->search()->update();
}


void YZInternalOptionPool::init() {
	// here you add new options
	options.append(new YZOptionString("backspace","eol", CXT_SESSION,global_scope, &doNothing, QStringList("bs"), QStringList::split(":","eol:indent:start")));
	options.append(new YZOptionBoolean("blocksplash",true, CXT_SESSION,global_scope, &doNothing, QStringList()));
	options.append(new YZOptionBoolean("cindent",false, CXT_BUFFER,local_scope, &doNothing, QStringList("cin")));
	QStringList cursor_shape;
	cursor_shape << "square" << "vbar" << "hbar";
	options.append(new YZOptionString("cursor","square", CXT_VIEW,local_scope, &changeCursor, QStringList(), cursor_shape) );
	options.append(new YZOptionString("cursorinsert","square", CXT_VIEW,local_scope, &changeCursor, QStringList(), cursor_shape) );
	options.append(new YZOptionString("cursorreplace","square", CXT_VIEW,local_scope, &changeCursor, QStringList(), cursor_shape) );
	options.append(new YZOptionString("encoding","locale", CXT_BUFFER,local_scope, &changeEncoding, QStringList("enc"),QStringList())); // XXX find the supported codecs
	options.append(new YZOptionString("fileencoding","", CXT_BUFFER,local_scope, &doNothing, QStringList("fenc"),QStringList()));
	options.append(new YZOptionBoolean("hlsearch",false, CXT_SESSION,global_scope, &updateHLSearch, QStringList("hls")));
	options.append(new YZOptionList("indentkeys", QStringList(), CXT_BUFFER,local_scope, &doNothing, QStringList("indk"), QStringList()));
	options.append(new YZOptionBoolean("incsearch",false, CXT_SESSION,global_scope, &doNothing, QStringList("is")));
	options.append(new YZOptionBoolean("list",false, CXT_VIEW,local_scope, &refreshView, QStringList()));
	MapOption lc;
	lc["trail"] = "-";
	lc["space"] = ".";
	lc["tab"] = ">";
	lc["eol"] = "$";
	lc["precedes"] = "<";
	lc["extends"] = ">";
	options.append(new YZOptionMap("listchars",lc, CXT_VIEW,global_scope, &viewUpdateListChars, QStringList("lcs"), lc.keys(), QStringList()));
	options.append(new YZOptionString("matchpairs","(){}[]", CXT_BUFFER,local_scope, &doNothing, QStringList("mps"), QStringList()));
	options.append(new YZOptionBoolean("number",false, CXT_VIEW,local_scope, &refreshView, QStringList("nu")));
	options.append(new YZOptionString("printer","qtprinter", CXT_VIEW,local_scope, &doNothing, QStringList(), QStringList::split(":","qtprinter:pslib")));
	options.append(new YZOptionBoolean("rightleft",false, CXT_VIEW,local_scope, &recalcView, QStringList("rl")));
	options.append(new YZOptionInteger("schema",0, CXT_BUFFER,local_scope, &refreshView, QStringList(), 0));
	options.append(new YZOptionString("syntax","", CXT_BUFFER,local_scope, &setSyntax, QStringList("syn"), QStringList())); // XXX put all name ofsyntaxes here
	options.append(new YZOptionInteger("tabstop",8, CXT_VIEW,local_scope, &recalcView, QStringList("ts"), 1));
	options.append(new YZOptionInteger("updatecount",200, CXT_SESSION,global_scope, &doNothing, QStringList("uc"), 1));
	options.append(new YZOptionBoolean("wrap",true, CXT_VIEW,local_scope, &recalcView, QStringList()));
	options.append(new YZOptionBoolean("startofline",true, CXT_VIEW,local_scope, &doNothing, QStringList("sol")));

	for( unsigned int i = 0; i < options.size(); i++ ) {
		mOptions[ "Global\\"+options[i]->name() ] = new YZOptionValue( *options[i]->defaultValue() );
	}
	setGroup("Global");

	//read config files now
	initConfFiles();
}

void YZInternalOptionPool::applyOption( YZOption* option, context_t ctx, scope_t scope, YZBuffer* b, YZView* v ) {
	if ( ctx == CXT_SESSION ) {
		option->apply( NULL, NULL );
	} else if ( ctx == CXT_BUFFER ) {
		if ( scope == global_scope ) {
			YZBufferMap bs = YZSession::me->buffers();
			YZBufferMap::Iterator it = bs.begin(), end = bs.end();
			for( ; it != end; ++it ) {
#if QT_VERSION < 0x040000
				b = it.data();
#else
				b = it.value();
#endif
				option->apply( b, v );
			}
		} else if ( b ) {
			option->apply( b, v );
		}
	} else if ( ctx == CXT_VIEW ) {
		if ( scope == global_scope ) {
			YZBufferMap bs = YZSession::me->buffers();
			YZBufferMap::Iterator it = bs.begin(), end = bs.end();
			for( ; it != end; ++it ) {
#if QT_VERSION < 0x040000
				b = it.data();
				QPtrList<YZView> vs = b->views();
#else
				b = it.value();
				QVector<YZView*> vs = b->views();
#endif
#if QT_VERSION < 0x040000
				for ( v = vs.first(); v; v = vs.next() )
					option->apply( b, v );
#else
				for ( int i = 0; i < vs.size(); ++i )
					option->apply( b, v.at(i) );
#endif
			}
		} else if ( v ) {
			option->apply( b, v );
		}
	}
}
bool YZInternalOptionPool::setOptionFromString( const QString& entry, scope_t user_scope, YZBuffer* b, YZView* v ) {
	bool test;
	return setOptionFromString( &test, entry, user_scope, b, v );
}
bool YZInternalOptionPool::setOptionFromString( bool* matched, const QString& entry, scope_t user_scope, YZBuffer* b, YZView* v ) {
	bool ret = false;
	*matched = false;
	unsigned int i;
	for ( i = 0; !(*matched) && i < options.size(); i++ ) {
		*matched = options[ i ]->context() != CXT_CONFIG && options[ i ]->match( entry );
	}
	if ( *matched ) {
		--i;
		scope_t scope = options[i]->scope();
		context_t ctx = options[i]->context();
		if ( user_scope != default_scope ) 
			scope = user_scope;
		setGroup( "Global" );
		if ( scope == local_scope ) {
			if ( b && ctx == CXT_BUFFER )
				setGroup( b->fileName() );
			else if ( v && ctx == CXT_VIEW )
				setGroup( v->getLocalOptionKey() );
		}
		ret = fillOptionFromString( options[i], entry );
		if ( ret )
			applyOption( options[i], ctx, scope, b, v );
	}
	return ret;
}
bool YZInternalOptionPool::fillOptionFromString( YZOption* opt, const QString& entry ) {
	QString option_key = currentGroup + "\\" + opt->name();
	YZOptionValue* ov = NULL;
	bool created = false;
	if ( mOptions.contains( option_key ) ) {
		ov = mOptions[ option_key ];
	} else {
		created = true;
		// we try to find a global one
		if ( mOptions.contains( "Global\\" + opt->name() ) )
			ov = new YZOptionValue( *mOptions[ "Global\\" + opt->name() ] );
		else
			ov = new YZOptionValue( *opt->defaultValue() );
	}
	bool ret = opt->setValue( entry, ov );
	if ( created ) {
		if ( ! ret ) // bad value, delete the newly created ov
			delete ov;
		else
			mOptions[ option_key ] = ov;
	}
	return ret;
}

const QString& YZInternalOptionPool::readStringOption( const QString& _key, const QString& def ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		return mOptions[ key ]->string();
	} else return def;
}

int YZInternalOptionPool::readIntegerOption( const QString& _key, int def ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		return mOptions[ key ]->integer();
	} else return def;
}

bool YZInternalOptionPool::readBooleanOption( const QString& _key, bool def ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains(  key ) ) {
		return mOptions[ key ]->boolean();
	}
	return def;
}

QStringList YZInternalOptionPool::readListOption( const QString& _key, const QStringList& def ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		return mOptions[ key ]->list();
	} 
	return def;
}
MapOption YZInternalOptionPool::readMapOption( const QString& _key ) {
	MapOption ret;
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		ret = mOptions[ key ]->map();
	} 
	return ret;
}
QColor YZInternalOptionPool::readColorOption( const QString& _key, const QColor& def ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		return mOptions[ key ]->color();
	} 
	return def;
}

const QString& YZInternalOptionPool::readQStringEntry( const QString& key , const QString& def ) {
	QString _key = currentGroup+"\\"+key;
	if ( mOptions.contains( _key ) )
		return mOptions[ _key ]->string();
	return def;
}
int YZInternalOptionPool::readIntEntry( const QString& key, int def ) {
	QString _key = currentGroup+"\\"+key;
	bool test;
	if ( mOptions.contains( _key ) )
		return YZOptionValue::integerFromString( &test, mOptions[ _key ]->string() );
	return def;
}
bool YZInternalOptionPool::readBoolEntry( const QString& key , bool def ) {
	QString _key = currentGroup+"\\"+key;
	bool test;
	if ( mOptions.contains( _key ) )
		return YZOptionValue::booleanFromString( &test, mOptions[ _key ]->string() );
	return def;
}
QStringList YZInternalOptionPool::readQStringListEntry( const QString& key, const QStringList& def ) {
	QString _key = currentGroup+"\\"+key;
	bool test;
	if ( mOptions.contains( _key ) )
		return YZOptionValue::listFromString( &test, mOptions[ _key ]->string() );
	return def;
}
QColor YZInternalOptionPool::readQColorEntry( const QString& key, const QColor& def ) {
	QString _key = currentGroup+"\\"+key;
	bool test;
	if ( mOptions.contains( _key ) )
		return YZOptionValue::colorFromString( &test, mOptions[ _key ]->string() );
	return def;
}

void YZInternalOptionPool::setQStringEntry( const QString& name, const QString& value ) {
	bool found = false, success = false;
	unsigned int i;
	YZOption* opt = NULL;
	for ( i = 0; !found && i < options.size(); i++ )
		found = options[ i ]->name() == name;
	if ( found )
		opt = options[ i-1 ];
	else
		opt = new YZOptionString( name, "", CXT_CONFIG,global_scope, &doNothing, QStringList(), QStringList() );

	success = fillOptionFromString( opt, name+'='+value );
	if ( ! success && !found )
		delete opt;
	else if ( success && !found )
		options.append( opt );
}
void YZInternalOptionPool::setBoolEntry( const QString& name, bool value ) {
	setQStringEntry( name, YZOptionValue::booleanToString( value ) );
}
void YZInternalOptionPool::setIntEntry( const QString& name, int value ) {
	setQStringEntry( name, YZOptionValue::integerToString( value ) );
}
void YZInternalOptionPool::setQStringListEntry( const QString& name, const QStringList& value ) {
	setQStringEntry( name, YZOptionValue::listToString( value ) );
}
void YZInternalOptionPool::setQColorEntry( const QString& name, const QColor& value ) {
	setQStringEntry( name, YZOptionValue::colorToString( value ) );
}

void YZInternalOptionPool::setGroup( const QString& group ) {
	currentGroup = group;
}

void YZInternalOptionPool::initConfFiles() {
	//first, do we have a config directory ?
#if QT_VERSION < 0x040000
	QDir homeConf( QDir::homeDirPath()+"/.yzis/" );
	if ( !homeConf.exists( QDir::homeDirPath()+"/.yzis/" ) )
		if ( !homeConf.mkdir(QDir::homeDirPath()+"/.yzis/", true) ) return;

	loadFrom(QDir::rootDirPath()+"/etc/yzis/yzis.conf");
	loadFrom(QDir::homeDirPath()+"/.yzis/yzis.conf");

	//load cache files
	loadFrom(QDir::homeDirPath()+"/.yzis/hl.conf");
#else
	QDir homeConf( QDir::homePath()+"/.yzis/" );
	if ( !homeConf.exists( QDir::homePath()+"/.yzis/" ) )
		if ( !homeConf.mkdir(QDir::homePath()+"/.yzis/", QDir::Recursive) ) return;

	loadFrom(QDir::rootPath()+"/etc/yzis/yzis.conf");
	loadFrom(QDir::homePath()+"/.yzis/yzis.conf");

	//load cache files
	loadFrom(QDir::homePath()+"/.yzis/hl.conf");
#endif
}

bool YZInternalOptionPool::hasGroup( const QString& group ) {
#if QT_VERSION < 0x040000
	QValueList<QString> keys = mOptions.keys();
//	qHeapSort( keys );
	QValueList<QString>::iterator it = keys.begin(), end=keys.end();
	for (; it != end ; ++it)
		if ( QStringList::split( "\\", ( *it ) )[ 0 ] == group ) return true;
	return false;
#else
	QList<QString> keys = mOptions.keys();
	for (int ab = 0 ; ab < keys.size() ; ++ab)
		if ( keys.at(ab).split( "\\" )[ 0 ] == group ) return true;
	return false;
#endif
}

void YZInternalOptionPool::cleanup() {
	QMap<QString,YZOptionValue*>::Iterator it = mOptions.begin(), end = mOptions.end();
	for ( ; it != end; ++it )
#if QT_VERSION < 0x040000
		delete it.data();
#else
		delete it.value();
#endif
	for( unsigned int i = 0; i < options.size(); i++ ) {
		delete options[i];
	}
}

bool YZInternalOptionPool::hasOption( const QString& _key ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	return (mOptions.contains( key ));
}

YZOptionValue* YZInternalOptionPool::getOption( const QString& option ) {
	QString key = option;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) )
		return mOptions[ key ];
	return NULL;
}

void YZInternalOptionPool::createOption(const QString& optionName, const QString& group, const QString& defaultValue, const QString& value, context_t ctx, value_t type ) {
	// TODO add scope_t parameter
	scope_t scope = local_scope;
	// we search for an alread existing option :
	bool found;
	unsigned int i;
	for ( i = 0; !found && i < options.size(); i++ ) {
		found = options[ i ]->name() == optionName;
	}
	if ( ! found ) {
		// create a new YZOption
		YZOption* opt = NULL;
		bool success = false;
		if ( type == boolean_t ) {
			bool d_v = YZOptionValue::booleanFromString( &success, defaultValue );
			if ( success )
				opt = new YZOptionBoolean( optionName, d_v, ctx,scope, &doNothing, QStringList() );
		} else if ( type == string_t ) {
			QString d_v = YZOptionValue::stringFromString( &success, defaultValue );
			if ( success ) 
				opt = new YZOptionString( optionName, d_v, ctx,scope, &doNothing, QStringList(), QStringList() );
		} else if ( type == integer_t ) {
			int d_v = YZOptionValue::integerFromString( &success, defaultValue );
			if ( success )
				opt = new YZOptionInteger( optionName, d_v, ctx, scope, &doNothing, QStringList() );
		} else if ( type == list_t ) {
			QStringList d_v = YZOptionValue::listFromString( &success, defaultValue );
			if ( success )
				opt = new YZOptionList( optionName, d_v, ctx,scope, &doNothing, QStringList(), QStringList() );
		} else if ( type == map_t ) {
			MapOption d_v = YZOptionValue::mapFromString( &success, defaultValue );
			if ( success )
				opt = new YZOptionMap( optionName, d_v, ctx,scope, &doNothing, QStringList(), d_v.keys(), QStringList() );
		} else if ( type == color_t ) {
			QColor d_v = YZOptionValue::colorFromString( &success, defaultValue );
			if ( success )
				opt = new YZOptionColor( optionName, d_v, ctx,scope, &doNothing, QStringList() );
		}
		if ( opt ) {
			options.append( opt );
			YZOptionValue* ov = new YZOptionValue( *opt->defaultValue() );
			success = opt->setValue( value, ov );
			if ( ! success ) { // bad value, we cannot add that new option. Delete the ov and the option itself.
				delete ov;
				delete opt;
			} else {
				mOptions[ group + "\\" + opt->name() ] = ov;
			}
		}
	}
}

void YZInternalOptionPool::updateOptions(const QString& oldPath, const QString& newPath) {
	QMap<QString,YZOptionValue*> newoptions;
	QStringList toDrop;

	//create the list of new options to add
	QMap<QString,YZOptionValue*>::Iterator it = mOptions.begin(), end = mOptions.end();
	for ( ; it != end; ++it ) {
		QString key = it.key();
		if (it.key().startsWith(oldPath)) {
			key.replace(oldPath,newPath);
#if QT_VERSION < 0x040000
			newoptions[key] = it.data();
#else
			newoptions[key] = it.value();
#endif
			toDrop << it.key();
		}
	}

	//drop old records
	for ( QStringList::Iterator it2 = toDrop.begin(); it2 != toDrop.end(); ++it2 ) {
		//dont delete the pointers, it is still used by the new option ;)
		mOptions.remove(*it2);
	}

	//add new mOptions into the QMap now
	it = newoptions.begin(), end = newoptions.end();
	for ( ; it != end; ++it ) {
#if QT_VERSION < 0x040000
		mOptions[it.key()] = it.data();
#else
		mOptions[it.key()] = it.value();
#endif
	}
}

