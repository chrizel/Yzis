/* This file is part of the Yzis libraries
 *  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id: kyzisedit.cpp 218 2003-05-29 23:06:22Z mikmak $
 */

#include "command.h"
#include "debug.h"
#include "view.h"

KYZisCommand::KYZisCommand(KYZisView *parent, const char *name)
	: KLineEdit( parent, name ) {
		_parent = parent;
}

KYZisCommand::~KYZisCommand() {
}

void KYZisCommand::keyPressEvent ( QKeyEvent * e ) {
	yzDebug()<< " KYZisCommand Got key : " << e->key()<< " Got ASCII : " << e->ascii() << " Got Unicode : " << e->text() << endl;
	if ( e->key() != 0 ) {
		_parent->sendKey(e->key(), e->state());
		e->accept();
	}
}

void KYZisCommand::focusInEvent (QFocusEvent *) {
	yzDebug() << "KYZisCommand : Focus IN -> EX mode" << endl;
	_parent->gotoExMode();
}

void KYZisCommand::focusOutEvent (QFocusEvent *e) {
	yzDebug() << "KYZisCommand : Focus OUT -> reject" << endl;
	if (_parent->getCurrentMode() == YZView::YZ_VIEW_MODE_EX) {
		return;
	}
	QWidget::focusOutEvent(e);
}

#include "command.moc"
