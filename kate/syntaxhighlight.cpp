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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

/* This file was taken from the Kate editor which is part of KDE
   Kate's code is published under the LGPL version 2 (and 2 only not any later 
   version)
   Copyright (C) 2003, 2004 Anders Lund <anders@alweb.dk>
   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2001,2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>
*/

//BEGIN INCLUDES
#include "syntaxhighlight.h"

#include "line.h"
#include "syntaxdocument.h"
#include "debug.h"
#include "buffer.h"
#include "view.h"
#include "internal_options.h"
#include "schema.h"
#include "session.h"
#include "resourcemgr.h"


#include "luaengine.h"
#include <QDir>
#include <QSet>
#include <QTextStream>
//END

//BEGIN defines
// same as in kmimemagic, no need to feed more data
#define YZIS_HL_HOWMANY 1024

// min. x seconds between two dynamic contexts reset
static const int YZIS_DYNAMIC_CONTEXTS_RESET_DELAY = 30 * 1000;

// x is a QString. if x is "true" or "1" this expression returns "true"
#define IS_TRUE(x) x.toLower() == QString("true") || x.toInt() == 1
//END defines

//BEGIN  Prviate HL classes

inline bool yzisInsideString (const QString &str, QChar ch)
{
  for (int i=0; i < str.length(); i++)
    if (*(str.unicode()+i) == ch)
      return true;

  return false;
}

class YzisHlItem
{
  public:
    YzisHlItem(int attribute, int context,signed char regionId, signed char regionId2);
    virtual ~YzisHlItem();

  public:
    // caller must keep in mind: LEN > 0 is a must !!!!!!!!!!!!!!!!!!!!!1
    // Now, the function returns the offset detected, or 0 if no match is found.
    // bool linestart isn't needed, this is equivalent to offset == 0.
    virtual int checkHgl(const QString& text, int offset, int len) = 0;

    virtual bool lineContinue() const {return false;}

    virtual QStringList *capturedTexts() {return 0;}
    virtual YzisHlItem *clone(const QStringList *) {return this;}

    static void dynamicSubstitute(QString& str, const QStringList *args);

    QVector<YzisHlItem*> subItems;
    int attr;
    int ctx;
    signed char region;
    signed char region2;

    bool lookAhead;

    bool dynamic;
    bool dynamicChild;
    bool firstNonSpace;
    bool onlyConsume;
    int column;

    // start enable flags, nicer than the virtual methodes
    // saves function calls
    bool alwaysStartEnable;
    bool customStartEnable;
};

class YzisHlContext
{
  public:
    YzisHlContext(const QString &_hlId, int attribute, int lineEndContext,int _lineBeginContext,
                  bool _fallthrough, int _fallthroughContext, bool _dynamic,bool _noIndentationBasedFolding);
    virtual ~YzisHlContext();
    YzisHlContext *clone(const QStringList *args);

    QVector<YzisHlItem*> items;
    QString hlId; ///< A unique highlight identifier. Used to look up correct properties.
    int attr;
    int ctx;
    int lineBeginContext;
    /** @internal anders: possible escape if no rules matches.
       false unless 'fallthrough="1|true"' (insensitive)
       if true, go to ftcxt w/o eating of string.
       ftctx is "fallthroughContext" in xml files, valid values are int or #pop[..]
       see in YzisHighlighting::doHighlight */
    bool fallthrough;
    int ftctx; // where to go after no rules matched

    bool dynamic;
    bool dynamicChild;
    bool noIndentationBasedFolding;
};

class YzisHlIncludeRule
{
  public:
    YzisHlIncludeRule(int ctx_=0, uint pos_=0, const QString &incCtxN_="", bool incAttrib=false)
      : ctx(ctx_)
      , pos( pos_)
      , incCtxN( incCtxN_ )
      , includeAttrib( incAttrib )
    {
      incCtx=-1;
    }
    //YzisHlIncludeRule(int ctx_, uint  pos_, bool incAttrib) {ctx=ctx_;pos=pos_;incCtx=-1;incCtxN="";includeAttrib=incAttrib}

  public:
    int ctx;
    uint pos;
    int incCtx;
    QString incCtxN;
    bool includeAttrib;
};

class YzisHlCharDetect : public YzisHlItem
{
  public:
    YzisHlCharDetect(int attribute, int context,signed char regionId,signed char regionId2, QChar);

    virtual int checkHgl(const QString& text, int offset, int len);
    virtual YzisHlItem* clone( const QStringList *args );

  private:
    QChar sChar;
};

class YzisHl2CharDetect : public YzisHlItem
{
  public:
    YzisHl2CharDetect(int attribute, int context, signed char regionId,signed char regionId2,  QChar ch1, QChar ch2);
    YzisHl2CharDetect(int attribute, int context,signed char regionId,signed char regionId2,  const QChar *ch);

    virtual int checkHgl(const QString& text, int offset, int len);
    virtual YzisHlItem* clone( const QStringList *args );

  private:
    QChar sChar1;
    QChar sChar2;
};

class YzisHlStringDetect : public YzisHlItem
{
  public:
    YzisHlStringDetect(int attribute, int context, signed char regionId,signed char regionId2, const QString &, bool inSensitive=false);

    virtual int checkHgl(const QString& text, int offset, int len);
    virtual YzisHlItem *clone( const QStringList *argS );

  private:
    const QString str;
    const int strLen;
    const bool _inSensitive;
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
    virtual ~YzisHlKeyword ();

    void addList(const QStringList &);
    virtual int checkHgl(const QString& text, int offset, int len);

  private:
    QVector< QSet<QString>* > dict;
    bool _caseSensitive;
    const QString& deliminators;
    int minLen;
    int maxLen;
};

class YzisHlInt : public YzisHlItem
{
  public:
    YzisHlInt(int attribute, int context, signed char regionId,signed char regionId2);

    virtual int checkHgl(const QString& text, int offset, int len);
};

class YzisHlFloat : public YzisHlItem
{
  public:
    YzisHlFloat(int attribute, int context, signed char regionId,signed char regionId2);
    virtual ~YzisHlFloat () {}

    virtual int checkHgl(const QString& text, int offset, int len);
};

class YzisHlCFloat : public YzisHlFloat
{
  public:
    YzisHlCFloat(int attribute, int context, signed char regionId,signed char regionId2);

    virtual int checkHgl(const QString& text, int offset, int len);
    int checkIntHgl(const QString& text, int offset, int len);
};

class YzisHlCOct : public YzisHlItem
{
  public:
    YzisHlCOct(int attribute, int context, signed char regionId,signed char regionId2);

    virtual int checkHgl(const QString& text, int offset, int len);
};

class YzisHlCHex : public YzisHlItem
{
  public:
    YzisHlCHex(int attribute, int context, signed char regionId,signed char regionId2);

    virtual int checkHgl(const QString& text, int offset, int len);
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
    ~YzisHlRegExpr() { delete Expr; }

    virtual int checkHgl(const QString& text, int offset, int len);
    virtual QStringList *capturedTexts();
    virtual YzisHlItem *clone( const QStringList *args );

  private:
    QRegExp *Expr;
    bool handlesLinestart;
    QString _regexp;
    bool _insensitive;
    bool _minimal;
};

class YzisHlDetectSpaces : public YzisHlItem
{
  public:
    YzisHlDetectSpaces (int attribute, int context,signed char regionId,signed char regionId2)
      : YzisHlItem(attribute,context,regionId,regionId2) {}

    virtual int checkHgl(const QString& text, int offset, int len)
    {
      int len2 = offset + len;
      while ((offset < len2) && text[offset].isSpace()) offset++;
      return offset;
    }
};

class YzisHlDetectIdentifier : public YzisHlItem
{
  public:
    YzisHlDetectIdentifier (int attribute, int context,signed char regionId,signed char regionId2)
      : YzisHlItem(attribute,context,regionId,regionId2) { alwaysStartEnable = false; }

    virtual int checkHgl(const QString& text, int offset, int len)
    {
      // first char should be a letter or underscore
      if ( text[offset].isLetter() || text[offset] == QChar ('_') )
      {
        // memorize length
        int len2 = offset+len;

        // one char seen
        offset++;

        // now loop for all other thingies
        while (
               (offset < len2)
               && (text[offset].isLetterOrNumber() || (text[offset] == QChar ('_')))
              )
          offset++;

        return offset;
      }

      return 0;
    }
};

//END

//BEGIN STATICS
YzisHlManager *YzisHlManager::s_self = 0;

static const bool trueBool = true;
static const QString stdDeliminator = QString (" \t.():!+,-<=>%&*/;?[]^{|}~\\");
//END

//BEGIN NON MEMBER FUNCTIONS
static YzisHlItemData::ItemStyles getDefStyleNum(QString name)
{
  if (name=="dsNormal") return YzisHlItemData::dsNormal;
  else if (name=="dsKeyword") return YzisHlItemData::dsKeyword;
  else if (name=="dsDataType") return YzisHlItemData::dsDataType;
  else if (name=="dsDecVal") return YzisHlItemData::dsDecVal;
  else if (name=="dsBaseN") return YzisHlItemData::dsBaseN;
  else if (name=="dsFloat") return YzisHlItemData::dsFloat;
  else if (name=="dsChar") return YzisHlItemData::dsChar;
  else if (name=="dsString") return YzisHlItemData::dsString;
  else if (name=="dsComment") return YzisHlItemData::dsComment;
  else if (name=="dsOthers")  return YzisHlItemData::dsOthers;
  else if (name=="dsAlert") return YzisHlItemData::dsAlert;
  else if (name=="dsFunction") return YzisHlItemData::dsFunction;
  else if (name=="dsRegionMarker") return YzisHlItemData::dsRegionMarker;
  else if (name=="dsError") return YzisHlItemData::dsError;

  return YzisHlItemData::dsNormal;
}
//END

//BEGIN YzisHlItem
YzisHlItem::YzisHlItem(int attribute, int context,signed char regionId,signed char regionId2)
  : attr(attribute),
    ctx(context),
    region(regionId),
    region2(regionId2),
    lookAhead(false),
    dynamic(false),
    dynamicChild(false),
    firstNonSpace(false),
    onlyConsume(false),
    column (-1),
    alwaysStartEnable (true),
    customStartEnable (false)
{
}

YzisHlItem::~YzisHlItem()
{
  //deepdbg()<<"In hlItem::~YzisHlItem()"<<endl;
  for (int i=0; i < subItems.size(); i++)
    delete subItems[i];
}

void YzisHlItem::dynamicSubstitute(QString &str, const QStringList *args)
{
  for (int i = 0; i < str.length() - 1; ++i)
  {
    if (str[i] == '%')
    {
      char c = str[i + 1].toLatin1();
      if (c == '%')
        str.replace(i, 1, "");
      else if (c >= '0' && c <= '9')
      {
        if ((int)(c - '0') < args->size())
        {
          str.replace(i, 2, (*args)[c - '0']);
          i += ((*args)[c - '0']).length() - 1;
        }
        else
        {
          str.replace(i, 2, "");
          --i;
        }
      }
    }
  }
}
//END

//BEGIN YzisHLCharDetect
YzisHlCharDetect::YzisHlCharDetect(int attribute, int context, signed char regionId,signed char regionId2, QChar c)
  : YzisHlItem(attribute,context,regionId,regionId2)
  , sChar(c)
{
}

int YzisHlCharDetect::checkHgl(const QString& text, int offset, int /*len*/)
{
  if (text[offset] == sChar)
    return offset + 1;

  return 0;
}

YzisHlItem *YzisHlCharDetect::clone(const QStringList *args)
{
  char c = sChar.toLatin1();

  if (c < '0' || c > '9' || (c - '0') >= args->size())
    return this;

  YzisHlCharDetect *ret = new YzisHlCharDetect(attr, ctx, region, region2, (*args)[c - '0'][0]);
  ret->dynamicChild = true;
  return ret;
}
//END

//BEGIN YzisHl2CharDetect
YzisHl2CharDetect::YzisHl2CharDetect(int attribute, int context, signed char regionId,signed char regionId2, QChar ch1, QChar ch2)
  : YzisHlItem(attribute,context,regionId,regionId2)
  , sChar1 (ch1)
  , sChar2 (ch2)
{
}

int YzisHl2CharDetect::checkHgl(const QString& text, int offset, int len)
{
  if ((len >= 2) && text[offset++] == sChar1 && text[offset++] == sChar2)
    return offset;

  return 0;
}

YzisHlItem *YzisHl2CharDetect::clone(const QStringList *args)
{
  char c1 = sChar1.toLatin1();
  char c2 = sChar2.toLatin1();

  if (c1 < '0' || c1 > '9' || (c1 - '0') >= args->size())
    return this;

  if (c2 < '0' || c2 > '9' || (c2 - '0') >= args->size())
    return this;

  YzisHl2CharDetect *ret = new YzisHl2CharDetect(attr, ctx, region, region2, (*args)[c1 - '0'][0], (*args)[c2 - '0'][0]);
  ret->dynamicChild = true;
  return ret;
}
//END

//BEGIN YzisHlStringDetect
YzisHlStringDetect::YzisHlStringDetect(int attribute, int context, signed char regionId,signed char regionId2,const QString &s, bool inSensitive)
  : YzisHlItem(attribute, context,regionId,regionId2)
  , str(inSensitive ? s.toUpper() : s)
  , strLen (str.length())
  , _inSensitive(inSensitive)
{
}

int YzisHlStringDetect::checkHgl(const QString& text, int offset, int len)
{
  if (len < strLen)
    return 0;

  if (_inSensitive)
  {
    for (int i=0; i < strLen; i++)
      if (text[offset++].toUpper() != str[i])
        return 0;

    return offset;
  }
  else
  {
    for (int i=0; i < strLen; i++)
      if (text[offset++] != str[i])
        return 0;

    return offset;
  }

  return 0;
}

