/**
 * $Id$
 */

#include "kyzisdoc.h"
#include "kyzisview.h"
#include "kyzis_factory.h"
#include "yz_debug.h"

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
		KYZisFactory::registerDocument( this );
}

KYZisDoc::~KYZisDoc () {
	KYZisFactory::deregisterDocument( this );
/*	_views.setAutoDelete( true );
	_views.clear();
	_views.setAutoDelete( false );*/
}

KTextEditor::View *KYZisDoc::createView ( QWidget *parent, const char * /*name*/) {
	KYZisView *v = new KYZisView (this, sess, parent);
	//FIXME : two lists
	addView(v);
	_views.append( v );
	return v;
}

void KYZisDoc::removeView( KTextEditor::View * v ) {
	if ( !v ) 
		return;

	_views.removeRef( v );
}

bool KYZisDoc::openFile () {
	yzDebug() << "openFile " << m_file << endl;
	path = m_file;
	load();
	return true;
}

bool KYZisDoc::saveFile () {
	YZBuffer::save();
	return true;
}

#include "kyzisdoc.moc"
