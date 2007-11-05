/*  This file is part of the Yzis libraries
*  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>,
*  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
*  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
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

/* Yzis */
#include "search.h"
#include "portability.h"
#include "debug.h"
#include "action.h"
#include "view.h"
#include "session.h"
#include "buffer.h"
#include "selection.h"

/* Qt */
#include <QList>

#define dbg()    yzDebug("YSearch")
#define err()    yzError("YSearch")

struct YSearch::Private
{
    void setCurrentSearch( const QString& pattern );
    YCursor doSearch( YBuffer *buffer, const YCursor from, const QString& pattern, bool reverse, bool skipline, bool* found );
    void highlightSearch( YView *view, YSelectionMap searchMap );
    bool active();

    QString mCurrentSearch;
};

YSearch::YSearch()
        : d(new Private)
{
    d->mCurrentSearch = QString();
}

YSearch::~YSearch()
{
    delete d;
}

YCursor YSearch::forward( YBuffer *buffer, const QString& pattern, bool* found, const YCursor from )
{
    YCursor tmp = d->doSearch( buffer, from, pattern, false, false, found );
    YSession::self()->saveJumpPosition( tmp );

    return tmp;
}

YCursor YSearch::backward( YBuffer *buffer, const QString& pattern, bool* found, const YCursor from )
{
    YCursor tmp = d->doSearch( buffer, from, pattern, true, false, found );
    YSession::self()->saveJumpPosition( tmp );

    return tmp;
}

YCursor YSearch::replayForward( YBuffer *buffer, bool* found, const YCursor from, bool skipline /*=false*/)
{
    return d->doSearch( buffer, from, d->mCurrentSearch, false, skipline, found );
}
YCursor YSearch::replayBackward( YBuffer *buffer, bool* found, const YCursor from, bool skipline /*=false*/)
{
    return d->doSearch( buffer, from, d->mCurrentSearch, true, skipline, found );
}

const QString& YSearch::currentSearch() const
{
    return d->mCurrentSearch;
}

bool YSearch::active()
{
    return d->active();
}

bool YSearch::Private::active()
{
    return ! ( mCurrentSearch.isNull() || mCurrentSearch.isEmpty() );
}

YCursor YSearch::Private::doSearch( YBuffer *buffer, const YCursor from, const QString& pattern, bool reverse, bool skipline, bool* found )
{
    dbg() << "YSearch::doSearch " << pattern << ", " << reverse << ", " << endl;
    *found = false;
    setCurrentSearch( pattern );
    int direction = reverse ? 0 : 1;

    YCursor cur( from ); // there used to be a null check here, but passing in a null cursor doesn't make sense

    if ( ! active() ) return cur;

    if ( skipline ) {
        cur.setX( 0 );
        if ( ! reverse ) cur.setY( qMin( (int)(cur.y() + 1), (int)(buffer->lineCount() - 1) ) );
    } else {
        cur.setX( qMax( (int)(cur.x() + direction), 0 ) );
    }

    // define absolute ranges for the buffer
    YCursor top( 0, 0 );
    YCursor bottom;
    bottom.setY( buffer->lineCount() - 1 );
    bottom.setX( qMax( (int)(buffer->textline( bottom.y() ).length() - 1), 0 ) );

    // cursor for the end of the search range
    YCursor end( bottom );
    if ( reverse ) end = top;

    // use an action to do the search
    int matchedLength;
    // dbg() << "begin = " << cur << endl;
    YCursor ret = buffer->action()->search( buffer, pattern, cur, end, &matchedLength, found );

    // check to see if we need to wrap
    if ( ! *found ) {
        dbg() << "search hits top or bottom" << endl;

        // adjust the cursors for the next search
        end = cur;
        if ( reverse ) {
            cur = bottom;
        } else {
            cur = top;
        }
        //  dbg() << "begin = " << cur << ", end = " << end << endl;

        // repeat the search with the new bounds
        ret = buffer->action()->search( buffer, pattern, cur, end, &matchedLength, found );
        if ( *found ) {
            YView *view = YSession::self()->findViewByBuffer( buffer );
            if ( view && reverse ) {
                view->displayInfo(_("search hit TOP, continuing at BOTTOM"));
            } else if ( view ) {
                view->displayInfo(_("search hit BOTTOM, continuing at TOP"));
            }
        }
    }
    // dbg() << "ret = " << ret << endl;

    return ret;
}