YzisHlItem *YzisHlStringDetect::clone(const QStringList *args)
{
  QString newstr = str;

  dynamicSubstitute(newstr, args);

  if (newstr == str)
    return this;

  YzisHlStringDetect *ret = new YzisHlStringDetect(attr, ctx, region, region2, newstr, _inSensitive);
  ret->dynamicChild = true;
  return ret;
}
//END

//BEGIN YzisHLRangeDetect
YzisHlRangeDetect::YzisHlRangeDetect(int attribute, int context, signed char regionId,signed char regionId2, QChar ch1, QChar ch2)
  : YzisHlItem(attribute,context,regionId,regionId2)
  , sChar1 (ch1)
  , sChar2 (ch2)
{
}

int YzisHlRangeDetect::checkHgl(const QString& text, int offset, int len)
{
  if (text[offset] == sChar1)
  {
    do
    {
      offset++;
      len--;
      if (len < 1) return 0;
    }
    while (text[offset] != sChar2);

    return offset + 1;
  }
  return 0;
}
//END

//BEGIN YzisHlKeyword
YzisHlKeyword::YzisHlKeyword (int attribute, int context, signed char regionId,signed char regionId2, bool casesensitive, const QString& delims)
  : YzisHlItem(attribute,context,regionId,regionId2)
  , _caseSensitive(casesensitive)
  , deliminators(delims)
  , minLen (0xFFFFFF)
  , maxLen (0)
{
  alwaysStartEnable = false;
  customStartEnable = true;
}

YzisHlKeyword::~YzisHlKeyword ()
{
  for (int i=0; i < dict.size(); ++i)
    delete dict[i];
}

void YzisHlKeyword::addList(const QStringList& list)
{
  for(int i=0; i < list.count(); ++i)
  {
    int len = list[i].length();

    if (minLen > len)
      minLen = len;

    if (maxLen < len)
      maxLen = len;

    if (len >= dict.size())
    {
      uint oldSize = dict.size();
      dict.resize (len+1);

      for (int m=oldSize; m < dict.size(); ++m)
        dict[m] = 0;
    }

    if (!dict[len])
      dict[len] = new QSet<QString> ();

	if ( _caseSensitive )
		dict[ len ]->insert( list[ i ] );
	else
		dict[ len ]->insert( list[ i ].toLower() );
  }
}

int YzisHlKeyword::checkHgl(const QString& text, int offset, int len)
{
  int offset2 = offset;
  int wordLen = 0;

  while ((len > wordLen) && !yzisInsideString (deliminators, text[offset2]))
  {
    offset2++;
    wordLen++;

    if (wordLen > maxLen) return 0;
  }

  if ( wordLen < minLen || !dict[ wordLen ] ) return 0;

  if ( _caseSensitive )
  {
	  if ( dict[ wordLen ]->contains( QString( text.unicode() + offset, wordLen ) ) )
		  return offset2;
  }
  else
  {
	  if ( dict[ wordLen ]->contains( QString( text.unicode() + offset, wordLen ).toLower() ) )
		  return offset2;
  }

  return 0;
}
//END

//BEGIN YzisHlInt
YzisHlInt::YzisHlInt(int attribute, int context, signed char regionId,signed char regionId2) 
    : YzisHlItem(attribute,context,regionId,regionId2) 
{
    alwaysStartEnable = false;
}

int YzisHlInt::checkHgl(const QString& text, int offset, int len)
{
  int offset2 = offset;

  while ((len > 0) && text[offset2].isDigit())
  {
    offset2++;
    len--;
  }

  if (offset2 > offset)
  {
    if (len > 0)
    {
      for (int i=0; i < subItems.size(); i++)
      {
        if ( (offset = subItems[i]->checkHgl(text, offset2, len)) )
          return offset;
      }
    }

    return offset2;
  }

  return 0;
}
//END

//BEGIN YzisHlFloat
YzisHlFloat::YzisHlFloat(int attribute, int context, signed char regionId,signed char regionId2)
  : YzisHlItem(attribute,context, regionId,regionId2)
{
  alwaysStartEnable = false;
}

int YzisHlFloat::checkHgl(const QString& text, int offset, int len)
{
  bool b = false;
  bool p = false;

  while ((len > 0) && text[offset].isDigit())
  {
    offset++;
    len--;
    b = true;
  }

  if ((len > 0) && (p = (text[offset] == '.')))
  {
    offset++;
    len--;

    while ((len > 0) && text[offset].isDigit())
    {
      offset++;
      len--;
      b = true;
    }
  }

  if (!b)
    return 0;

  if ((len > 0) && (text[offset].toAscii() == 'E'))
  {
    offset++;
    len--;
  }
  else
  {
    if (!p)
      return 0;
    else
    {
      if (len > 0)
      {
        for (int i=0; i < subItems.size(); i++)
        {
          int offset2 = subItems[i]->checkHgl(text, offset, len);

          if (offset2)
            return offset2;
        }
      }

      return offset;
    }
  }

  if ((len > 0) && (text[offset] == '-' || text[offset] =='+'))
  {
    offset++;
    len--;
  }

  b = false;

  while ((len > 0) && text[offset].isDigit())
  {
    offset++;
    len--;
    b = true;
  }

  if (b)
  {
    if (len > 0)
    {
      for (int i=0; i < subItems.size(); i++)
      {
        int offset2 = subItems[i]->checkHgl(text, offset, len);

        if (offset2)
          return offset2;
      }
    }

    return offset;
  }

  return 0;
}
//END

//BEGIN YzisHlCOct
YzisHlCOct::YzisHlCOct(int attribute, int context, signed char regionId,signed char regionId2)
  : YzisHlItem(attribute,context,regionId,regionId2)
{
  alwaysStartEnable = false;
}

int YzisHlCOct::checkHgl(const QString& text, int offset, int len)
{
  if (text[offset].toAscii() == '0')
  {
    offset++;
    len--;

    int offset2 = offset;

    while ((len > 0) && (text[offset2].toAscii() >= '0' && text[offset2].toAscii() <= '7'))
    {
      offset2++;
      len--;
    }

    if (offset2 > offset)
    {
      if ((len > 0) && (text[offset2].toAscii() == 'L' || text[offset].toAscii() == 'U' ))
        offset2++;

      return offset2;
    }
  }

  return 0;
}
//END

//BEGIN YzisHlCHex
YzisHlCHex::YzisHlCHex(int attribute, int context,signed char regionId,signed char regionId2)
  : YzisHlItem(attribute,context,regionId,regionId2)
{
  alwaysStartEnable = false;
}

int YzisHlCHex::checkHgl(const QString& text, int offset, int len)
{
  if ((len > 1) && (text[offset++].toAscii() == '0') && (text[offset++].toAscii() == 'X' ))
  {
    len -= 2;

    int offset2 = offset;

    while ((len > 0) && (text[offset2].isDigit() || (text[offset2].toAscii() >= 'A' && text[offset2].toAscii() <= 'F')))
    {
      offset2++;
      len--;
    }

    if (offset2 > offset)
    {
      if ((len > 0) && (text[offset2].toAscii() == 'L' || text[offset2].toAscii() == 'U' ))
        offset2++;

      return offset2;
    }
  }

  return 0;
}
//END

//BEGIN YzisHlCFloat
YzisHlCFloat::YzisHlCFloat(int attribute, int context, signed char regionId,signed char regionId2)
  : YzisHlFloat(attribute,context,regionId,regionId2)
{
  alwaysStartEnable = false;
}

int YzisHlCFloat::checkIntHgl(const QString& text, int offset, int len)
{
  int offset2 = offset;

  while ((len > 0) && text[offset].isDigit()) {
    offset2++;
    len--;
  }

  if (offset2 > offset)
     return offset2;

  return 0;
}

int YzisHlCFloat::checkHgl(const QString& text, int offset, int len)
{
  int offset2 = YzisHlFloat::checkHgl(text, offset, len);

  if (offset2)
  {
    if (text[offset2].toAscii() == 'F' )
      offset2++;

    return offset2;
  }
  else
  {
    offset2 = checkIntHgl(text, offset, len);

    if (offset2 && (text[offset2].toAscii() == 'F' ))
      return ++offset2;
    else
      return 0;
  }
}
//END

//BEGIN YzisHlAnyChar
YzisHlAnyChar::YzisHlAnyChar(int attribute, int context, signed char regionId,signed char regionId2, const QString& charList)
  : YzisHlItem(attribute, context,regionId,regionId2)
  , _charList(charList)
{
}

int YzisHlAnyChar::checkHgl(const QString& text, int offset, int)
{
  if (yzisInsideString (_charList, text[offset]))
    return ++offset;

  return 0;
}
//END

//BEGIN YzisHlRegExpr
YzisHlRegExpr::YzisHlRegExpr( int attribute, int context, signed char regionId,signed char regionId2, QString regexp, bool insensitive, bool minimal)
  : YzisHlItem(attribute, context, regionId,regionId2)
  , handlesLinestart (regexp.startsWith("^"))
  , _regexp(regexp)
  , _insensitive(insensitive)
  , _minimal(minimal)
{
  if (!handlesLinestart)
    regexp.prepend("^");

  Expr = new QRegExp(regexp, _insensitive ? Qt::CaseInsensitive : Qt::CaseSensitive );
  Expr->setMinimal(_minimal);
}

int YzisHlRegExpr::checkHgl(const QString& text, int offset, int /*len*/)
{
  if (offset && handlesLinestart)
    return 0;

  int offset2 = Expr->indexIn( text, offset, QRegExp::CaretAtOffset );

  if (offset2 == -1) return 0;

  return (offset + Expr->matchedLength());
}

QStringList *YzisHlRegExpr::capturedTexts()
{
  return new QStringList(Expr->capturedTexts());
}

YzisHlItem *YzisHlRegExpr::clone(const QStringList *args)
{
  QString regexp = _regexp;
  QStringList escArgs = *args;
  
  QStringList::Iterator it = escArgs.begin(), end = escArgs.end();
  for (; it != end; ++it)
  {
    (*it).replace(QRegExp("(\\W)"), "\\\\1");
  }

  dynamicSubstitute(regexp, &escArgs);

  if (regexp == _regexp)
    return this;

  // kdDebug (HL) << "clone regexp: " << regexp << endl;

  YzisHlRegExpr *ret = new YzisHlRegExpr(attr, ctx, region, region2, regexp, _insensitive, _minimal);
  ret->dynamicChild = true;
  return ret;
}
//END

//BEGIN YzisHlLineContinue
YzisHlLineContinue::YzisHlLineContinue(int attribute, int context, signed char regionId,signed char regionId2)
  : YzisHlItem(attribute,context,regionId,regionId2) {
}

int YzisHlLineContinue::checkHgl(const QString& text, int offset, int len)
{
  if ((len == 1) && (text[offset] == '\\'))
    return ++offset;

  return 0;
}
//END

//BEGIN YzisHlCStringChar
YzisHlCStringChar::YzisHlCStringChar(int attribute, int context,signed char regionId,signed char regionId2)
  : YzisHlItem(attribute,context,regionId,regionId2) {
}

// checks for C escaped chars \n and escaped hex/octal chars
static int checkEscapedChar(const QString& text, int offset, int& len)
{
  int i;
  if (text[offset] == '\\' && len > 1)
  {
    offset++;
    len--;

    switch(text[offset].toAscii())
    {
      case  'a': // checks for control chars
      case  'b': // we want to fall through
      case  'e':
      case  'f':

      case  'n':
      case  'r':
      case  't':
      case  'v':
      case '\'':
      case '\"':
      case '?' : // added ? ANSI C classifies this as an escaped char
      case '\\':
        offset++;
        len--;
        break;

      case 'x': // if it's like \xff
        offset++; // eat the x
        len--;
        // these for loops can probably be
        // replaced with something else but
        // for right now they work
        // check for hexdigits
        for (i = 0; (len > 0) && (i < 2) && (text[offset].toAscii() >= '0' && text[offset].toAscii() <= '9' || text[offset].toAscii() >= 'A' && text[offset].toAscii() <= 'F'); i++)
        {
          offset++;
          len--;
        }

        if (i == 0)
          return 0; // takes care of case '\x'

        break;

      case '0': case '1': case '2': case '3' :
      case '4': case '5': case '6': case '7' :
        for (i = 0; (len > 0) && (i < 3) && (text[offset].toAscii() >='0'&& text[offset].toAscii() <='7'); i++)
        {
          offset++;
          len--;
        }
        break;

      default:
        return 0;
    }

    return offset;
  }

  return 0;
}

int YzisHlCStringChar::checkHgl(const QString& text, int offset, int len)
{
  return checkEscapedChar(text, offset, len);
}
//END

//BEGIN YzisHlCChar
YzisHlCChar::YzisHlCChar(int attribute, int context,signed char regionId,signed char regionId2)
  : YzisHlItem(attribute,context,regionId,regionId2) {
}

int YzisHlCChar::checkHgl(const QString& text, int offset, int len)
{
  if ((len > 1) && (text[offset] == '\'') && (text[offset+1] != '\''))
  {
    int oldl;
    oldl = len;

    len--;

    int offset2 = checkEscapedChar(text, offset + 1, len);

    if (!offset2)
    {
      if (oldl > 2)
      {
        offset2 = offset + 2;
        len = oldl - 2;
      }
      else
      {
        return 0;
      }
    }

    if ((len > 0) && (text[offset2] == '\''))
      return ++offset2;
  }

  return 0;
}
//END

//BEGIN YzisHl2CharDetect
YzisHl2CharDetect::YzisHl2CharDetect(int attribute, int context, signed char regionId,signed char regionId2, const QChar *s)
  : YzisHlItem(attribute,context,regionId,regionId2) {
  sChar1 = s[0];
  sChar2 = s[1];
  }
//END YzisHl2CharDetect

YzisHlItemData::YzisHlItemData(const QString  name, int defStyleNum)
  : name(name), defStyleNum(defStyleNum) {
}

YzisHlData::YzisHlData(const QString &wildcards, const QString &mimetypes, const QString &identifier, int priority)
  : wildcards(wildcards), mimetypes(mimetypes), identifier(identifier), priority(priority)
{
}

