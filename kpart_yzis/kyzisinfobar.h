/* This file is part of the Yzis libraries
 *  Copyright (C) 2007 Lothar Braun <lothar@lobraun.de>
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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef _KYZIS_STATUS_H_
#define _KYZIS_STATUS_H_

#include <QWidget>

class QLabel;
class QString;

class KYZisInfoBar : public QWidget
{
Q_OBJECT
public:
	KYZisInfoBar( QWidget* );
	~KYZisInfoBar();

	void setMode( const QString& );
	void setFileName( const QString& );
	void setFileInfo( const QString& );
	void setLineInfo( const QString& );
private:
	QLabel* m_mode;
	QLabel* m_fileName;
	QLabel* m_fileInfo;
	QLabel* m_lineInfo;
};

#endif
