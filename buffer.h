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

class YZBuffer {

	//required ?
	friend class YZView;
	friend class YZSession;

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
	void addChar (int x, int y, const QString& c);

	/**
	 * Replaces a character in the buffer
	 * @param x position on the line where to change the character
	 * @param y line where the character is to be changed
	 * @param c the character which replaces the current one
	 */
	void chgChar (int x, int y, const QString& c);

	/**
	 * Deletes a character in the buffer
	 * @param x position on the line where to delete the character
	 * @param y line where the character is to be deleted
	 * @param count number of characters to delete
	 */
	void delChar (int x, int y, int count = 1);

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
	const QString& fileName() {return path;}

	/**
	 * Adds a new view to the buffer
	 * @param v the view to be added
	 */
	void addView (YZView *v);

	/**
	 * The list of view for this buffer
	 * @return a QValuelist of pointers to the views
	 */
	QValueList<YZView*> views() { return view_list; }

	/**
	 * Finds a view by its UID
	 * @param uid the unique ID of the view to search for
	 * @return a pointer to the view or NULL
	 */
	YZView* findView(int uid);

	/**
	 * Get the whole text of the buffer
	 * @return a QStringList containing the texts
	 */
	const QStringList& getText() { return text; }

	/**
	 * Opens a new line after the indicated position
	 * @param col the position in line where to add a \n
	 * @param line the line preceding the new line
	 */
	void addNewLine( int col, int line );

	/**
	 * Deletes the given line
	 * @param line the line number to delete
	 */
	void deleteLine( int line );

	yz_point motionPosition( int xstart, int ystart, YZMotion regexp );

	/**
	 * Finds a line in the buffer
	 * @param line the line to search for
	 * @return a QString reference on the line or NULL
	 */
	QString	findLine(unsigned int line);

	/**
	 * Number of lines in the buffer
	 * @return the number of lines
	 */
	unsigned int getLines( void ) { return text.count(); }

	/**
	 * Changes the filename
	 * @param _path the new filename ( and path )
	 */
	void setPath( const QString& _path ) { path = _path; }

	/**
	 * Unique ID of the buffer
	 */
	int myId;

protected:
	QString path;
	QValueList<YZView*> view_list;

	void	updateAllViews();

	/* readonly?, change, load, save, isclean?, ... */
	/* locking stuff will be here, too */
	QStringList text;
	YZSession *session;
	//counters of buffers
	static int buffer_ids;
};

#endif /*  YZ_BUFFER_H */

