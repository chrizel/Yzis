/* This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
 *  Copyright (C) 2005 Erlend Hamberg <hamberg@stud.ntnu.no>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

/* Qt */
#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>
#include <QTextStream>

/* yzis */
#include "internal_options.h"
#include "debug.h"
#include "session.h"
#include "search.h"
#include "view.h"
#include "buffer.h"

#define dbg()    yzDebug("YZInternalOptionPool")
#define err()    yzError("YZInternalOptionPool")


using namespace yzis;

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

	if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
		QTextStream stream( &f );
		QRegExp rx("\\[(.*)\\]");
		QRegExp rx2( "(.*)=(.*)" );
		uint idx = 0;
		while ( !stream.atEnd() ) {
			QString line(stream.readLine() ); // line of text excluding '\n'
			if ( line.trimmed().startsWith( "#" ) || line.isEmpty() ) continue; //skip comment and empty lines
			if ( rx.exactMatch( line ) ) {
				setGroup(rx.cap(1).trimmed());
			} else {
				if ( rx2.exactMatch( line ) ) {
					bool matched=false;
					if ( rx2.numCaptures() > 1 ) {
						setOptionFromString( &matched, rx2.cap(1).trimmed()+'='+rx2.cap(2).trimmed() );
						if ( !matched ) // this option is not known, probably a setting
							setQStringEntry( rx2.cap(1).trimmed(), rx2.cap(2).trimmed() );
					} else {
						setOptionFromString( line.trimmed() );
					}
				} else
					yzDebug( "YZInternalOptionPool" ) << "Error parsing line " << idx << " of " << file << endl;
			}
			idx++;
		}
		f.close();
	}
}