//BEGIN YzisHlContext

#define deepdbg()    yzDebug("YzisHlContext")
#define dbg()    yzDebug("YzisHlContext")
#define err()    yzError("YzisHlContext")

YzisHlContext::YzisHlContext (const QString &_hlId, int attribute, int lineEndContext, int _lineBeginContext, bool _fallthrough, 
	int _fallthroughContext, bool _dynamic, bool _noIndentationBasedFolding)
{
  hlId = _hlId;
  attr = attribute;
  ctx = lineEndContext;
  lineBeginContext = _lineBeginContext;
  fallthrough = _fallthrough;
  ftctx = _fallthroughContext;
  dynamic = _dynamic;
  dynamicChild = false;
  noIndentationBasedFolding=_noIndentationBasedFolding;
  if ( _noIndentationBasedFolding ) dbg()<<QString( "**********************_noIndentationBasedFolding is TRUE*****************" )<<endl;
}

YzisHlContext *YzisHlContext::clone(const QStringList *args)
{
  YzisHlContext *ret = new YzisHlContext(hlId, attr, ctx, lineBeginContext, fallthrough, ftctx, false,noIndentationBasedFolding);

  for (int n=0; n < items.size(); ++n)
  {
    YzisHlItem *item = items[n];
    YzisHlItem *i = (item->dynamic ? item->clone(args) : item);
    ret->items.append(i);
  }

  ret->dynamicChild = true;

  return ret;
}

YzisHlContext::~YzisHlContext()
{
  if (dynamicChild)
  {
    for (int n=0; n < items.size(); ++n)
    {
      if (items[n]->dynamicChild)
        delete items[n];
    }
  }
}
//END

//BEGIN YzisHighlighting

#undef deepdbg
#undef dbg
#undef err
#define deepdbg()   yzDeepDebug("YzisHighlighting")
#define dbg()       yzDebug("YzisHighlighting")
#define err()       yzError("YzisHighlighting")

YzisHighlighting::YzisHighlighting(const YzisSyntaxModeListItem *def) : refCount(0)
{
  errorsAndWarnings = "";
  building=false;
  noHl = false;
  m_foldingIndentationSensitive = false;
  folding=false;

  if (def == 0)
  {
    noHl = true;
    iName = "None"; // not translated internal name (for config and more)
    iNameTranslated = _("None"); // user visible name
    iSection = "";
    m_priority = 0;
    iHidden = false;
    m_additionalData.insert( "none", new HighlightPropertyBag );
    m_additionalData["none"]->deliminator = stdDeliminator;
    m_additionalData["none"]->wordWrapDeliminator = stdDeliminator;
    m_hlIndex[0] = "none";
  }
  else
  {
    iName = def->name;
    iNameTranslated = def->nameTranslated;
    iSection = def->section;
    iHidden = def->hidden;
    iWildcards = def->extension;
    iMimetypes = def->mimetype;
    identifier = def->identifier;
    iVersion=def->version;
    iAuthor=def->author;
    iLicense=def->license;
    m_priority=def->priority.toInt();
  }

   deliminator = stdDeliminator;
}

YzisHighlighting::~YzisHighlighting()
{
	cleanup();
}

void YzisHighlighting::cleanup()
{
  qDeleteAll( m_contexts );
  m_contexts.clear ();

  qDeleteAll( m_attributeArrays );
  m_attributeArrays.clear();

  internalIDList.clear();

}

void YzisHighlighting::generateContextStack(int *ctxNum, int ctx, QVector<short>* ctxs, int *prevLine)
{
  deepdbg()<<QString("Entering generateContextStack with %1").arg(ctx)<<endl;
  while (true)
  {
    if (ctx >= 0)
    {
      (*ctxNum) = ctx;

      ctxs->append ( *ctxNum );

      return;
    }
    else
    {
      if (ctx == -1)
      {
        (*ctxNum)=( (ctxs->isEmpty() ) ? 0 : (*ctxs)[ctxs->size()-1]);
      }
      else
      {
        int size = ctxs->size() + ctx + 1;

        if (size > 0)
        {
          ctxs->resize (size);
          (*ctxNum)=(*ctxs)[size-1];
        }
        else
        {
          ctxs->resize (0);
          (*ctxNum)=0;
        }

        ctx = 0;

        if ((*prevLine) >= (int)(ctxs->size()-1))
        {
          *prevLine=ctxs->size()-1;

          if ( ctxs->isEmpty() )
            return;

          YzisHlContext *c = contextNum((*ctxs)[ctxs->size()-1]);
          if (c && (c->ctx != -1))
          {
            //kdDebug(13010)<<"PrevLine > size()-1 and ctx!=-1)"<<endl;
            ctx = c->ctx;

            continue;
          }
        }
      }

      return;
    }
  }
}


/**
 * Creates a new dynamic context or reuse an old one if it has already been created.
 */
int YzisHighlighting::makeDynamicContext(YzisHlContext *model, const QStringList *args)
{
  QPair<YzisHlContext *, QString> key(model, args->front());
  short value;

  if (dynamicCtxs.contains(key))
    value = dynamicCtxs[key];
  else
  {
    dbg () << "new stuff: " << startctx << endl;

    YzisHlContext *newctx = model->clone(args);

    m_contexts.push_back (newctx);

    value = startctx++;
    dynamicCtxs[key] = value;
    YzisHlManager::self()->incDynamicCtxs();
  }

  // kdDebug(HL) << "Dynamic context: using context #" << value << " (for model " << model << " with args " << *args << ")" << endl;

  return value;
}

/**
 * Drop all dynamic contexts. Shall be called with extreme care, and shall be immediately
 * followed by a full HL invalidation.
 */
void YzisHighlighting::dropDynamicContexts()
{
  for (int i=base_startctx; i < m_contexts.size(); ++i)
    delete m_contexts[i];

  m_contexts.resize (base_startctx);

  dynamicCtxs.clear();
  startctx = base_startctx;
}

/**
 * Parse the text and fill in the context array and folding list array
 *
 * @param prevLine The previous line, the context array is picked up from that if present.
 * @param textLine The text line to parse
 * @param foldingList will be filled
 * @param ctxChanged will be set to reflect if the context changed
 */
void YzisHighlighting::doHighlight ( YLine *prevLine,
                                     YLine *textLine,
                                     QVector<uint>* foldingList,
                                     bool *ctxChanged )
{
  if (!textLine)
    return;

  // in all cases, remove old hl, or we will grow to infinite ;)
  textLine->clearAttributes ();

  if (noHl)
  {
    if (textLine->length() > 0)
      memset (textLine->attributes(), 0, textLine->length());

    return;
  }

  // duplicate the ctx stack, only once !
  QVector<short> ctx (prevLine->ctxArray());

  int ctxNum = 0;
  int previousLine = -1;
  YzisHlContext *context;

  if (ctx.isEmpty())
  {
    // If the stack is empty, we assume to be in Context 0 (Normal)
    context = contextNum(ctxNum);
  }
  else
  {
    // There does an old context stack exist -> find the context at the line start
    ctxNum = ctx[ctx.size()-1]; //context ID of the last character in the previous line

    //deepdbg() << "\t\tctxNum = " << ctxNum << " contextList[ctxNum] = " << contextList[ctxNum] << endl; // ellis

    //if (lineContinue)   dbg()<<QString("The old context should be %1").arg((int)ctxNum)<<endl;

    if (!(context = contextNum(ctxNum)))
      context = contextNum(0);

    deepdbg()<<"test1-2-1-text2"<<endl;

    previousLine=ctx.size()-1; //position of the last context ID of th previous line within the stack

    // hl continue set or not ???
    if (prevLine->hlLineContinue())
    {
      prevLine--;
    }
    else
    {
      generateContextStack(&ctxNum, context->ctx, &ctx, &previousLine); //get stack ID to use

      if (!(context = contextNum(ctxNum)))
        context = contextNum(0);
    }

    //kdDebug(13010)<<"test1-2-1-text4"<<endl;

    //if (lineContinue)   dbg()<<QString("The new context is %1").arg((int)ctxNum)<<endl;
  }

  // text, for programming convenience :)
  QChar lastChar = ' ';
  const QString& text = textLine->data();
  const int len = textLine->length();

  // calc at which char the first char occurs, set it to length of line if never
  const int firstChar = textLine->firstChar();
  const int startNonSpace = (firstChar == -1) ? len : firstChar;

  // last found item
  YzisHlItem *item = 0;

  // loop over the line, offset gives current offset
  int offset = 0;
  while (offset < len)
  {
    bool anItemMatched = false;
    bool standardStartEnableDetermined = false;
    bool customStartEnableDetermined = false;

    int index = 0;
    for (item = context->items.empty() ? 0 : context->items[0]; item; item = (++index < context->items.size()) ? context->items[index] : 0 )
    {
      // does we only match if we are firstNonSpace?
      if (item->firstNonSpace && (offset > startNonSpace))
        continue;

      // have we a column specified? if yes, only match at this column
      if ((item->column != -1) && (item->column != offset))
        continue;

      if (!item->alwaysStartEnable)
      {
        if (item->customStartEnable)
        {
          if (customStartEnableDetermined || yzisInsideString (m_additionalData[context->hlId]->deliminator, lastChar))
            customStartEnableDetermined = true;
          else
            continue;
        }
        else
        {
          if (standardStartEnableDetermined || yzisInsideString (stdDeliminator, lastChar))
            standardStartEnableDetermined = true;
          else
            continue;
        }
      }

      int offset2 = item->checkHgl(text, offset, len-offset);

      if (offset2 <= offset)
        continue;

      if (item->region2)
      {
        // kdDebug(13010)<<QString("Region mark 2 detected: %1").arg(item->region2)<<endl;
        if ( !foldingList->isEmpty() &&
			((item->region2 < 0) &&
			 (*foldingList)[foldingList->size()-2] == (uint)(-item->region2)) )
        {
          foldingList->resize (foldingList->size()-2);
        }
        else
        {
          foldingList->resize (foldingList->size()+2);
          (*foldingList)[foldingList->size()-2] = (uint)item->region2;
          if (item->region2<0) //check not really needed yet
            (*foldingList)[foldingList->size()-1] = offset2;
          else
          (*foldingList)[foldingList->size()-1] = offset;
        }

      }

      if (item->region)
      {
        // kdDebug(13010)<<QString("Region mark detected: %1").arg(item->region)<<endl;

      /* if ( !foldingList->isEmpty() && ((item->region < 0) && (*foldingList)[foldingList->size()-1] == -item->region ) )
        {
          foldingList->resize (foldingList->size()-1, QGArray::SpeedOptim);
        }
        else*/
        {
          foldingList->resize (foldingList->size()+2);
          (*foldingList)[foldingList->size()-2] = item->region;
          if (item->region<0) //check not really needed yet
            (*foldingList)[foldingList->size()-1] = offset2;
          else
            (*foldingList)[foldingList->size()-1] = offset;
        }

      }

      // regenerate context stack if needed
      if (item->ctx != -1)
      {
        generateContextStack (&ctxNum, item->ctx, &ctx, &previousLine);
        context = contextNum(ctxNum);
      }

      // dynamic context: substitute the model with an 'instance'
      if (context->dynamic)
      {
        QStringList *lst = item->capturedTexts();
        if (lst != 0)
        {
          // Replace the top of the stack and the current context
          int newctx = makeDynamicContext(context, lst);
          if (ctx.size() > 0)
            ctx[ctx.size() - 1] = newctx;
          ctxNum = newctx;
          context = contextNum(ctxNum);
        }
        delete lst;
      }

      // dominik: look ahead w/o changing offset?
      if (!item->lookAhead)
      {
        if (offset2 > len)
          offset2 = len;

        // even set attributes ;)
        memset ( textLine->attributes()+offset
               , item->onlyConsume ? context->attr : item->attr
			   , offset2-offset );

		int attribute = item->onlyConsume ? context->attr : item->attr;
		if ( attribute > 0 )
			textLine->addAttribute ( offset, offset2-offset, attribute );

        offset = offset2;
        lastChar = text[offset-1];
      }

      anItemMatched = true;
      break;
    }

    // something matched, continue loop
    if (anItemMatched)
      continue;

    // nothing found: set attribute of one char
    // anders: unless this context does not want that!
    if ( context->fallthrough )
    {
    // set context to context->ftctx.
      generateContextStack(&ctxNum, context->ftctx, &ctx, &previousLine);  //regenerate context stack
      context=contextNum(ctxNum);
    //kdDebug(13010)<<"context num after fallthrough at col "<<z<<": "<<ctxNum<<endl;
    // the next is necessary, as otherwise keyword (or anything using the std delimitor check)
    // immediately after fallthrough fails. Is it bad?
    // jowenn, can you come up with a nicer way to do this?
    /*  if (offset)
        lastChar = text[offset - 1];
      else
        lastChar = '\\';*/
      continue;
    }
    else
    {
      *(textLine->attributes() + offset) = context->attr;

	  if ( context->attr > 0 )
		  textLine->addAttribute ( offset, 1, context->attr );

      lastChar = text[offset];
      offset++;
    }
  }

  // has the context stack changed ?
  if (ctx == textLine->ctxArray())
  {
    if (ctxChanged)
      (*ctxChanged) = false;
  }
  else
  {
    if (ctxChanged)
      (*ctxChanged) = true;

    // assign ctx stack !
    textLine->setContext(ctx);
  }

  // write hl continue flag
  textLine->setHlLineContinue (item && item->lineContinue());

  if ( m_foldingIndentationSensitive ) {
      bool noindent=false;
      for ( int i=ctx.size()-1; i>=0; --i ) {
    	  if ( contextNum(ctx[i])->noIndentationBasedFolding ) {
            noindent=true;
            break;
    	  }
      }
      //XXX textLine->setNoIndentBasedFolding( noindent );
  }
}

