#include "kyzisedit.h"
#include <kdebug.h>

KYZisEdit::KYZisEdit(KYZisView *parent, const char *name) 
	: QTextEdit( parent, name ) {
		_parent = parent;
}

KYZisEdit::~KYZisEdit() {
}

void KYZisEdit::keyPressEvent ( QKeyEvent * e ) {
	kdDebug()<< " Got key : " << e->ascii() << endl;
	_parent->send_char(e->ascii());
	e->accept();
}

#include "kyzisedit.moc"
