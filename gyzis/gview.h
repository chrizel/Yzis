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

#ifndef GVIEW_H
#define GVIEW_H

#include <gtkmm/box.h>
#include "document.h"

class View:
	public Gtk::VBox,
	public Bakery::View<Document> {

		public:
			View();
			virtual ~View();

			//overrrides:
			virtual void load_from_document();
			virtual void save_to_document();

		protected:

			//Signal handlers:
			virtual void on_Entry_changed();

			//Child widgets:
//			Gtk::Entry m_Entry;
			Gtk::TextView m_Entry;
	};

#endif
