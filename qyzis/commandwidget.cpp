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

QYZisCommand::QYZisCommand(QYZisView *parent)
	: QLineEdit( parent ) {
		_parent = parent;
}

QYZisCommand::~QYZisCommand() {
}

void QYZisCommand::keyPressEvent ( QKeyEvent * e ) {
	yzDebug()<< " QYZisCommand Got key : " << e->key()<< " Got ASCII : " << e->text().toLatin1().constData() << " Got Unicode : " << e->text() << endl;
	QString modifiers;
	if ( e->QInputEvent::modifiers() & Qt::ShiftModifier ) modifiers += "<SHIFT>";
	if ( e->QInputEvent::modifiers() & Qt::AltModifier ) modifiers += "<ALT>";
	if ( e->QInputEvent::modifiers() & Qt::ControlModifier ) modifiers += "<CTRL>";
	if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Up || e->key() == Qt::Key_Down || e->key() == Qt::Key_Escape) {
		_parent->sendKey(_parent->editor()->convertKey( e->key() ), modifiers ) ;
		e->accept();
	} 
	else if ( ( e->QInputEvent::modifiers() & Qt::ControlModifier ) && e->key() == Qt::Key_C ) { // handle CTRL-C 
		_parent->sendKey( "c" , modifiers ) ;
		e->accept();
	}
	else QLineEdit::keyPressEvent( e );
}

void QYZisCommand::focusInEvent (QFocusEvent *) {
	yzDebug() << "QYZisCommand : Focus IN -> EX mode" << endl;
	if ( _parent->modePool()->currentType() != YZMode::MODE_EX 
			&& _parent->modePool()->currentType() != YZMode::MODE_SEARCH 
			&& _parent->modePool()->currentType() != YZMode::MODE_SEARCH_BACKWARD )
		_parent->modePool()->push( YZMode::MODE_EX );
}

void QYZisCommand::focusOutEvent (QFocusEvent *e) {
	yzDebug() << "QYZisCommand : Focus OUT -> reject" << endl;
	if ( _parent->modePool()->currentType() != YZMode::MODE_EX 
			&& _parent->modePool()->currentType() != YZMode::MODE_SEARCH 
			&& _parent->modePool()->currentType() != YZMode::MODE_SEARCH_BACKWARD )
		return;
	QWidget::focusOutEvent(e);
}

