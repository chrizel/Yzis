/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
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

#include "viewwidget.h"
#include "commandwidget.h"
#include "debug.h"
#include "editor.h"
#include "mode.h"
#include "view.h"
#include "session.h"
#include "qyzis.h"

#define dbg() yzDebug("QYZisCommand")
#define err() yzError("QYZisCommand")

QYZisCommand::QYZisCommand(QYZisView *parent)
	: QLineEdit( parent ) {
    _parent = parent;
    setFocusPolicy( Qt::ClickFocus );
}

QYZisCommand::~QYZisCommand() {
}

void QYZisCommand::keyPressEvent ( QKeyEvent * e ) {
	dbg()<< "keyPressEvent( modifier=" << e->modifiers() << ", key=" << e->key()<< ", ascii=" << e->text().toLatin1().constData() << ", unicode=" << e->text() << ")" << endl;
	QString mod;
	if ( e->modifiers() & Qt::ShiftModifier ) mod += "<SHIFT>";
	if ( e->modifiers() & Qt::AltModifier ) mod += "<ALT>";
	if ( e->modifiers() & Qt::ControlModifier ) mod += "<CTRL>";
	if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Up || e->key() == Qt::Key_Down || e->key() == Qt::Key_Escape) {
        dbg() << "keyPressEvent: sending to Session" << endl;
		YZSession::self()->sendKey( static_cast< YZView * >( _parent ), _parent->editor()->convertKey( e->key() ), mod ) ;
		e->accept();
	} 
	else if ( ( e->QInputEvent::modifiers() & Qt::ControlModifier ) && e->key() == Qt::Key_C ) { // handle CTRL-C 
        dbg() << "keyPressEvent: sending CONTROL-C to Session" << endl;
		YZSession::self()->sendKey( static_cast< YZView * >( _parent ), "c" , mod ) ;
		e->accept();
	}
	else {
        dbg() << "keyPressEvent: sending to QLineEdit" << endl;
        QLineEdit::keyPressEvent( e );
    }
}

void QYZisCommand::focusInEvent (QFocusEvent * e) {
	dbg() << "focusInEvent() " << endl;
	if ( _parent->modePool()->currentType() != YZMode::ModeEx 
			&& _parent->modePool()->currentType() != YZMode::ModeSearch 
			&& _parent->modePool()->currentType() != YZMode::ModeSearchBackward ) {
		_parent->modePool()->push( YZMode::ModeEx );
    }

    //e->accept();
    QLineEdit::focusInEvent(e);
}

void QYZisCommand::focusOutEvent (QFocusEvent *e) {
	dbg() << "focusOutEvent() " << endl;
    /*
	if ( _parent->modePool()->currentType() != YZMode::ModeEx 
			&& _parent->modePool()->currentType() != YZMode::ModeSearch 
			&& _parent->modePool()->currentType() != YZMode::ModeSearchBackward )
		return;
    */
	QLineEdit::focusOutEvent(e);
}

