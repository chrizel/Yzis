/**
 * $Id: kyzisedit.cpp 218 2003-05-29 23:06:22Z mikmak $
 */

#include "kyziscommand.h"
#include <kdebug.h>

KYZisCommand::KYZisCommand(KYZisView *parent, const char *name)
	: KComboBox( true, parent, name ) {
		_parent = parent;
}

KYZisCommand::~KYZisCommand() {
}

void KYZisCommand::keyPressEvent ( QKeyEvent * e ) {
	kdDebug()<< " Got key : " << e->key()<< " Got ASCII : " << e->ascii() << " Got Unicode : " << e->text() << endl;
	if ( e->ascii() != 0 ) {
		_parent->sendChar(e->ascii());
		e->accept();
	}
	KComboBox::keyPressEvent( e );
}

#include "kyziscommand.moc"
