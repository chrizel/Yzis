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

#include "keys.h"
#include "debug.h"
#include <QRegExp>
#include <iostream>

#define dbg()    yzDebug("YKeySequence")
#define err()    yzError("YKeySequence")

using namespace std;

QMap<QString, int> YKey::keyTable;
QMap<QString, int> YKey::aliasTable;

YKey::YKey(QChar rep, int modifiers)
    : mModifiers(modifiers)
{
    initKeyTable();

    parseBasicRep(rep);
}


void YKey::initKeyTable()
{
    if ( !  keyTable.empty() )
        return;
    
    keyTable["UP"] = Key_Up;
    keyTable["DOWN"] = Key_Down;
    keyTable["LEFT"] = Key_Left;
    keyTable["RIGHT"] = Key_Right;

    keyTable["SPACE"] = Key_Space;
    keyTable["TAB"] = Key_Tab;
    keyTable["BS"] = Key_BackSpace;
    keyTable["ENTER"] = Key_Enter;
    keyTable["ESC"] = Key_Esc;
    
    keyTable["INSERT"] = Key_Insert;
    keyTable["DEL"] = Key_Delete;
    keyTable["HOME"] = Key_Home;
    keyTable["END"] = Key_End;
    keyTable["PAGEUP"] = Key_PageUp;
    keyTable["PAGEDOWN"] = Key_PageDown;
    keyTable["LT"] = Key_LessThan;

    
    for(int i = 1; i <= 35; ++i)
        keyTable[QString("F%1").arg(i)] = Key_F1 + i - 1;
    
    keyTable["PAUSE"] = Key_Pause;
    keyTable["PRSCR"] = Key_PrintScreen;
    keyTable["BREAK"] = Key_Break;
    keyTable["CLEAR"] = Key_Clear;
    keyTable["PRIOR"] = Key_Prior;
    keyTable["BTAB"] = Key_BTab;
    keyTable["NEXT"] = Key_Next;
    keyTable["SYSREQ"] = Key_SysReq;
    
    keyTable["ALT"] = Key_Alt;
    keyTable["CTRL"] = Key_Ctrl;
    keyTable["SHIFT"] = Key_Shift;
    keyTable["META"] = Key_Meta;
    keyTable["INVALID"] = Key_Invalid;

    // Now the non-canonical aliases
    aliasTable["RETURN"] = Key_Enter;
    aliasTable["CR"] = Key_Enter;
    aliasTable["GT"] = Key_GreaterThan;
    aliasTable["PUP"] = Key_PageUp;
    aliasTable["PDOWN"] = Key_PageDown;
    aliasTable["DELETE"] = Key_Delete;
}


QString YKey::toBasicRep() const
{
    QString repr("NO_REP");
    
    if ( mKey <= 0xffff && mKey != '<') { // Just a unicode char
        repr = QString(QChar(mKey));
        return repr;        
    }

    QMap<QString, int>::const_iterator i = keyTable.constBegin();
    for(; i != keyTable.end(); ++i) {
        if ( mKey == i.value() )
            repr =  i.key();
    }
    

    return repr;
}

bool YKey::parseBasicRep(QString rep)
{
    // First deal with as-is characters
    if ( rep.length() == 1 ) {
        QChar c(rep.at(0));
        mKey = c.unicode();
        // Assume single unicode keys might be obtained by shift, so no extra info there
        mModifiers &= ~Mod_Shift;
        return true;
    }
    
    // Multiple-char descriptor, so use canonical upper case
    rep = rep.toUpper();

    if ( keyTable.contains(rep) ) {
        mKey = keyTable[rep];
        return true;
    }
    // It might be a noncanonical name, check
    if ( aliasTable.contains(rep) ) {
        mKey = aliasTable[rep];
        return true;
    }
    
    mKey = Key_Invalid;
    return false;
}

bool YKey::parseModifiers(const QString &mods)
{
    bool success = true;
    int pos = -1;
    QRegExp modPattern("[CSMA]-");
    
    while ( (pos = modPattern.indexIn(mods, pos+1)) != -1 ) {
        if ( mods.at(pos) == 'C' )
            mModifiers |= Mod_Ctrl;
        else if ( mods.at(pos) == 'S' )
            mModifiers |= Mod_Shift;
        else if ( mods.at(pos) == 'M' )
            mModifiers |= Mod_Meta;
        else if ( mods.at(pos) == 'A' )
            mModifiers |= Mod_Alt;
        else
            success = false;
    }

    return success;
}


