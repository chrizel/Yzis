/* This file is part of the Yzis libraries
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
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
 
#include "option.h"

YZOptionValue::YZOptionValue( YZOption* o ) {
	m_parent = o;
	m_type = invalid_t;
}
YZOptionValue::YZOptionValue( const YZOptionValue& ov ) {
	m_parent = ov.parent();
	switch( ov.type() ) {
		case boolean_t :
			setBoolean( ov.boolean() );
			break;
		case string_t :
			setString( ov.string() );
			break;
		case integer_t :
			setInteger( ov.integer() );
			break;
		case list_t :
			setList( ov.list() );
			break;
		case map_t :
			setMap( ov.map() );
			break;
		case color_t :
			setColor( ov.color() );
			break;
		default:
			break;
	}
}
YZOptionValue::~YZOptionValue() {
}

YZOption* YZOptionValue::parent() const {
	return m_parent;
}
value_t YZOptionValue::type() const {
	return m_type;
}
QString YZOptionValue::toString() {
	QString ret = QString::null;
	switch( type() ) {
		case boolean_t :
			ret = booleanToString( v_bool );
			break;
		case string_t :
			ret = stringToString( v_str );
			break;
		case integer_t :
			ret = integerToString( v_int );
			break;
		case list_t :
			ret = listToString( v_list );
			break;
		case map_t :
			ret = mapToString( v_map );
			break;
		case color_t :
			ret = colorToString( v_color );
			break;
		default:
			break;
	}
	return ret;
}
QString YZOptionValue::booleanToString( bool value ) {
	return value ? "true" : "false";
}
QString YZOptionValue::stringToString( const QString& value ) {
	return value;
}
QString YZOptionValue::integerToString( int value ) {
	return QString::number( value );
}
QString YZOptionValue::listToString( const QStringList& value ) {
	return value.join( "," );
}
QString YZOptionValue::mapToString( const MapOption& value ) {
	QString ret = "";
	QList<QString> keys = value.keys();
	for( int i = 0; i < keys.size(); i++ ) {
		if ( i > 0 ) ret += ",";
		ret += keys[i] + ":" + value[ keys[i] ];
	}
	return ret;
}
QString YZOptionValue::colorToString( const YZColor& value ) {
	return value.name();
}

bool YZOptionValue::booleanFromString( bool* success, const QString& value ) {
	bool ret = false;
	*success = false;
	if ( value == "yes" || value == "on" || value == "true" ) {
		*success = true;
		ret = true;
	} else if ( value == "no" || value == "off" || value == "false" ) {
		*success = true;
		ret = false;
	}
	return ret;
}
QString YZOptionValue::stringFromString( bool* success, const QString& value ) {
	*success = true;
	return value;
}
int YZOptionValue::integerFromString( bool* success, const QString& value ) {
	return value.toInt( success );
}
QStringList YZOptionValue::listFromString( bool* success, const QString& value ) {
	*success = true;
	return value.split( "," );
}
MapOption YZOptionValue::mapFromString( bool* success, const QString& value ) {
	*success = true;
	MapOption ret;
	QStringList vs = value.split( ",", QString::SkipEmptyParts );
	for( int i = 0; *success && i < vs.size(); i++ ) {
		int idx_v = vs[i].indexOf(':');
		if ( idx_v < 0 ) {
			*success = false;
		} else {
			ret[ vs[i].left( idx_v ) ] = vs[i].mid( idx_v + 1 );
		}
	}
	return ret;
}
YZColor YZOptionValue::colorFromString( bool* success, const QString& value ) {
	YZColor ret( value );
	*success = ret.isValid();
	return ret;
}

void YZOptionValue::setBoolean( bool value ) {
	v_bool = value;
	m_type = boolean_t;
}
void YZOptionValue::setString( const QString& value ) {
	v_str = value;
	m_type = string_t;
}
void YZOptionValue::setInteger( int value ) {
	v_int = value;
	m_type = integer_t;
}
void YZOptionValue::setList( const QStringList& value ) {
	v_list = value;
	m_type = list_t;
}
void YZOptionValue::setMap( const MapOption& value ) {
	v_map = value;
	m_type = map_t;
}
void YZOptionValue::setColor( const YZColor& value ) {
	v_color = value;
	m_type = color_t;
}

bool YZOptionValue::boolean() const {
	return v_bool;
}
const QString& YZOptionValue::string() const {
	return v_str;
}
int YZOptionValue::integer() const {
	return v_int;
}
const QStringList& YZOptionValue::list() const {
	return v_list;
}
const MapOption& YZOptionValue::map() const {
	return v_map;
}
const YZColor& YZOptionValue::color() const {
	return v_color;
}


YZOption::YZOption( const QString& name, context_t ctx, scope_t scope, ApplyOptionMethod m, QStringList aliases ) {
	m_name = name;
	m_ctx = ctx;
	m_scope = scope;
	m_apply = m;
	v_default = new YZOptionValue( this );
	m_aliases << name;
	m_aliases += aliases;
}
YZOption::~YZOption() {
	if ( v_default )
		delete v_default;
}
const QString& YZOption::name() const {
	return m_name;
}
context_t YZOption::context() const {
	return m_ctx;
}
scope_t YZOption::scope() const {
	return m_scope;
}
YZOptionValue* YZOption::defaultValue() {
	return v_default;
}
void YZOption::apply( YZBuffer* b, YZView* v ) {
	m_apply( b, v );
}
bool YZOption::match( const QString& entry ) {
	for( int i = 0; i < m_aliases.size(); i++ ) {
		if ( entry.startsWith( m_aliases[ i ] ) && !entry.mid( m_aliases[ i ].length() )[0].isLetter() )
			return true;
	}
	return false;
}
QString YZOption::readValue( const QString& entry, opt_action* action ) {
	*action = opt_invalid;
	QString value = entry;
	for( int i = 0; *action == opt_invalid && i < m_aliases.size(); i++ ) {
		if ( entry.startsWith( m_aliases[ i ] ) && ! entry.mid( m_aliases[ i ].length() )[ 0 ].isLetter() ) {
			QString data = entry.mid( m_aliases[ i ].length() );
			unsigned int idx = 1;
			if ( data[ 0 ] == '&' ) {
				*action = opt_reset;
			} else if ( data[ 0 ] == '=' || data[ 0 ] == ':' ) {
				*action = opt_set;
			} else {
				idx = 2;
				if ( data.startsWith("+=") ) {
					*action = opt_append;
				} else if ( data.startsWith("^=") ) {
					*action = opt_prepend;
				} else if ( data.startsWith("-=") ) {
					*action = opt_subtract;
				}
			}
			if ( *action != opt_invalid )
				value = data.mid( idx );
		}
	}
	return value;
}

YZOptionBoolean::YZOptionBoolean( const QString& name, bool v, context_t ctx, scope_t scope, ApplyOptionMethod m, QStringList aliases )
	: YZOption( name, ctx, scope, m, aliases ) {
	v_default->setBoolean( v );
	m_allValues << "true" << "false" << "on" << "off" << "yes" << "no";
}
YZOptionBoolean::~YZOptionBoolean() {
}

bool YZOptionBoolean::match( const QString& entry ) {
	bool ret = YZOption::match( entry );
	if ( !ret ) {
		for( int i = 0; !ret && i < m_aliases.size(); i++ ) {
			if ( entry == m_aliases[i] || entry == "no" + m_aliases[i] \
					|| entry == m_aliases[i] + "!" || entry == "inv" + m_aliases[i] )
				ret = true;
		}
	}
	return ret;
}

bool YZOptionBoolean::setValue( const QString& entry, YZOptionValue* value ) {
	bool ret = false;
	bool v = value->boolean();
	opt_action action;

	QString v_s = readValue( entry, &action );
	if ( action == opt_invalid ) {
		for( int i = 0; !ret && i < m_aliases.size(); i++ ) {
			if ( entry == m_aliases[i] ) {
				v = true;
				ret = true;
			} else if ( entry == "no" + m_aliases[i] ) {
				v = false;
				ret = true;
			} else if ( entry == "inv" + m_aliases[i] || entry == m_aliases[i] + "!" ) {
				v = ! v;
				ret = true;
			}
		}
	} else if ( action == opt_reset ) {
		ret = true;
		v = v_default->boolean();
	} else if ( action == opt_set ) {
		v = YZOptionValue::booleanFromString( &ret, v_s );
	}
	if ( ret )
		value->setBoolean( v );
	return ret;
}

YZOptionInteger::YZOptionInteger( const QString& name, int v, context_t ctx, scope_t scope, ApplyOptionMethod m, QStringList aliases, int min, int max ) 
	: YZOption( name, ctx, scope, m, aliases ) {
	v_min = min;
	v_max = max;
	v_default->setInteger( v );
}
YZOptionInteger::~YZOptionInteger() {
}

bool YZOptionInteger::setValue( const QString& entry, YZOptionValue* value ) {
	bool ret = false;
	int v = value->integer();
	opt_action action;

	QString v_s = readValue( entry, &action );
	ret = action != opt_invalid;
	if ( action != opt_reset )
		v = YZOptionValue::integerFromString( &ret, v_s );
	if ( ret ) {
		if ( action == opt_reset ) {
			v = v_default->integer();
		} else if ( action == opt_set ) {
			// nothing
		} else if ( action == opt_append ) {
			v += value->integer();
		} else if ( action == opt_prepend ) { // multiply
			v *= value->integer();
		} else if ( action == opt_subtract ) {
			v = value->integer() - v;
		}
		ret = ret && v >= v_min && v <= v_max;
	}
	if ( ret )
		value->setInteger( v );
	return ret;
}


YZOptionString::YZOptionString( const QString& name, const QString& v, context_t ctx, scope_t scope, ApplyOptionMethod m, QStringList aliases, QStringList values ) 
	: YZOption( name, ctx, scope, m, aliases ) {
	m_allValues = values;
	v_default->setString( v );
}
YZOptionString::~YZOptionString() {
}

bool YZOptionString::setValue( const QString& entry, YZOptionValue* value ) {
	bool ret = false;
	opt_action action;

	QString v = readValue( entry, &action );
	ret = action != opt_invalid;
	if ( ret ) {
		if ( action == opt_reset ) {
			v = v_default->string();
		} else if ( action == opt_set ) {
			// nothing
		} else if ( action == opt_append ) {
			v = value->string() + v;
		} else if ( action == opt_prepend ) {
			v = v + value->string();
		} else if ( action == opt_subtract ) {
			QString mv = value->string();
			v = mv.remove( v );
		}
		if ( m_allValues.size() > 0 ) 
			ret = m_allValues.contains( v ) > 0;
	}
	if ( ret )
		value->setString( v );
	return ret;
}


YZOptionList::YZOptionList( const QString& name, const QStringList& v, context_t ctx, scope_t scope, ApplyOptionMethod m, QStringList aliases, QStringList values ) 
	: YZOption( name, ctx, scope, m, aliases ) {
	m_allValues = values;
	v_default->setList( v );
}
YZOptionList::~YZOptionList() {
}

bool YZOptionList::setValue( const QString& entry, YZOptionValue* value ) {
	bool ret = false;
	QStringList v = value->list();
	opt_action action;

	QString v_s = readValue( entry, &action );
	ret = (action != opt_invalid);
	if ( action != opt_reset )
		v = YZOptionValue::listFromString( &ret, v_s );
	if ( ret ) {
		if ( action == opt_reset ) {
			v = v_default->list();
		} else if ( action == opt_set ) {
			// nothing
		} else if ( action == opt_append ) {
			v = value->list() + v;
		} else if ( action == opt_prepend ) {
			v = v + value->list();
		} else if ( action == opt_subtract ) {
			QStringList mv = value->list();
			for( int i = 0; i < v.size(); i++ )
				mv.removeAll( v[i] );
			v = mv;
		}
		if ( ret && m_allValues.size() > 0 ) {
			for( int i = 0; ret && i < v.size(); i++ ) {
				ret = m_allValues.contains( v[i] ) > 0;
			}
		}
	}
	if ( ret )
		value->setList( v );
	return ret;
}

YZOptionMap::YZOptionMap( const QString& name, const MapOption& v, context_t ctx, scope_t scope, ApplyOptionMethod m, QStringList aliases, QStringList keys, QStringList values ) 
	: YZOption( name, ctx, scope, m, aliases ) {
	m_allKeys = keys;
	m_allValues = values;
	v_default->setMap( v );
}
YZOptionMap::~YZOptionMap() {
}

bool YZOptionMap::setValue( const QString& entry, YZOptionValue* value ) {
	bool ret = false;
	MapOption v = value->map();
	opt_action action;
	
	QString v_s = readValue( entry, &action );
	ret = (action != opt_invalid);
	if ( action != opt_reset )
		v = YZOptionValue::mapFromString( &ret, v_s );
	if ( ret ) {
		if ( action == opt_reset ) {
			v = v_default->map();
		} else if ( action == opt_set ) {
			// nothing
		} else if ( action == opt_append || action == opt_prepend ) {
			MapOption mv = value->map();
			QList<QString> keys = v.keys();
			for ( int i = 0; i < keys.size(); i++ )
				mv[ keys[i] ] = v[ keys[i] ];
			v = mv;
		} else if ( action == opt_subtract ) {
			MapOption mv = value->map();
			QList<QString> keys = v.keys();
			for ( int i = 0; i < keys.size(); i++ )
				mv.remove( keys[i] );
			v = mv;
		}
		// check keys
		if ( ret ) {
			QList<QString> keys = v.keys();
			for( int i = 0; ret && i < keys.size(); i++ ) {
				ret = m_allKeys.contains( keys[i] ) > 0;
			}
		}
		// check values
		if ( ret && m_allValues.size() > 0 ) {
			QList<QString> values = v.values();
			for( int i = 0; ret && i < values.size(); i++ ) {
				ret = m_allValues.contains( values[i] ) > 0;
			}
		}
	}
	if ( ret )
		value->setMap( v );
	return ret;
}

YZOptionColor::YZOptionColor( const QString& name, const YZColor& v, context_t ctx, scope_t scope, ApplyOptionMethod m, QStringList aliases ) 
	: YZOption( name, ctx, scope, m, aliases ) {
	v_default->setColor( v );
}
YZOptionColor::~YZOptionColor() {
}

bool YZOptionColor::setValue( const QString& entry, YZOptionValue* value ) {
	bool ret = false;
	YZColor v = value->color();
	int idx = entry.indexOf('=');
	if ( idx >= 0 ) {
		v = YZOptionValue::colorFromString( &ret, entry.mid( idx+1 ) );
	}
	if ( ret )
		value->setColor( v );
	return ret;
}


