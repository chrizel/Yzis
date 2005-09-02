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

#ifndef YZ_OPTION
#define YZ_OPTION

#include "yzis.h"

#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qcolor.h>
#include <limits.h> // for INT_MAX and INT_MIN

typedef QMap<QString,QString> MapOption;

#include "buffer.h"
#include "view.h"

/**
 * Options Handling in libyzis
 * 	YEP003 - http://www.yzis.org/devel/YEP/YEP003
 */

enum opt_action {
	opt_invalid,
	opt_set,
	opt_reset,
	opt_append,
	opt_prepend,
	opt_subtract,
};

class YZOption;

class YZOptionValue {
	public :
		YZOptionValue( YZOption* o );
		YZOptionValue( const YZOptionValue& ov );
		virtual ~YZOptionValue();

		void setBoolean( bool value );
		void setString( const QString& value );
		void setInteger( int value );
		void setList( const QStringList& value );
		void setMap( const MapOption& value );
		void setColor( const QColor& value );

		bool boolean() const;
		const QString& string() const;
		int integer() const;
		const QStringList& list() const;
		const MapOption& map() const;
		const QColor& color() const;
		
		// the YZOption from which I'm the value
		YZOption* parent() const;
	
		value_t type() const;
		QString toString();

		static bool booleanFromString( bool* success, const QString& value );
		static QString stringFromString( bool* success, const QString& value );
		static int integerFromString( bool* success, const QString& value );
		static QStringList listFromString( bool* success, const QString& value );
		static MapOption mapFromString( bool* success, const QString& value );
		static QColor colorFromString( bool* success, const QString& value );

		static QString booleanToString( bool value );
		static QString stringToString( const QString& value );
		static QString integerToString( int value );
		static QString listToString( const QStringList& value );
		static QString mapToString( const MapOption& value );
		static QString colorToString( const QColor& value );

	private :
		YZOption* m_parent;
		bool v_bool;
		QString v_str;
		int v_int;
		QStringList v_list;
		MapOption v_map;
		QColor v_color;
		value_t m_type;
};

typedef void (*ApplyOptionMethod) ( YZBuffer* b, YZView* v );

class YZOption {
	public :
		YZOption( const QString& name, context_t ctx, scope_t scope, ApplyOptionMethod m, QStringList aliases );
		virtual ~YZOption();

		const QString& name() const;
		context_t context() const;
		scope_t scope() const;

		YZOptionValue* defaultValue();

		/**
		 * extract the option value from entry, if the value is correct, 
		 * fill YZOptionValue value with it and returns true, else returns false;
		 */
		virtual bool setValue( const QString& entry, YZOptionValue* value ) = 0;

		/**
		 * returns true if the entry match with our option
		 */
		virtual bool match( const QString& entry );
		
		void apply( YZBuffer* b = NULL, YZView* v = NULL );

	private :
		QString m_name;
		context_t m_ctx;
		scope_t m_scope;
		ApplyOptionMethod m_apply;

	protected :

		/**
		 * Read the entry and extract the action and the value.
		 */
		QString readValue( const QString& entry, opt_action* action );
		
		YZOptionValue* v_default;
		QStringList m_allValues;
		QStringList m_aliases;
};

class YZOptionBoolean : public YZOption {
	public :
		YZOptionBoolean( const QString& name, bool v, context_t ctx, scope_t scope, ApplyOptionMethod m, QStringList aliases );
		virtual ~YZOptionBoolean();

		virtual bool match( const QString& entry );
		virtual bool setValue( const QString& entry, YZOptionValue* value );
};

class YZOptionInteger : public YZOption {
	public :
		YZOptionInteger( const QString& name, int v, context_t ctx, scope_t scope, ApplyOptionMethod m, QStringList aliases, int min = INT_MIN, int max = INT_MAX );
		virtual ~YZOptionInteger();

		virtual bool setValue( const QString& entry, YZOptionValue* value );
	
	private :
		int v_min;
		int v_max;
};

class YZOptionString : public YZOption {
	public :
		YZOptionString( const QString& name, const QString& v, context_t ctx, scope_t scope, ApplyOptionMethod m, QStringList aliases, QStringList values );
		virtual ~YZOptionString();

		virtual bool setValue( const QString& entry, YZOptionValue* value );
};

class YZOptionList : public YZOption {
	public :
		YZOptionList( const QString& name, const QStringList& v, context_t ctx, scope_t scope, ApplyOptionMethod m, QStringList aliases, QStringList values );
		virtual ~YZOptionList();

		virtual bool setValue( const QString& entry, YZOptionValue* value );
};

class YZOptionMap : public YZOption {
	public :
		YZOptionMap( const QString& name, const MapOption& v, context_t ctx, scope_t scope, ApplyOptionMethod m, QStringList aliases, QStringList keys, QStringList values );
		virtual ~YZOptionMap();

		virtual bool setValue( const QString& entry, YZOptionValue* value );
	private :
		QStringList m_allKeys;
};

class YZOptionColor : public YZOption {
	public :
		YZOptionColor( const QString& name, const QColor& v, context_t ctx, scope_t scope, ApplyOptionMethod m, QStringList aliases );
		virtual ~YZOptionColor();

		virtual bool setValue( const QString& entry, YZOptionValue* value );
};

#endif