void YzisHighlighting::loadWildcards()
{
	YInternalOptionPool* config = YSession::self()->getOptions();
    QString extensionString = config->readQStringEntry("Highlighting " + iName + "/Wildcards", iWildcards);

    if (extensionSource != extensionString) {
    	regexpExtensions.clear();
    	plainExtensions.clear();

    	extensionSource = extensionString;

    	static QRegExp sep("\\s*;\\s*");

    	QStringList l = extensionSource.split( sep );

    	static QRegExp boringExpression("\\*\\.[\\d\\w]+");

    	QStringList::Iterator it = l.begin(), end = l.end();
    	for( ; it != end; ++it )
    		if (boringExpression.exactMatch(*it))
    			plainExtensions.append((*it).mid(1));
    		else
    			regexpExtensions.append(QRegExp((*it), Qt::CaseSensitive, QRegExp::Wildcard));
    }
}

QList<QRegExp>& YzisHighlighting::getRegexpExtensions()
{
  return regexpExtensions;
}

QStringList& YzisHighlighting::getPlainExtensions()
{
  return plainExtensions;
}

QString YzisHighlighting::getMimetypes()
{
	YInternalOptionPool* config = YSession::self()->getOptions();
	config->setGroup("Highlighting " + iName);
	return config->readQStringEntry("Highlighting " + iName + "/Mimetypes", iMimetypes);
}

int YzisHighlighting::priority()
{
	YInternalOptionPool* config = YSession::self()->getOptions();
	config->setGroup("Highlighting " + iName);

    return config->readIntEntry("Highlighting " + iName + "/Priority", m_priority);
}

#if 0
YzisHlData *YzisHighlighting::getData()
{/*
  KConfig *config = YzisHlManager::self()->getKConfig();
  config->setGroup("Highlighting " + iName);

  YzisHlData *hlData = new YzisHlData(
  config->readEntry("Wildcards", iWildcards),
  config->readEntry("Mimetypes", iMimetypes),
  config->readEntry("Identifier", identifier),
  config->readNumEntry("Priority", m_priority));

 return hlData;*/
    return new YzisHlData();
}
#endif

void YzisHighlighting::setData(YzisHlData *)
{/*
  KConfig *config = YzisHlManager::self()->getKConfig();
  config->setGroup("Highlighting " + iName);

  config->writeEntry("Wildcards",hlData->wildcards);
  config->writeEntry("Mimetypes",hlData->mimetypes);
  config->writeEntry("Priority",hlData->priority);*/
}

void YzisHighlighting::getYzisHlItemDataList (uint schema, YzisHlItemDataList &list)
{
  YInternalOptionPool* config = YSession::self()->getOptions();
  config->setGroup("Highlighting " + iName + " - Schema " + YSession::self()->schemaManager()->name(schema));

  list.clear();
  createYzisHlItemData(list);

  YzisHlItemData *p = list.at( 0 );
  for (int ab = 0 ; ab < list.size() && ( p=list.at( ab ) )!=0L; ++ab ) {
    QStringList s = config->readQStringListEntry(p->name);

    if (s.count()>0)
    {

      while(s.count()<9) s<<"";
      p->clear();

      QString tmp=s[0]; if (!tmp.isEmpty()) p->defStyleNum=tmp.toInt();

      QRgb col;

      tmp=s[1]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); p->setTextColor(col); }

      tmp=s[2]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); p->setSelectedTextColor(col); }

      tmp=s[3]; if (!tmp.isEmpty()) p->setBold(tmp!="0");

      tmp=s[4]; if (!tmp.isEmpty()) p->setItalic(tmp!="0");

      tmp=s[5]; if (!tmp.isEmpty()) p->setStrikeOut(tmp!="0");

      tmp=s[6]; if (!tmp.isEmpty()) p->setUnderline(tmp!="0");

      tmp=s[7]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); p->setBGColor(col); }

      tmp=s[8]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); p->setSelectedBGColor(col); }

    }
  }
}

/**
 * Saves the YzisHlData attribute definitions to the config file.
 *
 * @param schema The id of the schema group to save
 * @param list YzisHlItemDataList containing the data to be used
 */
void YzisHighlighting::setYzisHlItemDataList(uint schema, YzisHlItemDataList& list)
{
  YInternalOptionPool* config = YSession::self()->getOptions();
  config->setGroup("Highlighting " + iName + " - Schema " + YSession::self()->schemaManager()->name( schema ));

  QStringList settings;

  YzisHlItemData *p = list.at( 0 );
  for (int ab = 0 ; ab < list.size() && ( p=list.at( ab ) )!=0L; ++ab ) {
    settings.clear();
    settings<<QString::number(p->defStyleNum,10);
    settings<<(p->itemSet(YzisAttribute::TextColor)?QString::number(p->textColor().rgb(),16):"");
    settings<<(p->itemSet(YzisAttribute::SelectedTextColor)?QString::number(p->selectedTextColor().rgb(),16):"");
    settings<<(p->itemSet(YzisAttribute::Weight)?(p->bold()?"1":"0"):"");
    settings<<(p->itemSet(YzisAttribute::Italic)?(p->italic()?"1":"0"):"");
    settings<<(p->itemSet(YzisAttribute::StrikeOut)?(p->strikeOut()?"1":"0"):"");
    settings<<(p->itemSet(YzisAttribute::Underline)?(p->underline()?"1":"0"):"");
    settings<<(p->itemSet(YzisAttribute::BGColor)?QString::number(p->bgColor().rgb(),16):"");
    settings<<(p->itemSet(YzisAttribute::SelectedBGColor)?QString::number(p->selectedBGColor().rgb(),16):"");
    settings<<"---";
    config->setQStringListEntry(p->name,settings);
  }
}

/**
 * Increase the usage count, and trigger initialization if needed.
 */
void YzisHighlighting::use()
{
  if (refCount == 0)
    init();

  refCount++;
}

/**
 * Decrease the usage count, and trigger cleanup if needed.
 */
void YzisHighlighting::release()
{
  refCount--;

  if (refCount == 0)
    done();
}

/**
 * Initialize a context for the first time.
 */
void YzisHighlighting::init()
{
  if (noHl)
    return;

  for (int i=0; i < m_contexts.size(); ++i)
    delete m_contexts[i];
  m_contexts.clear ();
  makeContextList();

    //little hack (for yzis) to fill up our options tree with defaults settings
  YzisHlItemDataList list;
  getYzisHlItemDataList(0, list);
  setYzisHlItemDataList(0, list);
}

/**
 * If the there is no document using the highlighting style free the complete
 * context structure.
 */
void YzisHighlighting::done()
{
  if (noHl)
    return;

  cleanup();
}

/**
 * YzisHighlighting - createYzisHlItemData
 * This function reads the itemData entries from the config file, which specifies the
 * default attribute styles for matched items/contexts.
 *
 * @param list A reference to the internal list containing the parsed default config
 */
void YzisHighlighting::createYzisHlItemData(YzisHlItemDataList &list)
{
  // If no highlighting is selected we need only one default.
  if (noHl)
  {
    list.append(new YzisHlItemData(_( "Normal Text" ), YzisHlItemData::dsNormal));
    return;
  }

  // If the internal list isn't already available read the config file
  if (internalIDList.isEmpty())
    makeContextList();

  list=internalIDList;
}

/**
 * Adds the styles of the currently parsed highlight to the itemdata list
 */
void YzisHighlighting::addToYzisHlItemDataList()
{
  //Tell the syntax document class which file we want to parse and which data group
  YzisHlManager::self()->syntax->setIdentifier(buildIdentifier);
  YzisSyntaxContextData *data = YzisHlManager::self()->syntax->getGroupInfo("highlighting","itemData");

  //begin with the real parsing
  while (YzisHlManager::self()->syntax->nextGroup(data))
  {
    // read all attributes
    QString color = YzisHlManager::self()->syntax->groupData(data,QString("color"));
    QString selColor = YzisHlManager::self()->syntax->groupData(data,QString("selColor"));
    QString bold = YzisHlManager::self()->syntax->groupData(data,QString("bold"));
    QString italic = YzisHlManager::self()->syntax->groupData(data,QString("italic"));
    QString underline = YzisHlManager::self()->syntax->groupData(data,QString("underline"));
    QString strikeOut = YzisHlManager::self()->syntax->groupData(data,QString("strikeOut"));
    QString bgColor = YzisHlManager::self()->syntax->groupData(data,QString("backgroundColor"));
    QString selBgColor = YzisHlManager::self()->syntax->groupData(data,QString("selBackgroundColor"));

    YzisHlItemData* newData = new YzisHlItemData(
            buildPrefix+YzisHlManager::self()->syntax->groupData(data,QString("name")).simplified(),
            getDefStyleNum(YzisHlManager::self()->syntax->groupData(data,QString("defStyleNum"))));

    /* here the custom style overrides are specified, if needed */
    if (!color.isEmpty()) newData->setTextColor(YColor(color));
    if (!selColor.isEmpty()) newData->setSelectedTextColor(YColor(selColor));
    if (!bold.isEmpty()) newData->setBold( IS_TRUE(bold) );
    if (!italic.isEmpty()) newData->setItalic( IS_TRUE(italic) );
    // new attributes for the new rendering view
    if (!underline.isEmpty()) newData->setUnderline( IS_TRUE(underline) );
    if (!strikeOut.isEmpty()) newData->setStrikeOut( IS_TRUE(strikeOut) );
    if (!bgColor.isEmpty()) newData->setBGColor(YColor(bgColor));
    if (!selBgColor.isEmpty()) newData->setSelectedBGColor(YColor(selBgColor));

    internalIDList.append(newData);
  }

  //clean up
  if (data)
    YzisHlManager::self()->syntax->freeGroupInfo(data);
}

/**
 * YzisHighlighting - lookupAttrName
 * This function is  a helper for makeContextList and createYzisHlItem. It looks the given
 * attribute name in the itemData list up and returns it's index
 *
 * @param name the attribute name to lookup
 * @param iDl the list containing all available attributes
 *
 * @return The index of the attribute, or 0 if the attribute isn't found
 */
int  YzisHighlighting::lookupAttrName(const QString& name, YzisHlItemDataList &iDl)
{
  for (int i = 0; i < iDl.count(); i++)
    if (iDl.at(i)->name == buildPrefix+name)
      return i;

  dbg()<<"Couldn't resolve itemDataName"<<endl;
  return 0;
}

/**
 * YzisHighlighting - createYzisHlItem
 * This function is  a helper for makeContextList. It parses the xml file for
 * information.
 *
 * @param data Data about the item read from the xml file
 * @param iDl List of all available itemData entries.
 *            Needed for attribute name->index translation
 * @param RegionList list of code folding region names
 * @param ContextNameList list of context names
 *
 * @return A pointer to the newly created item object
 */
