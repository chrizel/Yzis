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

/* This file is part of the KDE libraries
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2000 Scott Manson <sdmanson@alltel.net>
*/

#include <qfile.h>
#include "syntaxdocument.h"
#include "debug.h"

/** Constructor
    Sets the current file to nothing and build the ModeList
*/
YzisSyntaxDocument::YzisSyntaxDocument(bool force)
  : QDomDocument()
{
  setupModeList(force);
  myModeList.setAutoDelete( true );
}

/** Destructor
    Do nothing yet
*/
YzisSyntaxDocument::~YzisSyntaxDocument()
{
}

/** If the open hl file is different from the one needed, it opens
    the new one and assign some other things.
    identifier = File name and path of the new xml needed
*/
bool YzisSyntaxDocument::setIdentifier(const QString& identifier)
{
  // if the current file is the same as the new one don't do anything.
  if(currentFile != identifier)
  {
    // let's open the new file
    QFile f( identifier );

    if ( f.open(IO_ReadOnly) )
    {
      // Let's parse the contets of the xml file
      /* The result of this function should be check for robustness,
         a false returned means a parse error */
      QString errorMsg;
      int line, col;
      bool success=setContent(&f,&errorMsg,&line,&col);

      // Ok, now the current file is the pretended one (identifier)
      currentFile = identifier;

      // Close the file, is not longer needed
      f.close();

      if (!success)
      {
        //KMessageBox::error(0L,i18n("<qt>The error <b>%4</b><br> has been detected in the file %1 at %2/%3</qt>").arg(identifier)
          //  .arg(line).arg(col).arg(i18n("QXml",errorMsg.utf8())));
        return false;
      }
    }
    else
    {
      // Oh o, we couldn't open the file.
      //KMessageBox::error( 0L, i18n("Unable to open %1").arg(identifier) );
      return false;
    }
  }
  return true;
}

/** Get the complete syntax mode list
*/
YzisSyntaxModeList YzisSyntaxDocument::modeList()
{
  return myModeList;
}

/**
 * Jump to the next group, YzisSyntaxContextData::currentGroup will point to the next group
 */
bool YzisSyntaxDocument::nextGroup( YzisSyntaxContextData* data)
{
  if(!data)
    return false;

  // No group yet so go to first child
  if (data->currentGroup.isNull())
  {
    // Skip over non-elements. So far non-elements are just comments
    QDomNode node = data->parent.firstChild();
    while (node.isComment())
      node = node.nextSibling();

    data->currentGroup = node.toElement();
  }
  else
  {
    // common case, iterate over siblings, skipping comments as we go
    QDomNode node = data->currentGroup.nextSibling();
    while (node.isComment())
      node = node.nextSibling();

    data->currentGroup = node.toElement();
  }

  return !data->currentGroup.isNull();
}

/**
 * Jump to the next item, YzisSyntaxContextData::item will point to the next item
 */
bool YzisSyntaxDocument::nextItem( YzisSyntaxContextData* data)
{
  if(!data)
    return false;

  if (data->item.isNull())
  {
    QDomNode node = data->currentGroup.firstChild();
    while (node.isComment())
      node = node.nextSibling();

    data->item = node.toElement();
  }
  else
  {
    QDomNode node = data->item.nextSibling();
    while (node.isComment())
      node = node.nextSibling();

    data->item = node.toElement();
  }

  return !data->item.isNull();
}

/**
 * This function is used to fetch the attributes of the tags of the item in a YzisSyntaxContextData.
 */
QString YzisSyntaxDocument::groupItemData( const YzisSyntaxContextData* data, const QString& name){
  if(!data)
    return QString::null;

  // If there's no name just return the tag name of data->item
  if ( (!data->item.isNull()) && (name.isEmpty()))
  {
    return data->item.tagName();
  }

  // if name is not empty return the value of the attribute name
  if (!data->item.isNull())
  {
    return data->item.attribute(name);
  }

  return QString::null;

}

QString YzisSyntaxDocument::groupData( const YzisSyntaxContextData* data,const QString& name)
{
  if(!data)
    return QString::null;

  if (!data->currentGroup.isNull())
  {
    return data->currentGroup.attribute(name);
  }
  else
  {
    return QString::null;
  }
}

void YzisSyntaxDocument::freeGroupInfo( YzisSyntaxContextData* data)
{
  if (data)
    delete data;
}

