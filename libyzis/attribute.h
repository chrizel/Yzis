/* This file is part of the Yzis libraries
 *  Copyright (C) 2003,2004 Mickael Marchand <marchand@kde.org>,
 *  Thomas Capricelli <orzel@freehackers.org>,
 *  Philippe Fremy <pfremy@freehackers.org>
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
 * This file was originally taken from Kate, KDE editor
 * The copyrights follow below :
   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>
*/

#ifndef __YZIS_ATTRIBUTE_H__
#define __YZIS_ATTRIBUTE_H__

#include <qcolor.h>
#include <qfont.h>

/**
 * The Attribute class incorporates all text decorations supported by Yzis.
 */
class YzisAttribute
{
public:
  enum items {
    Weight = 0x1,
    Bold = 0x2,
    Italic = 0x4,
    Underline = 0x8,
    StrikeOut = 0x10,
    Outline = 0x20,
    TextColor = 0x40,
    SelectedTextColor = 0x80,
    BGColor = 0x100,
    SelectedBGColor = 0x200
  };

  YzisAttribute();
  virtual ~YzisAttribute();

  QFont font(const QFont& ref);

/*  inline int width(YzisFontStruct& fs, const QString& text, int col, int tabWidth) const
  { return fs.width(text, col, bold(), italic(), tabWidth); };

  // Non-preferred function when you have a string and you want one char's width!!
  inline int width(YzisFontStruct& fs, const QChar& c, int tabWidth) const
  { return fs.width(c, bold(), italic(), tabWidth); };
*/
  inline bool itemSet(int item) const
  { return item & m_itemsSet; };

  inline bool isSomethingSet() const
  { return m_itemsSet; };

  inline int itemsSet() const
  { return m_itemsSet; };

  inline void clearAttribute(int item)
  { m_itemsSet &= (~item); }

  inline int weight() const
  { return m_weight; };

  void setWeight(int weight);

  inline bool bold() const
  { return weight() >= QFont::Bold; };
  
  void setBold(bool enable = true);

  inline bool italic() const
  { return m_italic; };
  
  void setItalic(bool enable = true);

  inline bool underline() const
  { return m_underline; };
  
  void setUnderline(bool enable = true);

  inline bool strikeOut() const
  { return m_strikeout; };

  void setStrikeOut(bool enable = true);

  inline const QColor& outline() const
  { return m_outline; };
  
  void setOutline(const QColor& color);

  inline const QColor& textColor() const
  { return m_textColor; };
  
  void setTextColor(const QColor& color);

  inline const QColor& selectedTextColor() const
  { return m_selectedTextColor; };

  void setSelectedTextColor(const QColor& color);

  inline const QColor& bgColor() const
  { return m_bgColor; };
  
  void setBGColor(const QColor& color);

  inline const QColor& selectedBGColor() const
  { return m_selectedBGColor; };
  
  void setSelectedBGColor(const QColor& color);

  YzisAttribute& operator+=(const YzisAttribute& a);

  friend bool operator ==(const YzisAttribute& h1, const YzisAttribute& h2);
  friend bool operator !=(const YzisAttribute& h1, const YzisAttribute& h2);

  virtual void changed() { m_changed = true; };
  bool isChanged() { bool ret = m_changed; m_changed = false; return ret; };

  void clear();

private:
  int m_weight;
  bool m_italic, m_underline, m_strikeout, m_changed;
  QColor m_outline, m_textColor, m_selectedTextColor, m_bgColor, m_selectedBGColor;
  int m_itemsSet;
};

#endif
