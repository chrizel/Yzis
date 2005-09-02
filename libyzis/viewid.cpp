//
// C++ Implementation: viewid
//
// Description: 
//
//
// Author: Mickael Marchand <mikmak@yzis.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "viewid.h"

#include <climits>

#include <qdatastream.h>

#include "debug.h"

const YZViewId YZViewId::invalid(UINT_MAX);

YZViewId::YZViewId()
	: myId( invalid.getNumber() )
{
}

YZViewId::YZViewId( unsigned int id )
	: myId( id )
{
}


YZViewId::~YZViewId()
{
}

YZViewId &YZViewId::operator=( const YZViewId &rhs )
{
	myId = rhs.myId;
	return *this;
}

const unsigned int YZViewId::getNumber() const
{
	return myId;
}

bool operator==(const YZViewId &lhs, const YZViewId &rhs)
{
	return lhs.getNumber() == rhs.getNumber();
}

bool operator!=(const YZViewId &lhs, const YZViewId &rhs)
{
	return lhs.getNumber() != rhs.getNumber();
}

bool operator<(const YZViewId &lhs, const YZViewId &rhs)
{
	return lhs.getNumber() < rhs.getNumber();
}

YZDebugStream &operator<<( YZDebugStream &lhs, const YZViewId &rhs ) 
{ 
	return lhs << rhs.getNumber(); 
}

QDataStream &operator>>( QDataStream &lhs, YZViewId &rhs )
{
	unsigned int id;
	rhs.myId = id;
	return lhs;
}
