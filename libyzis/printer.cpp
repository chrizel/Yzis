/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>
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

#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <math.h>

#include "printer.h"

#include "debug.h"

YZPrinter::YZPrinter( YZView *view ) : QPrinter(QPrinter::PrinterResolution) {

	mView = view;

	setPageSize( QPrinter::A4 );
	setColorMode( QPrinter::Color );
}

YZPrinter::~YZPrinter( ) {
}

void YZPrinter::printToFile( const QString& path ) {
	setOutputToFile( true );
	setOutputFileName( path );
}

void YZPrinter::run( ) {
	doPrint( );
}

void YZPrinter::doPrint( ) {
	QPainter p( this );

	QFont f( "fixed" );
	f.setFixedPitch( true );
	f.setStyleHint( QFont::TypeWriter );
	p.setFont( f );

	QPaintDeviceMetrics pdm( this );
	unsigned int height = pdm.height();
	unsigned int width = pdm.width();


	unsigned int linespace = p.fontMetrics().lineSpacing();
	unsigned int maxwidth = p.fontMetrics().maxWidth();

	unsigned int clipw = width / maxwidth - 1;
	unsigned int cliph = height / linespace - 1;
	
	unsigned int oldLinesVis = mView->getLinesVisible( );
	unsigned int oldColumnsVis = mView->getColumnsVisible( );

	bool number = YZSession::getBoolOption( "General\\number" );
	unsigned int marginLeft = 0;
	if ( number ) {
		marginLeft = ( 2 + QString::number( mView->myBuffer()->lineCount() ).length() );
	}

	bool oldWrap = YZSession::getBoolOption( "General\\wrap" );
	YZSession::mOptions.setGroup("General");
	YZSession::setBoolOption( "wrap", true );
	mView->setVisibleArea( clipw - marginLeft, cliph, false );
	unsigned int totalHeight = mView->drawTotalHeight();
	mView->setVisibleArea( clipw - marginLeft, totalHeight, false );
	mView->initDraw( 0, 0, 0, 0 );

	unsigned int lastLineNumber = 0;
	unsigned int pageNumber = 0;

	QRect titleRect( 0, 0, width, linespace + linespace / 2 );

	unsigned int topY = titleRect.height() + linespace;
	unsigned int curY = topY;
	unsigned int curX;

	cliph = ( height - topY ) / linespace;
	int nbPages = totalHeight / cliph + ( totalHeight % cliph ? 1 : 0 );

	while ( mView->drawNextLine( ) ) {
		if ( curY == topY ) {
			if ( pageNumber ) newPage( );
			++pageNumber;
			p.setPen( Qt::black );
			p.drawText( titleRect, Qt::AlignLeft | Qt::AlignVCenter, " " + mView->myBuffer()->fileName() );
			p.drawText( titleRect, Qt::AlignRight | Qt::AlignVCenter, QString::number( pageNumber ) + "/" + QString::number( nbPages ) + " " );
		}
		if ( number ) {
			unsigned int lineNumber = mView->drawLineNumber();
			if ( lineNumber != lastLineNumber ) {
				p.setPen( Qt::gray );
				p.drawText( 0, curY, QString::number( lineNumber ).rightJustify( marginLeft - 1, ' ' ) );
				lastLineNumber = lineNumber;
			}
		}
		curX = marginLeft * maxwidth;
		while ( mView->drawNextCol( ) ) {
			QColor c = mView->drawColor( );
			if ( c.isValid() && c != Qt::white ) p.setPen( mView->drawColor( ) );
			else p.setPen( Qt::black );
			p.drawText( curX, curY, mView->drawChar( ) );
			curX += mView->drawLength( ) * maxwidth;
		}
		curY += linespace * mView->drawHeight();
		if ( curY >= cliph * linespace + topY ) {
			// draw Rect
			p.setPen( Qt::black );
			p.drawRect( 0, 0, width, curY );
			if ( number )
				p.drawLine( marginLeft * maxwidth - maxwidth / 2, titleRect.height(), marginLeft * maxwidth - maxwidth / 2, curY );
			p.drawLine( titleRect.x(), titleRect.height(), titleRect.width(), titleRect.height() );
			curY = topY;
		}
	}
	if ( curY != topY ) {
		// draw Rect
		p.setPen( Qt::black );
		p.drawRect( 0, 0, width, curY );
		if ( number )
			p.drawLine( marginLeft * maxwidth - maxwidth / 2, titleRect.height(), marginLeft * maxwidth - maxwidth / 2, curY );
		p.drawLine( titleRect.x(), titleRect.height(), titleRect.width(), titleRect.height() );
	}

	p.end( );

	YZSession::mOptions.setGroup("General");
	YZSession::setBoolOption( "wrap", oldWrap );
	mView->setVisibleArea( oldColumnsVis, oldLinesVis, false );
}

