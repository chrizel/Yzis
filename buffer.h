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

	friend class YZView;
	friend class YZSession;

public:
	/** opens a buffer using the given file */
	YZBuffer(YZSession *sess,const QString& _path=QString::null);
	~YZBuffer();

	void addChar (int x, int y, const QString& c);
	void chgChar (int x, int y, const QString& c);
	void delChar (int x, int y, int count = 1);

	void load(void);
	void save(void);

	void addLine(const QString &l);

	const QString& fileName() {return path;}

	void addView (YZView *v);

	QValueList<YZView*> views() { return view_list; }

	YZView* findView(int uid);

	//const ?
	const QStringList& getText() { return text; }

	void addNewLine( int col, int line );

	void deleteLine( int line );

	yz_point motionPosition( int xstart, int ystart, YZMotion regexp );

	QString	findLine(unsigned int line);

	unsigned int getLines( void ) { return text.count(); }

	int myId;

protected:
	QString path;
	//QPtrList<YZView> view_list;
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

