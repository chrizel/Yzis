#include "kyzisview.h"
#include "kyzis_factory.h"
#include <qlayout.h>

KYZisView::KYZisView ( KYZisDoc *doc, QWidget *parent, const char *name ) 
	: KTextEditor::View (doc, parent, name),
		YZView(doc, 10) {
	editor = new KYZisEdit (this,"editor");
	QVBoxLayout *l = new QVBoxLayout(this);		
	l->add(editor);
	buffer = doc;
	editor->show();
}

KYZisView::~KYZisView () {
	delete editor;
}

#include "kyzisview.moc"
