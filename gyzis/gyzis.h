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
#include "gview.h"
#include "session.h"

class GYzis : public Bakery::App_WithDoc_Gtk, public YZSession
{
	public:
		static GYzis *self;

		GYzis(const char *);
		virtual ~GYzis();

		virtual void init();

		//YZSession
		void changeCurrentView( YZView* ) {}
		void deleteView ( int Id = -1 ) {}
		void deleteBuffer( YZBuffer *b ) {}
		void quit(int errorCode=0) {}
		void popupMessage( const QString& message ) {}
		bool promptYesNo(const QString& title, const QString& message) {}
		YZBuffer *createBuffer(const QString& path=QString::null) {}
		YZView* createView ( YZBuffer* ) {}
		void setFocusCommandLine() {}
		void setFocusMainWindow() {}



	protected:
		virtual void init_create_document();
		virtual App* new_instance();

		View m_View;
};

#endif
