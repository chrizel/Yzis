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

#include <qfile.h>
#include "syntaxdocument.h"
#include "syntaxhighlight.h"
#include "debug.h"
#include "line.h"


static const QString stdDeliminator = QString (" \t.():!+,-<=>%&*/;?[]^{|}~\\");
static const bool trueBool = true;

YzisSyntaxHighlight::YzisSyntaxHighlight() {
	noHl = false; //to disable syntax hl , turn this to true
	mSdoc = new YzisSyntaxDocument(); //some day we'll pass the langugage as an argument or make it static ...
	YzisSyntaxModeList list = mSdoc->modeList();
	YzisSyntaxModeListItem *item;
	for ( item = list.first(); item; item = list.next() ) {
		yzDebug() << "HL name : " << item->name << endl; 
	}
}

YzisSyntaxHighlight::~YzisSyntaxHighlight() {
}

void YzisSyntaxHighlight::highlight( YZLine *prevLine, YZLine *textLine, QMemArray<signed char> foldingList, bool *ctxChanged) {
  if (!textLine)
    return;

  if (noHl) {
    textLine->setAttribs(0,0,textLine->length());
    return;
  }

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

    if (!(context = contextNum(ctxNum)))
      context = contextNum(0);

    previousLine=prevLine->ctxArray().size()-1; //position of the last context ID of th previous line within the stack

    generateContextStack(&ctxNum, context->ctx, &ctx, &previousLine, lineContinue); //get stack ID to use

    if (!(context = contextNum(ctxNum)))
      context = contextNum(0);
  }

  // text, for programming convenience :)
  QChar lastChar = ' ';
  const QString& text = textLine->data();
  uint len = text.length();

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
/*          if (item->region)
          {
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
*/ // FOLDING
          generateContextStack(&ctxNum, item->ctx, &ctx, &previousLine);  //regenerate context stack

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
        //kdDebug(13010)<<"context num after fallthrough at col "<<z<<": "<<ctxNum<<endl;
        // the next is nessecary, as otherwise keyword (or anything using the std delimitor check)
        // immediately after fallthrough fails. Is it bad?
        // jowenn, can you come up with a nicer way to do this?
        if (z)
          lastChar = text[offset1 - 1];
        else
          lastChar = '\\';
        continue;
      }
      else
        textLine->setAttribs(context->attr,offset1,offset1 + 1);
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

void YzisSyntaxHighlight::generateContextStack(int *ctxNum, int ctx, QMemArray<short>* ctxs, int *prevLine, bool lineContinue) {
  yzDebug()<<QString("Entering generateContextStack with %1").arg(ctx)<<endl;

  if (lineContinue)
  {
    if ( !ctxs->isEmpty() )
    {
      (*ctxNum)=(*ctxs)[ctxs->size()-1];
      (*prevLine)--;
    }
    else
    {
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




//
//ITEMS
//

YzisHlItem::YzisHlItem(int attribute, int context,signed char regionId,signed char regionId2)
  : attr(attribute), ctx(context),region(regionId),region2(regionId2), lookAhead(false)  {
	  subItems=0;
}

YzisHlItem::~YzisHlItem() {
  if (subItems!=0) {
    subItems->setAutoDelete(true);
    subItems->clear();
    delete subItems;
  }
}

bool YzisHlItem::startEnable(const QChar& c)
{
	return false; //never used according to kate
}

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

YzisHlLineContinue::YzisHlLineContinue(int attribute, int context, signed char regionId,signed char regionId2)
  : YzisHlItem(attribute,context,regionId,regionId2) {
}

int YzisHlLineContinue::checkHgl(const QString& text, int offset, int len)
{
  if ((len == 1) && (text[offset] == '\\'))
    return ++offset;

  return 0;
}

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

