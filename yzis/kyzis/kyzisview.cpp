#include "kyzisview.h"
#include "kyzis_factory.h"
#include <qlayout.h>
#include <qevent.h>
#include <qapplication.h>
#include <kdebug.h>

KYZisView::KYZisView ( KYZisDoc *doc, QWidget *parent, const char *name )
	: KTextEditor::View (doc, parent, name),
		YZView(doc, 10) {
	last_event_done=0;
	editor = new KYZisEdit (this,"editor");
	status = new KStatusBar (this, "status");
	status->insertItem("Yzis Ready",0);
	status->setFixedHeight(status->height());
	
	QVBoxLayout *l = new QVBoxLayout(this);
	l->addWidget(editor);
	l->addWidget(status);

	register_manager(this);

	buffer = doc;
	editor->show();
	status->show();
	editor->setFocus();
}

KYZisView::~KYZisView () {
	delete editor;
	delete status;
}

void KYZisView::postEvent(yz_event ev) {
	kdDebug() << "postEvent " << events_nb_last <<endl;
	QCustomEvent *myev = new QCustomEvent (QEvent::User+events_nb_last);//so that we know what kind of event it is, may be useless ...
	QApplication::postEvent( this, myev ); //this gives Qt the priority before processing our own events
}

//receives previously generated events from Qt event loop. hopefully it will do
//what I want :)
void KYZisView::customEvent (QCustomEvent *) {
	yz_event *event;
	while (last_event_done < events_nb_last) {
		event = fetch_event(last_event_done++);
		kdDebug() << "** Processing Event " << last_event_done << " Id : " << event->id << endl;
		if (! event ) {
			kdDebug() << "OUPS, no event to fetch !" << endl;
			return;
		}
		switch ( event->id ) {
			case YZ_EV_SETLINE:
				kdDebug() << "event SETLINE" << *(event->u.setline.line) << endl;
				editor->setTextLine(event->u.setline.y,*(event->u.setline.line));
				break;
			case YZ_EV_SETCURSOR:
				kdDebug() << "event SETCURSOR" << endl;
				editor->setCursor (event->u.setcursor.x, event->u.setcursor.y);
				break;
			case YZ_EV_SETSTATUS:
				kdDebug() << "event SETSTATUS" << event->u.setstatus.text <<  endl;
				status->changeItem( event->u.setstatus.text,0 );
				break;
		}
 }
}

void KYZisView::scrollDown( int lines ) {
	kdDebug() << "ScrollDown " << lines <<endl;
	editor->scrollBy(0, lines * editor->fontMetrics().lineSpacing());
}

void KYZisView::scrollUp ( int lines ) {
	kdDebug() << "ScrollUp " << lines <<endl;
	editor->scrollBy(0, -1 * lines * editor->fontMetrics().lineSpacing());
}

#include "kyzisview.moc"