YzisHlItem *YzisHighlighting::createYzisHlItem(YzisSyntaxContextData *data,
                                               YzisHlItemDataList &iDl,
                                               QStringList *RegionList,
                                               QStringList *ContextNameList)
{
  // No highlighting -> exit
  if (noHl)
    return 0;

  // get the (tagname) itemd type
  QString dataname=YzisHlManager::self()->syntax->groupItemData(data,QString(""));

  // code folding region handling:
  QString beginRegionStr=YzisHlManager::self()->syntax->groupItemData(data,QString("beginRegion"));
  QString endRegionStr=YzisHlManager::self()->syntax->groupItemData(data,QString("endRegion"));

  signed char regionId=0;
  signed char regionId2=0;

  if (!beginRegionStr.isEmpty())
  {
    regionId = RegionList->indexOf(beginRegionStr);

    if (regionId==-1) // if the region name doesn't already exist, add it to the list
    {
      (*RegionList)<<beginRegionStr;
      regionId = RegionList->indexOf(beginRegionStr);
    }

    regionId++;

    deepdbg () << "########### BEG REG: "  << beginRegionStr << " NUM: " << regionId << endl;
  }

  if (!endRegionStr.isEmpty())
  {
    regionId2 = RegionList->indexOf(endRegionStr);

    if (regionId2==-1) // if the region name doesn't already exist, add it to the list
    {
      (*RegionList)<<endRegionStr;
      regionId2 = RegionList->indexOf(endRegionStr);
    }

    regionId2 = -regionId2 - 1;

    deepdbg () << "########### END REG: "  << endRegionStr << " NUM: " << regionId2 << endl;
  }

  int attr = 0;
  QString tmpAttr=YzisHlManager::self()->syntax->groupItemData(data,QString("attribute")).simplified();
  bool onlyConsume = tmpAttr.isEmpty();

  // only relevant for non consumer
  if (!onlyConsume)
  {
    if (QString("%1").arg(tmpAttr.toInt())==tmpAttr)
    {
      errorsAndWarnings+=QString("<B>%1</B>: Deprecated syntax. Attribute (%2) not addressed by symbolic name<BR>").
      arg(buildIdentifier).arg(tmpAttr);
      attr=tmpAttr.toInt();
    }
    else
      attr=lookupAttrName(tmpAttr,iDl);
  }

  // Info about context switch
  int context = -1;
  QString unresolvedContext;
  QString tmpcontext=YzisHlManager::self()->syntax->groupItemData(data,QString("context"));
  if (!tmpcontext.isEmpty())
    context=getIdFromString(ContextNameList, tmpcontext,unresolvedContext);

  // Get the char parameter (eg DetectChar)
  char chr;
  if (! YzisHlManager::self()->syntax->groupItemData(data,QString("char")).isEmpty())
    chr= (YzisHlManager::self()->syntax->groupItemData(data,QString("char")).toLatin1())[0];
  else
    chr=0;

  // Get the String parameter (eg. StringDetect)
  QString stringdata=YzisHlManager::self()->syntax->groupItemData(data,QString("String"));

  // Get a second char parameter (char1) (eg Detect2Chars)
  char chr1;
  if (! YzisHlManager::self()->syntax->groupItemData(data,QString("char1")).isEmpty())
    chr1= (YzisHlManager::self()->syntax->groupItemData(data,QString("char1")).toLatin1())[0];
  else
    chr1=0;

  // Will be removed eventuall. Atm used for StringDetect and RegExp
  bool insensitive = IS_TRUE( YzisHlManager::self()->syntax->groupItemData(data,QString("insensitive")) );

  // for regexp only
  bool minimal = IS_TRUE( YzisHlManager::self()->syntax->groupItemData(data,QString("minimal")) );

  // dominik: look ahead and do not change offset. so we can change contexts w/o changing offset1.
  bool lookAhead = IS_TRUE( YzisHlManager::self()->syntax->groupItemData(data,QString("lookAhead")) );

  bool dynamic= IS_TRUE(YzisHlManager::self()->syntax->groupItemData(data,QString("dynamic")) );

  bool firstNonSpace = IS_TRUE(YzisHlManager::self()->syntax->groupItemData(data,QString("firstNonSpace")) );

  int column = -1;
  QString colStr = YzisHlManager::self()->syntax->groupItemData(data,QString("column"));
  if (!colStr.isEmpty())
    column = colStr.toInt();

  //Create the item corresponding to it's type and set it's parameters
  YzisHlItem *tmpItem;

  if (dataname=="keyword")
  {
    YzisHlKeyword *keyword=new YzisHlKeyword(attr,context,regionId,regionId2,casesensitive,
                                             m_additionalData[ buildIdentifier ]->deliminator);

    //Get the entries for the keyword lookup list
    keyword->addList(YzisHlManager::self()->syntax->finddata("highlighting",stringdata));
    tmpItem=keyword;
  }
  else if (dataname=="Float") tmpItem= (new YzisHlFloat(attr,context,regionId,regionId2));
  else if (dataname=="Int") tmpItem=(new YzisHlInt(attr,context,regionId,regionId2));
  else if (dataname=="DetectChar") tmpItem=(new YzisHlCharDetect(attr,context,regionId,regionId2,chr));
  else if (dataname=="Detect2Chars") tmpItem=(new YzisHl2CharDetect(attr,context,regionId,regionId2,chr,chr1));
  else if (dataname=="RangeDetect") tmpItem=(new YzisHlRangeDetect(attr,context,regionId,regionId2, chr, chr1));
  else if (dataname=="LineContinue") tmpItem=(new YzisHlLineContinue(attr,context,regionId,regionId2));
  else if (dataname=="StringDetect") tmpItem=(new YzisHlStringDetect(attr,context,regionId,regionId2,stringdata,insensitive));
  else if (dataname=="AnyChar") tmpItem=(new YzisHlAnyChar(attr,context,regionId,regionId2,stringdata));
  else if (dataname=="RegExpr") tmpItem=(new YzisHlRegExpr(attr,context,regionId,regionId2,stringdata, insensitive, minimal));
  else if (dataname=="HlCChar") tmpItem= ( new YzisHlCChar(attr,context,regionId,regionId2));
  else if (dataname=="HlCHex") tmpItem= (new YzisHlCHex(attr,context,regionId,regionId2));
  else if (dataname=="HlCOct") tmpItem= (new YzisHlCOct(attr,context,regionId,regionId2));
  else if (dataname=="HlCFloat") tmpItem= (new YzisHlCFloat(attr,context,regionId,regionId2));
  else if (dataname=="HlCStringChar") tmpItem= (new YzisHlCStringChar(attr,context,regionId,regionId2));
  else if (dataname=="DetectSpaces") tmpItem= (new YzisHlDetectSpaces(attr,context,regionId,regionId2));
  else if (dataname=="DetectIdentifier") tmpItem= (new YzisHlDetectIdentifier(attr,context,regionId,regionId2));
  else
  {
    // oops, unknown type. Perhaps a spelling error in the xml file
    return 0;
  }

  // set lookAhead & dynamic properties
  tmpItem->lookAhead = lookAhead;
  tmpItem->dynamic = dynamic;
  tmpItem->firstNonSpace = firstNonSpace;
  tmpItem->column = column;
  tmpItem->onlyConsume = onlyConsume;

  if (!unresolvedContext.isEmpty())
  {
    unresolvedContextReferences.insert(&(tmpItem->ctx),unresolvedContext);
  }

  return tmpItem;
}

QString YzisHighlighting::hlKeyForAttrib( int i ) const
{
  int k = 0;
  QMap<int,QString>::const_iterator it = m_hlIndex.constEnd();
  while ( it != m_hlIndex.constBegin() )
  {
    --it;
    k = it.key();
    if ( i >= k )
      break;
  }
  return it.value();
}

bool YzisHighlighting::isInWord( QChar c, int attrib ) const
{
  static const QString sq = " \"'";
  return m_additionalData[ hlKeyForAttrib( attrib ) ]->deliminator.indexOf(c) < 0 && sq.indexOf(c) < 0;
}

bool YzisHighlighting::canBreakAt( QChar c, int attrib ) const
{
  static const QString sq("\"'");
  return (m_additionalData[ hlKeyForAttrib( attrib ) ]->wordWrapDeliminator.indexOf(c) != -1) && (sq.indexOf(c) == -1);
}

signed char YzisHighlighting::commentRegion(int attr) const {
  QString commentRegion=m_additionalData[ hlKeyForAttrib( attr ) ]->multiLineRegion;
  return (commentRegion.isEmpty()?0:(commentRegion.toShort()));
}

bool YzisHighlighting::canComment( int startAttrib, int endAttrib ) const
{
  QString k = hlKeyForAttrib( startAttrib );
  return ( k == hlKeyForAttrib( endAttrib ) &&
      ( ( !m_additionalData[k]->multiLineCommentStart.isEmpty() && !m_additionalData[k]->multiLineCommentEnd.isEmpty() ) ||
       ! m_additionalData[k]->singleLineCommentMarker.isEmpty() ) );
}

QString YzisHighlighting::getCommentStart( int attrib ) const
{
  return m_additionalData[ hlKeyForAttrib( attrib) ]->multiLineCommentStart;
}

QString YzisHighlighting::getCommentEnd( int attrib ) const
{
  return m_additionalData[ hlKeyForAttrib( attrib ) ]->multiLineCommentEnd;
}

QString YzisHighlighting::getCommentSingleLineStart( int attrib ) const
{
  return m_additionalData[ hlKeyForAttrib( attrib) ]->singleLineCommentMarker;
}

YzisHighlighting::CSLPos YzisHighlighting::getCommentSingleLinePosition(  int attrib ) const
{
  return m_additionalData[ hlKeyForAttrib( attrib ) ]->singleLineCommentPosition;
}


/**
 * Helper for makeContextList. It parses the xml file for
 * information, how single or multi line comments are marked
 */
void YzisHighlighting::readCommentConfig()
{
  YzisHlManager::self()->syntax->setIdentifier(buildIdentifier);
  YzisSyntaxContextData *data=YzisHlManager::self()->syntax->getGroupInfo("general","comment");

  QString cmlStart="", cmlEnd="", cmlRegion="", cslStart="";
  CSLPos cslPosition=CSLPosColumn0;

  if (data)
  {
    while  (YzisHlManager::self()->syntax->nextGroup(data))
    {
      if (YzisHlManager::self()->syntax->groupData(data,"name")=="singleLine") 
      {
        cslStart=YzisHlManager::self()->syntax->groupData(data,"start");
        QString cslpos=YzisHlManager::self()->syntax->groupData(data,"position");
        if (cslpos=="afterwhitespace")
          cslPosition=CSLPosAfterWhitespace;
        else
          cslPosition=CSLPosColumn0;
      } else if (YzisHlManager::self()->syntax->groupData(data,"name")=="multiLine")
      {
        cmlStart=YzisHlManager::self()->syntax->groupData(data,"start");
        cmlEnd=YzisHlManager::self()->syntax->groupData(data,"end");
        cmlRegion=YzisHlManager::self()->syntax->groupData(data,"region");
      }
    }
    YzisHlManager::self()->syntax->freeGroupInfo(data);
  }
  m_additionalData[buildIdentifier]->singleLineCommentMarker = cslStart;
  m_additionalData[buildIdentifier]->singleLineCommentPosition = cslPosition;
  m_additionalData[buildIdentifier]->multiLineCommentStart = cmlStart;
  m_additionalData[buildIdentifier]->multiLineCommentEnd = cmlEnd;
  m_additionalData[buildIdentifier]->multiLineRegion = cmlRegion;
}

/**
 * Helper for makeContextList. It parses the xml file for information,
 * if keywords should be treated case(in)sensitive and creates the keyword
 * delimiter list. Which is the default list, without any given weak deliminiators
 */
void YzisHighlighting::readGlobalKeywordConfig()
{
  deliminator = stdDeliminator;
  // Tell the syntax document class which file we want to parse
  deepdbg()<<"readGlobalKeywordConfig:BEGIN"<<endl;

  YzisHlManager::self()->syntax->setIdentifier(buildIdentifier);
  YzisSyntaxContextData *data = YzisHlManager::self()->syntax->getConfig("general","keywords");

  if (data)
  {
    deepdbg()<<"Found global keyword config"<<endl;

    if (YzisHlManager::self()->syntax->groupItemData(data,QString("casesensitive"))!="0")
      casesensitive=true;
    else
      casesensitive=false;

    //get the weak deliminators
    weakDeliminator=(YzisHlManager::self()->syntax->groupItemData(data,QString("weakDeliminator")));

    deepdbg()<<"weak delimiters are: "<<weakDeliminator<<endl;

    // remove any weakDelimitars (if any) from the default list and store this list.
    for (int s=0; s < weakDeliminator.length(); s++)
    {
      int f = deliminator.indexOf (weakDeliminator[s]);

      if (f > -1)
        deliminator.remove (f, 1);
    }

    QString addDelim = (YzisHlManager::self()->syntax->groupItemData(data,QString("additionalDeliminator")));

    if (!addDelim.isEmpty())
      deliminator=deliminator+addDelim;

    YzisHlManager::self()->syntax->freeGroupInfo(data);
  }
  else
  {
    //Default values
    casesensitive=true;
    weakDeliminator=QString("");
  }

  deepdbg()<<"readGlobalKeywordConfig:END"<<endl;

  deepdbg()<<"delimiterCharacters are: "<<deliminator<<endl;

  m_additionalData[buildIdentifier]->deliminator = deliminator;
}

/**
 * Helper for makeContextList. It parses the xml file for any wordwrap
 * deliminators, characters * at which line can be broken. In case no keyword
 * tag is found in the xml file, the wordwrap deliminators list defaults to the
 * standard denominators. In case a keyword tag is defined, but no
 * wordWrapDeliminator attribute is specified, the deliminator list as computed
 * in readGlobalKeywordConfig is used.
 *
 * @return the computed delimiter string.
 */
void YzisHighlighting::readWordWrapConfig()
{
  // Tell the syntax document class which file we want to parse
  deepdbg()<<"readWordWrapConfig:BEGIN"<<endl;

  YzisHlManager::self()->syntax->setIdentifier(buildIdentifier);
  YzisSyntaxContextData *data = YzisHlManager::self()->syntax->getConfig("general","keywords");

  QString wordWrapDeliminator = stdDeliminator;
  if (data)
  {
    deepdbg()<<"Found global keyword config"<<endl;

    wordWrapDeliminator = (YzisHlManager::self()->syntax->groupItemData(data,QString("wordWrapDeliminator")));
    //when no wordWrapDeliminator is defined use the deliminator list
    if ( wordWrapDeliminator.length() == 0 ) wordWrapDeliminator = deliminator;

    deepdbg() << "word wrap deliminators are " << wordWrapDeliminator << endl;

    YzisHlManager::self()->syntax->freeGroupInfo(data);
  }

  deepdbg()<<"readWordWrapConfig:END"<<endl;

  m_additionalData[buildIdentifier]->wordWrapDeliminator = wordWrapDeliminator;
}

void YzisHighlighting::readIndentationConfig()
{
  m_indentation = "";

  YzisHlManager::self()->syntax->setIdentifier(buildIdentifier);
  YzisSyntaxContextData *data = YzisHlManager::self()->syntax->getConfig("general","indentation");

  if (data)
  {
    m_indentation = (YzisHlManager::self()->syntax->groupItemData(data,QString("mode")));

    YzisHlManager::self()->syntax->freeGroupInfo(data);
  }
}

void YzisHighlighting::readFoldingConfig()
{
  // Tell the syntax document class which file we want to parse
  deepdbg()<<"readfoldignConfig:BEGIN"<<endl;

  YzisHlManager::self()->syntax->setIdentifier(buildIdentifier);
  YzisSyntaxContextData *data = YzisHlManager::self()->syntax->getConfig("general","folding");

  if (data)
  {
    deepdbg()<<"Found global keyword config"<<endl;

    if (YzisHlManager::self()->syntax->groupItemData(data,QString("indentationsensitive"))!="1")
      m_foldingIndentationSensitive=false;
    else
      m_foldingIndentationSensitive=true;

    YzisHlManager::self()->syntax->freeGroupInfo(data);
  }
  else
  {
    //Default values
    m_foldingIndentationSensitive = false;
  }

  deepdbg()<<"readfoldingConfig:END"<<endl;

  deepdbg()<<"############################ use indent for fold are: "<<m_foldingIndentationSensitive<<endl;
}

void  YzisHighlighting::createContextNameList(QStringList *ContextNameList,int ctx0)
{
  deepdbg()<<"creatingContextNameList:BEGIN"<<endl;

  if (ctx0 == 0)
      ContextNameList->clear();

  YzisHlManager::self()->syntax->setIdentifier(buildIdentifier);

  YzisSyntaxContextData *data=YzisHlManager::self()->syntax->getGroupInfo("highlighting","context");

  int id=ctx0;

  if (data)
  {
     while (YzisHlManager::self()->syntax->nextGroup(data))
     {
          QString tmpAttr=YzisHlManager::self()->syntax->groupData(data,QString("name")).simplified();
    if (tmpAttr.isEmpty())
    {
     tmpAttr=QString("!YZIS_INTERNAL_DUMMY! %1").arg(id);
     errorsAndWarnings +=QString("<B>%1</B>: Deprecated syntax. Context %2 has no symbolic name<BR>").arg(buildIdentifier).arg(id-ctx0);
    }
          else tmpAttr=buildPrefix+tmpAttr;
    (*ContextNameList)<<tmpAttr;
          id++;
     }
     YzisHlManager::self()->syntax->freeGroupInfo(data);
  }
  deepdbg()<<"creatingContextNameList:END"<<endl;

}

