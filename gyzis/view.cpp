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

#include "view.h"

View::View() {
	pack_start(m_Entry, Gtk::PACK_EXPAND_WIDGET);

	//Connect signals:
	//m_Entry.signal_changed().connect(sigc::mem_fun(*this, &View::on_Entry_changed));

	show_all();
}

View::~View() {
}

void View::load_from_document() {
	//m_Entry.set_text( get_document()->get_something() );
}

void View::save_to_document() {
	/*const Glib::ustring& strText =  m_Entry.get_text();

	if (strText != get_document()->get_something()) //Don't trigger 'modified' unless it really is different.
		get_document()->set_something(strText);*/
}

void View::on_Entry_changed() {
	//We don't really need to do this, but it updates the modified status,
	//and a more complicated View might need its Document updated all the time.
	save_to_document();
}

