/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>,
 *  Copyright (C) 2004 Brian M. Carlson <sandals@crustytoothpaste.ath.cx>,
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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

#include "printer.h"
#ifdef HAVE_LIBPS
#include <math.h>
extern "C" {
	#include <libps/pslib.h>
	#include <libps/pslib-mp.h>
}

#include "view.h"
#include "buffer.h"
#include "session.h"
#include "debug.h"
#if QT_VERSION < 0x040000
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#else
#include <QPaintDevice>
#include <QPainter>
#endif

YZPrinter::YZPrinter( YZView *view ) /*: QPrinter(QPrinter::PrinterResolution) */{
	PS_mp_init();
	PS_boot();

	mView = view;

	/*setPageSize( QPrinter::A4 );
	setColorMode( QPrinter::Color );*/
}

YZPrinter::~YZPrinter( ) {
}

void YZPrinter::printToFile( const QString& path ) {
	/*setOutputToFile( true );
	setOutputFileName( path );*/
	m_path=path;
}

void YZPrinter::run( ) {
	doPrint( );
}

void YZPrinter::doPrint( ) {
	const double fontsize=10.0;
	PSDoc *doc=PS_new();
	if (!doc)
		return;
#if QT_VERSION < 0x040000
	PS_open_file(doc, m_path.latin1());
#else
	PS_open_file(doc, m_path.toLatin1().data());
#endif
	PS_set_info(doc, "Creator", "Yzis");
	PS_set_info(doc, "Author", "");
#if QT_VERSION < 0x040000
	PS_set_info(doc, "Title", m_path.latin1());
#else
	PS_set_info(doc, "Title", m_path.toLatin1().data());
#endif
	// Set so it'll fit on both A4 and letter paper;
	// some of us live in the US, with archaic paper sizes. ;-)
	PS_set_info(doc, "BoundingBox", "0 0 596 792");
	int font;
	font=PS_findfont(doc, "Fixed", "", 0);
	yzDebug() << "findfont returned " << font << endl;
	if ( !font ) return; //no font => abort

	QPrinter lpr(QPrinter::PrinterResolution);
	QPainter p( &lpr );

	QFont f( "fixed" );
	f.setFixedPitch( true );
	f.setStyleHint( QFont::TypeWriter );
	p.setFont( f );

#if QT_VERSION < 0x040000
	QPaintDeviceMetrics pdm( &lpr );
	unsigned int height = pdm.height();
	unsigned int width = pdm.width();
#else
	unsigned int height = lpr.height();
	unsigned int width = lpr.width();
#endif

	unsigned int linespace = p.fontMetrics().lineSpacing();
	unsigned int maxwidth = p.fontMetrics().maxWidth();

	p.end();

	PS_set_value(doc, "leading", linespace);

	unsigned int clipw = width / maxwidth - 1;
	unsigned int cliph = height / linespace - 1;

	unsigned int oldLinesVis = mView->getLinesVisible( );
	unsigned int oldColumnsVis = mView->getColumnsVisible( );

	//should be current's view setting no ? XXX
	bool number = mView->getLocalBooleanOption( "number" );
	unsigned int marginLeft = 0;

	double red, green, blue;

	if ( number ) {
		marginLeft = ( 2 + QString::number( mView->myBuffer()->lineCount() ).length() );
	}

	YZOptionValue* ov_wrap = mView->getLocalOption( "wrap" );
	bool oldWrap = ov_wrap->boolean();
	ov_wrap->setBoolean( true );

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
	PS_begin_page(doc, 596, 792);
	PS_setfont(doc, font, fontsize);
	PS_set_parameter(doc, "hyphenation", "false");
	PS_set_parameter(doc, "linebreak", "true");
	PS_set_parameter(doc, "parbreak", "false");
	PS_set_value(doc, "parindent", 0.0);
	PS_set_value(doc, "numindentlines", 0.0);

	while ( mView->drawNextLine( ) ) {
		if ( curY == topY ) {
			if ( pageNumber ) {
				PS_end_page(doc);
				PS_begin_page(doc, 596, 792);
				PS_setfont(doc, font, fontsize);
				PS_set_value(doc, "leading", linespace);
			}
			++pageNumber;
			convertColor(Qt::black, red, green, blue);
			PS_setcolor(doc, "fillstroke", "rgb", red, green, blue, 0.0);
#if QT_VERSION < 0x040000
			PS_show_boxed(doc, (" "+mView->myBuffer()->fileName()).latin1(),
#else
			PS_show_boxed(doc, (" "+mView->myBuffer()->fileName()).toLatin1().data(),
#endif
					titleRect.x(), titleRect.y(), titleRect.width(),
					titleRect.height(), "left", "");
			PS_show_boxed(doc,
#if QT_VERSION < 0x040000
					(QString::number(pageNumber)+"/"+QString::number(nbPages)+" ").latin1(),
#else
					(QString::number(pageNumber)+"/"+QString::number(nbPages)+" ").toLatin1().data(),
#endif
					titleRect.x(), titleRect.y(), titleRect.width(),
					titleRect.height(), "right", "");
		}
		if ( number ) {
			unsigned int lineNumber = mView->drawLineNumber();
			if ( lineNumber != lastLineNumber ) {
				//p.setPen( Qt::gray );
				convertColor(Qt::gray, red, green, blue);
				PS_setcolor(doc, "fillstroke", "rgb", red, green, blue, 0.0);
				PS_moveto(doc, 0, curY);
#if QT_VERSION < 0x040000
				PS_show(doc, QString::number(lineNumber).rightJustify(marginLeft-1, ' ').latin1());
#else
				PS_show(doc, QString::number(lineNumber).rightJustified(marginLeft-1, ' ').toLatin1().data());
#endif
				lastLineNumber = lineNumber;
			}
		}
		curX = marginLeft * maxwidth;
		while ( mView->drawNextCol( ) ) {
			QColor c = mView->drawColor( );
			if ( c.isValid() && c != Qt::white )
				convertColor(mView->drawColor(), red, green, blue);
			else
				convertColor(Qt::black, red, green, blue);
			PS_setcolor(doc, "fillstroke", "rgb", red, green, blue, 0.0);
			char buf[2]={0, 0};
#if QT_VERSION < 0x040000
			buf[0]=mView->drawChar().latin1();
#else
			buf[0]=mView->drawChar().toLatin1();
#endif
			PS_show_xy(doc, buf, curX, curY);
			curX += mView->drawLength( ) * maxwidth;
		}
		curY += linespace * mView->drawHeight();
		if ( curY >= cliph * linespace + topY ) {
			// draw Rect
			convertColor(Qt::black, red, green, blue);
			PS_setcolor(doc, "fillstroke", "rgb", red, green, blue, 0.0);
			PS_rect(doc, 0, 0, width, curY);
			if ( number ) {
				PS_moveto(doc, marginLeft*maxwidth-maxwidth/2, titleRect.height());
				PS_lineto(doc, marginLeft*maxwidth-maxwidth/2, curY);
			}
			PS_moveto(doc, titleRect.x(), titleRect.height());
			PS_lineto(doc, titleRect.width(), titleRect.height());
			curY = topY;
		}
	}
	if ( curY != topY ) {
		// draw Rect
		convertColor(Qt::black, red, green, blue);
		PS_setcolor(doc, "fillstroke", "rgb", red, green, blue, 0.0);
		PS_rect(doc, 0, 0, width, curY);
		if ( number ) {
			PS_moveto(doc, marginLeft*maxwidth-maxwidth/2, titleRect.height());
			PS_lineto(doc, marginLeft*maxwidth-maxwidth/2, curY);
		}
		PS_moveto(doc, titleRect.x(), titleRect.height());
		PS_lineto(doc, titleRect.width(), titleRect.height());
	}
	PS_end_page(doc);
	PS_deletefont(doc, font);
	PS_close(doc);
	PS_delete(doc);
	PS_shutdown();

	ov_wrap->setBoolean( oldWrap );
	mView->setVisibleArea( oldColumnsVis, oldLinesVis, false );
}


void YZPrinter::convertColor(const QColor& c, double &r, double &g, double &b) {
	int r0, g0, b0;

	c.getRgb(&r0, &g0, &b0);
	r=r0;
	r/=255;
	g=g0;
	g/=255;
	b=b0;
	b/=255;
}
#endif
