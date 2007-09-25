/* This file is part of the Yzis libraries
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
*  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
*  Boston, MA 02110-1301, USA.
**/


#include "kyzisinfobar.h"

#include <QHBoxLayout>
#include <QLabel>

KYInfoBar::KYInfoBar(QWidget* parent)
        : QWidget(parent)
{
    QHBoxLayout* l = new QHBoxLayout( this );
    m_mode = new QLabel( this );
    m_fileName = new QLabel( this );
    m_fileInfo = new QLabel( this );
    m_lineInfo = new QLabel( this );

    l->addWidget( m_mode );
    l->addWidget( m_fileInfo );
    l->addWidget( m_lineInfo );
    l->addWidget( m_fileName );
}

KYInfoBar::~KYInfoBar()
{}

void KYInfoBar::setMode( const QString& mode )
{
    m_mode->setText( mode );
}

void KYInfoBar::setFileName( const QString& fileName )
{
    m_fileName->setText( fileName );
}

void KYInfoBar::setFileInfo( const QString& fileInfo )
{
    m_fileInfo->setText( fileInfo );
}

void KYInfoBar::setLineInfo( const QString& lineInfo )
{
    m_lineInfo->setText( lineInfo );
}

#include "kyzisinfobar.moc"
