/**
 * YZBuffer implementation
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "yz_buffer.h"
#include "yz_events.h"
#include "yz_view.h"
#include <qfile.h>
#include <qtextstream.h>

YZBuffer::YZBuffer(QString _path)
{
	path	= _path;
	view_nb	= 0;
//	lines_nb= 0;

//	line_first = line_last = NULL; // linked lines

	if (!_path.isEmpty()) load();
}

void YZBuffer::add_char (int x, int y, QChar c)
{
	/* brute force, we'll have events specific for that later on */
	QString l=find_line(y);
	if (!l) return;

	/* do the actual modification */
	l.insert(x, c);

	/* inform the views */
	post_event(mk_event_setline(y,&l));
}


void YZBuffer::chg_char (int x, int y, QChar c)
{
	/* brute force, we'll have events specific for that later on */
	QString l=find_line(y);
	if (!l) return;

	/* do the actual modification */
	l.remove(x,1);
	l.insert(x, c);

	/* inform the views */
	post_event(mk_event_setline(y,&l));
}

void YZBuffer::post_event(yz_event e)
{
	int l = e.u.setline.y;

	/* quite basic, we just check that the line is currently displayed */
	for (int i=0; i<view_nb; i++) {
		YZView *v = view_list[i];
		if (v->is_line_visible(l))
			v->post_event(e);
	}
}

void YZBuffer::add_view (YZView *v)
{
//FIXME	if (view_nb>YZ_MAX_VIEW)
//		panic("no more rom for new view in YZBuffer");

	//debug("adding view %p, number is %d", v, view_nb);
	view_list[view_nb] = v;
	update_view(view_nb++);
}

void YZBuffer::update_view(int view_nb)
{
	int y;
	YZView *view = view_list[view_nb];

	for (y=view->get_current(); y<text.count() && view->is_line_visible(y); y++) {

		QString l = find_line( view->get_current()+y);
		if (!l) continue;
		view->post_event(mk_event_setline(y,&l));
	}

	view->post_event(mk_event_setcursor(0,0));
	
//MM opening files may be done in different modes
	//view->post_event(mk_event_setstatus("Yzis Ready"));
}

void YZBuffer::update_all_views()
{
	for ( int i = 0 ; i < view_nb ; i++) update_view(i);
}

void  YZBuffer::add_line(QString &l)
{
/*	l->set_next(NULL);

	if (line_last)
		line_last->set_next(l);
	else line_first=l;

	line_last=l;*/
	text.append(l);
}


QString	YZBuffer::find_line(int line)
{
	/* sub-optimal, i know */
/*
	QString *l=NULL;

	for (l=line_first; l; l=l->next())
		if (l->line==line) return l;
		else if (l->line>line) return NULL;

	return NULL;*/
	QString t = text[ line ];
	return t;
}

void YZBuffer::load(void)
{
	QFile file( path );
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {
			QString line(stream.readLine() ); // line of text excluding '\n'
//			lines_nb++;
			add_line( line );
		}
		file.close();
	}
	update_all_views();
}

void YZBuffer::save(void)
{
	if (path.isEmpty()) {
		//error("called though path is null, ignored");
		return;
	}
}

