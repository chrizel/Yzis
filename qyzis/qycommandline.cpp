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

/* QYzis */
#include "qycommandline.h"
#include "qyview.h"
#include "qyedit.h"
#include "qyzis.h"

/* Yzis */
#include "debug.h"
#include "mode.h"
#include "view.h"
#include "buffer.h"
#include "session.h"

#define dbg() yzDebug("QYCommandLine")
#define err() yzError("QYCommandLine")

QYCommandLine::QYCommandLine(QYView * view)
        : QLineEdit( view )
{
    mView = view;
    setFocusPolicy( Qt::ClickFocus );
}

QYCommandLine::~QYCommandLine()
{}

void QYCommandLine::keyPressEvent ( QKeyEvent * e )
{
    dbg() << "keyPressEvent( modifier=" << e->modifiers() << ", key=" << e->key() << ", ascii=" << e->text().toLatin1().constData() << ", unicode=" << e->text() << ")" << endl;
    int modifiers = 0;
    if ( e->modifiers() & Qt::ShiftModifier ) modifiers |= YKey::Mod_Shift;
    if ( e->modifiers() & Qt::AltModifier ) modifiers |= YKey::Mod_Alt;
    if ( e->modifiers() & Qt::ControlModifier ) modifiers |= YKey::Mod_Ctrl;
    if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Up || e->key() == Qt::Key_Down || e->key() == Qt::Key_Escape) {
        dbg() << "keyPressEvent: sending to Session" << endl;
        YSession::self()->sendKey( static_cast< YView * >( mView ), mView->editor()->convertKey( e->key() ) ) ;
        e->accept();
    } else if ( ( e->QInputEvent::modifiers() & Qt::ControlModifier ) && e->key() == Qt::Key_C ) { // handle CTRL-C
        dbg() << "keyPressEvent: sending CONTROL-C to Session" << endl;
        YSession::self()->sendKey( static_cast< YView * >( mView ), YKey(YKey::Key_c, modifiers) ) ;
        e->accept();
    } else {
        dbg() << "keyPressEvent: sending to QLineEdit" << endl;
        QLineEdit::keyPressEvent( e );
    }
}

void QYCommandLine::focusInEvent (QFocusEvent * e)
{
    dbg() << "focusInEvent() for " << mView->myBuffer()->fileNameShort() << ", reason=" << e->reason() << endl;

    //e->accept();
    QLineEdit::focusInEvent(e);
}

void QYCommandLine::focusOutEvent (QFocusEvent *e)
{
    dbg() << "focusOutEvent() for " << mView->myBuffer()->fileNameShort() << " ,reason=" << e->reason() << endl;
    /*
    if ( mView->modePool()->currentType() != YMode::ModeEx 
    && mView->modePool()->currentType() != YMode::ModeSearch 
    && mView->modePool()->currentType() != YMode::ModeSearchBackward )
    return;
    */
    QLineEdit::focusOutEvent(e);
}