YzisSyntaxContextData* YzisSyntaxDocument::getSubItems(YzisSyntaxContextData* data)
{
  YzisSyntaxContextData *retval = new YzisSyntaxContextData;

  if (data != 0)
  {
    retval->parent = data->currentGroup;
    retval->currentGroup = data->item;
  }

  return retval;
}

bool YzisSyntaxDocument::getElement (QDomElement &element, const QString &mainGroupName, const QString &config)
{
  yzDebug() << "Looking for \"" << mainGroupName << "\" -> \"" << config << "\"." << endl;

  QDomNodeList nodes = documentElement().childNodes();

  // Loop over all these child nodes looking for mainGroupName
  for (unsigned int i=0; i<nodes.count(); i++)
  {
    QDomElement elem = nodes.item(i).toElement();
    if (elem.tagName() == mainGroupName)
    {
      // Found mainGroupName ...
      QDomNodeList subNodes = elem.childNodes();

      // ... so now loop looking for config
      for (unsigned int j=0; j<subNodes.count(); j++)
      {
        QDomElement subElem = subNodes.item(j).toElement();
        if (subElem.tagName() == config)
        {
          // Found it!
          element = subElem;
          return true;
        }
      }

      yzDebug() << "WARNING: \""<< config <<"\" wasn't found!" << endl;
      return false;
    }
  }

  yzDebug() << "WARNING: \""<< mainGroupName <<"\" wasn't found!" << endl;
  return false;
}

/**
 * Get the YzisSyntaxContextData of the QDomElement Config inside mainGroupName
 * YzisSyntaxContextData::item will contain the QDomElement found
 */
YzisSyntaxContextData* YzisSyntaxDocument::getConfig(const QString& mainGroupName, const QString &config)
{
  QDomElement element;
  if (getElement(element, mainGroupName, config))
  {
    YzisSyntaxContextData *data = new YzisSyntaxContextData;
    data->item = element;
    return data;
  }
  return 0;
}

/**
 * Get the YzisSyntaxContextData of the QDomElement Config inside mainGroupName
 * YzisSyntaxContextData::parent will contain the QDomElement found
 */
YzisSyntaxContextData* YzisSyntaxDocument::getGroupInfo(const QString& mainGroupName, const QString &group)
{
  QDomElement element;
  if (getElement(element, mainGroupName, group+"s"))
  {
    YzisSyntaxContextData *data = new YzisSyntaxContextData;
    data->parent = element;
    return data;
  }
  return 0;
}

/**
 * Returns a list with all the keywords inside the list type
 */
QStringList& YzisSyntaxDocument::finddata(const QString& mainGroup, const QString& type, bool clearList)
{
  yzDebug()<<"Create a list of keywords \""<<type<<"\" from \""<<mainGroup<<"\"."<<endl;
  if (clearList)
    m_data.clear();

  for(QDomNode node = documentElement().firstChild(); !node.isNull(); node = node.nextSibling())
  {
    QDomElement elem = node.toElement();
    if (elem.tagName() == mainGroup)
    {
      yzDebug()<<"\""<<mainGroup<<"\" found."<<endl;
      QDomNodeList nodelist1 = elem.elementsByTagName("list");

      for (uint l=0; l<nodelist1.count(); l++)
      {
        if (nodelist1.item(l).toElement().attribute("name") == type)
        {
          yzDebug()<<"List with attribute name=\""<<type<<"\" found."<<endl;
          QDomNodeList childlist = nodelist1.item(l).toElement().childNodes();

          for (uint i=0; i<childlist.count(); i++)
          {
            QString element = childlist.item(i).toElement().text().stripWhiteSpace();
            if (element.isEmpty())
              continue;
#ifndef NDEBUG
            if (i<6)
            {
              yzDebug()<<"\""<<element<<"\" added to the list \""<<type<<"\""<<endl;
            }
            else if(i==6)
            {
              yzDebug()<<"... The list continues ..."<<endl;
            }
#endif
            m_data += element;
          }

          break;
        }
      }
      break;
    }
  }

  return m_data;
}

