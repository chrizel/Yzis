/* This file is part of the KDE libraries
   Copyright (C) 2001,2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __KATE_HIGHLIGHT_H__
#define __KATE_HIGHLIGHT_H__

#include "line.h"
#include "attribute.h"

#include <qptrlist.h>
#include <qvaluelist.h>
#include <qregexp.h>
#include <qdict.h>
#include <qintdict.h>
#include <qmap.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qguardedptr.h>

class YzisHlContext;
class YzisHlItem;
class YzisHlItemData;
class YzisHlData;
class YzisEmbeddedHlInfo;
class YzisHlIncludeRule;
class YzisSyntaxDocument;
class YZLine;
class YzisSyntaxModeListItem;
class YzisSyntaxContextData;

class QPopupMenu;

// some typedefs
typedef QPtrList<YzisAttribute> YzisAttributeList;
typedef QValueList<YzisHlIncludeRule*> YzisHlIncludeRules;
typedef QPtrList<YzisHlItemData> YzisHlItemDataList;
typedef QPtrList<YzisHlData> YzisHlDataList;
typedef QMap<QString,YzisEmbeddedHlInfo> YzisEmbeddedHlInfos;
typedef QMap<int*,QString> YzisHlUnresolvedCtxRefs;

//Item Properties: name, Item Style, Item Font
class YzisHlItemData : public YzisAttribute
{
  public:
    YzisHlItemData(const QString  name, int defStyleNum);

    enum ItemStyles {
      dsNormal,
      dsKeyword,
      dsDataType,
      dsDecVal,
      dsBaseN,
      dsFloat,
      dsChar,
      dsString,
      dsComment,
      dsOthers,
      dsAlert,
      dsFunction,
      dsRegionMarker };

  public:
    const QString name;
    int defStyleNum;
};

class YzisHlData
{
  public:
    YzisHlData(const QString &wildcards, const QString &mimetypes,const QString &identifier, int priority);

  public:
    QString wildcards;
    QString mimetypes;
    QString identifier;
    int priority;
};

class YzisHighlighting
{
  public:
    YzisHighlighting(const YzisSyntaxModeListItem *def);
    ~YzisHighlighting();

  public:
    void doHighlight ( YZLine *prevLine,
                       YZLine *textLine,
                       QMemArray<signed char> *foldingList,
                       bool *ctxChanged );

    void loadWildcards();
    QValueList<QRegExp>& getRegexpExtensions();
    QStringList& getPlainExtensions();

    QString getMimetypes();

    // this pointer needs to be deleted !!!!!!!!!!
//    YzisHlData *getData();
    void setData(YzisHlData *);

    void setYzisHlItemDataList(uint schema, YzisHlItemDataList &);

    // both methodes return hard copies of the internal lists
    // the lists are cleared first + autodelete is set !
    // keep track that you delete them, or mem will be lost
    void getYzisHlItemDataListCopy (uint schema, YzisHlItemDataList &);

    inline QString name() const {return iName;}
    inline QString section() const {return iSection;}
    inline QString version() const {return iVersion;}
    QString author () const { return iAuthor; }
    QString license () const { return iLicense; }
    int priority();
    inline QString getIdentifier() const {return identifier;}
    void use();
    void release();
    bool isInWord(QChar c);

    inline QString getCommentStart() const {return cmlStart;};
    inline QString getCommentEnd()  const {return cmlEnd;};
    inline QString getCommentSingleLineStart() const { return cslStart;};

    void clearAttributeArrays ();

    QMemArray<YzisAttribute> *attributes (uint schema);

    inline bool noHighlighting () const { return noHl; };

  private:
    // make this private, nobody should play with the internal data pointers
    void getYzisHlItemDataList(uint schema, YzisHlItemDataList &);

    void init();
    void done();
    void makeContextList ();
    void handleYzisHlIncludeRules ();
    void handleYzisHlIncludeRulesRecursive(YzisHlIncludeRules::iterator it, YzisHlIncludeRules *list);
    int addToContextList(const QString &ident, int ctx0);
    void addToYzisHlItemDataList();
    void createYzisHlItemData (YzisHlItemDataList &list);
    void readGlobalKeywordConfig();
    void readCommentConfig();
    void readFoldingConfig ();

    // manipulates the ctxs array directly ;)
    void generateContextStack(int *ctxNum, int ctx, QMemArray<short> *ctxs, int *posPrevLine,bool lineContinue=false);

    YzisHlItem *createYzisHlItem(struct YzisSyntaxContextData *data, YzisHlItemDataList &iDl, QStringList *RegionList, QStringList *ContextList);
    int lookupAttrName(const QString& name, YzisHlItemDataList &iDl);

    void createContextNameList(QStringList *ContextNameList, int ctx0);
    int getIdFromString(QStringList *ContextNameList, QString tmpLineEndContext,/*NO CONST*/ QString &unres);

    YzisHlItemDataList internalIDList;

    QIntDict<YzisHlContext> contextList;
    inline YzisHlContext *contextNum (uint n) { return contextList[n]; }

    // make them pointers perhaps
    YzisEmbeddedHlInfos embeddedHls;
    YzisHlUnresolvedCtxRefs unresolvedContextReferences;
    QStringList RegionList;
    QStringList ContextNameList;

    bool noHl;
    bool folding;
    bool casesensitive;
    QString weakDeliminator;
    QString deliminator;

    QString cmlStart;
    QString cmlEnd;
    QString cslStart;
    QString iName;
    QString iSection;
    QString iWildcards;
    QString iMimetypes;
    QString identifier;
    QString iVersion;
    QString iAuthor;
    QString iLicense;
    int m_priority;
    int refCount;

    QString errorsAndWarnings;
    QString buildIdentifier;
    QString buildPrefix;
    bool building;
    uint itemData0;
    uint buildContext0Offset;
    YzisHlIncludeRules includeRules;
    QValueList<int> contextsIncludingSomething;
    bool m_foldingIndentationSensitive;

    QIntDict< QMemArray<YzisAttribute> > m_attributeArrays;

    QString extensionSource;
    QValueList<QRegExp> regexpExtensions;
    QStringList plainExtensions;

  public:
    inline bool foldingIndentationSensitive () { return m_foldingIndentationSensitive; }
    inline bool allowsFolding(){return folding;}
};

class YzisHlManager : public QObject
{
  Q_OBJECT

  private:
    YzisHlManager();

  public:
    ~YzisHlManager();

    static YzisHlManager *self();

//    inline KConfig *getKConfig() { return &m_config; };

    YzisHighlighting *getHl(int n);
    int nameFind(const QString &name);

    int detectHighlighting (class YZBuffer *doc);

    int findHl(YzisHighlighting *h) {return hlList.find(h);}
    QString identifierForName(const QString&);

    // methodes to get the default style count + names
    static uint defaultStyles();
    static QString defaultStyleName(int n);

    void getDefaults(uint schema, YzisAttributeList &);
    void setDefaults(uint schema, YzisAttributeList &);

    int highlights();
    QString hlName(int n);
    QString hlSection(int n);

  signals:
    void changed();

  private:
    int wildcardFind(const QString &fileName);
    int mimeFind(const QByteArray &contents);
    int realWildcardFind(const QString &fileName);
	QString findByContent( const QByteArray& contents );

  private:
    friend class YzisHighlighting;

    QPtrList<YzisHighlighting> hlList;
    QDict<YzisHighlighting> hlDict;

    static YzisHlManager *s_self;

    //KConfig m_config;
    QStringList commonSuffixes;

    YzisSyntaxDocument *syntax;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
