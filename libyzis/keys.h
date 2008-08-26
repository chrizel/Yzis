/* This file is part of the Yzis libraries
*  Copyright (C) 2007 Tim Northover <tim@pnorthover.freeserve.co.uk>
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

#ifndef _KEYEVENT_H
#define _KEYEVENT_H

#include "yzis.h"
#include <QString>
#include <QMap>
#include <QVector>

// Represents pure hardware keys
class YZIS_EXPORT YKey
{
public:
    enum 
    {
        // If this is changed, be sure to alter the representation functions too
        Key_Invalid = -1,
        // Next, printable characters represent themselves in ASCII for now.
        // Might be possible to extend to unicode later
        // Numeric
        Key_0 = '0',
        Key_1,Key_2,Key_3,Key_4,Key_5,Key_6,Key_7,Key_8,Key_9,
        // Latin alphabetic: represents self in ASCII
        Key_A = 'A',
        Key_B,Key_C,Key_D,Key_E,Key_F,Key_G,Key_H,Key_I,Key_J,Key_K,Key_L,
        Key_M,Key_N,Key_O,Key_P,Key_Q,Key_R,Key_S,Key_T,Key_U,Key_V,Key_W,
        Key_X,Key_Y,Key_Z,
        Key_a = 'a',
        Key_b,Key_c,Key_d,Key_e,Key_f,Key_g,Key_h,Key_i,Key_j,Key_k,Key_l,
        Key_m,Key_n,Key_o,Key_p,Key_q,Key_r,Key_s,Key_t,Key_u,Key_v,Key_w,
        Key_x,Key_y,Key_z,
        // Others
        Key_BackTick = '`',
        Key_Exclamation = '!',
        Key_DblQuote = '"',
        Key_Dollar = '$',
        Key_Percent = '%',
        Key_Caret = '^',
        Key_Ampersand = '&',
        Key_Asterisk = '*',
        Key_LeftParen = '(',
        Key_RightParen = ')',
        Key_Hyphen = '-',
        Key_UnderScore = '_',
        Key_Equals = '=',
        Key_Plus = '+',
        Key_LeftSBracket = '[',
        Key_RightSBracket = ']',
        Key_LeftCBracket = '{',
        Key_RightCBracket = '}',
        Key_Colon = ':',
        Key_SemiColon = ';',
        Key_At = '@',
        Key_Quote = '\'',
        Key_Hash = '#',
        Key_Tilde = '~',
        Key_BackSlash = '\\',
        Key_VBar = '|',
        Key_Comma = ',',
        Key_Period = '.',
        Key_GreaterThan = '>',
        Key_ForwardSlash = '/',
        Key_QuestionMark = '?',
        Key_Space = ' ',
        Key_Tab = '\t',
        Key_LessThan = '<',

        // F Keys
        Key_F1 = 0x100000,
        Key_F2, Key_F3, Key_F4, Key_F5, Key_F6, Key_F7, Key_F8,
        Key_F9, Key_F10, Key_F11, Key_F12,Key_F13,Key_F14,Key_F15,
        Key_F16,Key_F17,Key_F18,Key_F19,Key_F20,Key_F21,Key_F22,
        Key_F23,Key_F24,Key_F25,Key_F26,Key_F27,Key_F28,Key_F29,
        Key_F30,Key_F31,Key_F32,Key_F33,Key_F34,Key_F35,

        Key_Up, Key_Down, Key_Left, Key_Right, Key_Insert, Key_Delete, 
        Key_Home, Key_End, Key_PageUp, Key_PageDown,Key_Break,Key_Clear,
        Key_PrintScreen,Key_Prior,Key_BTab,Key_SysReq,Key_Next,

        Key_Esc, Key_BackSpace, Key_Enter, Key_Pause,

        Key_Alt, Key_Ctrl, Key_Shift,Key_Meta,
    };
    enum
    {
        Mod_None = 0,
        Mod_Shift = 0x1,
        Mod_Ctrl = 0x2,
        Mod_Alt = 0x8,
        Mod_Meta = 0x4,
    };
    
    YKey(int key = Key_Invalid, int modifiers=Mod_None)
        : mKey(key), mModifiers(modifiers) 
    { initKeyTable(); if ( isUnicode() ) mModifiers &= ~Mod_Shift; }
    YKey(QChar rep, int modifiers=Mod_None);


    QString toString() const;
    int fromString(const QString &);

    int modifiers() const
        { return mModifiers; }
    void addModifier(int m) 
        { mModifiers |= m; }
    void setKey(int key) 
        { mKey = key; }
    int key() const {
      return mKey;
    }
    void setKey(QChar ch) 
    { mKey = ch.unicode(); mModifiers &= ~Mod_Shift; }
        
    
    bool isUnicode() const {
        return mKey <= 0xffff && mKey >= 0;
    }

    operator QChar() const {
        if ( mKey <= 0xffff && mKey >= 0)
            return QChar(mKey);
        else
            return QChar();
    }

    bool operator==(const YKey &oth) const {
        return (mKey == oth.mKey) && (mModifiers == oth.mModifiers);
    }
    bool operator!=(const YKey &oth) const {
        return (mKey != oth.mKey) || (mModifiers != oth.mModifiers);
    }
    
private:
    QString toBasicRep() const;
    bool parseBasicRep(QString rep);
    bool parseModifiers(const QString &mods);

    void initKeyTable();
    
private:
    int mKey;
    char mModifiers;

    static QMap<QString, int> keyTable;
    static QMap<QString, int> aliasTable;
};

class YZIS_EXPORT YKeySequence
{
public:
    typedef QVector<YKey>::const_iterator const_iterator;
    
    explicit YKeySequence(const QString &);
    YKeySequence(const YKey &key);
    
    YKeySequence(const YKeySequence &seq);
    YKeySequence();

    ~YKeySequence() {
        delete mKeys;
    }
            
    QString toString() const;
    void appendString(QString input);
    
    int parseUInt(const_iterator &i) const;
    bool match(const_iterator &pos, const const_iterator &end) const;

    bool isEmpty() const {
        return mKeys->isEmpty();
    }
    int count() const {
        return mKeys->count();
    }
    void append(const YKey &k) {
        mKeys->append(k);
    }
    void clear() {
        mKeys->clear();
    }
    
    const_iterator end() const {
            return mKeys->end();
    }
    const_iterator begin() const {
            return mKeys->begin();
    }
    
    YKeySequence &operator =(const YKeySequence &from);
#ifdef DEBUG
    QString describe(void) { return mName; }
#endif
private:
    QVector<YKey> *mKeys;
#ifdef DEBUG
    QString mName;
#endif
};


#endif // _KEYEVENT_H