int YzisHighlighting::getIdFromString(QStringList *ContextNameList, QString tmpLineEndContext, /*NO CONST*/ QString &unres)
{
  unres="";
  int context;
  if ((tmpLineEndContext=="#stay") || (tmpLineEndContext.simplified().isEmpty()))
    context=-1;

  else if (tmpLineEndContext.startsWith("#pop"))
  {
    context=-1;
    for(;tmpLineEndContext.startsWith("#pop");context--)
    {
      tmpLineEndContext.remove(0,4);
//      dbg()<<"#pop found"<<endl;
    }
  }

  else if ( tmpLineEndContext.startsWith("##"))
  {
    QString tmp=tmpLineEndContext.right(tmpLineEndContext.length()-2);
    if (!embeddedHls.contains(tmp))  embeddedHls.insert(tmp,YzisEmbeddedHlInfo());
    unres=tmp;
    context=0;
  }

  else
  {
    context=ContextNameList->indexOf(buildPrefix+tmpLineEndContext);
    if (context==-1)
    {
      context=tmpLineEndContext.toInt();
      errorsAndWarnings+=QString(
    		  "<B>%1</B>:Deprecated syntax. Context %2 not addressed by a symbolic name"
    		  ).arg(buildIdentifier).arg(tmpLineEndContext);
    }
//#warning restructure this the name list storage.
//    context=context+buildContext0Offset;
  }
  return context;
}

/**
 * The most important initialization function for each highlighting. It's called
 * each time a document gets a highlighting style assigned. parses the xml file
 * and creates a corresponding internal structure
 */
void YzisHighlighting::makeContextList()
{
  if (noHl)  // if this a highlighting for "normal texts" only, tere is no need for a context list creation
    return;

  embeddedHls.clear();
  unresolvedContextReferences.clear();
  RegionList.clear();
  ContextNameList.clear();

  // prepare list creation. To reuse as much code as possible handle this
  // highlighting the same way as embedded onces
  embeddedHls.insert(iName,YzisEmbeddedHlInfo());

  bool something_changed;
  // the context "0" id is 0 for this hl, all embedded context "0"s have offsets
  startctx=base_startctx=0;
  // inform everybody that we are building the highlighting contexts and itemlists
  building=true;
  do
  {
    deepdbg()<<"**************** Outter loop in make ContextList"<<endl;
    deepdbg()<<"**************** Hl List count:"<<embeddedHls.count()<<endl;
    something_changed=false; //assume all "embedded" hls have already been loaded
    
    YzisEmbeddedHlInfos::const_iterator it=embeddedHls.begin(), end=embeddedHls.end();
    for (; it!=end;++it)
    {
      if (!it.value().loaded)  // we found one, we still have to load
      {
        deepdbg()<<"**************** Inner loop in make ContextList"<<endl;
        QString identifierToUse;
        deepdbg()<<"Trying to open highlighting definition file: "<< it.key()<<endl;
        if (iName==it.key())
          identifierToUse=identifier;  // the own identifier is known
        else
          identifierToUse=YzisHlManager::self()->identifierForName(it.key());

    	deepdbg()<<"Location is:"<< identifierToUse<<endl;

        buildPrefix=it.key()+':';  // attribute names get prefixed by the names
                                   // of the highlighting definitions they belong to

    	if (identifierToUse.isEmpty() ) dbg()<<"OHOH, unknown highlighting description referenced"<<endl;

        deepdbg()<<"setting ("<<it.key()<<") to loaded"<<endl;

        //mark hl as loaded
        it=embeddedHls.insert(it.key(),YzisEmbeddedHlInfo(true,startctx));
        //set class member for context 0 offset, so we don't need to pass it around
        buildContext0Offset=startctx;
        //parse one hl definition file
        startctx=addToContextList(identifierToUse,startctx);

        if (noHl) return;  // an error occurred

        base_startctx = startctx;
        something_changed=true; // something has been loaded
      }
    }
  } while (something_changed);  // as long as there has been another file parsed
  				// repeat everything, there could be newly added embedded hls.


  // at this point all needed highlighing (sub)definitions are loaded. It's time to resolve cross file
  //   references (if there are some
  deepdbg()<<"Unresolved contexts, which need attention: "<<unresolvedContextReferences.count()<<endl;

  //optimize this a littlebit
  for (YzisHlUnresolvedCtxRefs::iterator unresIt=unresolvedContextReferences.begin();
    unresIt!=unresolvedContextReferences.end();++unresIt)
  {
    //try to find the context0 id for a given unresolvedReference
    YzisEmbeddedHlInfos::const_iterator hlIt=embeddedHls.find(unresIt.value());
    if (hlIt!=embeddedHls.end())
      *(unresIt.key())=hlIt.value().context0;
  }

  // eventually handle YzisHlIncludeRules items, if they exist.
  // This has to be done after the cross file references, because it is allowed
  // to include the context0 from a different definition, than the one the rule
  // belongs to
  handleYzisHlIncludeRules();

  embeddedHls.clear(); //save some memory.
  unresolvedContextReferences.clear(); //save some memory
  RegionList.clear();  // I think you get the idea ;)
  ContextNameList.clear();


  // if there have been errors show them
  if (!errorsAndWarnings.isEmpty())
  deepdbg() << QString("There were warning(s) and/or error(s) while parsing the syntax highlighting configuration.") << errorsAndWarnings << endl;

  // we have finished
  building=false;
}

void YzisHighlighting::handleYzisHlIncludeRules()
{
  // if there are noe include rules to take care of, just return
  deepdbg()<<"YzisHlIncludeRules, which need attention: " <<includeRules.count()<<endl;
  if (includeRules.isEmpty()) return;

  buildPrefix="";
  QString dummy;

  //  By now the context0 references are resolved, now more or less
  //  only inner file references are resolved. If we decide that arbitrary inclusion is
  //  needed, this doesn't need to be changed, only the addToContextList
  //  method


  //resolove context names
  YzisHlIncludeRules::iterator it=includeRules.begin(), end = includeRules.end();
  for (;it!=end;)
  {
      if ((*it)->incCtx==-1) // context unresolved ?
      {
      if ((*it)->incCtxN.isEmpty())
      {
        // no context name given, and no valid context id set, so this item is
        // going to be removed
        YzisHlIncludeRules::iterator it1=it;
        ++it1;
        delete (*it);
        includeRules.erase(it);
        it=it1;
      }
      else
      {
        // resolve name to id
        (*it)->incCtx=getIdFromString(&ContextNameList,(*it)->incCtxN,dummy);
        deepdbg()<<"Resolved "<<(*it)->incCtxN<< " to "<<(*it)->incCtx<<" for include rule"<<endl;
        // It would be good to look here somehow, if the result is valid
      }
    }
  	else ++it; //nothing to do, already resolved (by the cross defintion reference resolver
  }

  // now that all YzisHlIncludeRule items should be valid and completely resolved,
  // do the real inclusion of the rules.
  // recursiveness is needed, because context 0 could include context 1, which
  // itself includes context 2 and so on.
  //  In that case we have to handle context 2 first, then 1, 0
  //TODO: catch circular references: eg 0->1->2->3->1
  while (!includeRules.isEmpty())
    handleYzisHlIncludeRulesRecursive(includeRules.begin(),&includeRules);
}

void YzisHighlighting::handleYzisHlIncludeRulesRecursive(YzisHlIncludeRules::iterator it, YzisHlIncludeRules *list)
{
  if (it==list->end()) return;  //invalid iterator, shouldn't happen, but better have a rule prepared ;)

  YzisHlIncludeRules::iterator it1=it;
  int ctx=(*it1)->ctx;

  // find the last entry for the given context in the YzisHlIncludeRules list
  // this is need if one context includes more than one. This saves us from
  // updating all insert positions:
  // eg: context 0:
  // pos 3 - include context 2
  // pos 5 - include context 3
  // During the building of the includeRules list the items are inserted in
  // ascending order, now we need it descending to make our life easier.
  while ((it!=list->end()) && ((*it)->ctx==ctx))
  {
    it1=it;
    ++it;
  }

  // iterate over each include rule for the context the function has been called for.
  while ((it1!=list->end()) && ((*it1)->ctx==ctx))
  {
    int ctx1=(*it1)->incCtx;

    //let's see, if the the included context includes other contexts
    for (YzisHlIncludeRules::iterator it2=list->begin();it2!=list->end();++it2)
    {
      if ((*it2)->ctx==ctx1)
      {
        //yes it does, so first handle that include rules, since we want to
        // include those subincludes too
        handleYzisHlIncludeRulesRecursive(it2,list);
        break;
      }
    }

    // if the context we want to include had sub includes, they are already inserted there.
    YzisHlContext *dest=m_contexts[ctx];
    YzisHlContext *src=m_contexts[ctx1];
//     kdDebug(3010)<<"linking included rules from "<<ctx<<" to "<<ctx1<<endl;

    // If so desired, change the dest attribute to the one of the src.
    // Required to make commenting work, if text matched by the included context
    // is a different highlight than the host context.
    if ( (*it1)->includeAttrib )
      dest->attr = src->attr;

    // insert the included context's rules starting at position p
    int p=(*it1)->pos;

    // remember some stuff
    int oldLen = dest->items.size();
    uint itemsToInsert = src->items.size();

    // resize target
    dest->items.resize (oldLen + itemsToInsert);

    // move old elements
    for (int i=oldLen-1; i >= p; --i)
      dest->items[i+itemsToInsert] = dest->items[i];

    // insert new stuff
    for (uint i=0; i < itemsToInsert; ++i  )
      dest->items[p+i] = src->items[i];

    it=it1; //backup the iterator
    --it1; //move to the next entry, which has to be take care of
    delete (*it); //free the already handled data structure
    list->erase(it); // remove it from the list
  }
}

/**
 * Add one highlight to the contextlist.
 *
 * @return the number of contexts after this is added.
 */
