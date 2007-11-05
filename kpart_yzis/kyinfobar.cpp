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


#include "kyinfobar.h"

#include "libyzis/debug.h"
#define dbg() yzDebug("KYInfoBar")
#define err() yzError("KYInfoBar")
#define deepdbg() yzDeepDebug("KYInfoBar")

#include <QHBoxLayout>
#include <QLabel>

KYInfoBar::KYInfoBar(QWidget* parent)
        : QWidget(parent)
{
    dbg() << QString("KYInfoBar( %1 )").arg(qp(parent->objectName())) << endl;
    QHBoxLayout* l = new QHBoxLayout( this );
    m_mode = new QLabel( this );
    m_fileName = new QLabel( this );
    m_fileInfo = new QLabel( this );
    m_lineInfo = new QLabel( this );
    m_infoMessage = new QLabel( this );

    l->addWidget( m_mode, 1 );
    l->addWidget( m_fileName, 1 );
    l->addWidget( m_fileInfo, 1 );
    l->addWidget( m_infoMessage, 100 );
    l->addWidget( m_lineInfo, 1 );
}

KYInfoBar::~KYInfoBar()
{
    dbg() << "~KYInfoBar()" << endl;
}

void KYInfoBar::setMode( const QString& mode )
{
    deepdbg() << "setMode( " << mode << " )" << endl;
    m_mode->setText( mode );
}

void KYInfoBar::setFileName( const QString& fileName )
{
    deepdbg() << "setFileName( " << filename << " )" << endl;
    m_fileName->setText( fileName );
}

void KYInfoBar::setFileInfo( bool isNew, bool isModified )
{
    deepdbg() << QString("setFileInfo( isNew=%1, isModified=%2 )").arg(isNew).arg(isModified) << endl;
    QString fileInfo;

    fileInfo += isNew ? 'N' : ' ';
    fileInfo += isModified ? 'M' : ' ';

    m_fileInfo->setText( fileInfo );
}

void KYInfoBar::setLineInfo( int bufferLine, int bufferColumn, int screenColumn, QString percentage )
{
    deepdbg() << QString("setLineInfo( %1, %2, %3, %4 )")
                 .arg(bufferLine)
                 .arg(bufferColumn)
                 .arg(screenColumn)
                 .arg(percentage)
              << endl;

    bool isNumber;
    percentage.toInt(&isNumber);
    if(isNumber)
        percentage += '%';

    if (bufferColumn != screenColumn)
        m_lineInfo->setText( QString("%1,%2-%3 (%4)")
                   .arg( bufferLine )
                   .arg( bufferColumn )
                   .arg( screenColumn )
                   .arg( percentage ) );
    else
        m_lineInfo->setText( QString("%1,%2 (%3)")
                   .arg( bufferLine )
                   .arg( bufferColumn )
                   .arg( percentage ) );
}

void KYInfoBar::setMessage( const QString& message )
{
    deepdbg() << "setMessage( " << message << " )" << endl;
    m_infoMessage->setText( message );
}

#include "kyinfobar.moc"
