#include "kyzisview.h"
#include "kyzis_factory.h"
#include <qlayout.h>
#include <qevent.h>
#include <qapplication.h>

KYZisView::KYZisView ( KYZisDoc *doc, QWidget *parent, const char *name ) 
	: KTextEditor::View (doc, parent, name),
		YZView(doc, 10) {
	editor = new KYZisEdit (this,"editor");
	status = new KStatusBar (this, "status");
	status->insertItem("Yzis Ready",0);
	
	QVBoxLayout *l = new QVBoxLayout(this);		
	l->addWidget(editor);
	l->addWidget(status);

	buffer = doc;
	editor->show();
	status->show();
}

KYZisView::~KYZisView () {
	delete editor;
}

void KYZisView::post_event(yz_event ev) {
	YZView::post_event(ev); //dad, do your job
	QCustomEvent myev (QEvent::User+ev.id);//so that we know what kind of event it is, may be useless ...
	QApplication::postEvent( this, &myev ); //this gives Qt the priority before processing our own events
}

//receives previously generated events from Qt event loop. hopefully it will do
//what I want :)
void KYZisView::customEvent (QCustomEvent *ev) {
	editor->setText( "e" );
}

#include "kyzisview.moc"
