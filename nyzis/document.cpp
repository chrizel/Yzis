/*
	  Copyright (c) 2004-2005 Mickael Marchand <marchand@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/**
 * $Id$
 */
#include <qglobal.h>
#if QT_VERSION < 0x040000
#include <qfile.h> // this is needed so that strange things don't happen which I don't understand
#else
#include <QFile>
#endif
#include "document.h"
#include "factory.h"

NYZisDoc::NYZisDoc (void) : YZBuffer(NYZFactory::self) {
}

NYZisDoc::~NYZisDoc () {
}

bool NYZisDoc::popupFileSaveAs() {
	// TODO
#if QT_VERSION < 0x040000
	for (  YZView *it = mViews.first(); it; it = mViews.next() )
		it->displayInfo ( "Save as not implemented yet, use :w<filename>" );
#else
	for ( int ab = 0 ; ab < mViews.size(); ++ab )
		mViews.at(ab)->displayInfo ( "Save as not implemented yet, use :w<filename>");
#endif
	return false;
}

void NYZisDoc::filenameChanged()
{
#if QT_VERSION < 0x040000
	for (  YZView *it = mViews.first(); it; it = mViews.next() )
		it->displayInfo ( QString("\"%1\" %2L, %3C" ).arg(fileName()).arg(lineCount()).arg(getWholeTextLength()));
#else
	for ( int ab = 0 ; ab < mViews.size(); ++ab )
		mViews.at(ab)->displayInfo ( QString("\"%1\" %2L, %3C" ).arg(fileName()).arg(lineCount()).arg(getWholeTextLength()));
#endif

}

