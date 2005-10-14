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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "gyzis.h"

GYzis *GYzis::self = 0;

GYzis::GYzis(const char *name) : Bakery::App_WithDoc_Gtk("WithDocView"), YZSession( name ) {
	if ( self ) {
		exitRequest( 1 );
	}
	self = this;
}

GYzis::~GYzis() {
	self = 0;
}

void GYzis::init() {
  type_vecStrings vecAuthors;
  vecAuthors.push_back("Mickael Marchand <marchand@kde.org>");
  set_about_information("M3", vecAuthors, "(C) 2004 Mickael Marchand <marchand@kde.org>", gettext("GNOME GUI of the Yzis editor."));

  Bakery::App_WithDoc_Gtk::init();

  add(m_View);
}

Bakery::App* GYzis::new_instance() {
  GYzis* pApp = new GYzis("test");
  return pApp;
}

void GYzis::init_create_document() {
  if(!m_pDocument)
  {
    m_pDocument = new Document();

    m_pDocument->set_view(&m_View);

    m_View.set_document(static_cast<Document*>(m_pDocument));
  }

  Bakery::App_WithDoc_Gtk::init_create_document(); //Sets window title. Doesn't recreate doc.
}

