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

#ifndef __YZ_SYNTAXHIGHLIGHT_H__
#define __YZ_SYNTAXHIGHLIGHT_H__

#include <qdom.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qapplication.h>
#include <qintdict.h>
#include <qobject.h>
#include <qdict.h>
#include "syntaxdocument.h"
#include "attribute.h"
#include "line.h"
#include <qregexp.h>

class YzisHlContext;

//Item Properties: name, Item Style, Item Font
class YzisHlItemData : public YzisAttribute
{
  public:
    YzisHlItemData(const QString name, int defStyleNum);

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


/**
 * Read and interpret the syntax file document to create HL structures
 */
class YzisSyntaxHighlight 
{
  public:
    /**
     * Constructor
     */
    YzisSyntaxHighlight();

    /**
     * Desctructor
     */
    ~YzisSyntaxHighlight();

    QIntDict<YzisHlContext> contextList;
	inline YzisHlContext *contextNum (uint n) { return contextList[n]; }
	void highlight( YZLine *prevLine, YZLine *textLine, QMemArray<signed char> foldingList, bool *ctxChanged);
    void generateContextStack(int *ctxNum, int ctx, QMemArray<short> *ctxs, int *posPrevLine,bool lineContinue=false);
	
  private:
	bool noHl;
	YzisSyntaxDocument *mSdoc;
};

class YzisHlItem
{
  public:
    YzisHlItem(int attribute, int context,signed char regionId, signed char regionId2);
    virtual ~YzisHlItem();

  public:
    virtual bool alwaysStartEnable() const { return true; };
    virtual bool hasCustomStartEnable() const { return false; };
    virtual bool startEnable(const QChar&);

    // Changed from using QChar*, because it makes the regular expression check very
    // inefficient (forces it to copy the string, very bad for long strings)
    // Now, the function returns the offset detected, or 0 if no match is found.
    // bool linestart isn't needed, this is equivalent to offset == 0.
    virtual int checkHgl(const QString& text, int offset, int len) = 0;

    virtual bool lineContinue(){return false;}

    QPtrList<YzisHlItem> *subItems;
    int attr;
    int ctx;
    signed char region;
    signed char region2;

    bool lookAhead;
};

class YzisHlContext
{
  public:
    YzisHlContext (int attribute, int lineEndContext,int _lineBeginContext, bool _fallthrough, int _fallthroughContext);

    QPtrList<YzisHlItem> items;
    int attr;
    int ctx;
    int lineBeginContext;
    bool fallthrough;
    int ftctx; // where to go after no rules matched
};

class YzisEmbeddedHlInfo
{
  public:
    YzisEmbeddedHlInfo() {loaded=false;context0=-1;}
    YzisEmbeddedHlInfo(bool l, int ctx0) {loaded=l;context0=ctx0;}

  public:
    bool loaded;
    int context0;
};

class YzisHlIncludeRule
{
  public:
    YzisHlIncludeRule(int ctx_, uint pos_, const QString &incCtxN_) {ctx=ctx_;pos=pos_;incCtxN=incCtxN_;incCtx=-1;}
    YzisHlIncludeRule(int ctx_, uint  pos_) {ctx=ctx_;pos=pos_;incCtx=-1;incCtxN="";}

  public:
    uint pos;
    int ctx;
    int incCtx;
    QString incCtxN;
};

class YzisHlCharDetect : public YzisHlItem
{
  public:
    YzisHlCharDetect(int attribute, int context,signed char regionId,signed char regionId2, QChar);
    virtual int checkHgl(const QString& text, int offset, int len);

  private:
    QChar sChar;
};

class YzisHl2CharDetect : public YzisHlItem
{
  public:
    YzisHl2CharDetect(int attribute, int context, signed char regionId,signed char regionId2,  QChar ch1, QChar ch2);
    YzisHl2CharDetect(int attribute, int context,signed char regionId,signed char regionId2,  const QChar *ch);

    virtual int checkHgl(const QString& text, int offset, int len);

