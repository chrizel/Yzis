/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation
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

/* This file was taken from the Kate editor which is part of KDE
   Kate's code is published under the LGPL version 2 (and 2 only not any later 
   version)
   Copyright (C) 2001,2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>
*/

#ifndef __KATE_HIGHLIGHT_H__
#define __KATE_HIGHLIGHT_H__

#include "attribute.h"

#if QT_VERSION < 0x040000
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qvaluevector.h>
#include <qregexp.h>
#include <qdict.h>
#include <qintdict.h>
#include <qmap.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qguardedptr.h>
#include <qdatetime.h>
#else
#include <QPair>
#include <QString>
#include <QMap>
#include <QHash>
#include <QTime>
#include <QLinkedList>
#endif

#ifndef WIN32
#include "magic.h"
#endif

class YzisHlContext;
class YzisHlItem;
class YzisHlItemData;
class YzisHlData;
class YzisHlIncludeRule;
class YzisSyntaxDocument;
class YZLine;
class YzisSyntaxModeListItem;
class YzisSyntaxContextData;

class QPopupMenu;

class YzisEmbeddedHlInfo
{
  public:
    YzisEmbeddedHlInfo() {loaded=false;context0=-1;}
    YzisEmbeddedHlInfo(bool l, int ctx0) {loaded=l;context0=ctx0;}

  public:
    bool loaded;
    int context0;
};

// some typedefs
#if QT_VERSION < 0x040000
typedef QPtrList<YzisAttribute> YzisAttributeList;
typedef QValueList<YzisHlIncludeRule*> YzisHlIncludeRules;
typedef QPtrList<YzisHlItemData> YzisHlItemDataList;
typedef QPtrList<YzisHlData> YzisHlDataList;
typedef QValueList<int> IntList;
#else
typedef QList<YzisAttribute*> YzisAttributeList;
typedef QLinkedList<YzisHlIncludeRule*> YzisHlIncludeRules;
typedef QList<YzisHlItemData*> YzisHlItemDataList;
typedef QList<YzisHlData*> YzisHlDataList;
typedef QList<int> IntList;
#endif
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
      dsRegionMarker,
      dsError	};

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
#if QT_VERSION < 0x040000
                       QMemArray<uint> *foldingList,
#else
                       QVector<uint> *foldingList,
#endif
                       bool *ctxChanged );

    void loadWildcards();
#if QT_VERSION < 0x040000
    QValueList<QRegExp>& getRegexpExtensions();
#else
    QList<QRegExp>& getRegexpExtensions();
#endif
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

    const QString &name() const {return iName;}
    const QString &nameTranslated() const {return iNameTranslated;}
    const QString &section() const {return iSection;}
    bool hidden() const {return iHidden;}
    const QString &version() const {return iVersion;}
    const QString &author () const { return iAuthor; }
    const QString &license () const { return iLicense; }
    int priority();
    const QString &getIdentifier() const {return identifier;}
    void use();
    void release();

    /**
     * @return true if the character @p c is not a deliminator character
     *     for the corresponding highlight.
     */
    bool isInWord( QChar c, int attrib=0 ) const;

    /**
     * @return true if the character @p c is a wordwrap deliminator as specified
     * in the general keyword section of the xml file.
     */
    bool canBreakAt( QChar c, int attrib=0 ) const;

    /**
    * @return true if @p beginAttr and @p endAttr are members of the same
    * highlight, and there are comment markers of either type in that.
    */
    bool canComment( int startAttr, int endAttr ) const;

    /**
    * @return 0 if highlighting which attr is a member of does not
    * define a comment region, otherwise the region is returned
    */
    signed char commentRegion(int attr) const;

    /**
     * @return the mulitiline comment start marker for the highlight
     * corresponding to @p attrib.
     */
    QString getCommentStart( int attrib=0 ) const;

    /**
     * @return the muiltiline comment end marker for the highlight corresponding
     * to @p attrib.
     */
    QString getCommentEnd( int attrib=0 ) const;

    /**
     * @return the single comment marker for the highlight corresponding
     * to @p attrib.
     */
    QString getCommentSingleLineStart( int attrib=0 ) const;

    /**
    * @return the attribute for @p context.
    */
    int attribute( int context ) const;

    void clearAttributeArrays ();

#if QT_VERSION < 0x040000
    QMemArray<YzisAttribute> *attributes (uint schema);
#else
    QVector<YzisAttribute> *attributes (uint schema);
#endif

    inline bool noHighlighting () const { return noHl; };

    // be carefull: all documents hl should be invalidated after calling this method!
    void dropDynamicContexts();

    QString indentation () { return m_indentation; }

  private:
    // make this private, nobody should play with the internal data pointers
    void getYzisHlItemDataList(uint schema, YzisHlItemDataList &);

    void init();
    void done();
    void makeContextList ();
    int makeDynamicContext(YzisHlContext *model, const QStringList *args);
    void handleYzisHlIncludeRules ();
    void handleYzisHlIncludeRulesRecursive(YzisHlIncludeRules::iterator it, YzisHlIncludeRules *list);
    int addToContextList(const QString &ident, int ctx0);
    void addToYzisHlItemDataList();
    void createYzisHlItemData (YzisHlItemDataList &list);
    void readGlobalKeywordConfig();
    void readWordWrapConfig();
    void readCommentConfig();
    void readIndentationConfig ();
    void readFoldingConfig ();

    // manipulates the ctxs array directly ;)
