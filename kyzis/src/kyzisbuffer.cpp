//
// C++ Implementation: kyzisbuffer
//
// Description:
//
//
// Author: Craig Howard <craig@choward.ca>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "kyzisbuffer.h"

#include "factory.h"
#include "ktexteditoriface.h"

KYZisBuffer::KYZisBuffer()
        : YZBuffer()
{
    m_iface = 0;
}


KYZisBuffer::~KYZisBuffer()
{
    delete m_iface;
}

void KYZisBuffer::makeActive()
{
    if ( !m_iface ) {
        m_iface = static_cast<KYZisFactory*>(YZSession::me)->createTextEditorIface();
        m_iface->setBuffer( this );
        setTextEditorIface( m_iface );
    }
}

void KYZisBuffer::makeInactive()
{
    delete m_iface;
    m_iface = 0;
}

void KYZisBuffer::makeHidden()
{
    delete m_iface;
    m_iface = 0;
}