int YzisHighlighting::addToContextList(const QString &ident, int ctx0)
{
  deepdbg()<<"=== Adding hl with ident '"<<ident<<"'"<<endl;

  buildIdentifier=ident;
  YzisSyntaxContextData *data, *datasub;
  YzisHlItem *c;

  QString dummy;

  // Let the syntax document class know, which file we'd like to parse
  if (!YzisHlManager::self()->syntax->setIdentifier(ident))
  {
    noHl=true;
    dbg() << "Since there has been an error parsing the highlighting description, this highlighting will be disabled" << endl;
    return 0;
  }

  // only read for the own stuff
  if (identifier == ident)
  {
    readIndentationConfig ();
  }

  RegionList<<"!YzisInternal_TopLevel!";

  m_hlIndex[internalIDList.count()] = ident;
  m_additionalData.insert( ident, new HighlightPropertyBag );

  // fill out the propertybag
  readCommentConfig();
  readGlobalKeywordConfig();
  readWordWrapConfig();

  readFoldingConfig ();

  QString ctxName;

  // This list is needed for the translation of the attribute parameter,
  // if the itemData name is given instead of the index
  addToYzisHlItemDataList();
  YzisHlItemDataList iDl = internalIDList;

  createContextNameList(&ContextNameList,ctx0);


  deepdbg()<<"Parsing Context structure"<<endl;
  //start the real work
  data=YzisHlManager::self()->syntax->getGroupInfo("highlighting","context");
  uint i=buildContext0Offset;
  if (data)
  {
    while (YzisHlManager::self()->syntax->nextGroup(data))
    {
      deepdbg()<<"Found a context in file, building structure now"<<endl;
      //BEGIN - Translation of the attribute parameter
      QString tmpAttr=YzisHlManager::self()->syntax->groupData(data,QString("attribute")).simplified();
      int attr;
      if (QString("%1").arg(tmpAttr.toInt())==tmpAttr)
        attr=tmpAttr.toInt();
      else
        attr=lookupAttrName(tmpAttr,iDl);
      //END - Translation of the attribute parameter

      ctxName=buildPrefix+YzisHlManager::self()->syntax->groupData(data,QString("lineEndContext")).simplified();
      QString tmpLineEndContext=YzisHlManager::self()->syntax->groupData(data,QString("lineEndContext")).simplified();

      int context;

      context=getIdFromString(&ContextNameList, tmpLineEndContext,dummy);

      QString tmpNIBF = YzisHlManager::self()->syntax->groupData( data, QString( "noIndentationBasedFolding" ) );
      bool noIndentationBasedFolding=IS_TRUE( tmpNIBF );

      //BEGIN get fallthrough props
      bool ft = false;
      int ftc = 0; // fallthrough context
      if ( i > 0 )  // fallthrough is not smart in context 0
      {
        QString tmpFt = YzisHlManager::self()->syntax->groupData(data, QString("fallthrough") );
        if ( IS_TRUE(tmpFt) )
          ft = true;
        if ( ft )
        {
          QString tmpFtc = YzisHlManager::self()->syntax->groupData( data, QString("fallthroughContext") );

          ftc=getIdFromString(&ContextNameList, tmpFtc,dummy);
          if (ftc == -1) ftc =0;

          deepdbg()<<"Setting fall through context (context "<<i<<"): "<<ftc<<endl;
        }
      }
      //END falltrhough props

      bool dynamic = false;
      QString tmpDynamic = YzisHlManager::self()->syntax->groupData(data, QString("dynamic") );
      if ( tmpDynamic.toLower() == "true" ||  tmpDynamic.toInt() == 1 )
        dynamic = true;

      YzisHlContext *ctxNew = new YzisHlContext (
        ident,
        attr,
        context,
        (YzisHlManager::self()->syntax->groupData(data,QString("lineBeginContext"))).isEmpty()?-1:
        (YzisHlManager::self()->syntax->groupData(data,QString("lineBeginContext"))).toInt(),
        ft, ftc, dynamic,noIndentationBasedFolding);

      m_contexts.push_back (ctxNew);

      deepdbg () << "INDEX: " << i << " LENGTH " << m_contexts.size()-1 << endl;

      //Let's create all items for the context
      while (YzisHlManager::self()->syntax->nextItem(data))
      {
//    kdDebug(13010)<< "In make Contextlist: Item:"<<endl;

      // KateHlIncludeRules : add a pointer to each item in that context
        // TODO add a attrib includeAttrib
      QString tag = YzisHlManager::self()->syntax->groupItemData(data,QString(""));
      if ( tag == "IncludeRules" ) //if the new item is an Include rule, we have to take special care
      {
        QString incCtx = YzisHlManager::self()->syntax->groupItemData( data, QString("context"));
        QString incAttrib = YzisHlManager::self()->syntax->groupItemData( data, QString("includeAttrib"));
        bool includeAttrib = ( incAttrib.toLower() == "true" || incAttrib.toInt() == 1 );
        // only context refernces of type NAME and ##Name are allowed
        if (incCtx.startsWith("##") || (!incCtx.startsWith("#")))
        {
          //#stay, #pop is not interesting here
          if (!incCtx.startsWith("#"))
          {
            // a local reference -> just initialize the include rule structure
            incCtx=buildPrefix+incCtx.simplified();
            includeRules.append(new YzisHlIncludeRule(i,m_contexts[i]->items.count(),incCtx, includeAttrib));
          }
          else
          {
            //a cross highlighting reference
            deepdbg()<<"Cross highlight reference <IncludeRules>"<<endl;
            YzisHlIncludeRule *ir=new YzisHlIncludeRule(i,m_contexts[i]->items.count(),"",includeAttrib);

            //use the same way to determine cross hl file references as other items do
            if (!embeddedHls.contains(incCtx.right(incCtx.length()-2)))
              embeddedHls.insert(incCtx.right(incCtx.length()-2),YzisEmbeddedHlInfo());

            unresolvedContextReferences.insert(&(ir->incCtx),
                incCtx.right(incCtx.length()-2));

            includeRules.append(ir);
          }
        }

        continue;
      }
      // TODO -- can we remove the block below??
#if 0
                QString tag = KateHlManager::self()->syntax->groupKateHlItemData(data,QString(""));
                if ( tag == "IncludeRules" ) {
                  // attrib context: the index (jowenn, i think using names here
                  // would be a cool feat, goes for mentioning the context in
                  // any item. a map or dict?)
                  int ctxId = getIdFromString(&ContextNameList,
                                               KateHlManager::self()->syntax->groupKateHlItemData( data, QString("context")),dummy); // the index is *required*
                  if ( ctxId > -1) { // we can even reuse rules of 0 if we want to:)
                    kdDebug(13010)<<"makeContextList["<<i<<"]: including all items of context "<<ctxId<<endl;
                    if ( ctxId < (int) i ) { // must be defined
                      for ( c = m_contexts[ctxId]->items.first(); c; c = m_contexts[ctxId]->items.next() )
                        m_contexts[i]->items.append(c);
                    }
                    else
                      kdDebug(13010)<<"Context "<<ctxId<<"not defined. You can not include the rules of an undefined context"<<endl;
                  }
                  continue; // while nextItem
                }
#endif
      c=createYzisHlItem(data,iDl,&RegionList,&ContextNameList);
      if (c)
      {
        m_contexts[i]->items.append(c);

        // Not supported completely atm and only one level. Subitems.(all have
        // to be matched to at once)
        datasub=YzisHlManager::self()->syntax->getSubItems(data);
        bool tmpbool;
        if (tmpbool=YzisHlManager::self()->syntax->nextItem(datasub))
        {
          for (;tmpbool;tmpbool=YzisHlManager::self()->syntax->nextItem(datasub))
          {
            c->subItems.resize (c->subItems.size()+1);
            c->subItems[c->subItems.size()-1] = createYzisHlItem(datasub,iDl,&RegionList,&ContextNameList);
          }                             }
          YzisHlManager::self()->syntax->freeGroupInfo(datasub);
                              // end of sublevel
        }
      }
      i++;
    }
  }

  YzisHlManager::self()->syntax->freeGroupInfo(data);

  if (RegionList.count()!=1)
    folding=true;

  folding = folding || m_foldingIndentationSensitive;

  //BEGIN Resolve multiline region if possible
  if (!m_additionalData[ ident ]->multiLineRegion.isEmpty()) {
    long commentregionid=RegionList.indexOf(m_additionalData[ ident ]->multiLineRegion );
    if (-1==commentregionid) {
      errorsAndWarnings+=QString("<B>%1</B>: Specified multiline comment region (%2) could not be resolved<BR>").arg(buildIdentifier).arg(m_additionalData[ ident ]->multiLineRegion);
      m_additionalData[ ident ]->multiLineRegion = QString();
      dbg()<<"ERROR comment region attribute could not be resolved"<<endl;

    } else {
      m_additionalData[ ident ]->multiLineRegion=QString::number(commentregionid+1);
      deepdbg()<<"comment region resolved to:"<<m_additionalData[ ident ]->multiLineRegion<<endl;
    }
  }
  //END Resolve multiline region if possible
  return i;
}

void YzisHighlighting::clearAttributeArrays ()
{
  for (  QHash< int, QVector<YzisAttribute> * >::iterator it(  m_attributeArrays.begin() ); it != m_attributeArrays.end(); ++it )
  {
    // k, schema correct, let create the data
    YzisAttributeList defaultStyleList;
//	defaultStyleList.setAutoDelete(true); //FIXME qt4 port (memleak ?)
    YzisHlManager::self()->getDefaults(it.key(), defaultStyleList);

    YzisHlItemDataList itemDataList;
    getYzisHlItemDataList(it.key(), itemDataList);

    uint nAttribs = itemDataList.count();
    QVector<YzisAttribute>* array = it.value();
    array->resize (nAttribs);

    for (uint z = 0; z < nAttribs; z++)
    {
      YzisHlItemData *itemData = itemDataList.at(z);
      YzisAttribute n = *defaultStyleList.at(itemData->defStyleNum);

      if (itemData && itemData->isSomethingSet())
        n += *itemData;

      (*array)[z] = n;
    }
	//clear the defaultStyleList //NOT A GOOD IDEA
/*	foreach( YzisAttribute *a, defaultStyleList ) 
		delete a;
	foreach( YzisAttribute *a, itemDataList )
		delete a;
*/
  }
}

QVector<YzisAttribute> *YzisHighlighting::attributes (uint schema)
{
  QVector<YzisAttribute> *array;

  // found it, already floating around
  if ((array = m_attributeArrays[schema]))
    return array;

  // ohh, not found, check if valid schema number
  if (!YSession::self()->schemaManager()->validSchema(schema))
  {
    // uhh, not valid :/, stick with normal default schema, it's always there !
    return attributes (0);
  }

  // k, schema correct, let create the data
  YzisAttributeList defaultStyleList;
  YzisHlManager::self()->getDefaults(schema, defaultStyleList);

  YzisHlItemDataList itemDataList;
  getYzisHlItemDataList(schema, itemDataList);

  uint nAttribs = itemDataList.count();
  array = new QVector<YzisAttribute> (nAttribs);

  for (uint z = 0; z < nAttribs; z++)
  {
    YzisHlItemData *itemData = itemDataList.at(z);
    YzisAttribute n = *defaultStyleList.at(itemData->defStyleNum);

    if (itemData && itemData->isSomethingSet())
      n += *itemData;

    (*array)[z] = n;
  }

  m_attributeArrays.insert(schema, array);

  return array;
}

void YzisHighlighting::getYzisHlItemDataListCopy (uint schema, YzisHlItemDataList &outlist)
{
  YzisHlItemDataList itemDataList;
  getYzisHlItemDataList(schema, itemDataList);

  outlist.clear ();
  for (int z=0; z < itemDataList.count(); z++)
    outlist.append (new YzisHlItemData (*itemDataList.at(z)));
}

//END
//BEGIN YzisHlManager

#undef deepdbg
#undef dbg
#undef err
#define err()     yzError("YzisHlManager")
#define dbg()     yzDebug("YzisHlManager")
#define deepdbg() yzDeepDebug("YzisHlManager")

YzisHlManager::YzisHlManager()
  : commonSuffixes (QString(".orig;.new;~;.bak;.BAK").split(";"))
  , syntax (new YzisSyntaxDocument())
  , dynamicCtxsCount(0)
  , forceNoDCReset(false)
{

  YzisSyntaxModeList modeList = syntax->modeList();
  for (int i=0; i < modeList.count(); i++)
  {
    YzisHighlighting *hl = new YzisHighlighting(modeList[i]);

    int insert = 0;
    for (; insert <= hlList.count(); insert++)
    {
      if (insert == hlList.count())
        break;

      if (  QString( hlList.at( insert )->section() + hlList.at( insert )->nameTranslated() ).toLower()
    		  > QString( hl->section() + hl->nameTranslated() ).toLower() )
        break;
    }

    hlList.insert (insert, hl);
    hlDict.insert (hl->name(), hl);

  }

  // Normal HL
  YzisHighlighting *hl = new YzisHighlighting(0);
  hlList.prepend (hl);
  hlDict.insert (hl->name(), hl);

  lastCtxsReset.start();

  //little hack to fill up our options tree with HL defaults
  YzisAttributeList list;
  getDefaults(0,list);
  setDefaults(0,list);
  foreach( YzisAttribute *a, list )
	  delete a;

  //read init files
  QString resource=resourceMgr()->findResource( ConfigScriptResource, "hl.lua" );
  if (! resource.isEmpty()) YLuaEngine::self()->source( resource );

  magicSet = magic_open( MAGIC_MIME | MAGIC_COMPRESS | MAGIC_SYMLINK );
  if ( magicSet == NULL ) {
    magic_close(magicSet);
  } else {
    const char * magic_db_path = NULL;
    QString magicResource = resourceMgr()->findResource( ConfigResource, "magic.mime" );

    if (! magicResource.isEmpty()) {
        magicResource = magicResource.mid( 0, magicResource.length()-5 );
        magic_db_path = strdup( (const char *) magicResource.toLocal8Bit() );
    }

    if (magic_load( magicSet, magic_db_path ) == -1) {
      dbg() << "YzisHlManager(): magic_load(" << (magic_db_path ? magic_db_path : "NULL" ) << ") error: " << magic_error( magicSet ) << endl;
      magic_close(magicSet);
      magicSet = NULL;
    } else {
      dbg() << "YzisHlManager(): magic database loaded" << endl;
    }
  }
}

YzisHlManager::~YzisHlManager()
{
  if ( magicSet )
    magic_close( magicSet );
  delete syntax;

  // we don't need to do it for hlDict, it uses the same pointers
  foreach( YzisHighlighting *hl, hlList )
	  delete hl;
}

//static KStaticDeleter<YzisHlManager> sdHlMan;

YzisHlManager *YzisHlManager::self()
{
  if ( !s_self )
    s_self = new YzisHlManager ();

  return s_self;
}

YzisHighlighting *YzisHlManager::getHl(int n)
{
  if (n < 0 || n >= (int) hlList.count())
    n = 0;

  return hlList.at(n);
}

int YzisHlManager::nameFind(const QString &name)
{
  int z (hlList.count() - 1);
  for (; z > 0; z--)
    if (hlList.at(z)->name().toLower() == name.toLower())
      return z;

  return z;
}

int YzisHlManager::detectHighlighting (YBuffer *doc)
{
  dbg() << "detectHighlighting( " << doc << " )" << endl;
  int hl = wildcardFind( doc->fileNameShort() );

  if (hl == -1)
  {
    hl = mimeFind( doc->fileNameShort() );
/*	QString buf = "";
    for ( unsigned int i = 0; i < doc->lineCount(); i++ ) {
    	buf += doc->textline( i ) + "\n";
    }
    hl = mimeFind( buf ); */
  /*
    QByteArray buf (YZIS_HL_HOWMANY);
    uint bufpos = 0;
    for (uint i=0; i < doc->lineCount(); i++)
    {
      QString line = doc->textline( i );
      uint len = line.length() + 1;

      if (bufpos + len > YZIS_HL_HOWMANY)
        len = YZIS_HL_HOWMANY - bufpos;

      memcpy(&buf[bufpos], (line + "\n").latin1(), len);

      bufpos += len;

      if (bufpos >= YZIS_HL_HOWMANY)
        break;
    }
    buf.resize( bufpos );
    hl = mimeFind (buf); */
  }

  return hl;
}

int YzisHlManager::wildcardFind(const QString &fileName)
{
  dbg() << "widcardFind( " << fileName << ")" << endl;
  int result = -1;
  if ((result = realWildcardFind(fileName)) != -1)
    return result;

  int length = fileName.length();
  QString backupSuffix = "~";
  if (fileName.endsWith(backupSuffix)) {
    if ((result = realWildcardFind(fileName.left(length - backupSuffix.length()))) != -1)
      return result;
  }

  QStringList::Iterator it = commonSuffixes.begin(), end = commonSuffixes.end();
  for (; it != end; ++it) {
    if (*it != backupSuffix && fileName.endsWith(*it)) {
      if ((result = realWildcardFind(fileName.left(length - (*it).length()))) != -1)
        return result;
    }
  }
  return -1;
}

