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

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "bakery/bakery.h"
#include "buffer.h"

class Document : public Bakery::Document, public YZBuffer
{
	public:
		Document();
		virtual ~Document();

		//overrides:
		virtual bool load_after();
		virtual bool save_before();

		void set_something(const Glib::ustring& strSomething);
		Glib::ustring get_something() const;

		//YZBuffer
		bool popupFileSaveAs() { return true;}
		void filenameChanged() {}
		void highlightingChanged() {}


	protected:
		Glib::ustring m_strSomething;

};
#endif