#if QT_VERSION < 0x040000
    void generateContextStack(int *ctxNum, int ctx, QMemArray<short> *ctxs, int *posPrevLine);
#else
    void generateContextStack(int *ctxNum, int ctx, QVector<short> *ctxs, int *posPrevLine);
#endif

    YzisHlItem *createYzisHlItem(YzisSyntaxContextData *data, YzisHlItemDataList &iDl, QStringList *RegionList, QStringList *ContextList);
    int lookupAttrName(const QString& name, YzisHlItemDataList &iDl);

    void createContextNameList(QStringList *ContextNameList, int ctx0);
    int getIdFromString(QStringList *ContextNameList, QString tmpLineEndContext,/*NO CONST*/ QString &unres);

    /**
     * @return the key to use for @p attrib in m_additionalData.
     */
    QString hlKeyForAttrib( int attrib ) const;

    YzisHlItemDataList internalIDList;

#if QT_VERSION < 0x040000
    QValueVector<YzisHlContext*> m_contexts;
#else
    QVector<YzisHlContext*> m_contexts;
#endif
    inline YzisHlContext *contextNum (uint n) { if (n < (uint)m_contexts.size()) return m_contexts[n]; return 0; }

    QMap< QPair<YzisHlContext *, QString>, short> dynamicCtxs;

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

    QString iName;
    QString iNameTranslated;
    QString iSection;
    bool iHidden;
    QString iWildcards;
    QString iMimetypes;
    QString identifier;
    QString iVersion;
    QString iAuthor;
    QString iLicense;
    QString m_indentation;
    int m_priority;
    int refCount;
    int startctx, base_startctx;

    QString errorsAndWarnings;
    QString buildIdentifier;
    QString buildPrefix;
    bool building;
    uint itemData0;
    uint buildContext0Offset;
    YzisHlIncludeRules includeRules;
    bool m_foldingIndentationSensitive;

#if QT_VERSION < 0x040000
    QIntDict< QMemArray<YzisAttribute> > m_attributeArrays;
#else
    QHash<int, QVector<YzisAttribute>* > m_attributeArrays;
#endif

    /**
     * This class holds the additional properties for one highlight
     * definition, such as comment strings, deliminators etc.
     *
     * When a highlight is added, a instance of this class is appended to
     * m_additionalData, and the current position in the attrib and context
     * arrays are stored in the indexes for look up. You can then use
     * hlKeyForAttrib or hlKeyForContext to find the relevant instance of this
     * class from m_additionalData.
     *
     * If you need to add a property to a highlight, add it here.
     */
    class HighlightPropertyBag {
      public:
        QString singleLineCommentMarker;
        QString multiLineCommentStart;
        QString multiLineCommentEnd;
        QString multiLineRegion;
        QString deliminator;
        QString wordWrapDeliminator;
    };

    /**
     * Highlight properties for each included highlight definition.
     * The key is the identifier
     */
#if QT_VERSION < 0x040000
    QDict<HighlightPropertyBag> m_additionalData;
#else
    QHash<QString,HighlightPropertyBag*> m_additionalData;
#endif

    /**
     * Fast lookup of hl properties, based on attribute index
     * The key is the starting index in the attribute array for each file.
     * @see hlKeyForAttrib
     */
    QMap<int, QString> m_hlIndex;


    QString extensionSource;
#if QT_VERSION < 0x040000
    QValueList<QRegExp> regexpExtensions;
#else
    QList<QRegExp> regexpExtensions;
#endif
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

#if QT_VERSION < 0x040000
    int findHl(YzisHighlighting *h) {return hlList.find(h);}
#else
    int findHl(YzisHighlighting *h) {return hlList.indexOf(h);}
#endif
    QString identifierForName(const QString&);

    // methodes to get the default style count + names
    static uint defaultStyles();
    static QString defaultStyleName(int n, bool translateNames = false);

    void getDefaults(uint schema, YzisAttributeList &);
    void setDefaults(uint schema, YzisAttributeList &);

    int highlights();
    QString hlName(int n);
    QString hlNameTranslated (int n);
    QString hlSection(int n);
    bool hlHidden(int n);

    void incDynamicCtxs() { ++dynamicCtxsCount; };
    uint countDynamicCtxs() { return dynamicCtxsCount; };
    void setForceNoDCReset(bool b) { forceNoDCReset = b; };

    // be carefull: all documents hl should be invalidated after having successfully called this method!
    bool resetDynamicCtxs();

  signals:
    void changed();

  private:
    int wildcardFind(const QString &fileName);
    int mimeFind(const QString &contents);
//    int mimeFind(const QByteArray &contents);
    int realWildcardFind(const QString &fileName);
	QString findByContent( const QString& contents );
//	QString findByContent( const QByteArray& contents );

  private:
    friend class YzisHighlighting;

#if QT_VERSION < 0x040000
    QPtrList<YzisHighlighting> hlList;
    QDict<YzisHighlighting> hlDict;
#else
    QList<YzisHighlighting*> hlList;
    QHash<QString,YzisHighlighting*> hlDict;
#endif

    static YzisHlManager *s_self;

    //KConfig m_config;
    QStringList commonSuffixes;

    YzisSyntaxDocument *syntax;
    uint dynamicCtxsCount;
    QTime lastCtxsReset;
    bool forceNoDCReset;
    magic_t magicSet;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
