/**
 * $Id$
 */

#include "kyzisdoc.h"
#include "kyzisview.h"
#include "kyzis_factory.h"
#include <kdebug.h>

static YZSession *sess = 0;
		
KYZisDoc::KYZisDoc (QWidget *parentWidget, const char * /*widgetName*/, QObject *parent, const char *name)
	: KTextEditor::Document(parent,name), YZBuffer(QString::null) {
		setInstance(KYZisFactory::instance());
		if ( !sess )
			sess = new YZSession();
		sess->addBuffer( this );
		KTextEditor::View *current = createView ( parentWidget, "doc widget" );
		insertChildClient(current);
		current->show();
		setWidget(current);
//		_views.setAutoDelete( true ); //we don't own !
}

KYZisDoc::~KYZisDoc () {
	delete sess;
}

KTextEditor::View *KYZisDoc::createView ( QWidget *parent, const char * /*name*/) {
	KYZisView *v = new KYZisView (this, sess, parent);
	//FIXME : two lists
	addView(v);
	_views.append( v );
	return v;
}

bool KYZisDoc::openFile () {
	kdDebug() << "openFile " << m_file << endl;
	path = m_file;
	load();
	return true;
}

bool KYZisDoc::saveFile () {
	YZBuffer::save();
	return true;
}

#include "kyzisdoc.moc"
