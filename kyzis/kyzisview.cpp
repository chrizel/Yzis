/**
 * $Id$
 */
#include "kyzisview.h"
#include "kyzis_factory.h"
#include <qlayout.h>
#include <qevent.h>
#include <qapplication.h>
#include <kdebug.h>

KYZisView::KYZisView ( KYZisDoc *doc, YZSession *_session, QWidget *parent, const char *name )
	: KTextEditor::View (doc, parent, name),
		YZView(doc, 10) {
	last_event_done=0;
	currentSession = _session;
	editor = new KYZisEdit (this,"editor");
	status = new KStatusBar (this, "status");
	status->insertItem("Yzis Ready",0);
	status->setFixedHeight(status->height());
	
	QVBoxLayout *l = new QVBoxLayout(this);
	l->addWidget(editor);
	l->addWidget(status);

	registerManager(this);

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
	QCustomEvent *myev = new QCustomEvent (QEvent::User);
	QApplication::postEvent( this, myev ); //this hopefully gives Qt the priority before processing our own events
}

//receives previously generated events from Qt event loop. hopefully it will do
//what I want :)
void KYZisView::customEvent (QCustomEvent *) {
	while ( true ) {
		yz_event event = fetchNextEvent();
		switch ( event.id ) {
			case YZ_EV_SETLINE:
				editor->setTextLine(event.setline.y,event.setline.line);
				break;
			case YZ_EV_SETCURSOR:
				kdDebug() << "event SETCURSOR" << endl;
				editor->setCursor (event.setcursor.x, event.setcursor.y);
				break;
			case YZ_EV_SETSTATUS:
				kdDebug() << "event SETSTATUS" << event.setstatus.text <<  endl;
				status->changeItem( event.setstatus.text,0 );
				break;
			case YZ_EV_NOOP:
				return;
		}
	}
}

void KYZisView::scrollDown( int lines ) {
	kdDebug() << "ScrollDown " << lines <<endl;
	editor->scrollBy(0, lines * editor->fontMetrics().lineSpacing());
	editor->update();
}

void KYZisView::scrollUp ( int lines ) {
	kdDebug() << "ScrollUp " << lines <<endl;
	editor->scrollBy(0, -1 * lines * editor->fontMetrics().lineSpacing());
	editor->update();
}

YZSession *KYZisView::getCurrentSession() {
	return currentSession;
}
#include "kyzisview.moc"
