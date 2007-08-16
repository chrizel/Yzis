/* This file is part of the Yzis libraries
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
#include "yzisinfo.h"
#include "debug.h"
#include "history.h"
#include "internal_options.h"
#include "mode_ex.h"
#include "buffer.h"
#include "mode_search.h"
#include "session.h"
#include "view.h"
#include "resourcemgr.h"
#include "yzisinfojumplistrecord.h"
#include "yzisinfostartpositionrecord.h"

/* System */
#include <iostream>

/* Qt */
#include <qfileinfo.h>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>

using namespace std;

class YInfoCursor;

#define dbg() yzDebug("YInfo")
#define err() yzError("YInfo")

/**
 * Constructor
 */

YInfo::YInfo()
{
    dbg() << HERE() << endl;
    mYzisinfo.setFileName( resourceMgr()->findResource( WritableConfigResource, "yzisinfo" ) );
    mYzisinfoInitialized = false;
}

/**
 * Constructor
 */

YInfo::YInfo( const QString & path )
{
    dbg() << HERE() << endl;
    mYzisinfo.setFileName( path );
}

/**
 * Destructor
 */

YInfo::~YInfo()
{
    dbg() << HERE() << endl;
}

/**
 * YZYsisinfo::readYzisinfo
 */

void YInfo::read(void)
{
    dbg() << HERE() << endl;

    if ( mYzisinfoInitialized ) {
        return ;
    }

    if ( mYzisinfo.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &mYzisinfo );
        QString line;

        while ( !stream.atEnd() ) {
            line = stream.readLine(); // line of text excluding '\n'
            line = line.trimmed();

            // Ignore empty lines

            if ( line.isEmpty() ) {
                continue;
            }

            // Ignore comment lines

            if ( line.startsWith("#") ) {
                continue;
            }

            // If we have got this far then we must have a line that is
            // a) Not a comment
            // b) Not empty
            // So split it into a list using whitespace as the separator

            QStringList list = line.split( QRegExp( "\\s" ));

            if ( list[0] == "hlsearch" ) {
                bool on = (list[1] == "on" );
                YSession::self()->getOptions()->setOptionFromString( &on, "hlsearch" );
            }

            if ( list[0].startsWith(":") || list[0] == "command_list" ) {
                YModeEx *ex = YSession::self()->getExPool();
                YZHistory *history = ex->getHistory();

                history->addEntry( (list.join(" ")).remove(0, 1) );
            }

            if ( list[0].startsWith("?") || list[0] == "search_list" ) {
                YModeSearch *search = dynamic_cast<YModeSearch*>(YSession::self()->getModes()[ YMode::ModeSearch ]);
                YZHistory *history = search->getHistory();

                history->addEntry( (list.join(" ")).remove(0, 1) );
            }

            if ( list[0].startsWith(">") || list[0] == "start_position" ) {
                mStartPosition.push_back( new YInfoStartPositionRecord( list[3], YCursor(list[1].toInt(), list[2].toInt())) );
            }

            if ( list[0].startsWith("_") || list[0] == "search_history" ) {
                mJumpList.push_back( new YInfoJumpListRecord( list[3], QPoint (list[1].toInt(), list[2].toInt())) );
                mCurrentJumpListItem++;
            }

            if ( list[0].startsWith("\"") ) {
                QChar key = list[0].at(1);

                QString line;
                QStringList contents;
                int length = list[3].toInt();

                if ( list[1] == "CHAR" ) {
                    for ( int i = 0; i < length; ++i ) {
                        contents << stream.readLine();
                    }
                } else {
                    contents << QString::null;

                    for ( int i = 0; i < length; ++i ) {
                        contents << stream.readLine();
                    }

                    contents << QString::null;
                }

                dbg() << "Key:<" << key.toAscii() << ">" << endl;
                dbg() << "Length:<" << contents.size() << ">" << endl;
                for ( int i = 0; i < contents.size(); ++i ) {
                    dbg() << "<" << contents.at(i) << ">" << endl;
                }

                YSession::self()->setRegister( key, contents );
            }
        }

        mYzisinfo.close();
    } else {
        dbg() << "Unable to open file " << mYzisinfo.fileName() << endl;
    }

    mYzisinfoInitialized = true;
}

/**
 * YInfo::updateStartPosition
 */

void YInfo::updateStartPosition( const YBuffer *buffer, const YCursor cursor)
{
    bool found = false;

    for ( StartPositionVector::Iterator it = mStartPosition.begin(); it != mStartPosition.end(); ++it ) {
        if ( (*it)->filename() == buffer->fileName() ) {
            found = true;
            mStartPosition.erase(it);
            mStartPosition.push_back( new YInfoStartPositionRecord( buffer->fileName(), cursor ) );
            return ;
        }
    }

    if ( ! found ) {
        mStartPosition.push_back( new YInfoStartPositionRecord( buffer->fileName(), cursor ) );
    }

    return ;
}

/**
 * YInfo::updateJumpList
 */

void YInfo::updateJumpList( const YBuffer *buffer, const QPoint pos )
{
    bool found = false;

    for ( JumpListVector::Iterator it = mJumpList.begin(); it != mJumpList.end(); ++it ) {
        if ( (*it)->filename() == buffer->fileName() ) {
            if ( (*it)->position() == pos) {
                found = true;
                break;
            }
        }
    }

    if ( ! found ) {
        mJumpList.push_back( new YInfoJumpListRecord( buffer->fileName(), pos ) );
    }

    return ;
}

/**
 * YInfo::write
 */

