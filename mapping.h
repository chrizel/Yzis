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

#ifndef YZ_MAPPING_H
#define YZ_MAPPING_H

/**
 * $Id$
 */

#include <qglobal.h>
#if QT_VERSION < 0x040000
#include <qstring.h>
#include <qmap.h>
#else
#include <QMap>
#include <QString>
#endif

/**
 * Handles key mappings
 */
class YZMapping {
	public:
		virtual ~YZMapping();

		static YZMapping *self();

		void addNormalMapping( const QString& key, const QString& map ) {
			registerModifier(key);
			mNormalMappings[key] = map;
		}

		void deleteNormalMapping( const QString& key ) {
			unregisterModifier(key);
			mNormalMappings.remove(key);	
		}

		void addVisualMapping( const QString& key, const QString& map ) {
			registerModifier(key);
			mVisualMappings[key] = map;
		}

		void deleteVisualMapping( const QString& key ) {
			unregisterModifier(key);
			mVisualMappings.remove(key);	
		}

		void addInsertMapping( const QString& key, const QString& map ) {
			registerModifier(key);
			mInsertMappings[key] = map;
		}

		void deleteInsertMapping( const QString& key ) {
			unregisterModifier(key);
			mInsertMappings.remove(key);	
		}

		void addCmdLineMapping( const QString& key, const QString& map ) {
			registerModifier(key);
			mCmdLineMappings[key] = map;
		}

		void deleteCmdLineMapping( const QString& key ) {
			unregisterModifier(key);
			mCmdLineMappings.remove(key);	
		}

		void addPendingOpMapping( const QString& key, const QString& map ) {
			registerModifier(key);
			mPendingOpMappings[key] = map;
		}

		void deletePendingOpMapping( const QString& key ) {
			unregisterModifier(key);
			mPendingOpMappings.remove(key);	
		}
	
		void addGlobalMapping( const QString& key, const QString& map ) {
			registerModifier(key);
			mGlobalMappings[key] = map;
		}

		void deleteGlobalMapping( const QString& key ) {
			unregisterModifier(key);
			mGlobalMappings.remove(key);	
		}
		bool applyMappings( QString& text, int modes, bool *mapped );
		bool applyNormalMappings( QString& text );
		bool applyVisualMappings( QString& text );
		bool applyCmdLineMappings( QString& text );
		bool applyInsertMappings( QString& text );
		bool applyPendingOpMappings( QString& text );
		bool applyGlobalMappings( QString& text );
		void registerModifier(const QString& map);
		void unregisterModifier(const QString& map);

	protected:
		YZMapping();
		static YZMapping *me;
		
	private:
		QMap<QString,QString> mNormalMappings;
		QMap<QString,QString> mVisualMappings;
		QMap<QString,QString> mInsertMappings;
		QMap<QString,QString> mCmdLineMappings;
		QMap<QString,QString> mPendingOpMappings;
		QMap<QString,QString> mGlobalMappings;

};

#endif
