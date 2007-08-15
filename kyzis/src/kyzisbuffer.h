//
// C++ Interface: kyzisbuffer
//
// Description:
//
//
// Author: Craig Howard <craig@choward.ca>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KYZISBUFFER_H
#define KYZISBUFFER_H

#include "buffer.h"

class KYZTextEditorIface;

/**
 @author Craig Howard <craig@choward.ca>
*/
class KYZisBuffer : public YZBuffer
{
public:
    KYZisBuffer();

    virtual ~KYZisBuffer();

    void setTextEditorIface( KYZTextEditorIface *iface )
    {
        m_iface = iface;
    }
    KYZTextEditorIface *getTextEditorIface()
    {
        return m_iface;
    }
    const KYZTextEditorIface *getTextEditorIface() const
    {
        return m_iface;
    }

private:
    virtual void makeActive();
    virtual void makeHidden();
    virtual void makeInactive();

    KYZTextEditorIface *m_iface;
};

#endif
