/* This file is part of the Yzis libraries
 *  Copyright (C) 2007 Lothar Braun <lothar@lobraun.de>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kyziscommandwidget.h"
#include "kyzisview.h"
#include "kyzissession.h"


#include <libyzis/view.h>
#include <libyzis/debug.h>
#include <libyzis/mode.h>

#include <QKeyEvent>


KYZisCommand::KYZisCommand( KYZisView *view )
	: KLineEdit( view ) 
{
	m_view = view;
}

KYZisCommand::~KYZisCommand() {
}

void KYZisCommand::keyPressEvent ( QKeyEvent * e ) {
	yzDebug()<< " KYZisCommand Got key : " << e->key() << " Got ASCII : " /*<< e->text().toAscii()*/ << " Got Unicode : " << e->text() << endl;
	QString modifiers;
	if ( e->modifiers() & Qt::ShiftModifier ) modifiers += "<SHIFT>";
	if ( e->modifiers() & Qt::AltModifier ) modifiers += "<ALT>";
	if ( e->modifiers() & Qt::ControlModifier ) modifiers += "<CTRL>";
	if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Up || e->key() == Qt::Key_Down || e->key() == Qt::Key_Escape) {
		KYZisSession::self()->sendKey(m_view, m_view->convertKey( e->key() ), modifiers ) ;
		e->accept();
	} 
	else if ( ( e->modifiers() & Qt::ControlModifier ) && e->key() == Qt::Key_C ) { // handle CTRL-C 
		KYZisSession::self()->sendKey( m_view, "c" , modifiers ) ;
		e->accept();
	}
	else KLineEdit::keyPressEvent( e );
}

void KYZisCommand::focusInEvent ( QFocusEvent* e ) {
	yzDebug() << "KYZisCommand : Focus IN -> EX mode" << endl;
	if ( m_view->modePool()->currentType() != YZMode::ModeEx 
			&& m_view->modePool()->currentType() != YZMode::ModeSearch 
			&& m_view->modePool()->currentType() != YZMode::ModeSearchBackward ) {
		m_view->modePool()->push( YZMode::ModeEx );
	}
	KLineEdit::focusInEvent( e );
}

void KYZisCommand::focusOutEvent ( QFocusEvent* e ) {
	yzDebug() << "KYZisCommand : Focus OUT -> reject" << endl;
	if ( m_view->modePool()->currentType() != YZMode::ModeEx 
			&& m_view->modePool()->currentType() != YZMode::ModeSearch 
			&& m_view->modePool()->currentType() != YZMode::ModeSearch )
		return;
	KLineEdit::focusOutEvent( e );
}

#include "kyziscommandwidget.moc"
