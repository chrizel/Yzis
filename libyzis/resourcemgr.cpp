/* This file is part of the Yzis libraries
*  Copyright (C) 2007 Philippe Fremy <phil@freehackers.org>
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

#include "resourcemgr.h"
#include "debug.h"
#include "translator.h"

#include <QDir>

#define dbg()    yzDebug("YResourceMgr")
#define err()    yzError("YResourceMgr")
#define ftl()    yzFatal("YResourceMgr")



// ================================================================
//
//                      YResourceMgr
//
// ================================================================

YResourceMgr::YResourceMgr()
{
    initConfig();
}

YResourceMgr::~YResourceMgr()
{
}

void YResourceMgr::initConfig()
{
    // handle ~/.yzis/
    QString yzisSuffix = ".yzis";
    bool isTmpDir = false;
    mYzisUserDir = QDir::homePath() + "/" + yzisSuffix + "/";
    QDir yzisUserDir( mYzisUserDir );
    if (! yzisUserDir.exists() ) {
        dbg().SPrintf("User dir does not exist, creating it: %s", qp(mYzisUserDir) );
        yzisUserDir.cdUp();
        if (! yzisUserDir.mkdir( yzisSuffix )) {
            isTmpDir = true;
            mYzisUserDir = QDir::tempPath() + "/";
            err() << "initConfig(): could not create yzis user directory, falling back on " << mYzisUserDir;
        }
    }
    yzisUserDir.setPath( mYzisUserDir );
    if ( (!QFileInfo(mYzisUserDir).isWritable()) && (!isTmpDir) ) {
        mYzisUserDir = QDir::tempPath() + "/";
        err() << "initConfig(): yzis user directory is not writable, falling back on " << mYzisUserDir;
        isTmpDir = true;
    }

    if ((! QFileInfo(mYzisUserDir).isWritable() )) {
        err() << "initConfig(): yzis user directory " << mYzisUserDir << " is not writable, falling back on " << mYzisUserDir;
        err() << "initConfig(): Yzis will not function properly" << endl;
    }

    dbg() << "initConfig(): yzis user directory set to " << mYzisUserDir << endl;
}

QString YResourceMgr::findResource( ResourceType type, const QString & fname )
{
    QString resource;
    QStringList dirCandidates;

    dbg() << "findResource(" << type << ", " << fname << ")" << endl;

    // Writable config is always in the config subdir
    if (type == WritableConfigResource) {
        resource = mYzisUserDir + fname;
        return resource;
    }

    // UserScriptResource may be an absolute path
    if (QFileInfo(fname).isAbsolute()) {
        dbg() << "findResource(): looking up absolute path: " << fname << endl;
        if (QFile::exists( fname )) return fname;
        return QString();
    }

    // look up in the different directories
    dirCandidates = resourceDirList( type );

    foreach( QString candidate, dirCandidates ) {
        resource = candidate + fname;
        dbg() << "findResource(): looking up " << resource << endl;
        if (QFile::exists( resource )) {
            dbg() << "findResource(): Found at " << resource << endl;
            return resource;
        }
        if ( !resource.endsWith(".lua") )
            resource += ".lua";
        if (QFile::exists( resource )) {
            dbg() << "findResource(): Found at " << resource << endl;
            return resource;
        }
    }

    dbg() << "findResource(): resource " << fname << " not found" << endl;
    return QString();
}

QStringList YResourceMgr::resourceDirList( ResourceType type )
{
    QStringList dirCandidates;

    QString subdir;
    switch ( type ) {
    case IndentResource:
        subdir = "/scripts/indent/";
        break;
    case SyntaxHlResource:
        subdir = "/syntax/";
        break;
    case ConfigScriptResource:
    case UserScriptResource:
        subdir = "/scripts/";
        break;
    case ConfigResource:
    case WritableConfigResource:
        subdir = "/";
        break;
    default:
        err().SPrintf( "Unknown resource type requested: %d\n", (int) type );
        return dirCandidates;
    }

    if (type == UserScriptResource) dirCandidates << "./";
    dirCandidates << mYzisUserDir + subdir;
    char * s = getenv("YZISHOME");
    if (s != NULL) dirCandidates << (s + subdir);
    dirCandidates << QString( PREFIX ) + "/share/yzis/" + subdir;

    return dirCandidates;
}

YDebugStream& operator<<( YDebugStream& out, const ResourceType & type )
{
    switch ( type ) {
    case IndentResource: out << "IndentResource"; break;
    case SyntaxHlResource: out << "SyntaxHlResource"; break;
    case ConfigScriptResource: out << "ConfigScriptResource"; break;
    case UserScriptResource: out << "UserScriptResource"; break;
    case ConfigResource: out << "ConfigResource"; break;
    case WritableConfigResource: out << "WritableConfigResource"; break;
    }
    return out;
}

