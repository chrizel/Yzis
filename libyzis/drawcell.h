/*  This file is part of the Yzis libraries
*  Copyright (C) 2008 Loic Pauleve <panard@inzenet.org>
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

#ifndef DRAWCELL_H
#define DRAWCELL_H

#include "portability.h"

/* Qt */
#include <QList>
#include <QSet>

/* Yzis */
#include "yzis.h"
#include "color.h"
#include "font.h"

class YZIS_EXPORT YDrawCell
{
public:
	YDrawCell();
	YDrawCell( const YDrawCell& cell );
	~YDrawCell();

	/* change properties for the whole cell */
	void addSelection( yzis::SelectionType selType );
	void delSelection( yzis::SelectionType selType );
	void setForegroundColor( const YColor& color );
	void setBackgroundColor( const YColor& color );
	void setFont( const YFont& font );
	void clear();
	
	int step( const QString& data );

	/* properties accessors */
	inline bool hasSelection( yzis::SelectionType selType ) const { return mSelections & selType; }
	inline YColor backgroundColor() const { return mColorBackground; }
	inline YColor foregroundColor() const { return mColorForeground; }
	inline YFont font() const { return mFont; }
	inline QString content() const { return mContent; };
	inline int width() const { return mContent.length(); };
	int widthForLength( int length ) const;
	int lengthForWidth( int width ) const;
	inline int length() const { return mSteps.count(); }

	/* steps (buffer <-> draw) */
	inline const QList<int> steps() const { return mSteps; }
	inline int stepsShift() const { return mStepsShift; }

	/* splitters */
	YDrawCell left( int column ) const;
	YDrawCell mid( int column ) const;
	YDrawCell left_steps( int steps ) const;
	YDrawCell mid_steps( int steps ) const;

    YZIS_DUMMY_COMPARISON_OPERATOR(YDrawCell)

private:
	int mSelections;
	YColor mColorForeground;
	YColor mColorBackground;
	YFont mFont;
	QString mContent;
	QList<int> mSteps;
	int mStepsShift;
};

YZIS_DUMMY_QHASH_FUNCTION(YDrawCell)

#endif
