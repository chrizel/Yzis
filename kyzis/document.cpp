/**
 * $Id$
 */

#include "document.h"
#include "viewwidget.h"
#include "factory.h"
#include "debug.h"

KYZisDoc::KYZisDoc (QWidget *parentWidget, const char * /*widgetName*/, QObject *parent, const char *name)
	: KTextEditor::Document(parent,name), YZBuffer(KYZisFactory::sess,QString::null) {
		setInstance(KYZisFactory::instance());
		KTextEditor::View *current = createView ( parentWidget, "doc widget" );
		insertChildClient(current);
		current->show();
		setWidget(current);
		KYZisFactory::registerDocument( this );
}

KYZisDoc::~KYZisDoc () {
	KYZisFactory::deregisterDocument( this );
}

KTextEditor::View *KYZisDoc::createView ( QWidget *parent, const char * /*name*/) {
	KYZisView *v = new KYZisView (this, parent);
	//FIXME : two lists
	//addView(v);
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

#include "document.moc"