void YZInternalOptionPool::saveTo(const QString& file, const QString& what, const QString& except, bool force ) {
	QFile f( file );

	if ( f.exists() && !force ) return;

	if ( f.open( QIODevice::WriteOnly ) ) {
		QTextStream stream( &f );
		QList<QString> keys = mOptions.keys();
		qSort( keys );
		QString cGroup = "";
		for ( int i = 0; i < keys.size(); ++i ) {
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
			if (b->fileIsModified() && YZSession::self()->guiPromptYesNo(_("File modified"), _("This file has been modified, do you want to save it ?"))){
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
	YZSession::self()->search()->update();
}


void YZInternalOptionPool::init() {
	// here you add new options
	options.append(new YZOptionString("backspace","eol", ContextSession,ScopeGlobal, &doNothing, QStringList("bs"), QStringList("eol" ) << "indent" << "start"));
	options.append(new YZOptionBoolean("blocksplash",true, ContextSession,ScopeGlobal, &doNothing, QStringList()));
	options.append(new YZOptionBoolean("startofline",true, ContextSession,ScopeGlobal, &doNothing, QStringList("sol")));
	options.append(new YZOptionBoolean("cindent",false, ContextBuffer,ScopeLocal, &doNothing, QStringList("cin")));
	QStringList cursor_shape;
	cursor_shape << "square" << "vbar" << "hbar";
	options.append(new YZOptionString("cursor","square", ContextView,ScopeLocal, &changeCursor, QStringList(), cursor_shape) );
	options.append(new YZOptionString("cursorinsert","square", ContextView,ScopeLocal, &changeCursor, QStringList(), cursor_shape) );
	options.append(new YZOptionString("cursorreplace","square", ContextView,ScopeLocal, &changeCursor, QStringList(), cursor_shape) );
	options.append(new YZOptionString("encoding","locale", ContextBuffer,ScopeLocal, &changeEncoding, QStringList("enc"),QStringList())); // XXX find the supported codecs
	options.append(new YZOptionString("fileencoding","", ContextBuffer,ScopeLocal, &doNothing, QStringList("fenc"),QStringList()));
	options.append(new YZOptionBoolean("hlsearch",false, ContextSession,ScopeGlobal, &updateHLSearch, QStringList("hls")));
	options.append(new YZOptionList("indentkeys", QStringList(), ContextBuffer,ScopeLocal, &doNothing, QStringList("indk"), QStringList()));
	options.append(new YZOptionBoolean("incsearch",false, ContextSession,ScopeGlobal, &doNothing, QStringList("is")));
	options.append(new YZOptionBoolean("list",false, ContextView,ScopeLocal, &refreshView, QStringList()));
	MapOption lc;
	lc["trail"] = "-";
	lc["space"] = ".";
	lc["tab"] = ">";
	lc["eol"] = "$";
	lc["precedes"] = "<";
	lc["extends"] = ">";
	options.append(new YZOptionMap("listchars",lc, ContextView,ScopeGlobal, &viewUpdateListChars, QStringList("lcs"), lc.keys(), QStringList()));
	options.append(new YZOptionString("matchpairs","(){}[]", ContextBuffer,ScopeLocal, &doNothing, QStringList("mps"), QStringList()));
	options.append(new YZOptionBoolean("number",false, ContextView,ScopeLocal, &refreshView, QStringList("nu")));
	options.append(new YZOptionString("printer","qtprinter", ContextView,ScopeLocal, &doNothing, QStringList(), QStringList("qtprinter" ) << "pslib"));
	options.append(new YZOptionBoolean("rightleft",false, ContextView,ScopeLocal, &recalcView, QStringList("rl")));
	options.append(new YZOptionBoolean("expandtab",false, ContextView,ScopeLocal, &recalcView, QStringList("et")));
	options.append(new YZOptionInteger("schema",0, ContextBuffer,ScopeLocal, &refreshView, QStringList(), 0));
	options.append(new YZOptionString("syntax","", ContextBuffer,ScopeLocal, &setSyntax, QStringList("syn"), QStringList())); // XXX put all name ofsyntaxes here
	options.append(new YZOptionInteger("tabstop",8, ContextView,ScopeLocal, &recalcView, QStringList("ts"), 1));
	options.append(new YZOptionInteger("updatecount",200, ContextSession,ScopeGlobal, &doNothing, QStringList("uc"), 1));
	options.append(new YZOptionBoolean("wrap",true, ContextView,ScopeLocal, &recalcView, QStringList()));
	options.append(new YZOptionBoolean("startofline",true, ContextView,ScopeLocal, &doNothing, QStringList("sol")));
	options.append(new YZOptionList("tags", QStringList( "tags" ), ContextSession, ScopeGlobal, &doNothing, QStringList(), QStringList()));
	options.append(new YZOptionList("complete", QStringList(".") << "w" << "b" << "u" << "t" << "i", ContextSession, ScopeGlobal, &doNothing, 
						QStringList("cpt"), QStringList()));

	for( int i = 0; i < options.size(); i++ ) {
		mOptions[ "Global\\"+options[i]->name() ] = new YZOptionValue( *options[i]->defaultValue() );
	}
	setGroup("Global");

	//read config files now
	initConfFiles();
}

void YZInternalOptionPool::applyOption( YZOption* option, OptContext ctx, OptScope scope, YZBuffer* b, YZView* v )
{
	YZASSERT(option);
	if ( ctx == ContextSession ) {
		option->apply( NULL, NULL );
	} else if ( ctx == ContextBuffer ) {
		if ( scope == ScopeGlobal ) {
			foreach( YZBuffer *buffer, YZSession::self()->buffers() )
				option->apply( buffer, v );
		} else if ( b ) {
			option->apply( b, v );
		}
	} else if ( ctx == ContextView ) {
		if ( scope == ScopeGlobal ) {
			foreach( YZBuffer *buffer, YZSession::self()->buffers() )
				foreach(  YZView *view, buffer->views() )
					option->apply( buffer, view );
		} else if ( v ) {
			option->apply( b, v );
		}
	}
}
bool YZInternalOptionPool::setOptionFromString( const QString& entry, OptScope user_scope, YZBuffer* b, YZView* v ) {
	bool test;
	return setOptionFromString( &test, entry, user_scope, b, v );
}
bool YZInternalOptionPool::setOptionFromString( bool* matched, const QString& entry, OptScope user_scope, YZBuffer* b, YZView* v ) {
	bool ret = false;
	*matched = false;
	int i;
	for ( i = 0; !(*matched) && i < options.size(); i++ ) {
		*matched = options[ i ]->context() != ContextNone && options[ i ]->match( entry );
	}
	if ( *matched ) {
		--i;
		OptScope scope = options[i]->scope();
		OptContext ctx = options[i]->context();
		if ( user_scope != ScopeDefault ) 
			scope = user_scope;
		setGroup( "Global" );
		if ( scope == ScopeLocal ) {
			if ( b && ctx == ContextBuffer )
				setGroup( b->fileName() );
			else if ( v && ctx == ContextView )
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

const QString& YZInternalOptionPool::readStringOption( const QString& _key, const QString& def ) const {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		return mOptions[ key ]->string();
	} else return def;
}

int YZInternalOptionPool::readIntegerOption( const QString& _key, int def ) const {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		return mOptions[ key ]->integer();
	} else return def;
}

bool YZInternalOptionPool::readBooleanOption( const QString& _key, bool def ) const {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains(  key ) ) {
		return mOptions[ key ]->boolean();
	}
	return def;
}

QStringList YZInternalOptionPool::readListOption( const QString& _key, const QStringList& def ) const {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		return mOptions[ key ]->list();
	} 
	return def;
}
MapOption YZInternalOptionPool::readMapOption( const QString& _key ) const {
	MapOption ret;
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		ret = mOptions[ key ]->map();
	} 
	return ret;
}
YZColor YZInternalOptionPool::readColorOption( const QString& _key, const YZColor& def ) const {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		return mOptions[ key ]->color();
	} 
	return def;
}

