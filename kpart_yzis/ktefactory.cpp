/*  This file is part of the Yzis libraries
*  Copyright (C) 2007 Lothar Braun <lothar@lobraun.de>
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


#include "ktefactory.h"
#include "ktedocument.h"
#include "kteeditor.h"
#include "kyzissession.h"

KTEFactory::KTEFactory(QObject* parent)
        : KTextEditor::Factory::Factory(parent)
{
    KYZisSession::initDebug(0, 0);
    KYZisSession::createInstance();
}

KTextEditor::Editor* KTEFactory::editor()
{
    return KTEEditor::self();
}


KParts::Part* KTEFactory::createPartObject(QWidget*, QObject*, const char*, const QStringList&)
{
    KTEDocument* doc = new KTEDocument(NULL);
    doc->setReadWrite(true);
    return doc;
}

K_EXPORT_COMPONENT_FACTORY( yzispart, KTEFactory );

#include "ktefactory.moc"
