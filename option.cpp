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
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
**/

#include "option.h"

#define dbg()    yzDebug("YOptionValue")
#define err()    yzError("YOptionValue")

using namespace yzis;

YOptionValue::YOptionValue( YOption* o )
{
    m_parent = o;
    m_type = TypeInvalid;
}
YOptionValue::YOptionValue( const YOptionValue& ov )
{
    m_parent = ov.parent();
    switch ( ov.type() ) {
    case yzis::TypeBool :
        setBoolean( ov.boolean() );
        break;
    case TypeString :
        setString( ov.string() );
        break;
    case yzis::TypeInt :
        setInteger( ov.integer() );
        break;
    case TypeList :
        setList( ov.list() );
        break;
    case TypeMap :
        setMap( ov.map() );
        break;
    case TypeColor :
        setColor( ov.color() );
        break;
    default:
        break;
    }
}
YOptionValue::~YOptionValue()
{}

YOption* YOptionValue::parent() const
{
    return m_parent;
}
OptType YOptionValue::type() const
{
    return m_type;
}
QString YOptionValue::toString() const
{
    QString ret;
    switch ( type() ) {
    case yzis::TypeBool :
        ret = booleanToString( v_bool );
        break;
    case TypeString :
        ret = stringToString( v_str );
        break;
    case yzis::TypeInt :
        ret = integerToString( v_int );
        break;
    case TypeList :
        ret = listToString( v_list );
        break;
    case TypeMap :
        ret = mapToString( v_map );
        break;
    case TypeColor :
        ret = colorToString( v_color );
        break;
    default:
        break;
    }
    return ret;
}
QString YOptionValue::booleanToString( bool value )
{
    return value ? "true" : "false";
}
QString YOptionValue::stringToString( const QString& value )
{
    return value;
}
QString YOptionValue::integerToString( int value )
{
    return QString::number( value );
}
QString YOptionValue::listToString( const QStringList& value )
{
    return value.join( "," );
}
QString YOptionValue::mapToString( const MapOption& value )
{
    QString ret = "";
    QList<QString> keys = value.keys();
    for ( int i = 0; i < keys.size(); i++ ) {
        if ( i > 0 ) ret += ',';
        ret += keys[i] + ':' + value[ keys[i] ];
    }
    return ret;
}
QString YOptionValue::colorToString( const YColor& value )
{
    return value.name();
}

