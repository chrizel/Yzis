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


#ifndef RESOURCE_MGR_H
#define RESOURCE_MGR_H

#include <QString>

#include "session.h"

class QStringList;
class YZDebugStream;


/** Resource type that one can look for in yzis. */
enum ResourceType {
    /** A script file run by user (eg: ":source " ).
      *
      * The search path for such a script includes the current  directory.
      * Those files are normally stored in the scripts subdirectory. */
    UserScriptResource,     

    /** A script file run by yzis (eg: hl.lua).
      *
      * The file is searched in the scripts subdirectory of the
      * different yzis config directories. */
    ConfigScriptResource,   

    /** A file managing indentation (script file).
      *
      * The file is looked up in the indent subdirectory.  */
    IndentResource,     

    /** A file managing syntax highlighting (xml file).
      *
      * The file is looked up in the syntax subdirectory. */
    SyntaxHlResource,   
    
    /**  A config file.
      *
      *  The file is looked up in the yzis directory. */
    ConfigResource,     

    /**  A config file writeable by yzis.
      *
      *  The file is not looked up. A fixed location is returned,
      *  where file can be read and written. The fixed location is the
      *  yzis directory. */
    WritableConfigResource,     
};


class YZIS_EXPORT YZResourceMgr
{

public:
    YZResourceMgr();
    ~YZResourceMgr();


    /** Try to find a resource in the different yzis directories.
      *
      * Resources are looked up in the following order and places:
      * - current directory (only for ScriptResource)
      * - [User Home]/.yzis
      * - $YZISHOME
      * - [Installation dir]/share/yzis
      *
      * Depending on the resource type, a subdirectory of the resource
      * tree is looked into. For example, looking for a SyntaxHlResource
      * cobol.xml will trigger a lookup in ~/.yzis/syntax/cobol.xml .
      *
      * If the resource is found, the methods returns a QString with the
      * absolute path of the file name. Else, it returns an empty QString.
      *
      * @param type the type of resource being looked up
      * @param fname the name of the file being looked up
      * @return an empty QString if the resource could not be found, or
      * the absolute path of the resource.
      */
    QString findResource( ResourceType type, const QString & fname );

    /** Get a list of all directories that are looked up for finding a
      * resource.
      *
      * This is useful when the caller wants to find many resource files
      * of the same type at once. libyzis uses it to lookup syntax files.
      *
      * The method includes the results of guiResourceDirList() as well.
      */
    QStringList resourceDirList( ResourceType type);

protected:
    /** Creates the user yzis directory and some default config files.
      *
      * This function is called when yzis is initialising. It will
      * create directory ~/.yzis on unix and [home]/.yzis on windows. It
      * will also create the subdirectory config for storing yzisinfo and
      * hl.info file.
      *
      * Those directories can be used by yzis to store config or cache
      * files.
      *
      * If the user directory can not be created or is not writeable, an
      * error popup is raised and the temporary directory is used instead.
      */
    void initConfig();

    /** Store the path of the yzis directory. */
    QString mYzisUserDir;

};

/** Shortcut for YZSession::self()->resourceMgr(); */
inline YZResourceMgr * resourceMgr() 
    { return YZSession::self()->resourceMgr(); }

YZDebugStream& operator<<( YZDebugStream& out, const ResourceType & type );

#endif // RESOURCE_MGR_H
