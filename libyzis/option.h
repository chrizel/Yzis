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

#ifndef YZ_OPTION
#define YZ_OPTION

/* System */
#include <limits.h> // for INT_MAX and INT_MIN

/* Qt */
#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>

/* yzis */
#include "color.h"
#include "yzis.h"

/** @file option.h
  * Some documentation
  */


/** Shortcut for a dictionary of string keys and values */
typedef QMap<QString, QString> MapOption;

/** Actions that can be done on an option */
enum OptAction {
    OptInvalid,     //!< no action set
    OptSet,         //!< simply set the option
    OptReset,       //!< reset the option to its default value
    OptAppend,      //!< append to the existing value
    OptPrepend,     //!< prepend to the existing value
    OptSubtract,    //!< substract (remove) from the existing value
};

class YOption;
class YView;
class YBuffer;

/** Option value holds the content of an option.
  *
  * Options can be:
  * - boolean
  * - string
  * - integer
  * - list of string
  * - dictionary of string keys and values (map)
  * - color
  *
  * The YOptionValue class contains member to convert the different
  * option values to and from string.
  *
  * YOptionValue is linked to a YOption that contains other option
  * characteristics, like name, default value and more.
  */
class YOptionValue
{
public :

    /** Construct an option value linked to a YOption */
    YOptionValue( YOption* o );

    /** Copy constructor */
    YOptionValue( const YOptionValue& ov );

    /** Destructor */
    virtual ~YOptionValue();

    /** Assign option value from a boolean \p value */
    void setBoolean( bool value );

    /** Assign option value from a string \p value */
    void setString( const QString& value );

    /** Assign option value from an int \p value */
    void setInteger( int value );

    /** Assign option value from a list of string \p value */
    void setList( const QStringList& value );

    /** Assign option value from a dictionary \p value */
    void setMap( const MapOption& value );

    /** Assign option value from a color \p value */
    void setColor( const YColor& value );

    bool boolean() const;           //!< boolean value of the option
    const QString& string() const;  //!< string value of the option
    int integer() const;            //!< integer value of the option
    const QStringList& list() const; //!< list value of the option
    const MapOption& map() const;   //!< dictionary value of the option
    const YColor& color() const;   //!< color value of the option

    //! The YOption from which I'm the value
    YOption* parent() const;

    yzis::OptType type() const; //!< type of the option

    /** Convert the option value to a string */
    QString toString() const;

    /** Convert a string into a boolen value.
      * Supported names are "true", "false", "on", "off", "yes", "no" */
    static bool booleanFromString( bool* success, const QString& value );
    /** Convert a string into a string.
      * Does nothing but is here for consistency */
    static QString stringFromString( bool* success, const QString& value );
    /** Convert a string into an integer.
      * Note that decimals are not supported */
    static int integerFromString( bool* success, const QString& value );
    /** Convert a string containing items separated by a comma into a list
      * of strings */
    static QStringList listFromString( bool* success, const QString& value );
    /** Convert a dictionary containing items separated by a comma and
      * colon into a dictionary of strings */
    static MapOption mapFromString( bool* success, const QString& value );
    /** Convert a string into a color.
      * See YColor() for the ways of expressing a color with a string */
    static YColor colorFromString( bool* success, const QString& value );

