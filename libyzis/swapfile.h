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

#ifndef YZ_SWAPFILE
#define YZ_SWAPFILE

#include <qstringlist.h>
#include "buffer.h"

class YZBuffer;

/**
 * Creates a swapfile on a buffer
 */
class YZSwapFile {
	public:
		/**
		 * Default constructor
		 */
		YZSwapFile(YZBuffer *b);

		/**
		 * Add an inputs event to history
		 */
		void addToSwap( int inputs, int modifiers );

		/**
		 * Clear the history
		 */
		void reset() { mHistory.clear(); }

		/**
		 * Writes the swap to the file
		 */
		void flush();

		/**
		 * Changes the swap filename
		 */
		void setFileName( const QString& filename );

		/**
		 * Deletes the swapfile
		 */
		void unlink();

	private:
		struct sE {
			int inputs;
			int modifiers;
		} sE;
		typedef struct sE swapEntry;

		QValueList<swapEntry> mHistory;
		YZBuffer *mParent;
		QString mFilename;
};

#endif
