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

#ifndef YZ_OPTION
#define YZ_OPTION

#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qcolor.h>
#include <qregexp.h>
#include <limits.h> // for INT_MAX and INT_MIN
#include "yzis.h"

/** The abstract base class of the classes representing options of a specific value type.
 * Provides the interface to retrieve basic information about the option: name,
 * group, context, default value, value type and if a value is valid.
 */
class YZOption {
protected:
	QString mName;
	context mContext;
	QString mDefault;
	QString mDescription;
public:
	YZOption(const QString &name, context cxt, const QString &def, const QString &desc=QString::null) {
		mName=name; mContext=cxt; mDefault=def; mDescription=desc;
	}
	virtual ~YZOption() {}
	const QString &getName() const { return mName; }
	context getContext() const { return mContext; }
	const QString &getStringDefault() const { return mDefault; }
	const QString &getDescription() const { return mDescription; }
	virtual value_t getValueType() const = 0;
	virtual bool isValid(const QString &value) const = 0;
};

/** Handles options whose value is an integer. You can specify a minimum and
 * a maximum value.
 */
class YZIntOption : public YZOption {
protected:
	int mMin, mMax;
public:
	YZIntOption(const QString &name, context cxt, int def, int min=INT_MIN, int max=INT_MAX);
	YZIntOption(const QString &name, context cxt, const QString &desc, int def, int min=INT_MIN, int max=INT_MAX);
	
	int getMin() const { return mMin; }
	int getMax() const { return mMax; }
	int getDefault() const { return mDefault.toInt(); }
	bool isValid(const QString &value) const ;
	value_t getValueType() const { return int_t; }
private:
	void init(int def, int min, int max) {
		mDefault=QString::number(def); mMin=min; mMax=max;
	}
};

/** Handles options whose value is a string. You can specify a regular expression
 * (regexp) which matches valid values.
 */
class YZStringOption : public YZOption {
protected:
	QRegExp mRegExp;
public:
	YZStringOption(const QString &name, context cxt, const QString &def, const QRegExp &regExp=QRegExp(".*"));
	YZStringOption(const QString &name, context cxt, const QString &desc, const QString &def, const QRegExp &regExp=QRegExp(".*"));
	
	const QString &getDefault() const { return getStringDefault(); }
	bool isValid(const QString &value) const;
	value_t getValueType() const { return string_t; }
private:
	void init(const QString &def, const QRegExp &regExp) {
		mDefault=def; mRegExp=regExp;
	}
};

/** Handles options which can be enabled or disabled, thus, which take a boolean
 * value.
 */
class YZBoolOption : public YZOption {
public:
	YZBoolOption(const QString &name, context cxt, bool def);
	YZBoolOption(const QString &name, context cxt, const QString &desc, bool def);
	
	bool getDefault() const { return mDefault == "yes" || mDefault == "on" || mDefault == "true"; }
	bool isValid(const QString &value) const;
	value_t getValueType() const { return bool_t; }
private:
	void init(bool def) {
		mDefault=def ? "yes" : "no";
	}
};

/** Handles options whose value is a color.
 */
class YZColorOption : public YZOption {
public:
	YZColorOption(const QString &name, context cxt, const QColor &def);
	YZColorOption(const QString &name, context cxt, const QString &desc, const QColor &def);
	
	QColor getDefault() const { return QColor(mDefault); }
	bool isValid(const QString &name) const;
	value_t getValueType() const { return color_t; }
private:
	void init(const QColor &def) {
		mDefault=def.name();
	}
};


#endif