int YzisHlManager::realWildcardFind(const QString &fileName)
{
  deepdbg() << "realWidcardFind( " << fileName << ")" << endl;
  static QRegExp sep("\\s*;\\s*");

  QList<YzisHighlighting*> highlights;

  for (int ab = 0 ; ab < hlList.size(); ++ab ) {
    YzisHighlighting *highlight = hlList.at(ab);
    highlight->loadWildcards();

    QStringList::Iterator it = highlight->getPlainExtensions().begin(), end = highlight->getPlainExtensions().end();
    for (; it != end; ++it)
      if (fileName.endsWith((*it)))
        highlights.append(highlight);

    for (int i = 0; i < (int)highlight->getRegexpExtensions().count(); i++) {
      QRegExp re = highlight->getRegexpExtensions()[i];
      if (re.exactMatch(fileName))
        highlights.append(highlight);
    }
  }

  if ( !highlights.isEmpty() )
  {
    int pri = -1;
    int hl = -1;

    for (int ab = 0 ; ab < highlights.size(); ++ab ) 
    {
      YzisHighlighting *highlight = highlights.at(ab);
      if (highlight!=0L && highlight->priority() > pri)
      {
        pri = highlight->priority();
        hl = hlList.indexOf (highlight);
      }
    }

    return hl;
  }

  return -1;
}

QString YzisHlManager::findByContent( const QString& contents ) {
    dbg() << "findByContent( " << contents << ")" << endl;
    if ( magicSet == NULL )
    	return QString();
    const char* magic_result = magic_file( magicSet, contents.toUtf8() );
    if ( magic_result ) {
    	dbg() << "findByContent(): Magic for " << contents << " results: " << magic_result << endl;
    	QString mime = QString( magic_result );
    	mime = mime.mid( 0, mime.indexOf( ';' ) );
        dbg() << "findByContent() return " << mime << endl;
    	return mime;
    }
    return QString();
}

int YzisHlManager::mimeFind(const QString &contents)
{
  dbg() << "mimeFind( " << contents << ")" << endl;
  static QRegExp sep("\\s*;\\s*");

  QString mt = findByContent( contents );

  QList<YzisHighlighting*> highlights;

  YzisHighlighting *highlight = hlList.at(0);
  for (int ab = 0 ; ab < hlList.size() && ( highlight=hlList.at( ab ) )!=0L; ++ab )
  {
    deepdbg() << "mimeFind(): checking highlighting " << highlight->name() << endl;
    QStringList l = highlight->getMimetypes().split( sep );

    QStringList::Iterator it = l.begin(), end = l.end();
    for( ; it != end; ++it )
    {
      deepdbg() << "mimeFind(): checking mimetype" << *it << " against "<< mt << endl;
      if ( *it == mt ) // faster than a regexp i guess?
        highlights.append (highlight);
    }
  }

  dbg() << "mimeFind(): number of highlighting found = " << highlights.size() << endl;

  if ( !highlights.isEmpty() )
  {
    int pri = -1;
    int hl = -1;

	YzisHighlighting *highlight = highlights.at(0);
    for (int ab = 0 ; ab < highlights.size() && ( highlight=highlights.at( ab ) )!=0L; ++ab )
    {
      if (highlight->priority() > pri)
      {
        pri = highlight->priority();
        hl = hlList.indexOf (highlight);
      }
    }

    return hl;
  }

  return -1;
}

uint YzisHlManager::defaultStyles()
{
  return 14;
}

QString YzisHlManager::defaultStyleName(int n, bool translateNames)
{
  static QStringList names;
  static QStringList translatedNames;

  if (names.isEmpty())
  {
    names << "Normal";
    names << "Keyword";
    names << "Data Type";
    names << "Decimal/Value";
    names << "Base-N Integer";
    names << "Floating Point";
    names << "Character";
    names << "String";
    names << "Comment";
    names << "Others";
    names << "Alert";
    names << "Function";
    // this next one is for denoting the beginning/end of a user defined folding region
    names << "Region Marker";
    // this one is for marking invalid input
    names << "Error";

    translatedNames << _("Normal");
    translatedNames << _("Keyword");
    translatedNames << _("Data Type");
    translatedNames << _("Decimal/Value");
    translatedNames << _("Base-N Integer");
    translatedNames << _("Floating Point");
    translatedNames << _("Character");
    translatedNames << _("String");
    translatedNames << _("Comment");
    translatedNames << _("Others");
    translatedNames << _("Alert");
    translatedNames << _("Function");
    // this next one is for denoting the beginning/end of a user defined folding region
    translatedNames << _("Region Marker");
    // this one is for marking invalid input
    translatedNames << _("Error");
  }

  return translateNames ? translatedNames[n] : names[n];
}

void YzisHlManager::getDefaults(uint schema, YzisAttributeList &list)
{
  YzisAttribute* normal = new YzisAttribute();
  normal->setTextColor(Qt::white);
  normal->setBGColor(Qt::black);
  normal->setSelectedTextColor(Qt::lightGray);
  list.append(normal);

  YzisAttribute* keyword = new YzisAttribute();
  keyword->setTextColor(Qt::yellow/*white*/);
  keyword->setSelectedTextColor(Qt::black);
//  keyword->setBold(true);
  list.append(keyword);

  YzisAttribute* dataType = new YzisAttribute();
  dataType->setTextColor(Qt::green/*darkRed*/);
  dataType->setSelectedTextColor(Qt::white);
  list.append(dataType);

  YzisAttribute* decimal = new YzisAttribute();
  decimal->setTextColor(Qt::magenta/*blue*/);
  decimal->setSelectedTextColor(Qt::cyan);
  list.append(decimal);

  YzisAttribute* basen = new YzisAttribute();
  basen->setTextColor(Qt::darkCyan);
  basen->setSelectedTextColor(Qt::cyan);
  list.append(basen);

  YzisAttribute* floatAttribute = new YzisAttribute();
  floatAttribute->setTextColor(Qt::darkMagenta);
  floatAttribute->setSelectedTextColor(Qt::cyan);
  list.append(floatAttribute);

  YzisAttribute* charAttribute = new YzisAttribute();
  charAttribute->setTextColor(Qt::magenta);
  charAttribute->setSelectedTextColor(Qt::magenta);
  list.append(charAttribute);

  YzisAttribute* string = new YzisAttribute();
  string->setTextColor(Qt::red);
//  string->setTextColor(YColor("#D00"));
  string->setSelectedTextColor(Qt::red);
  list.append(string);

  YzisAttribute* comment = new YzisAttribute();
  comment->setTextColor(Qt::lightGray/*darkGray*/);
  comment->setSelectedTextColor(Qt::gray);
  comment->setItalic(true);
  list.append(comment);

  YzisAttribute* others = new YzisAttribute();
  others->setTextColor(Qt::darkGreen);
  others->setSelectedTextColor(Qt::green);
  list.append(others);

  YzisAttribute* alert = new YzisAttribute();
  alert->setTextColor(Qt::lightGray);
  alert->setSelectedTextColor( YColor("#FCC") );
  alert->setBold(true);
  alert->setBGColor( YColor("red") );
  list.append(alert);

  YzisAttribute* functionAttribute = new YzisAttribute();
  functionAttribute->setTextColor(Qt::cyan/*darkBlue*/);
  functionAttribute->setSelectedTextColor(Qt::white);
  list.append(functionAttribute);

  YzisAttribute* regionmarker = new YzisAttribute();
  regionmarker->setTextColor(Qt::white);
  regionmarker->setBGColor(Qt::gray);
  regionmarker->setSelectedTextColor(Qt::gray);
  list.append(regionmarker);

  YzisAttribute* error = new YzisAttribute();
  error->setTextColor(Qt::red);
  error->setUnderline(true);
  error->setSelectedTextColor(Qt::red);
  list.append(error);

  YInternalOptionPool* config = YSession::self()->getOptions();
  config->setGroup("Default Item Styles - Schema " + YSession::self()->schemaManager()->name( schema ));

  for (uint z = 0; z < defaultStyles(); z++)
  {
    YzisAttribute *i = list.at(z);
    QStringList s = config->readQStringListEntry(defaultStyleName(z));
    if (!s.isEmpty())
    {
      while( s.count()<8)
        s << "";

      QString tmp;
      QRgb col;

      tmp=s[0]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); i->setTextColor(col); }

      tmp=s[1]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); i->setSelectedTextColor(col); }

      tmp=s[2]; if (!tmp.isEmpty()) i->setBold(tmp!="0");

      tmp=s[3]; if (!tmp.isEmpty()) i->setItalic(tmp!="0");

      tmp=s[4]; if (!tmp.isEmpty()) i->setStrikeOut(tmp!="0");

      tmp=s[5]; if (!tmp.isEmpty()) i->setUnderline(tmp!="0");

      tmp=s[6]; if (!tmp.isEmpty()) {
        if ( tmp != "-" )
        {
          col=tmp.toUInt(0,16);
          i->setBGColor(col);
        }
        else
          i->clearAttribute(YzisAttribute::BGColor);
      }
      tmp=s[7]; if (!tmp.isEmpty()) {
        if ( tmp != "-" )
        {
          col=tmp.toUInt(0,16);
          i->setSelectedBGColor(col);
        }
        else
          i->clearAttribute(YzisAttribute::SelectedBGColor);
      }
    }
  }
}

void YzisHlManager::setDefaults(uint schema, YzisAttributeList &list)
{
  YInternalOptionPool* config = YSession::self()->getOptions();
  config->setGroup("Default Item Styles - Schema " + YSession::self()->schemaManager()->name(schema));

  for (uint z = 0; z < defaultStyles(); z++)
  {
    QStringList settings;
    YzisAttribute *i = list.at(z);

    settings<<(i->itemSet(YzisAttribute::TextColor)?QString::number(i->textColor().rgb(),16):"");
    settings<<(i->itemSet(YzisAttribute::SelectedTextColor)?QString::number(i->selectedTextColor().rgb(),16):"");
    settings<<(i->itemSet(YzisAttribute::Weight)?(i->bold()?"1":"0"):"");
    settings<<(i->itemSet(YzisAttribute::Italic)?(i->italic()?"1":"0"):"");
    settings<<(i->itemSet(YzisAttribute::StrikeOut)?(i->strikeOut()?"1":"0"):"");
    settings<<(i->itemSet(YzisAttribute::Underline)?(i->underline()?"1":"0"):"");
    settings<<(i->itemSet(YzisAttribute::BGColor)?QString::number(i->bgColor().rgb(),16):"-");
    settings<<(i->itemSet(YzisAttribute::SelectedBGColor)?QString::number(i->selectedBGColor().rgb(),16):"-");
    settings<<"---";

    config->setQStringListEntry(defaultStyleName(z),settings);
  }
//  emit changed();
}

int YzisHlManager::highlights()
{
  return (int) hlList.count();
}

QString YzisHlManager::hlName(int n)
{
  return hlList.at(n)->name();
}

QString YzisHlManager::hlNameTranslated(int n)
{
  return hlList.at(n)->nameTranslated();
}

QString YzisHlManager::hlSection(int n)
{
  return hlList.at(n)->section();
}

bool YzisHlManager::hlHidden(int n)
{
  return hlList.at(n)->hidden();
}

QString YzisHlManager::identifierForName(const QString& name)
{
  YzisHighlighting *hl = 0;

  if ((hl = hlDict[name]))
    return hl->getIdentifier ();

  return QString();
}

bool YzisHlManager::resetDynamicCtxs()
{
  if (forceNoDCReset)
    return false;

  if (lastCtxsReset.elapsed() < YZIS_DYNAMIC_CONTEXTS_RESET_DELAY)
    return false;

  YzisHighlighting *hl = hlList.at( 0 );
  for (int ab = 0 ; ab < hlList.size() && ( hl=hlList.at( ab ) )!=0L; ++ab ) {
    hl->dropDynamicContexts();
  }

  dynamicCtxsCount = 0;
  lastCtxsReset.start();

  return true;
}
//END

/*
void YzisViewHighlightAction::init()
{
  m_doc = 0;
  subMenus.setAutoDelete( true );

  connect(popupMenu(),SIGNAL(aboutToShow()),this,SLOT(slotAboutToShow()));
}

void YzisViewHighlightAction::updateMenu (Yzis::Document *doc)
{
  m_doc = doc;
}

void YzisViewHighlightAction::slotAboutToShow()
{
  Yzis::Document *doc=m_doc;
  int count = YzisHlManager::self()->highlights();

  for (int z=0; z<count; z++)
  {
    QString hlName = YzisHlManager::self()->hlNameTranslated (z);
    QString hlSection = YzisHlManager::self()->hlSection (z);

    if (!YzisHlManager::self()->hlHidden(z))
    {
      if ( !hlSection.isEmpty() && !names.contains(hlName) )
      {
        if (!subMenusName.contains(hlSection))
        {
          subMenusName << hlSection;
          QPopupMenu *menu = new QPopupMenu ();
          subMenus.append(menu);
          popupMenu()->insertItem ( '&' + hlSection, menu);
        }

        int m = subMenusName.findIndex (hlSection);
        names << hlName;
        subMenus.at(m)->insertItem ( '&' + hlName, this, SLOT(setHl(int)), 0,  z);
      }
      else if (!names.contains(hlName))
      {
        names << hlName;
        popupMenu()->insertItem ( '&' + hlName, this, SLOT(setHl(int)), 0,  z);
      }
    }
  }

  if (!doc) return;

  for (uint i=0;i<subMenus.count();i++)
  {
    for (uint i2=0;i2<subMenus.at(i)->count();i2++)
    {
      subMenus.at(i)->setItemChecked(subMenus.at(i)->idAt(i2),false);
    }
  }
  popupMenu()->setItemChecked (0, false);

  int i = subMenusName.findIndex (YzisHlManager::self()->hlSection(doc->hlMode()));
  if (i >= 0 && subMenus.at(i))
    subMenus.at(i)->setItemChecked (doc->hlMode(), true);
  else
    popupMenu()->setItemChecked (0, true);
}

void YzisViewHighlightAction::setHl (int mode)
{
  Yzis::Document *doc=m_doc;

  if (doc)
    doc->setHlMode((uint)mode);
}
*/
// kate: space-indent on; indent-width 2; replace-tabs on;
