#include "kyzisdoc.h"
#include "kyzisview.h"
#include "kyzis_factory.h"

KYZisDoc::KYZisDoc (QWidget *parentWidget, const char *widgetName,QObject *parent, const char *name)
	: KTextEditor::Document(parent,name), YZBuffer() {
			setInstance(KYZisFactory::instance());
			KTextEditor::View *current = createView ( parentWidget, "doc widget" );
			insertChildClient(current);
			current->show();
			setWidget(current);
}

KYZisDoc::~KYZisDoc () {
}

KTextEditor::View *KYZisDoc::createView ( QWidget *parent, const char *name) {
	KYZisView *v = new KYZisView (this, parent);
	_views.append(v);
	return v;
}

bool KYZisDoc::openFile () {
	return true;
}

bool KYZisDoc::saveFile () {
	return true;
}

#include "kyzisdoc.moc"
