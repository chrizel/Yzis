/* This file is part of the Yzis libraries
 *  Copyright (C) 2003, 2004 Mickael Marchand <marchand@kde.org>
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

#ifndef YZ_MOTIONPOOL
#define YZ_MOTIONPOOL

#include <qmap.h>
#include <qregexp.h>
#include "cursor.h"
#include "view.h"

class YZBuffer;
class YZCursor;

enum type_t {
	REGEXP, //apply a regexp to make calculations (rex)
	RELATIVE //change cursor position with relative coordinates (x,y)
};

struct motion_t {
	QString rex; //the regexp to execute
	enum type_t type; //type of motion
	int x; // relative x movement
	int y; // relative y movement
	bool backward; //we need to calculate backwards (reverse)
	bool after; //return position before the motion match, otherwise return the position at the end of the match. This is of course useless for RELATIVE motions

	motion_t() {
		rex=""; type=REGEXP; x=0; y=0;
		backward=false;
		after=false;
	}
	motion_t( QString _rex, enum type_t _type, int _x, int _y, bool _backward, bool _after) {
		rex=_rex; type=_type; x=_x; y=_y;
		backward=_backward;
		after=_after;
	}
};
typedef struct motion_t YZMotion;

/**
 * This is the main place for handling motion objects 
 * This is used for operators commands like "d".
 * The goal is to handle commands like : "2d3w" to delete twice 3 words 
 * Basics: 
 * We use RegExps to define the motion objects ( but this can be extended ),
 * each object has an identifier key like "w" for words, "p" for paragraphs
 * etc...
 */
class YZMotionPool {
	public:
		YZMotionPool();
		~YZMotionPool();

		/**
		 * Adds a new motion to the pool of known motions
		 * @param regexp the motion to add
		 * @param key the string/regexp which is recognized in inputsbuffer to detect a motion
		 */
		void addMotion(const YZMotion& regexp, const QString& key);

		/**
		 * Initialize the pool
		 */
		void initPool();

		/**
		 * Tries to detect a motion string in the input buffer
		 * @param inputs the string to analyze
		 * @param motion the motion found if any or NULL
		 */
		YZMotion& findMotion ( const QString& inputs, bool *ok );

		/**
		 * Calculates coordinates of the cursor after applying the given motion
		 * @param inputsMotion the motion string to apply
		 * @param view the view on which to operate
		 * @param backward true if the motion goes backward in the text
		 * @param cursor result of the calculation
		 * @return whether a match was found
		 */
		bool applyMotion( const QString& inputsMotion, YZView *view, bool *backward, YZCursor *cursor );

		/**
		 * Calculates coordinates of the cursor after applying the given regexp motion
		 * @param inputsMotion the motion string to apply
		 * @param motion the motion to apply
		 * @param view the view on which to operate
		 * @param result result of the calculation
		 * @return whether a match was found
		 */
		bool applyRegexpMotion( const QString &inputsMotion, YZMotion& motion, YZView *view, YZCursor *result );

		/**
		 * Calculates coordinates of the cursor after applying the given motion
		 * @param inputsMotion the motion string to apply
		 * @param motion the motion to apply
		 * @param view the view on which to operate
		 * @param result result of the calculation
		 * @return whether a match was found
		 */
		bool applyRelativeMotion( const QString &inputsMotion, YZMotion& motion, YZView *view, YZCursor *result );

		/**
		 * Check whether the @param inputs match a known motion
		 */
		bool isValid( const QString& inputs );

	private:
		QMap<QString,YZMotion> pool;
};

#endif
