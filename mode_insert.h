/*  This file is part of the Yzis libraries
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
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

/**
 * $Id$
 */

#ifndef YZ_MODE_INSERT_H
#define YZ_MODE_INSERT_H

#include "mode.h"
#include "view.h"

class YZMode;
class YZView;

class YZModeInsert : public YZMode {
	public:
		YZModeInsert();
		virtual ~YZModeInsert() {}

		virtual void leave( YZView* mView );
		virtual void initModifierKeys();
		virtual cmd_state execCommand( YZView* mView, const QString& key );

		virtual cmd_state commandDefault( YZView* mView, const QString& key );
		virtual void commandHome( YZView* mView, const QString& key );
		virtual void commandEnd( YZView* mView, const QString& key );
		virtual void commandEscape( YZView* mView, const QString& key );
		virtual void commandInsert( YZView* mView, const QString& key );
		virtual void commandEx( YZView* mView, const QString& key );
		virtual void commandVisual( YZView* mView, const QString& key );
		virtual void commandInsertFromBelow( YZView* mView, const QString& key );
		virtual void commandInsertFromAbove( YZView* mView, const QString& key );
		virtual void commandCompletion( YZView* mView, const QString& key );
		virtual void commandCompletionPrevious( YZView* mView, const QString& key );
		virtual void commandCompletionNext( YZView* mView, const QString& key );
		virtual void commandDown( YZView* mView, const QString& key );
		virtual void commandUp( YZView* mView, const QString& key );
		virtual void commandLeft( YZView* mView, const QString& key );
		virtual void commandRight( YZView* mView, const QString& key );
		virtual void commandPageUp( YZView* mView, const QString& key );
		virtual void commandPageDown( YZView* mView, const QString& key );
		virtual void commandBackspace( YZView* mView, const QString& key );
		virtual void commandDel( YZView* mView, const QString& key );
		virtual void commandEnter( YZView* mView, const QString& key );
};

class YZModeReplace : public YZModeInsert {
	public :
		YZModeReplace();
		virtual ~YZModeReplace() {}
		
		virtual cmd_state commandDefault( YZView* mView, const QString& key );
		virtual void commandInsert( YZView* mView, const QString& key );
		virtual void commandBackspace( YZView* mView, const QString& key );
};

class YZModeCompletion : public YZMode {
	public :
		YZModeCompletion();
		virtual ~YZModeCompletion();

		virtual void leave( YZView* mView );
		virtual cmd_state execCommand( YZView* mView, const QString& _key );

	protected :
		QString doComplete( YZView* mView, bool forward );
		bool initCompletion( YZView* mView );
};

#endif

