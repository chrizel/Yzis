#include "kyzisview.h"
#include "kyzis_factory.h"
#include <qlayout.h>
#include <qevent.h>
#include <qapplication.h>
#include <kdebug.h>

KYZisView::KYZisView ( KYZisDoc *doc, QWidget *parent, const char *name )
	: KTextEditor::View (doc, parent, name),
		YZView(doc, 10) {
	editor = new KYZisEdit (this,"editor");
	status = new KStatusBar (this, "status");
	status->insertItem("Yzis Ready",0);
	
	QVBoxLayout *l = new QVBoxLayout(this);
	l->addWidget(editor);
	l->addWidget(status);

	register_manager(this);

	buffer = doc;
	editor->show();
	status->show();
}

KYZisView::~KYZisView () {
	delete editor;
}

void KYZisView::postEvent(yz_event ev) {
	kdDebug() << "postEvent " << ev.id <<endl;
//	YZView::post_event(ev); //dad, do your job
	QCustomEvent *myev = new QCustomEvent (QEvent::User+ev.id);//so that we know what kind of event it is, may be useless ...
	QApplication::postEvent( this, myev ); //this gives Qt the priority before processing our own events
}

//receives previously generated events from Qt event loop. hopefully it will do
//what I want :)
void KYZisView::customEvent (QCustomEvent *ev) {
	kdDebug() << "test" <<endl;
//	editor->setText( "e" );
}

#include "kyzisview.moc"
