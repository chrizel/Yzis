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

/**
 * $Id$
 */
#include "mapping.h"
#include "yzis.h"
#include "debug.h"

YZMapping *YZMapping::me = 0L;

YZMapping *YZMapping::self() {
	if (YZMapping::me == 0L) YZMapping::me = new YZMapping();
	return YZMapping::me;
}

YZMapping::YZMapping() {
}

YZMapping::~YZMapping() {
}

void YZMapping::applyNormalMappings( QString& text ) {
	yzDebug() << "Normal" << endl;
	QMap<QString,QString>::Iterator it;
	for (it = mNormalMappings.begin(); it != mNormalMappings.end(); ++it)
		text.replace(it.key(), it.data());
}

void YZMapping::applyVisualMappings( QString& text ) {
	QMap<QString,QString>::Iterator it;
	for (it = mVisualMappings.begin(); it != mVisualMappings.end(); ++it)
		text.replace(it.key(), it.data());
}

void YZMapping::applyCmdLineMappings( QString& text ) {
	QMap<QString,QString>::Iterator it;
	for (it = mCmdLineMappings.begin(); it != mCmdLineMappings.end(); ++it)
		text.replace(it.key(), it.data());
}

void YZMapping::applyPendingOpMappings( QString& text ) {
	QMap<QString,QString>::Iterator it;
	for (it = mPendingOpMappings.begin(); it != mPendingOpMappings.end(); ++it)
		text.replace(it.key(), it.data());
}

void YZMapping::applyInsertMappings( QString& text ) {
	QMap<QString,QString>::Iterator it;
	for (it = mInsertMappings.begin(); it != mInsertMappings.end(); ++it)
		text.replace(it.key(), it.data());
}

void YZMapping::applyGlobalMappings( QString& text ) {
	yzDebug() << "Global" << endl;
	QMap<QString,QString>::Iterator it;
	for (it = mGlobalMappings.begin(); it != mGlobalMappings.end(); ++it)
		text.replace(it.key(), it.data());
}

void YZMapping::applyMappings( QString& text, int modes ) {
	yzDebug() << "Text1: " << text << endl;
	if ( modes & normal || modes & visual || modes & pendingop)
		applyGlobalMappings(text);
	if ( modes & normal )
		applyNormalMappings(text);
	if ( modes & pendingop )
		applyPendingOpMappings(text);
	if ( modes & visual )
		applyVisualMappings(text);
	if ( modes & insert )
		applyInsertMappings(text);
	if ( modes & cmdline )
		applyCmdLineMappings(text);
	yzDebug() << "Text2: " << text << endl;
}