// Private
/** Generate the list of hl modes, store them in myModeList
    force: if true forces to rebuild the Mode List from the xml files (instead of configfile)
*/
void YzisSyntaxDocument::setupModeList (bool force)
{
  // If there's something in myModeList the Mode List was already built so, don't do it again
  if (!myModeList.isEmpty())
    return;

  // We'll store the ModeList in katesyntaxhighlightingrc
  /*KConfig config("katesyntaxhighlightingrc", false, false);

  // figure our if the kate install is too new
  config.setGroup ("General");
  if (config.readNumEntry ("Version") > config.readNumEntry ("CachedVersion"))
  {
    config.writeEntry ("CachedVersion", config.readNumEntry ("Version"));
    force = true;
  }*/

  // Let's get a list of all the xml files for hl
  //QStringList list = KGlobal::dirs()->findAllResources("data","katepart/syntax/*.xml",false,true);
  QStringList list;
  list << "/opt/kde/share/apps/katepart/syntax/cpp.xml";
  //just C++ for now

  // Let's iterate through the list and build the Mode List
  for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
  {
/*    // Each file has a group called:
    QString Group="Cache "+*it;

    // If the group exist and we're not forced to read the xml file, let's build myModeList for katesyntax..rc
    if ((config.hasGroup(Group)) && (!force))
    {
      // Let's go to this group
      config.setGroup(Group);

      // Let's make a new YzisSyntaxModeListItem to instert in myModeList from the information in katesyntax..rc
      YzisSyntaxModeListItem *mli=new YzisSyntaxModeListItem;
      mli->name       = config.readEntry("name"); // ### TODO: translation (bug #72220)
      mli->section    = i18n("Language Section",config.readEntry("section").utf8());
      mli->mimetype   = config.readEntry("mimetype");
      mli->extension  = config.readEntry("extension");
      mli->version    = config.readEntry("version");
      mli->priority   = config.readEntry("priority");
      mli->author    = config.readEntry("author");
      mli->license   = config.readEntry("license");
      mli->identifier = *it;

      // Apend the item to the list
      myModeList.append(mli);
    }
    else
    {*/
      // We're forced to read the xml files or the mode doesn't exist in the katesyntax...rc
      QFile f(*it);

      if (f.open(IO_ReadOnly))
      {
        // Ok we opened the file, let's read the contents and close the file
        /* the return of setContent should be checked because a false return shows a parsing error */
        QString errMsg;
        int line, col;

        bool success = setContent(&f,&errMsg,&line,&col);

        f.close();

        if (success)
        {
          QDomElement root = documentElement();

          if (!root.isNull())
          {
            // If the 'first' tag is language, go on
            if (root.tagName()=="language")
            {
              // let's make the mode list item.
              YzisSyntaxModeListItem *mli = new YzisSyntaxModeListItem;

              mli->name      = root.attribute("name");
              mli->section   = root.attribute("section");
              mli->mimetype  = root.attribute("mimetype");
              mli->extension = root.attribute("extensions");
              mli->version   = root.attribute("version");
              mli->priority  = root.attribute("priority");
              mli->author    = root.attribute("author");
              mli->license   = root.attribute("license");


              mli->identifier = *it;

              // Now let's write or overwrite (if force==true) the entry in katesyntax...rc
/*              config.setGroup(Group);
              config.writeEntry("name",mli->name);
              if (mli->section.isEmpty()) // ### TODO: can this happen at all?
                config.writeEntry("section","Other");
              else
                config.writeEntry("section",mli->section);
              config.writeEntry("mimetype",mli->mimetype);
              config.writeEntry("extension",mli->extension);
              config.writeEntry("version",mli->version);
              config.writeEntry("priority",mli->priority);
              config.writeEntry("author",mli->author);
              config.writeEntry("license",mli->license);
*/
              // Now that the data is in the config file, translate section
              if (mli->section.isEmpty()) // ### TODO: can this happen at all?
                mli->section    = "Language Section";
              else
                mli->section    = "Language Section"; // We need the i18n context for when reading again the config

              // Append the new item to the list.
              myModeList.append(mli);
            }
          }
		  yzDebug() << "SyntaxDocument fully build" << endl;
        }
        else
        {
          YzisSyntaxModeListItem *emli=new YzisSyntaxModeListItem;

          emli->section="Errors!";
          emli->mimetype="invalid_file/invalid_file";
          emli->extension="invalid_file.invalid_file";
          emli->version="1.";
          emli->name=QString("Error: %1").arg(*it);
          emli->identifier=(*it);

          myModeList.append(emli);
        }
      }
    //}
  }
}

