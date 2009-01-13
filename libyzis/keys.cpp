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

/* System */
#include <iostream>

/* Qt */
#include <QRegExp>

/* Project */
#include "keys.h"
#include "debug.h"

#define dbg()    yzDebug("YKeySequence")
#define err()    yzError("YKeySequence")

using namespace std;

QMap<QString, Qt::Key> keyTable;
QMap<QString, Qt::Key> aliasTable;

void initKeyTable()
{
    if ( !  keyTable.empty() )
        return;
    
    keyTable["UP"] = Qt::Key_Up;
    keyTable["DOWN"] = Qt::Key_Down;
    keyTable["LEFT"] = Qt::Key_Left;
    keyTable["RIGHT"] = Qt::Key_Right;

    keyTable["SPACE"] = Qt::Key_Space;
    keyTable["TAB"] = Qt::Key_Tab;
    keyTable["BS"] = Qt::Key_Backspace;
    keyTable["ENTER"] = Qt::Key_Enter;
    keyTable["RETURN"] = Qt::Key_Return;
    keyTable["ESC"] = Qt::Key_Escape;
    
    keyTable["INSERT"] = Qt::Key_Insert;
    keyTable["DEL"] = Qt::Key_Delete;
    keyTable["HOME"] = Qt::Key_Home;
    keyTable["END"] = Qt::Key_End;
    keyTable["PAGEUP"] = Qt::Key_PageUp;
    keyTable["PAGEDOWN"] = Qt::Key_PageDown;
    keyTable["LT"] = Qt::Key_Less;

    
    for(int i = 1; i <= 35; ++i)
        keyTable[QString("F%1").arg(i)] = (Qt::Key) (Qt::Key_F1 + i - 1);
    
    keyTable["PAUSE"] = Qt::Key_Pause;
    keyTable["PRSCR"] = Qt::Key_Print;
//    keyTable["BREAK"] = Qt::Key_Break;
    keyTable["CLEAR"] = Qt::Key_Clear;
//    keyTable["PRIOR"] = Qt::Key_Prior;
    keyTable["BTAB"] = Qt::Key_Backtab;
//    keyTable["NEXT"] = Qt::Key_Next;
    keyTable["SYSREQ"] = Qt::Key_SysReq;
    
    keyTable["ALT"] = Qt::Key_Alt;
    keyTable["CTRL"] = Qt::Key_Control;
    keyTable["SHIFT"] = Qt::Key_Shift;
    keyTable["META"] = Qt::Key_Meta;
    keyTable["INVALID"] = Qt::Key_unknown;

    // Now the non-canonical aliases
    aliasTable["CR"] = Qt::Key_Enter;
    aliasTable["GT"] = Qt::Key_Greater;
    aliasTable["PUP"] = Qt::Key_PageUp;
    aliasTable["PDOWN"] = Qt::Key_PageDown;
    aliasTable["DELETE"] = Qt::Key_Delete;
}

YKey::YKey(QChar rep, Qt::KeyboardModifiers modifiers)
    : mModifiers(modifiers)
{
    initKeyTable();
    parseBasicRep(rep);
}

YKey::YKey(Qt::Key key, Qt::KeyboardModifiers modifiers)
    : mKey(key)
    , mModifiers(modifiers) 
{
    initKeyTable();
    if ( isUnicode() )
        mModifiers &= ~Qt::ShiftModifier;
}


QString YKey::toBasicRep() const
{
    if ( (mKey <= 0xffff) ) // Just a unicode char
        return QString(QChar(mKey));

    // reverse lookup
    QString s = keyTable.key(mKey);
    if (!s.isNull())
        return s;

	/* see #280
	 * related to multi-key character composition
	 * (AltGr, Multi_key, Codeinput, etc...
	 */
    //return QString("NO_REP"); see issue #280
	dbg() << "no match for key " << mKey << endl;
	return QString::null;
}

bool YKey::parseBasicRep(QString rep)
{
    // First deal with as-is characters
    if ( rep.length() == 1 ) {
        QChar c(rep.at(0));
        mKey = (Qt::Key) c.unicode();
        // Assume single unicode keys might be obtained by shift, so no extra info there
        mModifiers &= ~Qt::ShiftModifier;
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
    
    mKey = Qt::Key_unknown;
    return false;
}

bool YKey::parseModifiers(const QString &mods)
{
    bool success = true;
    int pos = -1;
    QRegExp modPattern("[CSMA]-");
    
    while ( (pos = modPattern.indexIn(mods, pos+1)) != -1 ) {
        if ( mods.at(pos) == 'C' )
            mModifiers |= Qt::ControlModifier;
		else if ( mods.at(pos) == 'S' )
			mModifiers |= Qt::ShiftModifier;
        else if ( mods.at(pos) == 'M' )
            mModifiers |= Qt::MetaModifier;
        else if ( mods.at(pos) == 'A' )
            mModifiers |= Qt::AltModifier;
        else
            success = false;
    }

    return success;
}


/* Encode key event into vim form. */
QString YKey::toString() const
{
//    QChar c(mKey);
    QString repr = toBasicRep();
    QString mod;

    if ( mModifiers & Qt::ControlModifier)
        mod += "C-";
    if ( mModifiers & Qt::MetaModifier)
        mod += "M-";
    if ( mModifiers & Qt::AltModifier)
        mod += "A-";
	if ( mModifiers & Qt::ShiftModifier )
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
    
    mKey = Qt::Key_unknown;
    mModifiers = Qt::NoModifier;
    
    charFormat.indexIn(key);

    if ( charFormat.matchedLength() == -1 ) {
        mKey = Qt::Key_unknown;
        return -1;
    }

    if ( charFormat.matchedLength() == 1 ) { // Have single char
        mModifiers = Qt::NoModifier;
        basicKey = charFormat.cap(3);
    }
    else {
        basicKey = charFormat.cap(2);
        if ( ! parseModifiers(charFormat.cap(1)) ) {
            mKey = Qt::Key_unknown;
            return -1;
        }
    }
    
    if ( ! parseBasicRep(basicKey) ) {
        mKey = Qt::Key_unknown;
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
        
        
