/**
 * $Id$
 */
#include "viewwidget.h"
#include "factory.h"
#include <qlayout.h>
#include <qevent.h>
#include <kapplication.h>
#include "debug.h"

KYZisView::KYZisView ( KYZisDoc *doc, QWidget *parent, const char *name )
	: KTextEditor::View (doc, parent, name), YZView(doc, KYZisFactory::sess, 10)
{
	editor = new KYZisEdit (this,"editor");
	status = new KStatusBar (this, "status");
	command = new KYZisCommand ( this, "command");

	status->insertItem("Yzis Ready",0,1);
	status->setItemAlignment(0,Qt::AlignLeft);

	status->insertItem("",80,80,0);
	status->setItemAlignment(0,Qt::AlignLeft);

	status->insertItem("Yzis Ready",0,1);
	status->setItemAlignment(0,Qt::AlignRight);

	status->insertItem("",99,0,true);
	status->setItemAlignment(99,Qt::AlignRight);

	QVBoxLayout *l = new QVBoxLayout(this);
	l->addWidget(editor);
	l->addWidget( command );
	l->addWidget(status);

	KYZisFactory::registerView( this );

	buffer = doc;
	editor->show();
	status->show();
	setFocusPolicy( StrongFocus );
	editor->setFocus();
}

KYZisView::~KYZisView () {
	if ( buffer ) buffer->removeView(this);
	KYZisFactory::deregisterView( this );
}

void KYZisView::showEvent ( QShowEvent* ) {
	yzDebug() << "View " << myId << " got activated" << endl;
	KYZisFactory::s_self->setCurrentView( this );
}

#include "viewwidget.moc"
