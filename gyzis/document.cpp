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

#include "document.h"
#include "gyzis.h"

Document::Document() : YZBuffer(GYzis::self) {
	set_file_extension("bakery_withdocview");
}

Document::~Document() {
}

bool Document::load_after() {
	bool bTest = Bakery::Document::load_after();

	if(bTest) {
		//See comment in save_before().
		m_strSomething = get_contents();
	} 

	return bTest; 
}

bool Document::save_before() {
	//You would ususally save your many pieces of data in some kind of file format,
	//e.g. XML.
	set_contents(m_strSomething);
	return Bakery::Document::save_before();
}

void Document::set_something(const Glib::ustring& strSomething) {
	m_strSomething = strSomething;

	set_modified();
}

Glib::ustring Document::get_something() const {
	return m_strSomething;
}