const QString& YZInternalOptionPool::readQStringEntry( const QString& key , const QString& def ) const {
	QString _key = currentGroup+"\\"+key;
	if ( mOptions.contains( _key ) )
		return mOptions[ _key ]->string();
	return def;
}
int YZInternalOptionPool::readIntEntry( const QString& key, int def ) const {
	QString _key = currentGroup+"\\"+key;
	bool test;
	if ( mOptions.contains( _key ) )
		return YZOptionValue::integerFromString( &test, mOptions[ _key ]->string() );
	return def;
}
bool YZInternalOptionPool::readBoolEntry( const QString& key , bool def ) const {
	QString _key = currentGroup+"\\"+key;
	bool test;
	if ( mOptions.contains( _key ) )
		return YZOptionValue::booleanFromString( &test, mOptions[ _key ]->string() );
	return def;
}
QStringList YZInternalOptionPool::readQStringListEntry( const QString& key, const QStringList& def ) const {
	QString _key = currentGroup+"\\"+key;
	bool test;
	if ( mOptions.contains( _key ) )
		return YZOptionValue::listFromString( &test, mOptions[ _key ]->string() );
	return def;
}
YZColor YZInternalOptionPool::readYZColorEntry( const QString& key, const YZColor& def ) const {
	QString _key = currentGroup+"\\"+key;
	bool test;
	if ( mOptions.contains( _key ) )
		return YZOptionValue::colorFromString( &test, mOptions[ _key ]->string() );
	return def;
}