bool YOptionValue::booleanFromString( bool* success, const QString& value )
{
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
QString YOptionValue::stringFromString( bool* success, const QString& value )
{
    *success = true;
    return value;
}
int YOptionValue::integerFromString( bool* success, const QString& value )
{
    return value.toInt( success );
}
QStringList YOptionValue::listFromString( bool* success, const QString& value )
{
    *success = true;
    return value.split( "," );
}
MapOption YOptionValue::mapFromString( bool* success, const QString& value )
{
    *success = true;
    MapOption ret;
    QStringList vs = value.split( ",", QString::SkipEmptyParts );
    for ( int i = 0; *success && i < vs.size(); i++ ) {
        int idx_v = vs[i].indexOf(':');
        if ( idx_v < 0 ) {
            *success = false;
        } else {
            ret[ vs[i].left( idx_v ) ] = vs[i].mid( idx_v + 1 );
        }
    }
    return ret;
}
YColor YOptionValue::colorFromString( bool* success, const QString& value )
{
    YColor ret( value );
    *success = ret.isValid();
    return ret;
}

void YOptionValue::setBoolean( bool value )
{
    v_bool = value;
    m_type = yzis::TypeBool;
}
void YOptionValue::setString( const QString& value )
{
    v_str = value;
    m_type = TypeString;
}
void YOptionValue::setInteger( int value )
{
    v_int = value;
    m_type = yzis::TypeInt;
}
void YOptionValue::setList( const QStringList& value )
{
    v_list = value;
    m_type = TypeList;
}
void YOptionValue::setMap( const MapOption& value )
{
    v_map = value;
    m_type = TypeMap;
}
void YOptionValue::setColor( const YColor& value )
{
    v_color = value;
    m_type = TypeColor;
}

bool YOptionValue::boolean() const
{
    return v_bool;
}
const QString& YOptionValue::string() const
{
    return v_str;
}
int YOptionValue::integer() const
{
    return v_int;
}
const QStringList& YOptionValue::list() const
{
    return v_list;
}
const MapOption& YOptionValue::map() const
{
    return v_map;
}
const YColor& YOptionValue::color() const
{
    return v_color;
}


YOption::YOption( const QString& name, OptContext ctx, OptScope scope, ApplyOptionMethod m, const QStringList& aliases )
{
    m_name = name;
    m_ctx = ctx;
    m_scope = scope;
    m_apply = m;
    v_default = new YOptionValue( this );
    m_aliases << name;
    m_aliases += aliases;
}
YOption::~YOption()
{
    if ( v_default )
        delete v_default;
}
const QString& YOption::name() const
{
    return m_name;
}
OptContext YOption::context() const
{
    return m_ctx;
}
OptScope YOption::scope() const
{
    return m_scope;
}
YOptionValue* YOption::defaultValue()
{
    return v_default;
}
void YOption::apply( YBuffer* b, YView* v )
{
    m_apply( b, v );
}
bool YOption::match( const QString& entry )
{
    for ( int i = 0; i < m_aliases.size(); i++ ) {
        if ( entry.startsWith( m_aliases[ i ] ) && !entry.mid( m_aliases[ i ].length() )[0].isLetter() )
            return true;
    }
    return false;
}
QString YOption::readValue( const QString& entry, OptAction* action )
{
    *action = OptInvalid;
    QString value = entry;
    for ( int i = 0; *action == OptInvalid && i < m_aliases.size(); i++ ) {
        if ( entry.startsWith( m_aliases[ i ] ) && ! entry.mid( m_aliases[ i ].length() )[ 0 ].isLetter() ) {
            QString data = entry.mid( m_aliases[ i ].length() );
            unsigned int idx = 1;
            if ( data[ 0 ] == '&' ) {
                *action = OptReset;
            } else if ( data[ 0 ] == '=' || data[ 0 ] == ':' ) {
                *action = OptSet;
            } else {
                idx = 2;
                if ( data.startsWith("+=") ) {
                    *action = OptAppend;
                } else if ( data.startsWith("^=") ) {
                    *action = OptPrepend;
                } else if ( data.startsWith("-=") ) {
                    *action = OptSubtract;
                }
            }
            if ( *action != OptInvalid )
                value = data.mid( idx );
        }
    }
    return value;
}

YOptionBoolean::YOptionBoolean( const QString& name, bool v, OptContext ctx, OptScope scope, ApplyOptionMethod m, const QStringList& aliases )
        : YOption( name, ctx, scope, m, aliases )
{
    v_default->setBoolean( v );
    m_allValues << "true" << "false" << "on" << "off" << "yes" << "no";
}
YOptionBoolean::~YOptionBoolean()
{}

bool YOptionBoolean::match( const QString& entry )
{
    bool ret = YOption::match( entry );
    if ( !ret ) {
        for ( int i = 0; !ret && i < m_aliases.size(); i++ ) {
            if ( entry == m_aliases[i] || entry == "no" + m_aliases[i] \
                    || entry == m_aliases[i] + '!' || entry == "inv" + m_aliases[i] )
                ret = true;
        }
    }
    return ret;
}

bool YOptionBoolean::setValue( const QString& entry, YOptionValue* value )
{
    bool ret = false;
    bool v = value->boolean();
    OptAction action;

    QString v_s = readValue( entry, &action );
    if ( action == OptInvalid ) {
        for ( int i = 0; !ret && i < m_aliases.size(); i++ ) {
            if ( entry == m_aliases[i] ) {
                v = true;
                ret = true;
            } else if ( entry == "no" + m_aliases[i] ) {
                v = false;
                ret = true;
            } else if ( entry == "inv" + m_aliases[i] || entry == m_aliases[i] + '!' ) {
                v = ! v;
                ret = true;
            }
        }
    } else if ( action == OptReset ) {
        ret = true;
        v = v_default->boolean();
    } else if ( action == OptSet ) {
        v = YOptionValue::booleanFromString( &ret, v_s );
    }
    if ( ret )
        value->setBoolean( v );
    return ret;
}

YOptionInteger::YOptionInteger( const QString& name, int v, OptContext ctx, OptScope scope, ApplyOptionMethod m, const QStringList& aliases, int min, int max )
        : YOption( name, ctx, scope, m, aliases )
{
    v_min = min;
    v_max = max;
    v_default->setInteger( v );
}
YOptionInteger::~YOptionInteger()
{}

bool YOptionInteger::setValue( const QString& entry, YOptionValue* value )
{
    bool ret = false;
    int v = value->integer();
    OptAction action;

    QString v_s = readValue( entry, &action );
    ret = action != OptInvalid;
    if ( action != OptReset )
        v = YOptionValue::integerFromString( &ret, v_s );
    if ( ret ) {
        if ( action == OptReset ) {
            v = v_default->integer();
        } else if ( action == OptSet ) {
            // nothing
        } else if ( action == OptAppend ) {
            v += value->integer();
        } else if ( action == OptPrepend ) { // multiply
            v *= value->integer();
        } else if ( action == OptSubtract ) {
            v = value->integer() - v;
        }
        ret = ret && v >= v_min && v <= v_max;
    }
    if ( ret )
        value->setInteger( v );
    return ret;
}


YOptionString::YOptionString( const QString& name, const QString& v, OptContext ctx, OptScope scope, ApplyOptionMethod m, const QStringList& aliases, const QStringList& values )
        : YOption( name, ctx, scope, m, aliases )
{
    m_allValues = values;
    v_default->setString( v );
}
YOptionString::~YOptionString()
{}

bool YOptionString::setValue( const QString& entry, YOptionValue* value )
{
    bool ret = false;
    OptAction action;

    QString v = readValue( entry, &action );
    ret = action != OptInvalid;
    if ( ret ) {
        if ( action == OptReset ) {
            v = v_default->string();
        } else if ( action == OptSet ) {
            // nothing
        } else if ( action == OptAppend ) {
            v = value->string() + v;
        } else if ( action == OptPrepend ) {
            v = v + value->string();
        } else if ( action == OptSubtract ) {
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


YOptionList::YOptionList( const QString& name, const QStringList& v, OptContext ctx, OptScope scope, ApplyOptionMethod m, const QStringList& aliases, const QStringList& values )
        : YOption( name, ctx, scope, m, aliases )
{
    m_allValues = values;
    v_default->setList( v );
}
YOptionList::~YOptionList()
{}

bool YOptionList::setValue( const QString& entry, YOptionValue* value )
{
    bool ret = false;
    QStringList v = value->list();
    OptAction action;

    QString v_s = readValue( entry, &action );
    ret = (action != OptInvalid);
    if ( action != OptReset )
        v = YOptionValue::listFromString( &ret, v_s );
    if ( ret ) {
        if ( action == OptReset ) {
            v = v_default->list();
        } else if ( action == OptSet ) {
            // nothing
        } else if ( action == OptAppend ) {
            v = value->list() + v;
        } else if ( action == OptPrepend ) {
            v = v + value->list();
        } else if ( action == OptSubtract ) {
            QStringList mv = value->list();
            for ( int i = 0; i < v.size(); i++ )
                mv.removeAll( v[i] );
            v = mv;
        }
        if ( ret && m_allValues.size() > 0 ) {
            for ( int i = 0; ret && i < v.size(); i++ ) {
                ret = m_allValues.contains( v[i] ) > 0;
            }
        }
    }
    if ( ret )
        value->setList( v );
    return ret;
}

YOptionMap::YOptionMap( const QString& name, const MapOption& v, OptContext ctx, OptScope scope, ApplyOptionMethod m, const QStringList& aliases, QStringList keys, QStringList values )
        : YOption( name, ctx, scope, m, aliases )
{
    m_allKeys = keys;
    m_allValues = values;
    v_default->setMap( v );
}
YOptionMap::~YOptionMap()
{}

bool YOptionMap::setValue( const QString& entry, YOptionValue* value )
{
    bool ret = false;
    MapOption v = value->map();
    OptAction action;

    QString v_s = readValue( entry, &action );
    ret = (action != OptInvalid);
    if ( action != OptReset )
        v = YOptionValue::mapFromString( &ret, v_s );
    if ( ret ) {
        if ( action == OptReset ) {
            v = v_default->map();
        } else if ( action == OptSet ) {
            // nothing
        } else if ( action == OptAppend || action == OptPrepend ) {
            MapOption mv = value->map();
            QList<QString> keys = v.keys();
            for ( int i = 0; i < keys.size(); i++ )
                mv[ keys[i] ] = v[ keys[i] ];
            v = mv;
        } else if ( action == OptSubtract ) {
            MapOption mv = value->map();
            QList<QString> keys = v.keys();
            for ( int i = 0; i < keys.size(); i++ )
                mv.remove( keys[i] );
            v = mv;
        }
        // check keys
        if ( ret ) {
            QList<QString> keys = v.keys();
            for ( int i = 0; ret && i < keys.size(); i++ ) {
                ret = m_allKeys.contains( keys[i] ) > 0;
            }
        }
        // check values
        if ( ret && m_allValues.size() > 0 ) {
            QList<QString> values = v.values();
            for ( int i = 0; ret && i < values.size(); i++ ) {
                ret = m_allValues.contains( values[i] ) > 0;
            }
        }
    }
    if ( ret )
        value->setMap( v );
    return ret;
}

YOptionColor::YOptionColor( const QString& name, const YColor& v, OptContext ctx, OptScope scope, ApplyOptionMethod m, const QStringList& aliases )
        : YOption( name, ctx, scope, m, aliases )
{
    v_default->setColor( v );
}
YOptionColor::~YOptionColor()
{}

bool YOptionColor::setValue( const QString& entry, YOptionValue* value )
{
    bool ret = false;
    YColor v = value->color();
    int idx = entry.indexOf('=');
    if ( idx >= 0 ) {
        v = YOptionValue::colorFromString( &ret, entry.mid( idx + 1 ) );
    }
    if ( ret )
        value->setColor( v );
    return ret;
}


