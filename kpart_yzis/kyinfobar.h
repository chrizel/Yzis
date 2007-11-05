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

#ifndef _KY_INFOBAR_H_
#define _KY_INFOBAR_H_

#include "libyzis/statusbariface.h"

#include <QWidget>

class QLabel;
class QString;

/** Yzis KPart status bar implementation */
class KYInfoBar : public QWidget, public YStatusBarIface
{
    Q_OBJECT
public:
    /** Initialize the status bar */
    KYInfoBar( QWidget *parent );
    virtual ~KYInfoBar();

    /** Displays current mode */
    virtual void setMode( const QString& mode );
    /** Displays current file name */
    virtual void setFileName( const QString& filename );
    /** Displays current file status information */
    virtual void setFileInfo( bool isNew, bool isModified );
    /** Displays current position within the buffer */
    virtual void setLineInfo( int bufferLine, int bufferColumn, int screenColumn, QString percentage );
    /** Displays an informational message */
    virtual void setMessage( const QString& message );

private:
    /** Area where current mode is shown */
    QLabel* m_mode;
    /** Area where file name is shown */
    QLabel* m_fileName;
    /** Area where file status information is shown */
    QLabel* m_fileInfo;
    /** Area where view's position withing the buffer is shown */
    QLabel* m_lineInfo;
    /** Area where informational messages are shown */
    QLabel* m_infoMessage;
};

#endif