    /** Convert boolean into string.
      * \return true or false */
    static QString booleanToString( bool value );
    /** Convert a string into a string.
      * Does actually nothing, is here only for consistency.
      * \return true or false */
    static QString stringToString( const QString& value );
    /** Convert an integer into a string.
      * See <a href="http://doc.trolltech.com/qt4/QString.html">QString
      * documentation</a> for the supported parameters. */
    static QString integerToString( int value );
    /** Convert a list of strings into a string.
      * Separate each entry by a comma (",") */
    static QString listToString( const QStringList& value );
    /** Convert a dictionary of strings into a string.
      * Separate each key-value pair by a comma (",") and each key
      * from its value by a colon (":") */
    static QString mapToString( const MapOption& value );
    /** Convert a color into a string.
      * See YColor() for the ways of expressing a color with a string */
    static QString colorToString( const YColor& value );

private :
    YOption* m_parent; //!< YOption() which stores this YOptionValue() */
    bool v_bool;        //!< boolean value when the option is of type TypeBool
    QString v_str;      //!< string value when the option is of type TypeString
    int v_int;          //!< int value when the option is of type TypeInt
    QStringList v_list; //!< list of string value when the option is of type TypeList
    MapOption v_map;    //!< map option value when the option is of type TypeMap
    YColor v_color;    //!< color value when the option is of type TypeColor
    yzis::OptType m_type;   //!< type of the option
};

/** A function that does something to a view or a buffer.
  * Used by YOption to apply an option action to an attached view or buffer */
typedef void (*ApplyOptionMethod) ( YBuffer* b, YView* v );

/** Class holding a full option
  *
  * An option is composed of:
  * - a context (yzis::OptContext)
  * - a scope
  * - an method to apply different actions
  * - a list of possible aliases
  * - a value
  */
class YOption
{
public :

    /** Constructor
      *
      * \param name name of the option
      * \param ctx  context (session, buffer or view)
      * \param scope scope of the option (default, local, global)
      * \param m a method to apply different actions on an option
      * \param aliases a list of aliases
      */
    YOption( const QString& name, yzis::OptContext ctx, yzis::OptScope scope, ApplyOptionMethod m, const QStringList& aliases );

    /** Destructor */
    virtual ~YOption();

    /** name of the option */
    const QString& name() const;

    /** Context of the option: buffer, view or session */
    yzis::OptContext context() const;

    /** Scope of the option: global, local or default */
    yzis::OptScope scope() const;

    /** Default value of the option */
    YOptionValue* defaultValue();

    /**
     * extract the option value from entry, if the value is correct, 
     * fill YOptionValue value with it and returns true, else returns false;
     */
    virtual bool setValue( const QString& entry, YOptionValue* value ) = 0;

    /**
     * returns true if the entry match with our option
     */
    virtual bool match( const QString& entry );

    /** Apply the option.
      *
      * This calls the method that was defined in the constructor */
    void apply( YBuffer* b = NULL, YView* v = NULL );

private :
    QString m_name; //!< name of the option
    yzis::OptContext m_ctx; //!< context of the option
    yzis::OptScope m_scope; //!< scope of the option
    ApplyOptionMethod m_apply; //!< method of the option

protected :

    /**
           * Parse the text provided in entry and extract the action that is
           * contained in the option name and the value.
           * 
           * Possible actions depending on entry are:
           * "myoption&"      --> reset the option myoption
           * "myoption+=data --> append data to myoption
           * "myoption-=data --> subtract data to myoption
           * "myoption^=data --> prepend data to myoption
           * "myoption=data --> set myoption to data
           * "myoption:data --> set myoption to data
           *
           * The action is then interpreted differently depending on the actualy
           * type of the value of the option.
           *
           * \return the value of the option (content of the option)
     */
    QString readValue( const QString& entry, OptAction* action );

    YOptionValue* v_default; //!< default value of the option
    QStringList m_allValues;  //!< XXX what is it for ?
    QStringList m_aliases;    //!< aliases of the option
};

/** An option containing a boolean */
class YOptionBoolean : public YOption
{
public :
    /** Constructor.
      *
      * \param name name of the option 
      * \param v default value
      * \param ctx  context (session, buffer or view)
      * \param scope scope of the option (default, local, global)
      * \param m a method to apply different actions on an option
      * \param aliases a list of aliases
      */
    YOptionBoolean( const QString& name, bool v, yzis::OptContext ctx, yzis::OptScope scope, ApplyOptionMethod m, const QStringList& aliases );

    /** Destructor */
    virtual ~YOptionBoolean();

