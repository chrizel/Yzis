#include "kyzisview.h"
#include "kyzis_factory.h"

KYZisView::KYZisView ( KYZisDoc *doc, QWidget *parent, const char *name ) 
: KTextEditor::View (doc, parent, name),
	YZView(doc, 10) {
	editor = new KYZisEdit (this,"editor");
	setInstance(KYZisFactory::instance());
	buffer = doc;
	editor->show();
}

KYZisView::~KYZisView () {
	delete editor;
}

#include "kyzisview.moc"
