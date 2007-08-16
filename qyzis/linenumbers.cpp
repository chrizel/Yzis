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

#include "linenumbers.h"

#include "viewwidget.h"

#define dbg()    yzDebug("QYNumberLabel")
#define err()    yzError("QYNumberLabel")

QYNumberLabel::QYNumberLabel( const QFont& f ) : QLabel()
{
    setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    setAutoFillBackground(true);
    QPalette p = palette();
    p.setColor( QPalette::Window, Qt::lightGray );
    p.setColor( QPalette::WindowText, Qt::black );
    setPalette(p);
    setFont(f);
}
QYNumberLabel::~QYNumberLabel()
{}
void QYNumberLabel::setNumber( int n )
{
    setText( ' ' + QString::number(n) + ' ' );
}
void QYNumberLabel::setFont( const QFont& f )
{
    setFixedHeight( QFontMetrics(f).lineSpacing() );
    QLabel::setFont( f );
}

QYLineNumbers::QYLineNumbers( QYView* parent )
        : QWidget( parent )
{
    m_view = parent;
    setAutoFillBackground(true);
    QPalette p = palette();
    p.setColor( QPalette::Window, Qt::lightGray );
    p.setColor( QPalette::WindowText, Qt::black );
    setPalette(p);
    rows = new QVBoxLayout( this );
    rows->setSpacing(0);
    rows->setMargin(0);
}
QYLineNumbers::~QYLineNumbers()
{}

void QYLineNumbers::setLineCount( int lines )
{
    setUpdatesEnabled(false);
    if ( rows->count() > lines ) {
        QLayoutItem* row;
        while ( (row = rows->takeAt(lines)) ) {
            delete row;
        }
    } else {
        for ( int i = rows->count(); i < lines; ++i ) {
            rows->addWidget( new QYNumberLabel( font() ) );
        }
    }
    setUpdatesEnabled(true);
}

void QYLineNumbers::setFont( const QFont& f )
{
    QWidget::setFont( f );
    for ( int i = 0; i < rows->count(); ++i ) {
        static_cast<QYNumberLabel*>(rows->itemAt(i)->widget())->setFont(f);
    }
}

void QYLineNumbers::scroll( int dy )
{
    setUpdatesEnabled(false);
    if ( dy < 0 ) {
        for ( int i = dy; i < 0; ++i ) {
            // remove top
            QWidget* w = rows->itemAt(0)->widget();
            rows->removeWidget(w);
            delete w;
            // add empty bot
            rows->addWidget( new QYNumberLabel( font() ) );
        }
    } else if ( dy > 0 ) {
        for ( int i = 0; i < dy; ++i ) {
            // remove bot
            QWidget* w = rows->itemAt(rows->count() - 1)->widget();
            rows->removeWidget(w);
            delete w;
            // add empty top
            rows->insertWidget(0, new QYNumberLabel(font()));
        }
    }
    setUpdatesEnabled(true);
}

void QYLineNumbers::setLineNumber( int y, int h, int line )
{
    QYNumberLabel* n = static_cast<QYNumberLabel*>(rows->itemAt(y)->widget());
    if ( h == 0 && line > 0 ) {
        n->setNumber( line );
    } else {
        n->clear();
    }
}

void QYLineNumbers::setMaxLineNumber( int line )
{
    setFixedWidth( fontMetrics().width( ' ' + QString::number(line) + ' ' ) );
}

