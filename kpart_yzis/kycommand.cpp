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

#include "kycommand.h"
#include "kyview.h"
#include "kysession.h"


#include <libyzis/view.h>
#include <libyzis/debug.h>
#include <libyzis/mode.h>

#include <QKeyEvent>


KYCommand::KYCommand( KYView *view )
        : KLineEdit( view )
{
    m_view = view;
}

KYCommand::~KYCommand()
{}

void KYCommand::keyPressEvent ( QKeyEvent * e )
{
    yzDebug() << " KYCommand Got key : " << e->key() << " Got ASCII : " /*<< e->text().toAscii()*/ << " Got Unicode : " << e->text() << endl;
    int modifiers = 0;
    if ( e->modifiers() & Qt::ShiftModifier ) modifiers |= YKey::Mod_Shift;
    if ( e->modifiers() & Qt::AltModifier ) modifiers |= YKey::Mod_Alt;
    if ( e->modifiers() & Qt::ControlModifier ) modifiers |= YKey::Mod_Ctrl;
    if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Up || e->key() == Qt::Key_Down || e->key() == Qt::Key_Escape) {
        KYSession::self()->sendKey(m_view, m_view->convertKey( e->key() ) ) ;
        e->accept();
    } else if ( ( e->modifiers() & Qt::ControlModifier ) && e->key() == Qt::Key_C ) { // handle CTRL-C
        KYSession::self()->sendKey( m_view, YKey( YKey::Key_c , modifiers ) ) ;
        e->accept();
    } else KLineEdit::keyPressEvent( e );
}

void KYCommand::focusInEvent ( QFocusEvent* e )
{
    yzDebug() << "KYCommand : Focus IN -> EX mode" << endl;
    if ( m_view->modePool()->currentType() != YMode::ModeEx
            && m_view->modePool()->currentType() != YMode::ModeSearch
            && m_view->modePool()->currentType() != YMode::ModeSearchBackward ) {
        m_view->modePool()->push( YMode::ModeEx );
    }
    KLineEdit::focusInEvent( e );
}

void KYCommand::focusOutEvent ( QFocusEvent* e )
{
    yzDebug() << "KYCommand : Focus OUT -> reject" << endl;
    if ( m_view->modePool()->currentType() == YMode::ModeEx
            || m_view->modePool()->currentType() == YMode::ModeSearch
            || m_view->modePool()->currentType() == YMode::ModeSearch )
        m_view->modePool()->pop();
    KLineEdit::focusOutEvent( e );
}

#include "kycommand.moc"
