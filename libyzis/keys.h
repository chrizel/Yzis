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
//

/** This is basically a QKeyEvent, although we can not use a QKeyEvent for
 * two reasons
 *  1) it is part of QtGui and not QtCore
 *  2) it might have some more information that we don't care about (sizeoff==24)
 */
class YZIS_EXPORT YKey
{
public:
    YKey(int key = Qt::Key_unknown, Qt::KeyboardModifiers modifiers=Qt::NoModifier, const QString & text = QString());

    QString toString() const;
    int fromString(const QString &);

    int modifiers() const { return mModifiers; }

    void addModifier(Qt::KeyboardModifier m) { mModifiers |= m; }
    void setKey(Qt::Key key) { mKey = key; mText=QChar(key);}

    QString text() const { return mText; }
    int key() const { return mKey; }

    YKey & operator=(const YKey &oth) {
        mKey = oth.mKey;
        mText = oth.mText;
        mModifiers = oth.mModifiers;
        return *this;
    }

    bool operator==(const YKey &oth) const {
        return (mKey == oth.mKey) && (mText == oth.mText) && (mModifiers == oth.mModifiers);
    }
    bool operator!=(const YKey &oth) const { return !operator==(oth); }
    
private:
    QString toBasicRep() const;
    bool parseBasicRep(QString rep);
    bool parseModifiers(const QString &mods);
    
private:
    int mKey;
    QString mText;
    Qt::KeyboardModifiers mModifiers; 
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
