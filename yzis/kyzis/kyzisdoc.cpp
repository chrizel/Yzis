#include "kyzisdoc.h"
#include "kyzisview.h"

KYZisDoc::KYZisDoc (QWidget *parentWidget, const char *widgetName,QObject *parent, const char *name)
	: KTextEditor::Document(parent,name),
		YZBuffer() {

}

KYZisDoc::~KYZisDoc () {

}

KTextEditor::View *KYZisDoc::createView ( QWidget *parent, const char *name) {
	KYZisView *v = new KYZisView (this, parent);
	return v;
}

bool KYZisDoc::openFile () {
	return true;
}

bool KYZisDoc::saveFile () {
	return true;
}

#include "kyzisdoc.moc"
