/* This file is part of the Yzis libraries
 *  Copyright (C) 2003,2004 Mickael Marchand <marchand@kde.org>,
 *  Thomas Capricelli <orzel@freehackers.org>
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

#ifndef YZIS_H
#define YZIS_H
/**
 * yzis.h
 *
 * Main include file for the yzis project
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <qstringlist.h>

#define VERSION_MAJOR	0
#define VERSION_MINOR	0
#define VERSION_PATCH	1

#define VERSION_CHAR	"M1"
#define VERSION_CHAR_LONG "Yzis Milestone 1"
#define VERSION_CHAR_DATE "Released on may, 19th, 2004"

struct yzpoint {
	int x;
	int y;
};

typedef struct yzpoint yz_point;

#endif /* YZIS_H */
