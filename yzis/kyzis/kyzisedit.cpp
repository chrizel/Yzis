#include "kyzisedit.h"

KYZisEdit::KYZisEdit(KYZisView *parent, const char *name) 
	: QTextEdit( parent, name ) {
		_parent = parent;
}

KYZisEdit::~KYZisEdit() {
}

void KYZisEdit::keyPressEvent ( QKeyEvent * e ) {
//X 	setText( e->text() );
	//FIXME should send text() unicode ...
	_parent->send_char(e->key());
	e->accept();
}

#include "kyzisedit.moc"
