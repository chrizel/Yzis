/* This file is part of the KDE libraries
   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>
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

//BEGIN INCLUDES
#include "syntaxhighlight.h"
#include "syntaxhighlight.moc"

#include "line.h"
#include "syntaxdocument.h"
#include "debug.h"
#include "buffer.h"

#include <qstringlist.h>
#include <qtextstream.h>
//END

// same as in kmimemagic, no need to feed more data
#define KATE_HL_HOWMANY 1024

//BEGIN  Prviate HL classes

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
    YzisHlContext (int attribute, int lineEndContext,int _lineBeginContext,
               bool _fallthrough, int _fallthroughContext);

    QPtrList<YzisHlItem> items;
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

  return YzisHlItemData::dsNormal;
}
//END

//BEGIN YzisHlItem
YzisHlItem::YzisHlItem(int attribute, int context,signed char regionId,signed char regionId2)
  : attr(attribute), ctx(context),region(regionId),region2(regionId2), lookAhead(false)  {
	  subItems=0;
}

YzisHlItem::~YzisHlItem()
{
  //yzDebug(13010)<<"In hlItem::~YzisHlItem()"<<endl;
  if (subItems!=0)
  {
    subItems->setAutoDelete(true);
    subItems->clear();
    delete subItems;
  }
}

bool YzisHlItem::startEnable(const QChar& c)
{
  // ONLY called when alwaysStartEnable() overridden
  // IN FACT not called at all, copied into doHighlight()...
  Q_ASSERT(false);
  return stdDeliminator.find(c) != -1;
}
//END

//BEGIN HLCharDetect
YzisHlCharDetect::YzisHlCharDetect(int attribute, int context, signed char regionId,signed char regionId2, QChar c)
  : YzisHlItem(attribute,context,regionId,regionId2)
  , sChar(c)
{
}

int YzisHlCharDetect::checkHgl(const QString& text, int offset, int len)
{
  if (len && text[offset] == sChar)
    return offset + 1;

  return 0;
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
  if (len < 2)
    return offset;

  if (text[offset++] == sChar1 && text[offset++] == sChar2)
    return offset;

  return 0;
}
//END

//BEGIN YzisHlStringDetect
YzisHlStringDetect::YzisHlStringDetect(int attribute, int context, signed char regionId,signed char regionId2,const QString &s, bool inSensitive)
  : YzisHlItem(attribute, context,regionId,regionId2)
  , str(inSensitive ? s.upper() : s)
  , _inSensitive(inSensitive)
{
}

int YzisHlStringDetect::checkHgl(const QString& text, int offset, int len)
{
  if (len < (int)str.length())
    return 0;

  if (QConstString(text.unicode() + offset, str.length()).string().find(str, 0, !_inSensitive) == 0)
    return offset + str.length();

  return 0;
}

//END

//BEGIN HLRangeDetect
YzisHlRangeDetect::YzisHlRangeDetect(int attribute, int context, signed char regionId,signed char regionId2, QChar ch1, QChar ch2)
  : YzisHlItem(attribute,context,regionId,regionId2)
  , sChar1 (ch1)
  , sChar2 (ch2)
{
}

int YzisHlRangeDetect::checkHgl(const QString& text, int offset, int len)
{
  if ((len > 0) && (text[offset] == sChar1))
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
  , dict (113, casesensitive)
  , _caseSensitive(casesensitive)
  , deliminators(delims)
{
}

bool YzisHlKeyword::alwaysStartEnable() const
{
  return false;
}

bool YzisHlKeyword::hasCustomStartEnable() const
{
  return true;
}

bool YzisHlKeyword::startEnable(const QChar& c)
{
  return deliminators.find(c) != -1;
}

// If we use a dictionary for lookup we don't really need
// an item as such we are using the key to lookup
void YzisHlKeyword::addWord(const QString &word)
{
  dict.insert(word,&trueBool);
}

void YzisHlKeyword::addList(const QStringList& list)
{
  for(uint i=0;i<list.count();i++) dict.insert(list[i], &trueBool);
}

int YzisHlKeyword::checkHgl(const QString& text, int offset, int len)
{
  if (len == 0 || dict.isEmpty()) return 0;

  int offset2 = offset;

  while (len > 0 && deliminators.find(text[offset2]) == -1 )
  {
    offset2++;
    len--;
  }

  if (offset2 == offset) return 0;

  if ( dict.find(text.mid(offset, offset2 - offset)) ) return offset2;

  return 0;
}
//END

//BEGIN YzisHlInt
YzisHlInt::YzisHlInt(int attribute, int context, signed char regionId,signed char regionId2)
  : YzisHlItem(attribute,context,regionId,regionId2)
{
}

bool YzisHlInt::alwaysStartEnable() const
{
  return false;
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
    if (subItems)
    {
      for (YzisHlItem *it = subItems->first(); it; it = subItems->next())
      {
        if ( (offset = it->checkHgl(text, offset2, len)) )
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
}

bool YzisHlFloat::alwaysStartEnable() const
{
  return false;
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

  if ((len > 0) && ((text[offset] & 0xdf) == 'E'))
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
      if (subItems)
      {
        for (YzisHlItem *it = subItems->first(); it; it = subItems->next())
        {
          int offset2 = it->checkHgl(text, offset, len);

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
    if (subItems)
    {
      for (YzisHlItem *it = subItems->first(); it; it = subItems->next())
      {
        int offset2 = it->checkHgl(text, offset, len);

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
}

bool YzisHlCOct::alwaysStartEnable() const
{
  return false;
}

int YzisHlCOct::checkHgl(const QString& text, int offset, int len)
{
  if ((len > 0) && text[offset] == '0')
  {
    offset++;
    len--;

    int offset2 = offset;

    while ((len > 0) && (text[offset2] >= '0' && text[offset2] <= '7'))
    {
      offset2++;
      len--;
    }

    if (offset2 > offset)
    {
      if ((len > 0) && ((text[offset2] & 0xdf) == 'L' || (text[offset] & 0xdf) == 'U' ))
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
}

bool YzisHlCHex::alwaysStartEnable() const
{
  return false;
}

int YzisHlCHex::checkHgl(const QString& text, int offset, int len)
{
  if ((len > 1) && (text[offset++] == '0') && ((text[offset++] & 0xdf) == 'X' ))
  {
    len -= 2;

    int offset2 = offset;

    while ((len > 0) && (text[offset2].isDigit() || ((text[offset2] & 0xdf) >= 'A' && (text[offset2] & 0xdf) <= 'F')))
    {
      offset2++;
      len--;
    }

    if (offset2 > offset)
    {
      if ((len > 0) && ((text[offset2] & 0xdf) == 'L' || (text[offset2] & 0xdf) == 'U' ))
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
}

bool YzisHlCFloat::alwaysStartEnable() const
{
  return false;
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
    if ((text[offset2] & 0xdf) == 'F' )
      offset2++;

    return offset2;
  }
  else
  {
    offset2 = checkIntHgl(text, offset, len);

    if (offset2 && ((text[offset2] & 0xdf) == 'F' ))
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

int YzisHlAnyChar::checkHgl(const QString& text, int offset, int len)
{
  if ((len > 0) && _charList.find(text[offset]) != -1)
    return ++offset;

  return 0;
}
//END

//BEGIN YzisHlRegExpr
YzisHlRegExpr::YzisHlRegExpr( int attribute, int context, signed char regionId,signed char regionId2, QString regexp, bool insensitive, bool minimal )
  : YzisHlItem(attribute, context, regionId,regionId2)
  , handlesLinestart (regexp.startsWith("^"))
{
  if (!handlesLinestart)
    regexp.prepend("^");

  Expr = new QRegExp(regexp, !insensitive);
  Expr->setMinimal(minimal);
}

int YzisHlRegExpr::checkHgl(const QString& text, int offset, int /*len*/)
{
  if (offset && handlesLinestart)
    return 0;

  int offset2 = Expr->search( text, offset, QRegExp::CaretAtOffset );

  if (offset2 == -1) return 0;

  return (offset + Expr->matchedLength());
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

    switch(text[offset])
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
        for (i = 0; (len > 0) && (i < 2) && (text[offset] >= '0' && text[offset] <= '9' || (text[offset] & 0xdf) >= 'A' && (text[offset] & 0xdf) <= 'F'); i++)
        {
          offset++;
          len--;
        }

        if (i == 0)
          return 0; // takes care of case '\x'

        break;

      case '0': case '1': case '2': case '3' :
      case '4': case '5': case '6': case '7' :
        for (i = 0; (len > 0) && (i < 3) && (text[offset] >='0'&& text[offset] <='7'); i++)
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

YzisHlItemData::YzisHlItemData(const QString  name, int defStyleNum)
  : name(name), defStyleNum(defStyleNum) {
}

YzisHlData::YzisHlData(const QString &wildcards, const QString &mimetypes, const QString &identifier, int priority)
  : wildcards(wildcards), mimetypes(mimetypes), identifier(identifier), priority(priority)
{
}

YzisHlContext::YzisHlContext (int attribute, int lineEndContext, int _lineBeginContext, bool _fallthrough, int _fallthroughContext)
{
  attr = attribute;
  ctx = lineEndContext;
  lineBeginContext = _lineBeginContext;
  fallthrough = _fallthrough;
  ftctx = _fallthroughContext;
}

YzisHl2CharDetect::YzisHl2CharDetect(int attribute, int context, signed char regionId,signed char regionId2, const QChar *s)
  : YzisHlItem(attribute,context,regionId,regionId2) {
  sChar1 = s[0];
  sChar2 = s[1];
}

//BEGIN YzisHighlighting
YzisHighlighting::YzisHighlighting(const YzisSyntaxModeListItem *def) : refCount(0)
{
  m_attributeArrays.setAutoDelete (true);

  errorsAndWarnings = "";
  building=false;
  noHl = false;
  m_foldingIndentationSensitive = false;
  folding=false;
  internalIDList.setAutoDelete(true);

  if (def == 0)
  {
    noHl = true;
    iName = "None";
    iSection = "";
    m_priority = 0;
  }
  else
  {
    iName = def->name;
    iSection = def->section;
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
  contextList.setAutoDelete( true );
}

void YzisHighlighting::generateContextStack(int *ctxNum, int ctx, QMemArray<short>* ctxs, int *prevLine, bool lineContinue)
{
  //yzDebug(13010)<<QString("Entering generateContextStack with %1").arg(ctx)<<endl;

  if (lineContinue)
  {
    if ( !ctxs->isEmpty() )
    {
      (*ctxNum)=(*ctxs)[ctxs->size()-1];
      (*prevLine)--;
    }
    else
    {
      //yzDebug(13010)<<QString("generateContextStack: line continue: len ==0");
      (*ctxNum)=0;
    }

    return;
  }

  if (ctx >= 0)
  {
    (*ctxNum) = ctx;

    ctxs->resize (ctxs->size()+1, QGArray::SpeedOptim);
    (*ctxs)[ctxs->size()-1]=(*ctxNum);
  }
  else
  {
    if (ctx < -1)
    {
      while (ctx < -1)
      {
        if ( ctxs->isEmpty() )
          (*ctxNum)=0;
        else
        {
          ctxs->resize (ctxs->size()-1, QGArray::SpeedOptim);
          //yzDebug(13010)<<QString("generate context stack: truncated stack to :%1").arg(ctxs->size())<<endl;
          (*ctxNum) = ( (ctxs->isEmpty() ) ? 0 : (*ctxs)[ctxs->size()-1]);
        }

        ctx++;
      }

      ctx = 0;

      if ((*prevLine) >= (int)(ctxs->size()-1))
      {
        *prevLine=ctxs->size()-1;

        if ( ctxs->isEmpty() )
          return;

        if (contextNum((*ctxs)[ctxs->size()-1]) && (contextNum((*ctxs)[ctxs->size()-1])->ctx != -1))
        {
          //yzDebug(13010)<<"PrevLine > size()-1 and ctx!=-1)"<<endl;
          generateContextStack(ctxNum, contextNum((*ctxs)[ctxs->size()-1])->ctx,ctxs, prevLine);
          return;
        }
      }
    }
    else
    {
      if (ctx == -1)
        (*ctxNum)=( (ctxs->isEmpty() ) ? 0 : (*ctxs)[ctxs->size()-1]);
    }
  }
}

/*******************************************************************************************
        YzisHighlighting - doHighlight
        Increase the usage count and trigger initialization if needed

                        * input: signed char *oCtx  Pointer to the "stack" of the previous line
         uint *oCtxLen    Size of the stack
         YZLine *textline  Current textline to work on
                        *************
                        * output: (YZLine *textline)
                        *************
                        * return value: signed char*  new context stack at the end of the line
*******************************************************************************************/

void YzisHighlighting::doHighlight ( YZLine *prevLine,
                                     YZLine *textLine,
                                     QMemArray<signed char>* foldingList,
                                     bool *ctxChanged )
{
  if (!textLine)
    return;

  if (noHl)
  {
    textLine->setAttribs(0,0,textLine->length());
    return;
  }

//  yzDebug(13010)<<QString("The context stack length is: %1").arg(oCtx.size())<<endl;
  // if (lineContinue) yzDebug(13010)<<"Entering with lineContinue flag set"<<endl;

  // duplicate the ctx stack, only once !
  QMemArray<short> ctx;
  ctx.duplicate (prevLine->ctxArray());

  // line continue flag !
  bool lineContinue = prevLine->hlLineContinue();

  int ctxNum = 0;
  int previousLine = -1;
  YzisHlContext *context;

  if ( prevLine->ctxArray().isEmpty() )
  {
    // If the stack is empty, we assume to be in Context 0 (Normal)
    context=contextNum(ctxNum);
  }
  else
  {
    // There does an old context stack exist -> find the context at the line start
    ctxNum=ctx[prevLine->ctxArray().size()-1]; //context ID of the last character in the previous line

    //yzDebug(13010) << "\t\tctxNum = " << ctxNum << " contextList[ctxNum] = " << contextList[ctxNum] << endl; // ellis

    //if (lineContinue)   yzDebug(13010)<<QString("The old context should be %1").arg((int)ctxNum)<<endl;

    if (!(context = contextNum(ctxNum)))
      context = contextNum(0);

    //yzDebug(13010)<<"test1-2-1-text2"<<endl;

    previousLine=prevLine->ctxArray().size()-1; //position of the last context ID of th previous line within the stack

    //yzDebug(13010)<<"test1-2-1-text3"<<endl;
    generateContextStack(&ctxNum, context->ctx, &ctx, &previousLine, lineContinue); //get stack ID to use

    //yzDebug(13010)<<"test1-2-1-text4"<<endl;

    if (!(context = contextNum(ctxNum)))
      context = contextNum(0);

    //if (lineContinue)   yzDebug(13010)<<QString("The new context is %1").arg((int)ctxNum)<<endl;
  }

  // text, for programming convenience :)
  QChar lastChar = ' ';
  const QString& text = textLine->data();
  uint len = textLine->length();

  int offset1 = 0;
  uint z = 0;
  YzisHlItem *item = 0;

  while (z < len)
  {
    bool found = false;
    bool standardStartEnableDetermined = false;
    bool standardStartEnable = false;

    for (item = context->items.first(); item != 0L; item = context->items.next())
    {
      bool thisStartEnabled = false;

      if (item->alwaysStartEnable())
      {
        thisStartEnabled = true;
      }
      else if (!item->hasCustomStartEnable())
      {
        if (!standardStartEnableDetermined)
        {
          standardStartEnable = stdDeliminator.find(lastChar) != -1;
          standardStartEnableDetermined = true;
        }

        thisStartEnabled = standardStartEnable;
      }
      else if (item->startEnable(lastChar))
      {
        thisStartEnabled = true;
      }

      if (thisStartEnabled)
      {
        int offset2 = item->checkHgl(text, offset1, len-z);

        if (offset2 > offset1)
        {
          if(!item->lookAhead)
            textLine->setAttribs(item->attr,offset1,offset2);

          if (item->region)
          {
//              yzDebug(13010)<<QString("Region mark detected: %1").arg(item->region)<<endl;

            if ( !foldingList->isEmpty() && ((item->region < 0) && (*foldingList)[foldingList->size()-1] == -item->region ) )
            {
              foldingList->resize (foldingList->size()-1, QGArray::SpeedOptim);
            }
            else
            {
              foldingList->resize (foldingList->size()+1, QGArray::SpeedOptim);
              (*foldingList)[foldingList->size()-1] = item->region;
            }

          }

          if (item->region2)
          {
//              yzDebug(13010)<<QString("Region mark 2 detected: %1").arg(item->region2)<<endl;

            if ( !foldingList->isEmpty() && ((item->region2 < 0) && (*foldingList)[foldingList->size()-1] == -item->region2 ) )
            {
              foldingList->resize (foldingList->size()-1, QGArray::SpeedOptim);
            }
            else
            {
              foldingList->resize (foldingList->size()+1, QGArray::SpeedOptim);
              (*foldingList)[foldingList->size()-1] = item->region2;
            }

          }

          generateContextStack(&ctxNum, item->ctx, &ctx, &previousLine);  //regenerate context stack

      //yzDebug(13010)<<QString("generateContextStack has been left in item loop, size: %1").arg(ctx.size())<<endl;
    //    yzDebug(13010)<<QString("current ctxNum==%1").arg(ctxNum)<<endl;

          context=contextNum(ctxNum);

          // dominik: look ahead w/o changing offset?
          if (!item->lookAhead)
          {
            z = z + offset2 - offset1 - 1;
            offset1 = offset2 - 1;
          }
          found = true;
          break;
        }
      }
    }

    lastChar = text[offset1];

    // nothing found: set attribute of one char
    // anders: unless this context does not want that!
    if (!found)
    {
      if ( context->fallthrough )
      {
        // set context to context->ftctx.
        generateContextStack(&ctxNum, context->ftctx, &ctx, &previousLine);  //regenerate context stack
        context=contextNum(ctxNum);
        //yzDebug(13010)<<"context num after fallthrough at col "<<z<<": "<<ctxNum<<endl;
        // the next is nessecary, as otherwise keyword (or anything using the std delimitor check)
        // immediately after fallthrough fails. Is it bad?
        // jowenn, can you come up with a nicer way to do this?
        if (z)
          lastChar = text[offset1 - 1];
        else
          lastChar = '\\';
        continue;
      }
      else {
        textLine->setAttribs(context->attr,offset1,offset1 + 1);
	  }
    }

    // dominik: do not change offset if we look ahead
    if (!(item && item->lookAhead))
    {
      offset1++;
      z++;
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
}

void YzisHighlighting::loadWildcards()
{
/*  KConfig *config = YzisHlManager::self()->getKConfig();
  config->setGroup("Highlighting " + iName);

  QString extensionString = config->readEntry("Wildcards", iWildcards);

  if (extensionSource != extensionString) {
  */
	QString extensionString = iWildcards;
    regexpExtensions.clear();
    plainExtensions.clear();

    extensionSource = extensionString;

    static QRegExp sep("\\s*;\\s*");

    QStringList l = QStringList::split( sep, extensionSource );

    static QRegExp boringExpression("\\*\\.[\\d\\w]+");

    for( QStringList::Iterator it = l.begin(); it != l.end(); ++it )
      if (boringExpression.exactMatch(*it))
        plainExtensions.append((*it).mid(1));
      else
        regexpExtensions.append(QRegExp((*it), true, true));
	/*
  }*/
}

QValueList<QRegExp>& YzisHighlighting::getRegexpExtensions()
{
  return regexpExtensions;
}

QStringList& YzisHighlighting::getPlainExtensions()
{
  return plainExtensions;
}

QString YzisHighlighting::getMimetypes()
{/*
  KConfig *config = YzisHlManager::self()->getKConfig();
  config->setGroup("Highlighting " + iName);

  return config->readEntry("Mimetypes", iMimetypes);*/
	return QString();
}

int YzisHighlighting::priority()
{/*
  KConfig *config = YzisHlManager::self()->getKConfig();
  config->setGroup("Highlighting " + iName);

  return config->readNumEntry("Priority", m_priority);*/
	return 0;
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

void YzisHighlighting::setData(YzisHlData *hlData)
{/*
  KConfig *config = YzisHlManager::self()->getKConfig();
  config->setGroup("Highlighting " + iName);

  config->writeEntry("Wildcards",hlData->wildcards);
  config->writeEntry("Mimetypes",hlData->mimetypes);
  config->writeEntry("Priority",hlData->priority);*/
}

void YzisHighlighting::getYzisHlItemDataList (uint schema, YzisHlItemDataList &list)
{
  list.clear();
  createYzisHlItemData(list);

	/*
  KConfig *config = YzisHlManager::self()->getKConfig();
  config->setGroup("Highlighting " + iName + " - Schema " + YzisFactory::self()->schemaManager()->name(schema));

  list.clear();
  createYzisHlItemData(list);

  for (YzisHlItemData *p = list.first(); p != 0L; p = list.next())
  {
    QStringList s = config->readListEntry(p->name);

//    yzDebug()<<p->name<<s.count()<<endl;
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
  }*/
}

/*******************************************************************************************
        YzisHighlighting - setYzisHlItemDataList
        saves the YzisHlItemData / attribute / style definitions to the apps configfile.
        Especially needed for user overridden values.

                        * input: YzisHlItemDataList &list             :reference to the list, whose
                        *                                        items should be saved
                        *        KConfig *config                :Pointer KDE configuration
                        *                                        class, which should be used
                        *                                        as storage
                        *************
                        * output: none
                        *************
                        * return value: none
*******************************************************************************************/

void YzisHighlighting::setYzisHlItemDataList(uint schema, YzisHlItemDataList &list)
{/*
  KConfig *config = YzisHlManager::self()->getKConfig();
  config->setGroup("Highlighting " + iName + " - Schema " + YzisFactory::self()->schemaManager()->name(schema));

  QStringList settings;

  for (YzisHlItemData *p = list.first(); p != 0L; p = list.next())
  {
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
    config->writeEntry(p->name,settings);
  }*/
}

/*******************************************************************************************
        YzisHighlighting - use
        Increase the usage count and trigger initialization if needed

                        * input: none
                        *************
                        * output: none
                        *************
                        * return value: none
*******************************************************************************************/

void YzisHighlighting::use()
{
  if (refCount == 0)
    init();

  refCount++;
}

/*******************************************************************************************
        YzisHighlighting - release
        Decrease the usage count and trigger a cleanup if needed

                        * input: none
                        *************
                        * output: none
                        *************
                        * return value: none
*******************************************************************************************/

void YzisHighlighting::release()
{
  refCount--;

  if (refCount == 0)
    done();
}

/*******************************************************************************************
        YzisHighlighting - init
        If it's the first time a particular highlighting is used create the needed contextlist

                        * input: none
                        *************
                        * output: none
                        *************
                        * return value: none
*******************************************************************************************/

void YzisHighlighting::init()
{
  if (noHl)
    return;

  contextList.clear ();
  makeContextList();
}


/*******************************************************************************************
        YzisHighlighting - done
        If the there is no document using the highlighting style free the complete context
        structure.

                        * input: none
                        *************
                        * output: none
                        *************
                        * return value: none
*******************************************************************************************/

void YzisHighlighting::done()
{
  if (noHl)
    return;

  contextList.clear ();
}

/*******************************************************************************************
        YzisHighlighting - createYzisHlItemData
        This function reads the itemData entries from the config file, which specifies the
        default attribute styles for matched items/contexts.

                        * input: none
                        *************
                        * output: YzisHlItemDataList &list            :A reference to the internal
                                                                list containing the parsed
                                                                default config
                        *************
                        * return value: none
*******************************************************************************************/

void YzisHighlighting::createYzisHlItemData(YzisHlItemDataList &list)
{
  // If no highlighting is selected we need only one default.
  if (noHl)
  {
    list.append(new YzisHlItemData("Normal Text", YzisHlItemData::dsNormal));
    return;
  }

  // If the internal list isn't already available read the config file
  if (internalIDList.isEmpty())
    makeContextList();

  list=internalIDList;
}

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
              buildPrefix+YzisHlManager::self()->syntax->groupData(data,QString("name")).simplifyWhiteSpace(),
              getDefStyleNum(YzisHlManager::self()->syntax->groupData(data,QString("defStyleNum"))));


      /* here the custom style overrides are specified, if needed */
      if (!color.isEmpty()) newData->setTextColor(QColor(color));
      if (!selColor.isEmpty()) newData->setSelectedTextColor(QColor(selColor));
      if (!bold.isEmpty()) newData->setBold(bold=="true" || bold=="1");
      if (!italic.isEmpty()) newData->setItalic(italic=="true" || italic=="1");
      // new attributes for the new rendering view
      if (!underline.isEmpty()) newData->setUnderline(underline=="true" || underline=="1");
      if (!strikeOut.isEmpty()) newData->setStrikeOut(strikeOut=="true" || strikeOut=="1");
      if (!bgColor.isEmpty()) newData->setBGColor(QColor(bgColor));
      if (!selBgColor.isEmpty()) newData->setSelectedBGColor(QColor(selBgColor));

      internalIDList.append(newData);
  }

  //clean up
  if (data)
    YzisHlManager::self()->syntax->freeGroupInfo(data);
}

/*******************************************************************************************
        YzisHighlighting - lookupAttrName
        This function is  a helper for makeContextList and createYzisHlItem. It looks the given
        attribute name in the itemData list up and returns it's index

                        * input: QString &name                  :the attribute name to lookup
                        *        YzisHlItemDataList &iDl               :the list containing all
                        *                                         available attributes
                        *************
                        * output: none
                        *************
                        * return value: int                     :The index of the attribute
                        *                                        or 0
*******************************************************************************************/

int  YzisHighlighting::lookupAttrName(const QString& name, YzisHlItemDataList &iDl)
{
  for (uint i = 0; i < iDl.count(); i++)
    if (iDl.at(i)->name == buildPrefix+name)
      return i;

  yzDebug(13010)<<"Couldn't resolve itemDataName"<<endl;
  return 0;
}

/*******************************************************************************************
        YzisHighlighting - createYzisHlItem
        This function is  a helper for makeContextList. It parses the xml file for
        information, how single or multi line comments are marked

                        * input: YzisSyntaxContextData *data : Data about the item read from
                        *                                  the xml file
                        *        YzisHlItemDataList &iDl :       List of all available itemData
                        *                                   entries. Needed for attribute
                        *                                   name->index translation
      *   QStringList *RegionList  : list of code folding region names
      *   QStringList ContextList  : list of context names
                        *************
                        * output: none
                        *************
                        * return value: YzisHlItem * :          Pointer to the newly created item
                        *                                   object
*******************************************************************************************/

YzisHlItem *YzisHighlighting::createYzisHlItem(struct YzisSyntaxContextData *data, YzisHlItemDataList &iDl,QStringList *RegionList, QStringList *ContextNameList)
{
  // No highlighting -> exit
  if (noHl)
    return 0;

  // get the (tagname) itemd type
  QString dataname=YzisHlManager::self()->syntax->groupItemData(data,QString(""));

  // BEGIN - Translation of the attribute parameter
  QString tmpAttr=YzisHlManager::self()->syntax->groupItemData(data,QString("attribute")).simplifyWhiteSpace();
  int attr;
  if (QString("%1").arg(tmpAttr.toInt())==tmpAttr)
  {
    errorsAndWarnings+=QString( "<B>%1</B>: Deprecated syntax. Attribute (%2) not addressed by symbolic name<BR>" ).arg(buildIdentifier).arg(tmpAttr);
    attr=tmpAttr.toInt();
  }
  else
    attr=lookupAttrName(tmpAttr,iDl);
  // END - Translation of the attribute parameter

  // Info about context switch
  int context;
  QString tmpcontext=YzisHlManager::self()->syntax->groupItemData(data,QString("context"));


  QString unresolvedContext;
  context=getIdFromString(ContextNameList, tmpcontext,unresolvedContext);

  // Get the char parameter (eg DetectChar)
  char chr;
  if (! YzisHlManager::self()->syntax->groupItemData(data,QString("char")).isEmpty())
    chr= (YzisHlManager::self()->syntax->groupItemData(data,QString("char")).latin1())[0];
  else
    chr=0;

  // Get the String parameter (eg. StringDetect)
  QString stringdata=YzisHlManager::self()->syntax->groupItemData(data,QString("String"));

  // Get a second char parameter (char1) (eg Detect2Chars)
  char chr1;
  if (! YzisHlManager::self()->syntax->groupItemData(data,QString("char1")).isEmpty())
    chr1= (YzisHlManager::self()->syntax->groupItemData(data,QString("char1")).latin1())[0];
  else
    chr1=0;

  // Will be removed eventuall. Atm used for StringDetect
  bool insensitive=( YzisHlManager::self()->syntax->groupItemData(data,QString("insensitive")).lower() == QString("true") );
  // anders: very reasonable for regexp too!

  // for regexp only
  bool minimal = ( YzisHlManager::self()->syntax->groupItemData(data,QString("minimal")).lower() == QString("true") );

  // dominik: look ahead and do not change offset. so we can change contexts w/o changing offset1.
  bool lookAhead=( YzisHlManager::self()->syntax->groupItemData(data,QString("lookAhead")).lower() == QString("true") );


  // code folding region handling:
  QString beginRegionStr=YzisHlManager::self()->syntax->groupItemData(data,QString("beginRegion"));
  QString endRegionStr=YzisHlManager::self()->syntax->groupItemData(data,QString("endRegion"));

  signed char regionId=0;
  signed char regionId2=0;

  if (!beginRegionStr.isEmpty())
  {
    regionId = RegionList->findIndex(beginRegionStr);

    if (regionId==-1) // if the region name doesn't already exist, add it to the list
    {
      (*RegionList)<<beginRegionStr;
      regionId = RegionList->findIndex(beginRegionStr);
    }

    regionId++;

    yzDebug () << "########### BEG REG: "  << beginRegionStr << " NUM: " << regionId << endl;
  }

  if (!endRegionStr.isEmpty())
  {
    regionId2 = RegionList->findIndex(endRegionStr);

    if (regionId2==-1) // if the region name doesn't already exist, add it to the list
    {
      (*RegionList)<<endRegionStr;
      regionId2 = RegionList->findIndex(endRegionStr);
    }

    regionId2 = -regionId2 - 1;

    yzDebug () << "########### END REG: "  << endRegionStr << " NUM: " << regionId2 << endl;
  }

  //Create the item corresponding to it's type and set it's parameters
  YzisHlItem *tmpItem;

  if (dataname=="keyword")
  {
    YzisHlKeyword *keyword=new YzisHlKeyword(attr,context,regionId,regionId2,casesensitive,
      deliminator);

    //Get the entries for the keyword lookup list
    keyword->addList(YzisHlManager::self()->syntax->finddata("highlighting",stringdata));
    tmpItem=keyword;
  } else
    if (dataname=="Float") tmpItem= (new YzisHlFloat(attr,context,regionId,regionId2)); else
    if (dataname=="Int") tmpItem=(new YzisHlInt(attr,context,regionId,regionId2)); else
    if (dataname=="DetectChar") tmpItem=(new YzisHlCharDetect(attr,context,regionId,regionId2,chr)); else
    if (dataname=="Detect2Chars") tmpItem=(new YzisHl2CharDetect(attr,context,regionId,regionId2,chr,chr1)); else
    if (dataname=="RangeDetect") tmpItem=(new YzisHlRangeDetect(attr,context,regionId,regionId2, chr, chr1)); else
    if (dataname=="LineContinue") tmpItem=(new YzisHlLineContinue(attr,context,regionId,regionId2)); else
    if (dataname=="StringDetect") tmpItem=(new YzisHlStringDetect(attr,context,regionId,regionId2,stringdata,insensitive)); else
    if (dataname=="AnyChar") tmpItem=(new YzisHlAnyChar(attr,context,regionId,regionId2,stringdata)); else
    if (dataname=="RegExpr") tmpItem=(new YzisHlRegExpr(attr,context,regionId,regionId2,stringdata, insensitive, minimal)); else
    if (dataname=="HlCChar") tmpItem= ( new YzisHlCChar(attr,context,regionId,regionId2));else
    if (dataname=="HlCHex") tmpItem= (new YzisHlCHex(attr,context,regionId,regionId2));else
    if (dataname=="HlCOct") tmpItem= (new YzisHlCOct(attr,context,regionId,regionId2)); else
    if (dataname=="HlCFloat") tmpItem= (new YzisHlCFloat(attr,context,regionId,regionId2)); else
    if (dataname=="HlCStringChar") tmpItem= (new YzisHlCStringChar(attr,context,regionId,regionId2)); else

  {
    // oops, unknown type. Perhaps a spelling error in the xml file
    return 0;
  }

  // set lookAhead property
  tmpItem->lookAhead = lookAhead;

  if (!unresolvedContext.isEmpty())
  {
    unresolvedContextReferences.insert(&(tmpItem->ctx),unresolvedContext);
  }
  return tmpItem;
}


/*******************************************************************************************
        YzisHighlighting - isInWord

                        * input: Qchar c       Character to investigate
                        *************
                        * output: none
                        *************
                        * return value: returns true, if c is no deliminator
*******************************************************************************************/

bool YzisHighlighting::isInWord(QChar c)
{
  static const QString sq("\"'");
  return deliminator.find(c) == -1 && sq.find(c) == -1;
}

/*******************************************************************************************
        YzisHighlighting - readCommentConfig
        This function is  a helper for makeContextList. It parses the xml file for
        information, how single or multi line comments are marked

                        * input: none
                        *************
                        * output: none
                        *************
                        * return value: none
*******************************************************************************************/

void YzisHighlighting::readCommentConfig()
{
  YzisHlManager::self()->syntax->setIdentifier(buildIdentifier);
  YzisSyntaxContextData *data=YzisHlManager::self()->syntax->getGroupInfo("general","comment");

  if (data)
  {
    while  (YzisHlManager::self()->syntax->nextGroup(data))
    {
      if (YzisHlManager::self()->syntax->groupData(data,"name")=="singleLine")
        cslStart=YzisHlManager::self()->syntax->groupData(data,"start");

      if (YzisHlManager::self()->syntax->groupData(data,"name")=="multiLine")
      {
        cmlStart=YzisHlManager::self()->syntax->groupData(data,"start");
        cmlEnd=YzisHlManager::self()->syntax->groupData(data,"end");
      }
    }

    YzisHlManager::self()->syntax->freeGroupInfo(data);
  }
  else
  {
    cslStart = "";
    cmlStart = "";
    cmlEnd = "";
  }
}

/*******************************************************************************************
        YzisHighlighting - readGlobalKeyWordConfig
        This function is  a helper for makeContextList. It parses the xml file for
        information, if keywords should be treated case(in)sensitive and creates the keyword
        delimiter list. Which is the default list, without any given weak deliminiators

                        * input: none
                        *************
                        * output: none
                        *************
                        * return value: none
*******************************************************************************************/


void YzisHighlighting::readGlobalKeywordConfig()
{
  // Tell the syntax document class which file we want to parse
  yzDebug(13010)<<"readGlobalKeywordConfig:BEGIN"<<endl;

  YzisHlManager::self()->syntax->setIdentifier(buildIdentifier);
  YzisSyntaxContextData *data = YzisHlManager::self()->syntax->getConfig("general","keywords");

  if (data)
  {
    yzDebug(13010)<<"Found global keyword config"<<endl;

    if (YzisHlManager::self()->syntax->groupItemData(data,QString("casesensitive"))!="0")
      casesensitive=true;
    else
      casesensitive=false;

    //get the weak deliminators
    weakDeliminator=(YzisHlManager::self()->syntax->groupItemData(data,QString("weakDeliminator")));

    yzDebug(13010)<<"weak delimiters are: "<<weakDeliminator<<endl;

    // remove any weakDelimitars (if any) from the default list and store this list.
    for (uint s=0; s < weakDeliminator.length(); s++)
    {
      int f = deliminator.find (weakDeliminator[s]);

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

  yzDebug(13010)<<"readGlobalKeywordConfig:END"<<endl;

  yzDebug(13010)<<"delimiterCharacters are: "<<deliminator<<endl;
}


void YzisHighlighting::readFoldingConfig()
{
  // Tell the syntax document class which file we want to parse
  yzDebug(13010)<<"readfoldignConfig:BEGIN"<<endl;

  YzisHlManager::self()->syntax->setIdentifier(buildIdentifier);
  YzisSyntaxContextData *data = YzisHlManager::self()->syntax->getConfig("general","folding");

  if (data)
  {
    yzDebug(13010)<<"Found global keyword config"<<endl;

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

  yzDebug(13010)<<"readfoldingConfig:END"<<endl;

  yzDebug(13010)<<"############################ use indent for fold are: "<<m_foldingIndentationSensitive<<endl;
}

void  YzisHighlighting::createContextNameList(QStringList *ContextNameList,int ctx0)
{
  yzDebug(13010)<<"creatingContextNameList:BEGIN"<<endl;

  if (ctx0 == 0)
      ContextNameList->clear();

  YzisHlManager::self()->syntax->setIdentifier(buildIdentifier);

  YzisSyntaxContextData *data=YzisHlManager::self()->syntax->getGroupInfo("highlighting","context");

  int id=ctx0;

  if (data)
  {
     while (YzisHlManager::self()->syntax->nextGroup(data))
     {
          QString tmpAttr=YzisHlManager::self()->syntax->groupData(data,QString("name")).simplifyWhiteSpace();
    if (tmpAttr.isEmpty())
    {
     tmpAttr=QString("!KATE_INTERNAL_DUMMY! %1").arg(id);
     errorsAndWarnings +=QString("<B>%1</B>: Deprecated syntax. Context %2 has no symbolic name<BR>").arg(buildIdentifier).arg(id-ctx0);
    }
          else tmpAttr=buildPrefix+tmpAttr;
    (*ContextNameList)<<tmpAttr;
          id++;
     }
     YzisHlManager::self()->syntax->freeGroupInfo(data);
  }
  yzDebug(13010)<<"creatingContextNameList:END"<<endl;

}

int YzisHighlighting::getIdFromString(QStringList *ContextNameList, QString tmpLineEndContext, /*NO CONST*/ QString &unres)
{
  unres="";
  int context;
  if ((tmpLineEndContext=="#stay") || (tmpLineEndContext.simplifyWhiteSpace().isEmpty())) context=-1;
      else if (tmpLineEndContext.startsWith("#pop"))
      {
           context=-1;
           for(;tmpLineEndContext.startsWith("#pop");context--)
           {
               tmpLineEndContext.remove(0,4);
               yzDebug(13010)<<"#pop found"<<endl;
           }
      }
      else
  if ( tmpLineEndContext.startsWith("##"))
  {
    QString tmp=tmpLineEndContext.right(tmpLineEndContext.length()-2);
    if (!embeddedHls.contains(tmp))  embeddedHls.insert(tmp,YzisEmbeddedHlInfo());
    unres=tmp;
    context=0;
  }
  else
  {
    context=ContextNameList->findIndex(buildPrefix+tmpLineEndContext);
    if (context==-1)
    {
      context=tmpLineEndContext.toInt();
      errorsAndWarnings+=QString("<B>%1</B>:Deprecated syntax. Context %2 not addressed by a symbolic name").arg(buildIdentifier).arg(tmpLineEndContext);
    }
//#warning restructure this the name list storage.
//    context=context+buildContext0Offset;
  }
  return context;
}

/*******************************************************************************************
        YzisHighlighting - makeContextList
        That's the most important initialization function for each highlighting. It's called
        each time a document gets a highlighting style assigned. parses the xml file and
        creates a corresponding internal structure

                        * input: none
                        *************
                        * output: none
                        *************
                        * return value: none
*******************************************************************************************/

void YzisHighlighting::makeContextList()
{
  if (noHl)  // if this a highlighting for "normal texts" only, tere is no need for a context list creation
    return;

  embeddedHls.clear();
  unresolvedContextReferences.clear();
  RegionList.clear();
  ContextNameList.clear();

  // prepare list creation. To reuse as much code as possible handle this highlighting the same way as embedded onces
  embeddedHls.insert(iName,YzisEmbeddedHlInfo());

  bool something_changed;
  int startctx=0;  // the context "0" id is 0 for this hl, all embedded context "0"s have offsets
  building=true;  // inform everybody that we are building the highlighting contexts and itemlists
  do
  {
  yzDebug(13010)<<"**************** Outter loop in make ContextList"<<endl;
  yzDebug(13010)<<"**************** Hl List count:"<<embeddedHls.count()<<endl;
  something_changed=false; //assume all "embedded" hls have already been loaded
  for (YzisEmbeddedHlInfos::const_iterator it=embeddedHls.begin(); it!=embeddedHls.end();++it)
  {
    if (!it.data().loaded)  // we found one, we still have to load
    {
      yzDebug(13010)<<"**************** Inner loop in make ContextList"<<endl;
      QString identifierToUse;
      yzDebug(13010)<<"Trying to open highlighting definition file: "<< it.key()<<endl;
      if (iName==it.key()) identifierToUse=identifier;  // the own identifier is known
      else
        identifierToUse=YzisHlManager::self()->identifierForName(it.key()); // all others have to be looked up

      yzDebug(13010)<<"Location is:"<< identifierToUse<<endl;

      buildPrefix=it.key()+':';  // attribute names get prefixed by the names of the highlighting definitions they belong to

      if (identifierToUse.isEmpty() ) yzDebug()<<"OHOH, unknown highlighting description referenced"<<endl;

      yzDebug()<<"setting ("<<it.key()<<") to loaded"<<endl;
      it=embeddedHls.insert(it.key(),YzisEmbeddedHlInfo(true,startctx)); //mark hl as loaded
      buildContext0Offset=startctx;  //set class member for context 0 offset, so we don't need to pass it around
      startctx=addToContextList(identifierToUse,startctx);  //parse one hl definition file
      if (noHl) return;  // an error occurred
      something_changed=true; // something has been loaded

    }
  }
  } while (something_changed);  // as long as there has been another file parsed repeat everything, there could be newly added embedded hls.


  /* at this point all needed highlighing (sub)definitions are loaded. It's time to resolve cross file
     references (if there are some
  */
  yzDebug(13010)<<"Unresolved contexts, which need attention: "<<unresolvedContextReferences.count()<<endl;
//optimize this a littlebit
  for (YzisHlUnresolvedCtxRefs::iterator unresIt=unresolvedContextReferences.begin();
    unresIt!=unresolvedContextReferences.end();++unresIt)
  {
    //try to find the context0 id for a given unresolvedReference
    YzisEmbeddedHlInfos::const_iterator hlIt=embeddedHls.find(unresIt.data());
    if (hlIt!=embeddedHls.end())
      *(unresIt.key())=hlIt.data().context0;
  }

  /*eventually handle YzisHlIncludeRules items, if they exist.
    This has to be done after the cross file references, because it is allowed
    to include the context0 from a different definition, than the one the rule belongs to */
  handleYzisHlIncludeRules();

  embeddedHls.clear(); //save some memory.
  unresolvedContextReferences.clear(); //save some memory
  RegionList.clear();  // I think you get the idea ;)
  ContextNameList.clear();


// if there have been errors show them
  if (!errorsAndWarnings.isEmpty())
  yzDebug() << QString("There were warning(s) and/or error(s) while parsing the syntax highlighting configuration.") << errorsAndWarnings << endl;

// we have finished
  building=false;
}

void YzisHighlighting::handleYzisHlIncludeRules()
{

  // if there are noe include rules to take care of, just return
  yzDebug(13010)<<"YzisHlIncludeRules, which need attention: " <<includeRules.count()<<endl;
  if (includeRules.isEmpty()) return;

  buildPrefix="";
  QString dummy;

  /*by now the context0 references are resolved, now more or less only inner file references are resolved.
  If we decide that arbitrary inclusion is needed, this doesn't need to be changed, only the addToContextList
  method
   */

  //resolove context names
  for (YzisHlIncludeRules::iterator it=includeRules.begin();it!=includeRules.end();)
  {

  if ((*it)->incCtx==-1) // context unresolved ?
  { //yes

    if ((*it)->incCtxN.isEmpty())
    {
      // no context name given, and no valid context id set, so this item is going to be removed
      YzisHlIncludeRules::iterator it1=it;
      ++it1;
      delete (*it);
      includeRules.remove(it);
      it=it1;
    }
    else
    {
      // resolve name to id
      (*it)->incCtx=getIdFromString(&ContextNameList,(*it)->incCtxN,dummy);
      yzDebug()<<"Resolved "<<(*it)->incCtxN<< " to "<<(*it)->incCtx<<" for include rule"<<endl;
      // It would be good to look here somehow, if the result is valid
    }
  } else ++it; //nothing to do, already resolved (by the cross defintion reference resolver
  }

  // now that all YzisHlIncludeRule items should be valid and completely resolved, do the real inclusion of the rules.
  // recursiveness is needed, because context 0 could include context 1, which itself includes context 2 and so on.
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

  /*find the last entry for the given context in the YzisHlIncludeRules list
     this is need if one context includes more than one. This saves us from updating all insert positions:
    eg: context 0:
    pos 3 - include context 2
    pos 5 - include context 3
    During the building of the includeRules list the items are inserted in ascending order, now we need it
    descending to make our life easier.
  */
  while ((it!=list->end()) && ((*it)->ctx==ctx))
  {
    it1=it;
    ++it;
//    yzDebug()<<"loop1"<<endl;
  }
  // iterate over each include rule for the context the function has been called for.
  while ((it1!=list->end()) && ((*it1)->ctx==ctx))
  {
//    yzDebug()<<"loop2"<<endl;


    int ctx1=(*it1)->incCtx;

    //let's see, if the the included context includes other contexts
    for (YzisHlIncludeRules::iterator it2=list->begin();it2!=list->end();++it2)
    {
//      yzDebug()<<"loop3"<<endl;

      if ((*it2)->ctx==ctx1)
      {
        //yes it does, so first handle that include rules, since we want to
        // include those subincludes too
        handleYzisHlIncludeRulesRecursive(it2,list);
        break;
      }
    }

    // if the context we want to include had sub includes, they are already inserted there.
    YzisHlContext *dest=contextList[ctx];
    YzisHlContext *src=contextList[ctx1];
    uint p=(*it1)->pos; //insert the included context's rules starting at position p
    for ( YzisHlItem *c = src->items.first(); c; c=src->items.next(), p++ )
                        dest->items.insert(p,c);

    it=it1; //backup the iterator
    --it1; //move to the next entry, which has to be take care of
    delete (*it); //free the already handled data structure
    list->remove(it); // remove it from the list
  }
}

int YzisHighlighting::addToContextList(const QString &ident, int ctx0)
{
  buildIdentifier=ident;
  YzisSyntaxContextData *data, *datasub;
  YzisHlItem *c;

  QString dummy;

  // Let the syntax document class know, which file we'd like to parse
  if (!YzisHlManager::self()->syntax->setIdentifier(ident))
  {
  noHl=true;
  yzDebug() << QString("Since there has been an error parsing the highlighting description, this highlighting will be disabled") << endl;
  return 0;
  }

  RegionList<<"!YzisInternal_TopLevel!";
  readCommentConfig();
  readGlobalKeywordConfig();
  readFoldingConfig ();

  QString ctxName;

  // This list is needed for the translation of the attribute parameter, if the itemData name is given instead of the index
  addToYzisHlItemDataList();
  YzisHlItemDataList iDl = internalIDList;

  createContextNameList(&ContextNameList,ctx0);

  yzDebug(13010)<<"Parsing Context structure"<<endl;
  //start the real work
  data=YzisHlManager::self()->syntax->getGroupInfo("highlighting","context");
  uint i=buildContext0Offset;
  if (data)
    {
      while (YzisHlManager::self()->syntax->nextGroup(data))
        {
    yzDebug(13010)<<"Found a context in file, building structure now"<<endl;
          // BEGIN - Translation of the attribute parameter
          QString tmpAttr=YzisHlManager::self()->syntax->groupData(data,QString("attribute")).simplifyWhiteSpace();
          int attr;
          if (QString("%1").arg(tmpAttr.toInt())==tmpAttr)
            attr=tmpAttr.toInt();
          else
            attr=lookupAttrName(tmpAttr,iDl);
          // END - Translation of the attribute parameter

    ctxName=buildPrefix+YzisHlManager::self()->syntax->groupData(data,QString("lineEndContext")).simplifyWhiteSpace();

    QString tmpLineEndContext=YzisHlManager::self()->syntax->groupData(data,QString("lineEndContext")).simplifyWhiteSpace();
    int context;

    context=getIdFromString(&ContextNameList, tmpLineEndContext,dummy);

          // BEGIN get fallthrough props
          bool ft = false;
          int ftc = 0; // fallthrough context
          if ( i > 0 ) { // fallthrough is not smart in context 0
            QString tmpFt = YzisHlManager::self()->syntax->groupData(data, QString("fallthrough") );
            if ( tmpFt.lower() == "true" ||  tmpFt.toInt() == 1 )
              ft = true;
            if ( ft ) {
              QString tmpFtc = YzisHlManager::self()->syntax->groupData( data, QString("fallthroughContext") );

          ftc=getIdFromString(&ContextNameList, tmpFtc,dummy);
        if (ftc == -1) ftc =0;

              yzDebug(13010)<<"Setting fall through context (context "<<i<<"): "<<ftc<<endl;
            }
          }

          // END falltrhough props
          contextList.insert (i, new YzisHlContext (
            attr,
            context,
            (YzisHlManager::self()->syntax->groupData(data,QString("lineBeginContext"))).isEmpty()?-1:
            (YzisHlManager::self()->syntax->groupData(data,QString("lineBeginContext"))).toInt(),
            ft, ftc
                                       ));


            //Let's create all items for the context
            while (YzisHlManager::self()->syntax->nextItem(data))
              {
//    yzDebug(13010)<< "In make Contextlist: Item:"<<endl;

                // YzisHlIncludeRules : add a pointer to each item in that context

                QString tag = YzisHlManager::self()->syntax->groupItemData(data,QString(""));
                if ( tag == "IncludeRules" ) { //if the new item is an Include rule, we have to take special care
      QString incCtx=YzisHlManager::self()->syntax->groupItemData( data, QString("context"));
      // only context refernces of type NAME and ##Name are allowed
      if (incCtx.startsWith("##") || (!incCtx.startsWith("#"))) { //#stay, #pop is not interesting here
        if (!incCtx.startsWith("#")) { // a local reference -> just initialize the include rule structure
          incCtx=buildPrefix+incCtx.simplifyWhiteSpace();
          includeRules.append(new YzisHlIncludeRule(i,contextList[i]->items.count(),incCtx));
        }
        else { //a cross highlighting reference
          yzDebug()<<"Cross highlight reference <IncludeRules>"<<endl;
          YzisHlIncludeRule *ir=new YzisHlIncludeRule(i,contextList[i]->items.count());
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
#if 0
                QString tag = YzisHlManager::self()->syntax->groupYzisHlItemData(data,QString(""));
                if ( tag == "IncludeRules" ) {
                  // attrib context: the index (jowenn, i think using names here would be a cool feat, goes for mentioning the context in any item. a map or dict?)
                  int ctxId = getIdFromString(&ContextNameList,
      YzisHlManager::self()->syntax->groupYzisHlItemData( data, QString("context")),dummy); // the index is *required*
                  if ( ctxId > -1) { // we can even reuse rules of 0 if we want to:)
                    yzDebug(13010)<<"makeContextList["<<i<<"]: including all items of context "<<ctxId<<endl;
                    if ( ctxId < (int) i ) { // must be defined
                      for ( c = contextList[ctxId]->items.first(); c; c = contextList[ctxId]->items.next() )
                        contextList[i]->items.append(c);
                    }
                    else
                      yzDebug(13010)<<"Context "<<ctxId<<"not defined. You can not include the rules of an undefined context"<<endl;
                  }
                  continue; // while nextItem
                }
#endif
    c=createYzisHlItem(data,iDl,&RegionList,&ContextNameList);
    if (c)
      {
                                contextList[i]->items.append(c);

                                // Not supported completely atm and only one level. Subitems.(all have to be matched to at once)
        datasub=YzisHlManager::self()->syntax->getSubItems(data);
        bool tmpbool;
        if (tmpbool=YzisHlManager::self()->syntax->nextItem(datasub))
          {
            c->subItems=new QPtrList<YzisHlItem>;
            for (;tmpbool;tmpbool=YzisHlManager::self()->syntax->nextItem(datasub))
                                            c->subItems->append(createYzisHlItem(datasub,iDl,&RegionList,&ContextNameList));
                                        }
        YzisHlManager::self()->syntax->freeGroupInfo(datasub);
                                // end of sublevel
      }
//    yzDebug(13010)<<"Last line in loop"<<endl;
              }
          i++;
        }
      }

  YzisHlManager::self()->syntax->freeGroupInfo(data);
  if (RegionList.count()!=1) folding=true;
  folding = folding || m_foldingIndentationSensitive;
  return i;
}

void YzisHighlighting::clearAttributeArrays ()
{
  for ( QIntDictIterator< QMemArray<YzisAttribute> > it( m_attributeArrays ); it.current(); ++it )
  {
    // k, schema correct, let create the data
    YzisAttributeList defaultStyleList;
    defaultStyleList.setAutoDelete(true);
    YzisHlManager::self()->getDefaults(it.currentKey(), defaultStyleList);

    YzisHlItemDataList itemDataList;
    getYzisHlItemDataList(it.currentKey(), itemDataList);

    uint nAttribs = itemDataList.count();
    QMemArray<YzisAttribute> *array = it.current();
    array->resize (nAttribs);

    for (uint z = 0; z < nAttribs; z++)
    {
      YzisHlItemData *itemData = itemDataList.at(z);
      YzisAttribute n = *defaultStyleList.at(itemData->defStyleNum);

      if (itemData && itemData->isSomethingSet())
        n += *itemData;

      array->at(z) = n;
    }
  }
}

QMemArray<YzisAttribute> *YzisHighlighting::attributes (uint schema)
{
  QMemArray<YzisAttribute> *array;

  // found it, allready floating around
  if ((array = m_attributeArrays[schema]))
    return array;

  // ohh, not found, check if valid schema number
/*  if (!YzisFactory::self()->schemaManager()->validSchema(schema))
  {
    // uhh, not valid :/, stick with normal default schema, it's always there !
    return attributes (0);
  }
*/
  // k, schema correct, let create the data
  YzisAttributeList defaultStyleList;
  defaultStyleList.setAutoDelete(true);
  YzisHlManager::self()->getDefaults(schema, defaultStyleList);

  YzisHlItemDataList itemDataList;
  getYzisHlItemDataList(schema, itemDataList);

  uint nAttribs = itemDataList.count();
  array = new QMemArray<YzisAttribute> (nAttribs);

  for (uint z = 0; z < nAttribs; z++)
  {
    YzisHlItemData *itemData = itemDataList.at(z);
    YzisAttribute n = *defaultStyleList.at(itemData->defStyleNum);

    if (itemData && itemData->isSomethingSet())
      n += *itemData;

    array->at(z) = n;
  }

  m_attributeArrays.insert(schema, array);

  return array;
}

void YzisHighlighting::getYzisHlItemDataListCopy (uint schema, YzisHlItemDataList &outlist)
{
  YzisHlItemDataList itemDataList;
  getYzisHlItemDataList(schema, itemDataList);

  outlist.clear ();
  outlist.setAutoDelete (true);
  for (uint z=0; z < itemDataList.count(); z++)
    outlist.append (new YzisHlItemData (*itemDataList.at(z)));
}

//END

//BEGIN YzisHlManager
YzisHlManager::YzisHlManager()
  : QObject()
//  , m_config ("katesyntaxhighlightingrc", false, false)
  , commonSuffixes (QStringList::split(";", ".orig;.new;~;.bak;.BAK"))
  , syntax (new YzisSyntaxDocument())
{
  hlList.setAutoDelete(true);
  hlDict.setAutoDelete(false);

  YzisSyntaxModeList modeList = syntax->modeList();
  for (uint i=0; i < modeList.count(); i++)
  {
    YzisHighlighting *hl = new YzisHighlighting(modeList.at(i));

    uint insert = 0;
    for (; insert <= hlList.count(); insert++)
    {
      if (insert == hlList.count())
        break;

      if ( QString(hlList.at(insert)->section() + hlList.at(insert)->name()).lower()
            > QString(hl->section() + hl->name()).lower() )
        break;
    }

    hlList.insert (insert, hl);
    hlDict.insert (hl->name(), hl);
  }

  // Normal HL
  YzisHighlighting *hl = new YzisHighlighting(0);
  hlList.prepend (hl);
  hlDict.insert (hl->name(), hl);
}

YzisHlManager::~YzisHlManager()
{
  delete syntax;
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
    if (hlList.at(z)->name() == name)
      return z;

  return z;
}

int YzisHlManager::detectHighlighting (YZBuffer *doc)
{
  int hl = wildcardFind( doc->fileName() );

  if (hl == -1)
  {
    QByteArray buf (KATE_HL_HOWMANY);
    uint bufpos = 0;
    for (uint i=0; i < doc->lineCount(); i++)
    {
      QString line = doc->textline( i );
      uint len = line.length() + 1;

      if (bufpos + len > KATE_HL_HOWMANY)
        len = KATE_HL_HOWMANY - bufpos;

      memcpy(&buf[bufpos], (line + "\n").latin1(), len);

      bufpos += len;

      if (bufpos >= KATE_HL_HOWMANY)
        break;
    }
    buf.resize( bufpos );

//    hl = mimeFind (buf);
  }

  return hl;
}

int YzisHlManager::wildcardFind(const QString &fileName)
{
	yzDebug() << "WidcardFind " << fileName << endl;
  int result = -1;
  if ((result = realWildcardFind(fileName)) != -1)
    return result;

  int length = fileName.length();
  QString backupSuffix = "~";
  if (fileName.endsWith(backupSuffix)) {
    if ((result = realWildcardFind(fileName.left(length - backupSuffix.length()))) != -1)
      return result;
  }

  for (QStringList::Iterator it = commonSuffixes.begin(); it != commonSuffixes.end(); ++it) {
    if (*it != backupSuffix && fileName.endsWith(*it)) {
      if ((result = realWildcardFind(fileName.left(length - (*it).length()))) != -1)
        return result;
    }
  }
  return -1;
}

int YzisHlManager::realWildcardFind(const QString &fileName)
{
	yzDebug() << "realWidcardFind " << fileName << endl;
  static QRegExp sep("\\s*;\\s*");

  QPtrList<YzisHighlighting> highlights;

  for (YzisHighlighting *highlight = hlList.first(); highlight != 0L; highlight = hlList.next()) {
    highlight->loadWildcards();

    for (QStringList::Iterator it = highlight->getPlainExtensions().begin(); it != highlight->getPlainExtensions().end(); ++it)
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

    for (YzisHighlighting *highlight = highlights.first(); highlight != 0L; highlight = highlights.next())
    {
      if (highlight->priority() > pri)
      {
        pri = highlight->priority();
        hl = hlList.findRef (highlight);
      }
    }

    return hl;
  }
  return -1;
}

QString YzisHlManager::findByContent( const QByteArray& contents ) {
#if 0
	struct magic_set *ms = magic_open( MAGIC_MIME );
	if ( ms == NULL ) {
		magic_close(ms);
		return QString::null;
	}
	if (magic_load(ms,NULL) == -1) {
		yzDebug() << "Magic error " << magic_error( ms ) << endl;
		magic_close(ms);
		return QString::null;
	}
	const char *magic_result = magic_buffer( ms, contents.data(), contents.size() );
	magic_close(ms);
	if ( magic_result ) {
		QString mime = QString( magic_result );
		mime = mime.mid( 0, mime.find( ';' ) );
		yzDebug() << "Magic result " << mime << endl;
		return mime;
	} else return QString::null;
#endif
	return QString::null;
}

int YzisHlManager::mimeFind(const QByteArray &contents)
{
  static QRegExp sep("\\s*;\\s*");

  int accuracy = 0;
//  KMimeType::Ptr mt = KMimeType::findByContent( contents, &accuracy );
  QString mt = findByContent( contents );

  QPtrList<YzisHighlighting> highlights;

  for (YzisHighlighting *highlight = hlList.first(); highlight != 0L; highlight = hlList.next())
  {
    QStringList l = QStringList::split( sep, highlight->getMimetypes() );

    for( QStringList::Iterator it = l.begin(); it != l.end(); ++it )
    {
      if ( *it == mt ) // faster than a regexp i guess?
        highlights.append (highlight);
    }
  }

  if ( !highlights.isEmpty() )
  {
    int pri = -1;
    int hl = -1;

    for (YzisHighlighting *highlight = highlights.first(); highlight != 0L; highlight = highlights.next())
    {
      if (highlight->priority() > pri)
      {
        pri = highlight->priority();
        hl = hlList.findRef (highlight);
      }
    }
    return hl;
  }
  return -1;
}

uint YzisHlManager::defaultStyles()
{
  return 13;
}

QString YzisHlManager::defaultStyleName(int n)
{
  static QStringList names;

  if (names.isEmpty())
  {
    names << QString("Normal");
    names << QString("Keyword");
    names << QString("Data Type");
    names << QString("Decimal/Value");
    names << QString("Base-N Integer");
    names << QString("Floating Point");
    names << QString("Character");
    names << QString("String");
    names << QString("Comment");
    names << QString("Others");
    names << QString("Alert");
    names << QString("Function");
    // this next one is for denoting the beginning/end of a user defined folding region
    names << QString("Region Marker");
  }

  return names[n];
}

void YzisHlManager::getDefaults(uint schema, YzisAttributeList &list)
{
  list.setAutoDelete(true);

  YzisAttribute* normal = new YzisAttribute();
  normal->setTextColor(Qt::white);
  normal->setSelectedTextColor(Qt::black);
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
  string->setTextColor(Qt::magenta/*red*/);
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
  alert->setTextColor(Qt::white);
  alert->setSelectedTextColor(Qt::red);
  alert->setBold(true);
  alert->setBGColor(Qt::red);
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

/*  KConfig *config = YzisHlManager::self()->self()->getKConfig();
  config->setGroup("Default Item Styles - Schema " + YzisFactory::self()->schemaManager()->name(schema));

  for (uint z = 0; z < defaultStyles(); z++)
  {
    YzisAttribute *i = list.at(z);
    QStringList s = config->readListEntry(defaultStyleName(z));
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
         col=tmp.toUInt(0,16); i->setBGColor(col); }

      tmp=s[7]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); i->setSelectedBGColor(col); }

    }
  }
  */
}

void YzisHlManager::setDefaults(uint schema, YzisAttributeList &list)
{
/*  KConfig *config =  YzisHlManager::self()->self()->getKConfig();
  config->setGroup("Default Item Styles - Schema " + YzisFactory::self()->schemaManager()->name(schema));

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
    settings<<(i->itemSet(YzisAttribute::BGColor)?QString::number(i->bgColor().rgb(),16):"");
    settings<<(i->itemSet(YzisAttribute::SelectedBGColor)?QString::number(i->selectedBGColor().rgb(),16):"");
    settings<<"---";

    config->writeEntry(defaultStyleName(z),settings);
  }
*/
  emit changed();
}

int YzisHlManager::highlights()
{
  return (int) hlList.count();
}

QString YzisHlManager::hlName(int n)
{
  return hlList.at(n)->name();
}

QString YzisHlManager::hlSection(int n)
{
  return hlList.at(n)->section();
}

QString YzisHlManager::identifierForName(const QString& name)
{
  YzisHighlighting *hl = 0;

  if ((hl = hlDict[name]))
    return hl->getIdentifier ();

  return QString();
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
    QString hlName = YzisHlManager::self()->hlName (z);
    QString hlSection = YzisHlManager::self()->hlSection (z);

    if ( !hlSection.isEmpty() && (names.contains(hlName) < 1) )
    {
      if (subMenusName.contains(hlSection) < 1)
      {
        subMenusName << hlSection;
        QPopupMenu *menu = new QPopupMenu ();
        subMenus.append(menu);
        popupMenu()->insertItem (hlSection, menu);
      }

      int m = subMenusName.findIndex (hlSection);
      names << hlName;
      subMenus.at(m)->insertItem ( hlName, this, SLOT(setHl(int)), 0,  z);
    }
    else if (names.contains(hlName) < 1)
    {
      names << hlName;
      popupMenu()->insertItem ( hlName, this, SLOT(setHl(int)), 0,  z);
    }
  }

  if (!doc) return;

  for (uint i=0;i<subMenus.count();i++)
  {
    for (uint i2=0;i2<subMenus.at(i)->count();i2++)
      subMenus.at(i)->setItemChecked(subMenus.at(i)->idAt(i2),false);
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