void YSearch::Private::setCurrentSearch( const QString& pattern )
{
    if ( mCurrentSearch == pattern ) return ;
    mCurrentSearch = pattern;

    YSelectionMap searchMap;
    foreach( YBuffer *b, YSession::self()->buffers() ) {
        QList<YView*> views = b->views();

        searchMap.clear();

        /** search all **/
        bool doIt = YSession::self()->getBooleanOption( "hlsearch" );
        if ( doIt ) {
            YView* v = views.front();
            YCursor from( 0, 0 );
            YCursor cur( from );
            YCursor end;
            end.setY( b->lineCount() - 1 );
            end.setX( qMax( (int)(b->textline( end.y() ).length() - 1), 0 ) );

            bool found = true;
            int matchedLength = 0;
            int pos = 0;
            do {
                from = b->action()->search( v->myBuffer(), mCurrentSearch, cur, end, &matchedLength, &found );
                if ( found && matchedLength > 0 ) {
                    cur = from;
                    cur.setX( cur.x() + matchedLength - 1 );
                    YInterval sel( from, cur );
                    cur.setX( cur.x() + 1 );
                    searchMap.insert( pos++, sel );
                }
            } while ( found );
        }

        foreach( YView *view, views )
        highlightSearch( view, searchMap );
    }
}

void YSearch::highlightLine( YBuffer* buffer, int line )
{
    if ( d->mCurrentSearch.isNull() || d->mCurrentSearch.isEmpty() ) return ;
    bool doIt = YSession::self()->getBooleanOption( "hlsearch" );
    if ( doIt ) {
        QList<YView*> views = buffer->views();
        YView* v = views.front();
        YCursor from( 0, line );
        YCursor cur( from );
        YCursor end( buffer->textline( line ).length(), line );

        YSelection* searchMap = v->getSelectionPool()->search();
        searchMap->delInterval( YInterval( from, end ) );

        if ( end.x() > 0 ) end.setX( end.x() - 1 );

        bool found;
        int matchedLength = 0;
        do {
            from = buffer->action()->search( v->myBuffer(), d->mCurrentSearch, cur, end, &matchedLength, &found );
            if ( found && matchedLength > 0 ) {
                cur = from;
                cur.setX( cur.x() + matchedLength - 1 );
                searchMap->addInterval( YInterval( from, cur ) );
                cur.setX( cur.x() + 1 );
                //    dbg() << "cur = " << cur << "; end = " << end << endl;
            }
        } while ( found );

        foreach( YView *view, views ) {
            view->getSelectionPool()->setSearch( searchMap );
            view->sendPaintEvent( 0, line, qMax( (int)(buffer->textline( line ).length() - 1), 0 ), line );
        }
    }
}

void YSearch::shiftHighlight( YBuffer* buffer, int fromLine, int shift )
{
    QList<YView*> views = buffer->views();
    if ( views.isEmpty() ) // no views
        return ;
    YView* v = views.front();
    if ( v ) {
        YSelectionMap searchMap = v->getSelectionPool()->search()->map();

        if ( shift + fromLine < 0 ) fromLine = -shift;
        int size = searchMap.size();
        for ( int i = 0; i < size; i++ ) {
            YCursor to = searchMap[ i ].toPos();
            if ( to.y() < fromLine ) continue;

            YCursor from = searchMap[ i ].fromPos();
            from.setY( from.y() + shift);
            to.setY( to.y() + shift);

            searchMap[ i ].setFromPos( from );
            searchMap[ i ].setToPos( to );
        }

        foreach( YView *view, views )
        d->highlightSearch( view, searchMap );
    }
}

void YSearch::Private::highlightSearch( YView *view, YSelectionMap searchMap )
{
    view->setPaintAutoCommit( false );
    YSelection* vMap = view->getSelectionPool()->search();
    view->sendPaintEvent( vMap->map(), false );
    vMap->clear();
    if ( YSession::self()->getBooleanOption( "hlsearch" ) ) {
        vMap->setMap( searchMap );
        //  dbg() << "new search Map : " << *(vMap) << endl;
        view->sendPaintEvent( vMap->map() );
    }
    view->commitPaintEvent();
}

void YSearch::update()
{
    if ( ! active() ) return ;
    if ( YSession::self()->getBooleanOption( "hlsearch" ) ) {
        // force creating hl selection
        QString pattern = d->mCurrentSearch;
        d->mCurrentSearch = "";
        d->setCurrentSearch( pattern );
    } else {
        YSelectionMap searchMap;
        // clear current hl search selection
        foreach( YBuffer *b, YSession::self()->buffers())
        foreach( YView *view, b->views() )
        d->highlightSearch( view, searchMap );
    }
}

