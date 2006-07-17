/* This file is part of the QYzis
 *  Copyright (C) 2006 Loic Pauleve <panard@inzenet.org>
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

#ifndef QYZIS_LINENUMBERS_H
#define QYZIS_LINENUMBERS_H

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class QYZisView;

class LineNumber : public QLabel {
	Q_OBJECT

	public :
		LineNumber( const QFont& f );
		virtual ~LineNumber();

		void setNumber( int n );
		void setFont( const QFont& );
};


class QYZisLineNumbers : public QWidget {
	Q_OBJECT

	public :
		QYZisLineNumbers( QYZisView* parent );
		virtual ~QYZisLineNumbers();

		void setLineNumber( int y, int h, int line );
		void setMaxLineNumber( int line );

		void setFont( const QFont& f );
		void scroll( int dy );

		void setLineCount( int lines );
	
	protected :
		QVBoxLayout* rows;

	private :

		QYZisView* m_view;

};

#endif