void YInfo::write()
{
    dbg() << HERE() << endl;
    if ( mYzisinfo.open( QIODevice::WriteOnly ) ) {
        QTextStream write( &mYzisinfo );
        write.setCodec( QTextCodec::codecForName("utf8"));
        // Write header

        write << "# This yzisinfo file was generated by Yzis " << VERSION_CHAR << "." << endl;
        write << "# You may edit it if you're careful!" << endl;
        write << endl;

        // Write whether hlsearch is on or off

        write << "# Set hlsearch on or off:" << endl;
        write << "hlsearch ";
        if ( YSession::self()->getBooleanOption( "hlsearch") ) {
            write << "on" << endl;
        } else {
            write << "off" << endl;
        }
        write << endl;

        write << "# Command Line History (oldest to newest):" << endl;
        saveExHistory( write );
        write << endl;

        write << "# Search String History (oldest to newest):" << endl;
        saveSearchHistory( write );
        write << endl;

        write << "# Position to start at when opening file (oldest to newest):" << endl;
        saveStartPosition( write );
        write << endl;

        write << "# Jump list (oldest to newest):" << endl;
        saveJumpList( write );
        write << endl;

        write << "# Registers:" << endl;
        saveRegistersList( write );
        write << endl;

        mYzisinfo.close();
    }
}

/**
 * YInfo::saveExHistory
 */

void YInfo::saveExHistory( QTextStream & write )
{
    dbg() << HERE() << endl;
    YZHistory *history = YSession::self()->getExPool()->getHistory();
    history->writeToStream( write );
}

/**
 * YInfo::saveSearchHistory
 */

void YInfo::saveSearchHistory( QTextStream & write )
{
    dbg() << HERE() << endl;
    YModeSearch *search = dynamic_cast<YModeSearch*>(YSession::self()->getModes()[ YMode::ModeSearch ] );
    YZHistory *history = search->getHistory();
    history->writeToStream( write );
}

/**
 * YInfo::saveStartPosition
 */

void YInfo::saveStartPosition( QTextStream & write )
{
    dbg() << HERE() << endl;

    int start = 0;
    int end = mStartPosition.count();

    if ( end > 100 ) {
        start = end - 100;
    }

    for ( int i = start; i < end; ++i ) {
        write << "> ";
        dbg() << (mStartPosition[i])->position().x();
        write << (mStartPosition[i])->position().x();
        write << " ";
        dbg() << (mStartPosition[i])->position().y();
        write << (mStartPosition[i])->position().y();
        write << " ";
        dbg() << (mStartPosition[i])->filename() << endl;
        write << (mStartPosition[i])->filename() << endl;
    }
}

/**
 * YZYsisinfo::saveJumpList
 */

void YInfo::saveJumpList( QTextStream & write )
{
    dbg() << HERE() << endl;

    int start = 0;
    int end = mJumpList.count();

    if ( end > 100 ) {
        start = end - 100;
    }

    for ( int i = start; i < end; ++i ) {
        write << "_" << " ";
        write << mJumpList[i]->position().x();
        write << " ";
        write << mJumpList[i]->position().y();
        write << " ";
        write << mJumpList[i]->filename() << endl;
    }
}

/**
 * YInfo::saveRegisters
 */

void YInfo::saveRegistersList( QTextStream & write )
{
    dbg() << HERE() << endl;

    QList<QChar> list = YSession::self()->getRegisters();

    for ( int i = 0; i < list.size(); ++i ) {
        QStringList contents = YSession::self()->getRegister( list.at(i) );

        write << "\"" << list.at(i) << " ";

        if ( contents.size() >= 3 ) {
            write << "LINE  " << contents.size() - 2 << endl;
        } else {
            write << "CHAR  " << contents.size() << endl;
        }

        for ( int j = 0; j < contents.size(); ++j ) {
            if ( ( contents.at(j) ).isNull() ) {
                continue;
            }

            write << contents.at(j) << "\n";
        }
    }
}

/**
 * YInfo::startPosition
 */

YCursor YInfo::startPosition( const QString& filename ) const
{

    for ( StartPositionVector::ConstIterator it = mStartPosition.begin(); it != mStartPosition.end(); ++it ) {
        if ( (*it)->filename() == filename ) {
            return (*it)->position();
        }
    }

    return YCursor(0, 0);
}
YCursor YInfo::startPosition( const YBuffer *buffer ) const
{
    return startPosition( buffer->fileName() );
}

/**
 * YInfo::searchPosition
 */

YCursor YInfo::searchPosition( const YBuffer * )
{

    for ( JumpListVector::Iterator it = mJumpList.begin(); it != mJumpList.end(); ++it ) {
        /*if ( (*it)->filename() == buffer->fileName() ) {
         return (*it)->previousSearchPosition();
        }*/
    }

    return YSession::self()->currentView()->getBufferCursor();
}

const YCursor YInfo::previousJumpPosition()
{

    bool found = false;
    bool repeating = false;

    while ( true ) {
        if ( mCurrentJumpListItem == 0 ) {
            // Make sure we don't end up in a endless loop
            if ( repeating ) {
                break;
            }

            repeating = true;
            mCurrentJumpListItem = mJumpList.count();
        }

        --mCurrentJumpListItem;

        if ( mJumpList[mCurrentJumpListItem]->filename() == YSession::self()->currentView()->myBuffer()->fileName() ) {
            found = true;
            break;
        }
    }

    if ( found ) {
        return mJumpList[mCurrentJumpListItem]->position();
    } else {
        return YSession::self()->currentView()->getCursor();
    }
}


/*
 * END OF FILE
 */
