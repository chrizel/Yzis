#include "kyzisdoc.h"
#include "kyzisview.h"
#include "kyzis_factory.h"
#include <kdebug.h>

KYZisDoc::KYZisDoc (QWidget *parentWidget, const char *widgetName,QObject *parent, const char *name)
	: KTextEditor::Document(parent,name), YZBuffer(QString::null) {
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
	//FIXME : two lists
	_views.append(v);
	add_view(v);
	return v;
}

bool KYZisDoc::openFile () {
	kdDebug() << "openFile " << m_file << endl;
	path = m_file;
	load();
	return true;
}

bool KYZisDoc::saveFile () {
	return true;
}

#include "kyzisdoc.moc"
