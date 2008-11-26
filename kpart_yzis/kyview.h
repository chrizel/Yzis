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
*  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
*  Boston, MA 02110-1301, USA.
**/

#ifndef _KYZIS_VIEW_H_
#define _KYZIS_VIEW_H_

#include <libyzis/buffer.h>
#include <qyzis/qyview.h>

class KYView : public QYView
{
    Q_OBJECT
public:
    KYView( YBuffer* doc, QWidget* parent );
    virtual ~KYView();
};

#endif