/* Encode key event into vim form. */
QString YKey::toString() const
{
    QChar c(mKey);
    QString repr = toBasicRep();
    QString mod;

    if ( mModifiers & Mod_Ctrl )
        mod += "C-";
    if ( mModifiers & Mod_Meta )
        mod += "M-";
    if ( mModifiers & Mod_Alt )
        mod += "A-";
    if ( mModifiers & Mod_Shift && ! (c.isUpper() || c.isLower() ) )
            mod += "S-";

    if ( mod.length() || repr.length() > 1 )
        repr = "<" + mod + repr + ">";

    return repr;
}

// Return value is length of input used in forming key, -1 on fail
int YKey::fromString(const QString &key)
{
    QRegExp charFormat("^<((?:\\w-)*)([^>]+)>|^(.)");
    
    QString basicKey;
    
    mKey = Key_Invalid;
    mModifiers = Mod_None;
    
    charFormat.indexIn(key);

    if ( charFormat.matchedLength() == -1 ) {
        mKey = Key_Invalid;
        return -1;
    }

    if ( charFormat.matchedLength() == 1 ) { // Have single char
        mModifiers = Mod_None;
        basicKey = charFormat.cap(3);
    }
    else {
        basicKey = charFormat.cap(2);
        if ( ! parseModifiers(charFormat.cap(1)) ) {
            mKey = Key_Invalid;
            return -1;
        }
    }
    
    if ( ! parseBasicRep(basicKey) ) {
        mKey = Key_Invalid;
        return -1;
    }

    return charFormat.matchedLength();
}

YKeySequence::YKeySequence(const YKey &key)
{
    mKeys = new QVector<YKey>; mKeys->append(key);
#ifdef DEBUG
    mName = QString("single key : %1").arg(key.toString());
#endif
}

#if 1
YKeySequence::YKeySequence() 
{
    mKeys = new QVector<YKey>; mKeys->clear();
#ifdef DEBUG
    mName = QString("(empty constructor)");
#endif
}
#endif

YKeySequence::YKeySequence(const YKeySequence &from)
{
    mKeys = new QVector<YKey>;
    for(const_iterator i = from.mKeys->begin(); i != from.mKeys->end(); ++i)
        mKeys->append(*i);
#ifdef DEBUG
    mName = QString("copy of %1").arg(from.mName);
#endif
}

YKeySequence &YKeySequence::operator=(const YKeySequence &from)
{
    mKeys->clear();
    for(const_iterator i = from.mKeys->begin(); i != from.mKeys->end(); ++i)
        mKeys->append(*i);
    return *this;
}


YKeySequence::YKeySequence(const QString &input)
{
    mKeys = new QVector<YKey>;
    appendString(input);
#ifdef DEBUG
    mName = input;
#endif
}

void YKeySequence::appendString(QString input)
{
    int used;
    YKey k;
    
    while( input.count() ) {
        used = k.fromString(input);
        if ( used == -1 ) {
	    dbg() << "Asked to parse invalid key string";
            break;
	}
        mKeys->append(k);
        input = input.mid(used);
    }
}

QString YKeySequence::toString() const
{
    QString repr;
    
    for(const_iterator i = mKeys->begin(); i != mKeys->end(); ++i)
        repr += i->toString();
    return repr;
}

bool YKeySequence::match(const_iterator &pos, const const_iterator &othEnd) const
{
    const_iterator thisPos = begin();
    for(; pos != othEnd && thisPos != end(); ++pos, ++thisPos) {
        if ( *pos != *thisPos )
            return false;
    }
    
    // We've gone through one or both, determine which
    if ( thisPos == end() )
        return true;
    
    return false;
}

int YKeySequence::parseUInt(const_iterator &pos) const
{
    int tot = 0;
    QChar cur;
    
    cur = *pos;
    if ( !cur.isDigit() || cur.digitValue() == 0 )
        return -1;
    
    while ( cur.isDigit() ) {
        tot *= 10;
        tot += cur.digitValue();
        ++pos;
        if ( pos == end() )
            return tot;
        cur = *pos;
    }
    return tot;
}
        
        
