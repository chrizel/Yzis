#ifndef YZ_BUFFER_H
#define YZ_BUFFER_H
/**
 * yz_interface.h
 *
 * YZBuffer - abstractino for buffer/file
 */

#include "yz_line.h"
#include "yz_events.h"

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
	void add_line(YZLine *l);

protected:
	void add_view (YZView *v);

	QString path;
	YZView	*view_list[YZ_MAX_VIEW];	// should be growable 
	int	view_nb;

private:
	void	post_event(yz_event e);
	void	update_view(int view);
	YZLine	*find_line(int line);

	YZLine *line_first, *line_last;
	int	lines_nb; // number of lines in this buffer

	/* readonly?, change, load, save, isclean?, ... */
	/* locking stuff will be here, too */
};

#endif /*  YZ_BUFFER_H */

