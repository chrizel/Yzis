#ifndef YZ_BUFFER_H
#define YZ_BUFFER_H
/**
 * $Id$
 */

#include <qstringlist.h>
#include <qptrlist.h>
#include "yz_events.h"
#include "yzis.h"
#include "yz_motion.h"

class YZView;

class YZBuffer {

	friend class YZView;
	friend class YZSession;

public:
	/** opens a buffer using the given file */
	YZBuffer(const QString& _path=QString::null);

	void addChar (int x, int y, QChar c);
	void chgChar (int x, int y, QChar c);
	void delChar (int x, int y, int count = 1);

	void load(void);
	void save(void);

	void addLine(QString &l);

	QString fileName() {return path;}

	void addView (YZView *v);

	QPtrList<YZView> views() { return view_list; }

	const QStringList& getText() { return text; }

	void addNewLine( int col, int line );

	void deleteLine( int line );

	yz_point motionPosition( int xstart, int ystart, YZMotion regexp );

	QString	findLine(unsigned int line);

	unsigned int getLines( void ) { return text.count(); }

protected:
	QString path;
	QPtrList<YZView> view_list;
	int	view_nb;

	void	postEvent(yz_event e);
	void	updateAllViews();

	/* readonly?, change, load, save, isclean?, ... */
	/* locking stuff will be here, too */
	QStringList text;
};

#endif /*  YZ_BUFFER_H */

