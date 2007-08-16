/* This file is part of the Yzis libraries
*  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>
*  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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

#include "portability.h"
#ifdef HAVE_LIBPS
#ifndef YZ_PRINTER_H
#define YZ_PRINTER_H

#include <qprinter.h>
#include <qcolor.h>

class YView;

class YZPrinter /*: public QPrinter*/
{

public:
    YZPrinter( YView *view );
    virtual ~YZPrinter( );

    void printToFile( const QString& path );
    void run( );

private:
    /* methods */
    void doPrint( );
    void convertColor(const QColor& c, double &r, double &g, double &b);

    /* members */
    YView *mView;
    QString m_path;

};

#endif
#endif
