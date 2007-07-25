/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
 *  Copyright (C) 2003-2004 Philippe Fremy <phil@freehackers.org>
 *  Copyright (C) 2003-2004 Pascal "Poizon" Maillard <poizon@gmx.at>
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
 *  Copyright (C) 2005 Erlend Hamberg <hamberg@stud.ntnu.no>
 *  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
 *  Copyright (C) 2005 Craig Howard	<craig@choward.ca>
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
#include "tags_interface.h"
#include "internal_options.h"
#include "readtags/readtags.h"
#include "session.h"
#include "debug.h"
#include "view.h"
#include "buffer.h"
#include "tags_stack.h"
#include "yzisinfojumplistrecord.h"

/* System */
#include <assert.h>

/* Qt */
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QList>
#include <QVector>

static QList<tagFile*> tagfilelist;
static QList<QString> tagfilenames;
// lastsearch is needed, since readtags does a pointer assignment to remember
// the last search.  Otherwise, the temporary gets destroyed and tagNext fails.
static QString lastsearch;

static bool tagFileAlreadyOpen( const QString& filename ) {
	return tagfilenames.indexOf( filename ) != -1;
}

static tagFile* doOpenTagFile( QString &filename ) {
	tagFile *tagfile = NULL;
	YZASSERT_MSG( !tagFileAlreadyOpen( filename ), "Tried to open the tag file again" );
	
	// first, if the filename starts with ./, replace the dot with
	// the current buffer's path
	if ( filename.startsWith( QString(".") + QDir::separator() ) ) {
		QFileInfo file( YZSession::self()->currentView()->myBuffer()->fileName() );
		filename.replace( 0, 1, file.absoluteDir().absolutePath() );
	}
	
	QFileInfo tagfilename( filename );
	bool found = tagfilename.exists();
	
	// if we found a tag file, open it
	if ( found ) {
		tagFileInfo info;
		tagfile = tagsOpen( filename.toUtf8().data(), &info );
	}

	return tagfile;
}

static bool openTagFile() {
	QStringList tagsOption = YZSession::self()->getOptions()->readListOption("tags", QStringList("tags"));
	bool foundATagFile = false;
	
	for( int i = 0; i < tagsOption.size(); ++i ) {
		tagFile *tagfile = doOpenTagFile( tagsOption[i] );
		if ( tagfile ) {
			foundATagFile = true;
			tagfilelist.push_back( tagfile );
			tagfilenames.push_back( tagsOption[i] );
		}
	}
	
	return foundATagFile;
}

static void closeTagFile() {
	YZASSERT_MSG( tagfilelist.size() > 0, "Tried to close an already closed tag file");
	
	for( int i = 0; i < tagfilelist.size(); ++i ) {
		tagsClose( tagfilelist[i] );
	}
	
	tagfilelist.clear();
	tagfilenames.clear();
}

static void switchToViewOfFilename( const QString &filename )
{
	YZBuffer *buffer = YZSession::self()->findBuffer( filename );
	YZView *view = YZSession::self()->findViewByBuffer( buffer );
	
	if ( !buffer && !view ) {
		view = YZSession::self()->createBufferAndView( filename );
	} else if ( !view ) {
		view = YZSession::self()->createView( buffer );
	}
	
	YZSession::self()->setCurrentView( view );
}

static void doJumpToTag ( const YZTagStackItem &entry ) {
	YZBuffer * b = YZSession::self()->currentView()->myBuffer();

	QFileInfo file( entry.filename );
	QString filepath = file.absoluteFilePath();
	QString pattern = entry.pattern;
	
	// if the tag is in a different file, we have to change buffers
	if ( filepath != YZSession::self()->currentView()->myBuffer()->fileName() ) {
		switchToViewOfFilename( filepath );
	}

	pattern = pattern.mid( 2, pattern.length() - 4 );
	yzDebug("doJumpToTag") << "mid = " << pattern << endl;
	pattern = pattern.replace("\\", "");
	pattern = pattern.replace("(", "\\(");
	pattern = pattern.replace(")", "\\)");
	pattern = pattern.replace("{", "\\{");
	pattern = pattern.replace("}", "\\}");
	pattern = pattern.replace("*", "\\*");
	pattern = pattern.replace("/", "\\/");
	yzDebug("doJumpToTag") << "After escaping = " << pattern << endl;
	QRegExp rx(pattern);
	
	int lineCount = static_cast<int>( b->lineCount() );
	
	for( int i = 0; i < lineCount; i++ )
	{
		int pos = rx.indexIn(b->textline(i));
		
		if ( pos != -1 ) {
			YZSession::self()->currentView()->centerViewVertically( i );
			YZSession::self()->currentView()->gotoxy( 0, i, true );
			YZSession::self()->saveJumpPosition();
			break;
		}
	}
} 