  private:
    QChar sChar1;
    QChar sChar2;
};

class YzisHlStringDetect : public YzisHlItem
{
  public:
    YzisHlStringDetect(int attribute, int context, signed char regionId,signed char regionId2, const QString &, bool inSensitive=false);

    virtual int checkHgl(const QString& text, int offset, int len);

  private:
    const QString str;
    bool _inSensitive;
};

class YzisHlRangeDetect : public YzisHlItem
{
  public:
    YzisHlRangeDetect(int attribute, int context, signed char regionId,signed char regionId2, QChar ch1, QChar ch2);

    virtual int checkHgl(const QString& text, int offset, int len);

  private:
    QChar sChar1;
    QChar sChar2;
};

class YzisHlKeyword : public YzisHlItem
{
  public:
    YzisHlKeyword(int attribute, int context,signed char regionId,signed char regionId2, bool casesensitive, const QString& delims);

    virtual void addWord(const QString &);
    virtual void addList(const QStringList &);
    virtual int checkHgl(const QString& text, int offset, int len);
    virtual bool startEnable(const QChar& c);
    virtual bool alwaysStartEnable() const;
    virtual bool hasCustomStartEnable() const;

  private:
    QDict<bool> dict;
    bool _caseSensitive;
    const QString& deliminators;
};

class YzisHlInt : public YzisHlItem
{
  public:
    YzisHlInt(int attribute, int context, signed char regionId,signed char regionId2);

    virtual int checkHgl(const QString& text, int offset, int len);
    virtual bool alwaysStartEnable() const;
};

class YzisHlFloat : public YzisHlItem
{
  public:
    YzisHlFloat(int attribute, int context, signed char regionId,signed char regionId2);
    virtual ~YzisHlFloat () {}

    virtual int checkHgl(const QString& text, int offset, int len);
    virtual bool alwaysStartEnable() const;
};

class YzisHlCFloat : public YzisHlFloat
{
  public:
    YzisHlCFloat(int attribute, int context, signed char regionId,signed char regionId2);

    virtual int checkHgl(const QString& text, int offset, int len);
    int checkIntHgl(const QString& text, int offset, int len);
    virtual bool alwaysStartEnable() const;
};

class YzisHlCOct : public YzisHlItem
{
  public:
    YzisHlCOct(int attribute, int context, signed char regionId,signed char regionId2);

    virtual int checkHgl(const QString& text, int offset, int len);
    virtual bool alwaysStartEnable() const;
};

class YzisHlCHex : public YzisHlItem
{
  public:
    YzisHlCHex(int attribute, int context, signed char regionId,signed char regionId2);

    virtual int checkHgl(const QString& text, int offset, int len);
    virtual bool alwaysStartEnable() const;
};

class YzisHlLineContinue : public YzisHlItem
{
  public:
    YzisHlLineContinue(int attribute, int context, signed char regionId,signed char regionId2);

    virtual bool endEnable(QChar c) {return c == '\0';}
    virtual int checkHgl(const QString& text, int offset, int len);
    virtual bool lineContinue(){return true;}
};

class YzisHlCStringChar : public YzisHlItem
{
  public:
    YzisHlCStringChar(int attribute, int context, signed char regionId,signed char regionId2);

    virtual int checkHgl(const QString& text, int offset, int len);
};

class YzisHlCChar : public YzisHlItem
{
  public:
    YzisHlCChar(int attribute, int context,signed char regionId,signed char regionId2);

    virtual int checkHgl(const QString& text, int offset, int len);
};

class YzisHlAnyChar : public YzisHlItem
{
  public:
    YzisHlAnyChar(int attribute, int context, signed char regionId,signed char regionId2, const QString& charList);

    virtual int checkHgl(const QString& text, int offset, int len);

  private:
    const QString _charList;
};

class YzisHlRegExpr : public YzisHlItem
{
  public:
    YzisHlRegExpr(int attribute, int context,signed char regionId,signed char regionId2 ,QString expr, bool insensitive, bool minimal);
    ~YzisHlRegExpr() { delete Expr; };

    virtual int checkHgl(const QString& text, int offset, int len);

  private:
    QRegExp *Expr;
    bool handlesLinestart;
};


#endif
