/**
 * YZBuffer implementation
 */


#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "yz_buffer.h"
#include "yz_events.h"
#include "yz_view.h"

YZBuffer::YZBuffer(QString _path)
{
	path	= _path;
	view_nb	= 0;
	lines_nb= 0;

	line_first = line_last = NULL; // linked lines

	if (!_path.isEmpty()) load();
}

void YZBuffer::add_char (int x, int y, QChar c)
{
	/* brute force, we'll have events specific for that later on */
	YZLine *l=find_line(y);
	if (!l) return;

	/* do the actual modification */
	l->insert(x, c);

	/* inform the views */
	yz_event e;
	e.id			= YZ_EV_SETLINE;
	e.u.setline.y		= y;
	e.u.setline.line	= l;
	
	post_event(e);
}


void YZBuffer::chg_char (int x, int y, QChar c)
{
	/* brute force, we'll have events specific for that later on */
	YZLine *l=find_line(y);

	/* do the actual modification */
	l->remove(x,1);
	l->insert(x, c);

	/* inform the views */
	yz_event e;
	e.id			= YZ_EV_SETLINE;
	e.u.setline.y		= y;
	e.u.setline.line	= l;

	post_event(e);
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
	view_list[view_nb++] = v;

	update_view(view_nb-1);
} 

void YZBuffer::update_view(int view_nb)
{
	int y;
	yz_event e;
	YZView *view = view_list[view_nb];

	for (y=view->get_current(); y<lines_nb && view->is_line_visible(y); y++) {

		YZLine *l = find_line( view->get_current()+y);
//		yz_assert(l, "find_line failed");
		/* post an event about updating this line */

		e.id			= YZ_EV_SETLINE;
		e.u.setline.y		= y;
		e.u.setline.line	= l;

//		debug("y is %d, l is %p", y, l);
//		yz_printfirst("YZBuffer::update_view, line is : ", l->data);

		view->post_event(e);
	}

	e.id = YZ_EV_SETCURSOR;
	e.u.setcursor.x = 0;
	e.u.setcursor.y = 0;
	view->post_event(e);

	view->post_event(mk_event_setstatus("Yzis Ready"));
}

void  YZBuffer::add_line(YZLine *l)
{
	l->set_next(NULL);

	if (line_last)
		line_last->set_next(l);
	else line_first=l;

	line_last=l;
}


YZLine	*YZBuffer::find_line(int line)
{
	/* sub-optimal, i know */

	YZLine *l=NULL;

	for (l=line_first; l; l=l->next())
		if (l->line==line) return l;
		else if (l->line>line) return NULL;

	return NULL;
}


void YZBuffer::load(void)
{
	FILE *f;
	size_t len;
	char buf[YZ_LINE_DEFAULT_LENGTH+2];
	char *ptr;
	int dismiss=false;

//	debug("entering");
	if (path.isEmpty()) {
//		error("called though path is null, ignored");
		return;
	}

	f = fopen(path, "r");
	if ( !f) {
//		error("Can't open file, errno is %d", errno);
		return;
	}

	len = 0; // len is the number of valid byte in buf[]
	lines_nb	= 0;
	do { // read the whole file
		int a;
		a = fread(buf, sizeof(char), YZ_LINE_DEFAULT_LENGTH-len, f);
//		debug("read %d bytes from file", a);
		len +=a;


		while (1) {
			for (ptr=buf; (ptr-buf)<len && *ptr!='\n'; ptr++); // find the next '\n'
	
			if (len<=0) break;
			/* quickly handle dismiss case */
			if (dismiss) 
				if ( (ptr-buf)>=len ) {
					/* we are at the end of the buffer, dismiss everything..*/
					len =0;
//					debug("deleting.. continue");
					break;
				}
				else {
					/* *ptr == 0, dismiss until \0, included  */
					ptr++;
					len -= (ptr-buf); memcpy(buf, ptr, len); // remove handled part
//					debug("removing %d bytes from buf", ptr-buf);
//					debug("deleting.. end");
					dismiss = false;
					continue;
				}

			if ( (ptr-buf)>=len) {
				/* we reached the end of the buffer */
				if ( (ptr-buf)<YZ_LINE_DEFAULT_LENGTH )
					break; // read some more data, the buffer is too small

				/* this line is definetely too long */
				YZLine *line = new YZLine(lines_nb++, buf/*, len-1*/); // ptr-buf == len, here
				len=0;
				/* add the new line */
				add_line(line);
//				debug("adding a looooong YZLine");
				dismiss = true;
//				debug("deleting... begin");
				continue;
			}


//			yz_assert( *ptr=='\n', "this should not happen, *ptr== %d, ", *ptr);
			//			here *ptr='\n'
			ptr++;
			/* we found a whole line, use it */
			YZLine *line = new YZLine(lines_nb++, buf/*, ptr-buf-1*/);
			len -= (ptr-buf); memcpy(buf, ptr, len); // remove handled part
//			debug("removing %d bytes from buf", ptr-buf);
			/* add the new line */
			add_line(line);
//			debug("adding a normal YZLine");
		} // while (1)

	} while (!feof(f));

	/* flush the buffer */
//	debug("flushing what remains in buffer");
	while (len>0) {
		for (ptr=buf; (ptr-buf)<len && *ptr!='\n'; ptr++); // find the next '\n'

		/* quickly handle dismiss case */
		if (dismiss) 
			if ( (ptr-buf)>=len ) {
				/* we are at the end of the buffer, dismiss everything..*/
				len =0;
//				debug("deleting.. continue");
				break;
			}
			else {
				/* *ptr == 0, dismiss until \0, included  */
				ptr++;
				len -= (ptr-buf); memcpy(buf, ptr, len); // remove handled part
//				debug("removing %d bytes from buf", ptr-buf);
//				debug("deleting.. end");
				dismiss = false;
				continue;
			}

		if ( (ptr-buf)>=len && (ptr-buf)>=YZ_LINE_DEFAULT_LENGTH ) {
			/* this line is definetely too long */
			YZLine *line = new YZLine(lines_nb++, buf/*, len-1*/); // ptr-buf == len, here
			/* add the new line */
			add_line(line);
//			debug("adding a looooong YZLine");
			break; // nothing left in the buffer
		}


		// everything left has no \n in it, and len should be >0
	//	yz_assert(len>0, "oops, len is not >0 here, it should though");
		ptr++;
		YZLine *line = new YZLine(lines_nb++, buf/*, len-1*/);
		add_line(line);
//		debug("adding a normal YZLine");
	} // while (1)


//	debug("closing input file");
	fclose(f);
}


void YZBuffer::save(void)
{
	if (path.isEmpty()) {
		//error("called though path is null, ignored");
		return;
	}
}