static bool jumpToJumpRecord(const YZYzisinfoJumpListRecord *record)
{	
	YZBuffer *buffer = YZSession::self()->currentView()->myBuffer();
	
	// check to see if we have to change buffers before jumping
	if ( record->filename() != buffer->fileName() ) {
		// TODO: is this necessary?  It was in the old code, but it seems
		// like it just gets in the way when using kyzis (nyzis may be another matter)
		if ( buffer->fileIsModified() ) {
			YZSession::self()->guiPopupMessage( _("File has been modified") );
			return false;
		}
		
		switchToViewOfFilename( record->filename() );
	}
	
	const YZCursor &cursor = record->position();
	YZSession::self()->currentView()->centerViewVertically( cursor.y() );
	YZSession::self()->currentView()->gotodxdy( cursor, true );
	
	return true;
}

static void readAllMatchingTags( const YZTagStackItem &initialTag )
{
	int tagResult;
	QVector<YZTagStackItem> tags;
	tags.push_back( initialTag );
	
	for ( int i = 0; i < tagfilelist.size(); ++i ) {
		for ( ;; ) {
			tagEntry entry;
			tagResult = tagsFindNext( tagfilelist[i], &entry );
			if ( tagResult == TagSuccess ) {
				tags.push_back( YZTagStackItem( entry.address.pattern, entry.file ) );
			} else {
				break;
			}
		}
	}
	
	YZSession::self()->getTagStack().storeMatchingTags( tags );
}

static void showNumMatches()
{
	YZTagStack &stack = YZSession::self()->getTagStack();
	unsigned int cur = stack.getNumCurMatchingTag() + 1; // +1 is because number is 0 based
	unsigned int max = stack.getNumMatchingTags();
	
	if ( max > 1 ) {
		// TODO: is this localized properly?  I doubt it.
		QString msg("Tag %1 of %2");
		YZSession::self()->currentView()->guiDisplayInfo( msg.arg( cur ).arg( max ) );
	}
}

void tagReset () {
	closeTagFile();
}

void tagJumpTo ( const QString &word ) {
	// Guardian for empty tag search
	if ( word.isNull() ) {
		return;
	}

	if ( !openTagFile() ) {
		YZSession::self()->guiPopupMessage( _("Unable to find tag file") );
		return;
	}
	
	tagEntry entry;
	lastsearch = word.toUtf8();
	int tagResult;
	
	// look through each tag file in order for the tag
	// if we find one, don't search the rest of the tag files
	for ( int i = 0; i < tagfilelist.size(); ++i) {
		tagResult = tagsFind( tagfilelist[i], &entry, lastsearch.toUtf8(), TAG_FULLMATCH );
		
		if ( tagResult == TagSuccess ) {
			YZTagStack &stack = YZSession::self()->getTagStack();
			YZTagStackItem item( entry.address.pattern, entry.file );
			stack.push();

			doJumpToTag ( item );
			readAllMatchingTags( item );
			
			showNumMatches();	
			
			break;
		}
	}
	
	closeTagFile();
}

void tagNext () {
	YZTagStack &stack = YZSession::self()->getTagStack();
	const YZTagStackItem *entry = stack.moveToNext();
	
	if ( entry ) {
		doJumpToTag( *entry );
		
		showNumMatches();
	}
	else {
		YZSession::self()->currentView()->guiDisplayInfo( _("Could not find next tag") );
	}
}

void tagPrev () {
	YZTagStack &stack = YZSession::self()->getTagStack();
	const YZTagStackItem *entry = stack.moveToPrevious();
	
	if ( entry ) {
		doJumpToTag( *entry );

		showNumMatches();
	} else {
		YZSession::self()->currentView()->guiDisplayInfo( _("Could not find previous tag") );
	}
}

void tagPop () {
	YZTagStack &stack = YZSession::self()->getTagStack();
	
	if ( stack.empty() ) {
		YZSession::self()->currentView()->guiDisplayInfo( _("At bottom of tag stack") );
		return;
	}
	
	const YZYzisinfoJumpListRecord *head = stack.getHead();
	if ( jumpToJumpRecord( head ) ) {
		stack.pop();

		showNumMatches();
	}
}

void tagStartsWith(const QString &prefix, QStringList &list)
{
	if ( !openTagFile() ) {
		return;
	}
	
	for ( int i = 0; i < tagfilelist.size(); ++i ) {
		tagEntry entry;
		int tagResult = tagsFind( tagfilelist[i], &entry, prefix.toUtf8(), TAG_PARTIALMATCH );
		
		while ( tagResult == TagSuccess ) {
			list.push_back( entry.name );
			
			tagResult = tagsFindNext( tagfilelist[i], &entry );
		}
	}
		
	closeTagFile();
}
