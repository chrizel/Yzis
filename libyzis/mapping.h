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

#ifndef YZ_MAPPING_H
#define YZ_MAPPING_H

/**
 * $Id$
 */

#include <qstring.h>
#include <qmap.h>

/**
 * Handles key mappings
 */
class YZMapping {
	public:
		virtual ~YZMapping();

		static YZMapping *self();

		void addNormalMapping( const QString& key, const QString& map ) {
			mNormalMappings[key] = map;
		}

		void deleteNormalMapping( const QString& key ) {
			mNormalMappings.erase(key);	
		}

		void addVisualMapping( const QString& key, const QString& map ) {
			mVisualMappings[key] = map;
		}

		void deleteVisualMapping( const QString& key ) {
			mVisualMappings.erase(key);	
		}

		void addInsertMapping( const QString& key, const QString& map ) {
			mInsertMappings[key] = map;
		}

		void deleteInsertMapping( const QString& key ) {
			mInsertMappings.erase(key);	
		}

		void addCmdLineMapping( const QString& key, const QString& map ) {
			mCmdLineMappings[key] = map;
		}

		void deleteCmdLineMapping( const QString& key ) {
			mCmdLineMappings.erase(key);	
		}

		void addPendingOpMapping( const QString& key, const QString& map ) {
			mPendingOpMappings[key] = map;
		}

		void deletePendingOpMapping( const QString& key ) {
			mPendingOpMappings.erase(key);	
		}
	
		void addGlobalMapping( const QString& key, const QString& map ) {
			mGlobalMappings[key] = map;
		}

		void deleteGlobalMapping( const QString& key ) {
			mGlobalMappings.erase(key);	
		}
		bool applyMappings( QString& text, int modes, bool *mapped );
		bool applyNormalMappings( QString& text );
		bool applyVisualMappings( QString& text );
		bool applyCmdLineMappings( QString& text );
		bool applyInsertMappings( QString& text );
		bool applyPendingOpMappings( QString& text );
		bool applyGlobalMappings( QString& text );

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
