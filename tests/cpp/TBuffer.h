/* This file is part of the Yzis libraries
 *  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
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


#ifndef TYZ_BUFFER_H
#define TYZ_BUFFER_H
/**
 * $Id: session.h 388 2004-03-03 23:44:13Z mikmak $
 */

#include "libyzis/buffer.h"


class TYZBuffer : public YZBuffer {
public:
	TYZBuffer(YZSession *sess)
    : YZBuffer(sess) {}

    virtual bool popupFileSaveAs() { return true; }
	virtual void filenameChanged() {}
	virtual void highlightingChanged() {}
};

#endif // TYZ_BUFFER_H

