#ifndef YZ_BUFFER_H
#define YZ_BUFFER_H
/**
 * yz_interface.h
 *
 * YZBuffer - abstractino for buffer/file
 */

#include "yz_events.h"
#include <qstringlist.h>

#define	YZ_MAX_VIEW 50

class YZView;

class YZBuffer {

	friend class YZView;

public:
	/** opens a buffer using the given file */
	YZBuffer(QString _path=QString::null);

	void addChar (int x, int y, QChar c);
	void chgChar (int x, int y, QChar c);

	void load(void);
	void save(void);

	/* linked list handling */
	void addLine(QString &l);

protected:
	void addView (YZView *v);

	QString path;
	YZView	*view_list[YZ_MAX_VIEW];	// should be growable 
	int	view_nb;

private:
	void	postEvent(yz_event e);
	void	updateView(int view);
	void	updateAllViews();
	QString	findLine(int line);

	/* readonly?, change, load, save, isclean?, ... */
	/* locking stuff will be here, too */
	QStringList text;
};

#endif /*  YZ_BUFFER_H */

