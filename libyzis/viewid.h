//
// C++ Interface: viewid
//
// Description: 
//
//
// Author: Craig Howard <craig@choward.ca>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef VIEWID_H
#define VIEWID_H

class YZDebugStream;
class QDataStream;

/**
	@author Craig Howard <craig@choward.ca>
*/
class YZViewId {
public:
	explicit YZViewId();
	explicit YZViewId( unsigned int id );
	~YZViewId();
	
	const unsigned int getNumber() const;
	
	YZViewId &operator=( const YZViewId &rhs );
	
	static const YZViewId invalid;

private:
	unsigned int myId;
	
	friend QDataStream &operator>>( QDataStream &lhs, YZViewId &rhs );
};

extern bool operator==( const YZViewId &lhs, const YZViewId &rhs );
extern bool operator!=( const YZViewId &lhs, const YZViewId &rhs );
extern bool operator<( const YZViewId &lhs, const YZViewId &rhs );

extern QDataStream &operator>>( QDataStream &lhs, YZViewId &rhs );
extern YZDebugStream &operator<<( YZDebugStream &lhs, const YZViewId &rhs );

#endif