    virtual bool match( const QString& entry );
    virtual bool setValue( const QString& entry, YOptionValue* value );
};

/** An option containing an integer */
class YOptionInteger : public YOption
{
public :
    /** Constructor.
      *
      * \param name name of the option 
      * \param v default value
      * \param ctx  context (session, buffer or view)
      * \param scope scope of the option (default, local, global)
      * \param m a method to apply different actions on an option
      * \param aliases a list of aliases
      * \param min the mininum value taken by the option
      * \param max the maximum value taken by the option
      */
    YOptionInteger( const QString& name, int v, yzis::OptContext ctx, yzis::OptScope scope, ApplyOptionMethod m, const QStringList& aliases, int min = INT_MIN, int max = INT_MAX );

    /** Destructor */
    virtual ~YOptionInteger();

    virtual bool setValue( const QString& entry, YOptionValue* value );

private :
    int v_min; //!< the minimum value taken by the option
    int v_max; //!< the maximum value taken by the option
};

/** An option containing a string */
class YOptionString : public YOption
{
public :
    /** Constructor.
      *
      * \param name name of the option 
      * \param v default value
      * \param ctx  context (session, buffer or view)
      * \param scope scope of the option (default, local, global)
      * \param m a method to apply different actions on an option
      * \param aliases a list of aliases
      * \param values XXX [not clear what it is used for]
      */
    YOptionString( const QString& name, const QString& v, yzis::OptContext ctx, yzis::OptScope scope, ApplyOptionMethod m, const QStringList& aliases, const QStringList& values );
    /** Destructor */
    virtual ~YOptionString();

    virtual bool setValue( const QString& entry, YOptionValue* value );
};

/** An option containing a list of strings */
class YOptionList : public YOption
{
public :
    /** Constructor.
      *
      * \param name name of the option 
      * \param v default value
      * \param ctx  context (session, buffer or view)
      * \param scope scope of the option (default, local, global)
      * \param m a method to apply different actions on an option
      * \param aliases a list of aliases
      * \param values XXX [not clear what it is for]
      */
    YOptionList( const QString& name, const QStringList& v, yzis::OptContext ctx, yzis::OptScope scope, ApplyOptionMethod m, const QStringList& aliases, const QStringList& values );
    /** Destructor */
    virtual ~YOptionList();

    virtual bool setValue( const QString& entry, YOptionValue* value );
};

/** An option containing a dictionary with key and values being strings */
class YOptionMap : public YOption
{
public :
    /** Constructor.
      *
      * \param name name of the option 
      * \param v default value
      * \param ctx  context (session, buffer or view)
      * \param scope scope of the option (default, local, global)
      * \param m a method to apply different actions on an option
      * \param aliases a list of aliases
      * \param keys [not clear what it is used for]
      * \param values [not clear what it is used for]
      */
    YOptionMap( const QString& name, const MapOption& v, yzis::OptContext ctx, yzis::OptScope scope, ApplyOptionMethod m, const QStringList& aliases, QStringList keys, QStringList values );
    /** Destructor */
    virtual ~YOptionMap();

    virtual bool setValue( const QString& entry, YOptionValue* value );
private :
    QStringList m_allKeys; //!< XXX [not clear what it is used for]
};

/** An option containing a color */
class YOptionColor : public YOption
{
public :
    /** Constructor.
      *
      * \param name name of the option 
      * \param v default value
      * \param ctx  context (session, buffer or view)
      * \param scope scope of the option (default, local, global)
      * \param m a method to apply different actions on an option
      * \param aliases a list of aliases
      */
    YOptionColor( const QString& name, const YColor& v, yzis::OptContext ctx, yzis::OptScope scope, ApplyOptionMethod m, const QStringList& aliases );
    /** Destructor */
    virtual ~YOptionColor();

    virtual bool setValue( const QString& entry, YOptionValue* value );
};

#endif
