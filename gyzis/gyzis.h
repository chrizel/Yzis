/* This file is part of the Yzis Project
 *  Copyright (C) 2004 Mickael Marchand <marchand@kde.org>
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

#ifndef GYZIS_H
#define GYZIS_H

#include "bakery/bakery.h"
#include "document.h"
#include "view.h"

//Inherit from Bakery::App_WithDoc for Document support.
class GYzis : public Bakery::App_WithDoc_Gtk
{
public:
  GYzis();
  virtual ~GYzis();

  virtual void init();

protected:
  virtual void init_create_document();
  virtual App* new_instance();

  View m_View;
};

#endif