void YZInternalOptionPool::setQStringEntry( const QString& name, const QString& value ) {
	bool found = false, success = false;
	int i;
	YZOption* opt = NULL;
	for ( i = 0; !found && i < options.size(); i++ )
		found = options[ i ]->name() == name;
	if ( found )
		opt = options[ i-1 ];
	else
		opt = new YZOptionString( name, "", ContextNone,ScopeGlobal, &doNothing, QStringList(), QStringList() );

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
void YZInternalOptionPool::setYZColorEntry( const QString& name, const YZColor& value ) {
	setQStringEntry( name, YZOptionValue::colorToString( value ) );
}

void YZInternalOptionPool::setGroup( const QString& group ) {
	currentGroup = group;
}

void YZInternalOptionPool::initConfFiles() {
	//first, do we have a config directory ?
	QDir homeConf( QDir::homePath()+"/.yzis/" );
	if ( !homeConf.exists( QDir::homePath()+"/.yzis/" ) )
		if ( !homeConf.mkpath(QDir::homePath()+"/.yzis/") ) return;

	loadFrom(QDir::rootPath()+"/etc/yzis/yzis.conf");
	loadFrom(QDir::homePath()+"/.yzis/yzis.conf");

	//load cache files
	loadFrom(QDir::homePath()+"/.yzis/hl.conf");
}

bool YZInternalOptionPool::hasGroup( const QString& group ) const {
	QList<QString> keys = mOptions.keys();
	for (int ab = 0 ; ab < keys.size() ; ++ab)
		if ( keys.at(ab).split( "\\" )[ 0 ] == group ) return true;
	return false;
}

void YZInternalOptionPool::cleanup() {
	QMap<QString,YZOptionValue*>::Iterator it = mOptions.begin(), end = mOptions.end();
	for ( ; it != end; ++it )
		delete it.value();
	for( int i = 0; i < options.size(); i++ ) {
		delete options[i];
	}
}

bool YZInternalOptionPool::hasOption( const QString& _key ) const {
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

void YZInternalOptionPool::createOption(const QString& optionName, const QString& group, const QString& defaultValue, const QString& value, OptContext ctx, OptType type ) {
	// TODO add OptScope parameter
	OptScope scope = ScopeLocal;
	// we search for an already existing option :
	bool found = false;
	int i;
	for ( i = 0; !found && i < options.size(); i++ ) {
		found = options[ i ]->name() == optionName;
	}
	if ( ! found ) {
		// create a new YZOption
		YZOption* opt = NULL;
		bool success = false;
		if ( type == yzis::TypeBool ) {
			bool d_v = YZOptionValue::booleanFromString( &success, defaultValue );
			if ( success )
				opt = new YZOptionBoolean( optionName, d_v, ctx,scope, &doNothing, QStringList() );
		} else if ( type == TypeString ) {
			QString d_v = YZOptionValue::stringFromString( &success, defaultValue );
			if ( success ) 
				opt = new YZOptionString( optionName, d_v, ctx,scope, &doNothing, QStringList(), QStringList() );
		} else if ( type == yzis::TypeInt ) {
			int d_v = YZOptionValue::integerFromString( &success, defaultValue );
			if ( success )
				opt = new YZOptionInteger( optionName, d_v, ctx, scope, &doNothing, QStringList() );
		} else if ( type == TypeList ) {
			QStringList d_v = YZOptionValue::listFromString( &success, defaultValue );
			if ( success )
				opt = new YZOptionList( optionName, d_v, ctx,scope, &doNothing, QStringList(), QStringList() );
		} else if ( type == TypeMap ) {
			MapOption d_v = YZOptionValue::mapFromString( &success, defaultValue );
			if ( success )
				opt = new YZOptionMap( optionName, d_v, ctx,scope, &doNothing, QStringList(), d_v.keys(), QStringList() );
		} else if ( type == TypeColor ) {
			YZColor d_v = YZOptionValue::colorFromString( &success, defaultValue );
			if ( success )
				opt = new YZOptionColor( optionName, d_v, ctx,scope, &doNothing, QStringList() );
		}
		if ( opt ) {
			options.append( opt );
			YZOptionValue* ov = new YZOptionValue( *opt->defaultValue() );
			success = opt->setValue( value, ov );
			if ( ! success ) { // bad value, we cannot add that new option. Delete the ov and the option itself.
				delete ov;
				options.removeLast();
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
			newoptions[key] = it.value();
			toDrop << it.key();
		}
	}

	//drop old records
	for ( QStringList::Iterator it2 = toDrop.begin(); it2 != toDrop.end(); ++it2 ) {
		//don't delete the pointers, it is still used by the new option ;)
		mOptions.remove(*it2);
	}

	//add new mOptions into the QMap now
	it = newoptions.begin(), end = newoptions.end();
	for ( ; it != end; ++it ) {
		mOptions[it.key()] = it.value();
	}
}

