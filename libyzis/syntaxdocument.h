/* This file is part of the Yzis libraries
 *  Copyright (C) 2003,2004 Mickael Marchand <marchand@kde.org>
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
 * This file was taken from Kate, the KDE editor
 * The original license is LGPL and the copyrights follow below
 */

/* This file is part of the KDE libraries (Kate editor)
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2000 Scott Manson <sdmanson@alltel.net>
*/

#ifndef __YZ_SYNTAXDOCUMENT_H__
#define __YZ_SYNTAXDOCUMENT_H__

#include <qdom.h>
#include <qstringlist.h>
#include <qapplication.h>
#include <qobject.h>

/**
 * Information about each syntax hl Mode
 */
class YzisSyntaxModeListItem
{
  public:
    QString name;
    QString nameTranslated;
    QString section;
    QString mimetype;
    QString extension;
    QString identifier;
    QString version;
    QString priority;
    QString author;
    QString license;
    bool hidden;
};

/**
 * List of the YzisSyntaxModeListItems holding all the syntax mode list items
 */
typedef QValueList<YzisSyntaxModeListItem*> YzisSyntaxModeList;

/**
 * Class holding the data around the current QDomElement
 */
class YzisSyntaxContextData
{
  public:
    QDomElement parent;
    QDomElement currentGroup;
    QDomElement item;
};

/**
 * Store and manage the information about Syntax Highlighting.
 */
class YzisSyntaxDocument : public QDomDocument
{
  public:
    /**
     * Constructor
     * Sets the current file to nothing and build the ModeList
     * @param force fore the update of the hl cache
     */
    YzisSyntaxDocument(bool force = false);

    /**
     * Desctructor
     */
    ~YzisSyntaxDocument();

    /**
	 * If the open hl file is different from the one needed, it opens
     * the new one and assign some other things.
     * @param identifier file name and path of the new xml needed
     */
    bool setIdentifier(const QString& identifier);

    /**
     * Get the mode list
     * @return mode list
     */
    const YzisSyntaxModeList &modeList() { return myModeList; }

    /**
     * Jump to the next group, YzisSyntaxContextData::currentGroup will point to the next group
     * @param data context
     * @return success
     */
    bool nextGroup(YzisSyntaxContextData* data);

    /**
     * Jump to the next item, YzisSyntaxContextData::item will point to the next item
     * @param data context
     * @return success
     */
    bool nextItem(YzisSyntaxContextData* data);

    /**
     * This function is used to fetch the attributes of the tags.
     */
    QString groupItemData(const YzisSyntaxContextData* data,const QString& name);
    QString groupData(const YzisSyntaxContextData* data,const QString& name);

    void freeGroupInfo(YzisSyntaxContextData* data);
    YzisSyntaxContextData* getSubItems(YzisSyntaxContextData* data);

    /**
     * Get the YzisSyntaxContextData of the DomElement Config inside mainGroupName
     * It just fills YzisSyntaxContextData::item
     */
    YzisSyntaxContextData* getConfig(const QString& mainGroupName, const QString &config);

    /**
     * Get the YzisSyntaxContextData of the QDomElement Config inside mainGroupName
     * YzisSyntaxContextData::parent will contain the QDomElement found
     */
    YzisSyntaxContextData* getGroupInfo(const QString& mainGroupName, const QString &group);

    /**
     * Returns a list with all the keywords inside the list type
     */
    QStringList& finddata(const QString& mainGroup,const QString& type,bool clearList=true);


  private:
    /**
     * Generate the list of hl modes, store them in myModeList
     * force: if true forces to rebuild the Mode List from the xml files (instead of katesyntax...rc)
     * @param force if true forces to rebuild the Mode List from the xml files (instead of katesyntax...rc)
	 */
    void setupModeList(bool force);

    /**
     * Used by getConfig and getGroupInfo to traverse the xml nodes and
     * evenually return the found element
    */
    bool getElement (QDomElement &element, const QString &mainGroupName, const QString &config);

    /**
     * List of mode items
     */
    YzisSyntaxModeList myModeList;
	QStringList findAllResources( const char *type, const QString& filter, bool recursive, bool unique) const;

    /**
     * current parsed filename
     */
    QString currentFile;

    /**
     * last found data out of the xml
     */
    QStringList m_data;
};

#endif
