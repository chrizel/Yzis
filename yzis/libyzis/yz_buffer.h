#ifndef YZ_BUFFER_H
#define YZ_BUFFER_H
/**
 * yz_interface.h
 *
 * YZBuffer - abstractino for buffer/file
 */

//#include "yz_line.h"
#include "yz_events.h"
#include <qstringlist.h>

#define	YZ_MAX_VIEW 50

class YZView;

class YZBuffer {

	friend class YZView;

public:
	/** opens a buffer using the given file */
	YZBuffer(QString _path=QString::null);

	void add_char (int x, int y, QChar c);
	void chg_char (int x, int y, QChar c);

	void load(void);
	void save(void);

	/* linked list handling */
	void add_line(QString &l);

protected:
	void add_view (YZView *v);

	QString path;
	YZView	*view_list[YZ_MAX_VIEW];	// should be growable 
	int	view_nb;

private:
	void	post_event(yz_event e);
	void	update_view(int view);
	void	update_all_views();
	QString	find_line(int line);

//	QString *line_first, *line_last;
//	int	lines_nb; // number of lines in this buffer

	/* readonly?, change, load, save, isclean?, ... */
	/* locking stuff will be here, too */
	QStringList text;
};

#endif /*  YZ_BUFFER_H */

