/* This file is part of the Yzis libraries
 *  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
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

#ifndef YZ_BUFFER_H
#define YZ_BUFFER_H
/**
 * $Id$
 */

#include <qstringlist.h>
#include <qptrlist.h>
#include "yzis.h"
#include "motion.h"
#include "session.h"

class YZView;
class YZLine;

typedef QPtrList<YZLine> YZBufferData;


/**
 * A buffer is the implementation of the content of a file
 * A buffer can have multiple views. Every buffer is registered in a
 * @ref YZSession
 */
class YZBuffer {
public:
	/** 
	 * Creates a new buffer
	 * @param sess the session to which the buffer belongs to
	 * @param _path the file to open in this buffer
	 * */
	YZBuffer(YZSession *sess,const QString& _path=QString::null);

	/**
	 * Default destructor
	 */
	~YZBuffer();

	/**
	 * Adds a character to the buffer
	 * @param x position on the line where to add the character
	 * @param y line where the character is to be added
	 * @param c the character to add
	 */
	void addChar (unsigned int x, unsigned int y, const QString& c);

	/**
	 * Replaces a character in the buffer
	 * @param x position on the line where to change the character
	 * @param y line where the character is to be changed
	 * @param c the character which replaces the current one
	 */
	void chgChar (unsigned int x, unsigned int y, const QString& c);

	/**
	 * Deletes a character in the buffer
	 * @param x position on the line where to delete the character
	 * @param y line where the character is to be deleted
	 * @param count number of characters to delete
	 */
	void delChar (unsigned int x, unsigned int y, unsigned int count = 1);

	/**
	 * Opens the file and fills in internals structure with its content
	 */
	void load();

	/**
	 * Dump the current internal structure to the file to be saved
	 */
	void save();

	/**
	 * Appends a new line at the end of file
	 * @param l the line of text to be appended
	 */
	void addLine(const QString &l);

	/**
	 * Get the current filename of the buffer
	 * @return the filename
	 */
	const QString& fileName() {return mPath;}

	/**
	 * Adds a new view to the buffer
	 * @param v the view to be added
	 */
	void addView (YZView *v);

	/**
	 * The list of view for this buffer
	 * @return a QValuelist of pointers to the views
	 */
	QValueList<YZView*> views() { return mViews; }

	/**
	 * Find the first view of this buffer
	 * Temporary function
	 */
	YZView* firstView();

	/**
	 * Finds a view by its UID
	 * @param uid the unique ID of the view to search for
	 * @return a pointer to the view or NULL
	 */
	YZView* findView(unsigned int uid);

#if 0
	/**
	 * Get the whole text of the buffer
	 * @return a QStringList containing the texts
	 */
	const QStringList& getText() { return mText; }
#endif

	/**
	 * Opens a new line after the indicated position
	 * @param col the position in line where to add a \n
	 * @param line the line preceding the new line
	 */
	void addNewLine( unsigned int col, unsigned int line );

	/**
	 * Deletes the given line
	 * @param line the line number to delete
	 */
	void deleteLine( unsigned int line );

	/**
	 * @internal, not implemented yet
	 */
	yz_point motionPosition( unsigned int xstart, unsigned int ystart, YZMotion regexp );

	/**
	 * Finds a line in the buffer
	 * @param line the line to search for
	 * @return a QString reference on the line or NULL
	 */
	QString	data(unsigned int line);

	/**
	 * Finds the @ref YZLine pointer for a line in the buffer
	 * @param line the line to return
	 * @return a YZLine pointer or 0 if none
	 */
	YZLine *at(unsigned int no) { return mText.at(no); }

	/**
	 * Number of lines in the buffer
	 * @return the number of lines
	 */
	unsigned int lineCount() { return mText.count(); }

	/**
	 * Changes the filename
	 * @param _path the new filename ( and path )
	 */
	void setPath( const QString& _path ) { mPath = _path; }

	/**
	 * Unique ID of the buffer
	 */
	unsigned int myId;

protected:
	void updateAllViews();

	QString mPath;
	QValueList<YZView*> mViews;

	YZBufferData mText;
	YZSession *mSession;
};

#endif /*  YZ_BUFFER_H */

