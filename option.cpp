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

#include "option.h"

bool YZIntOption::isValid(const QString &value) const {
	bool ok;
	int i=value.toInt(&ok);
	return ok ? i>=mMin && i<=mMax : false;
}

bool YZStringOption::isValid(const QString &value) const {
	return mRegExp.exactMatch(value);
}

bool YZBoolOption::isValid(const QString &value) const {
	return value=="yes" || value=="on" || value=="true" ||
	       value=="no" || value=="off" || value=="false";
}

bool YZColorOption::isValid(const QString &name) const {
	return QColor(name).isValid();
}

YZIntOption::YZIntOption( const QString & name, context cxt, int def, int min, int max )
		:YZOption(name, cxt, QString::null) {
	init(def, min, max);
}

YZIntOption::YZIntOption( const QString & name, context cxt, const QString & desc, int def, int min, int max )
		:YZOption(name, cxt, desc) {
	init(def, min, max);
}

YZStringOption::YZStringOption( const QString & name, context cxt, const QString & def, const QRegExp & regExp )
		:YZOption(name, cxt, QString::null) {
	init(def, regExp);
}

YZStringOption::YZStringOption( const QString & name, context cxt, const QString & desc, const QString & def, const QRegExp & regExp )
		:YZOption(name, cxt, desc) {
	init(def, regExp);
}

YZBoolOption::YZBoolOption( const QString & name, context cxt, bool def )
		:YZOption(name, cxt, QString::null) {
	init(def);
}

YZBoolOption::YZBoolOption( const QString & name, context cxt, const QString & desc, bool def )
		:YZOption(name, cxt, desc) {
	init(def);
}

YZColorOption::YZColorOption( const QString & name, context cxt, const QColor & def )
		:YZOption(name, cxt, QString::null) {
	init(def);
}

YZColorOption::YZColorOption( const QString & name, context cxt, const QString & desc, const QColor & def )
		:YZOption(name, cxt, desc) {
	init(def);
}

