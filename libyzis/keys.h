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
    YKey(Qt::Key key = Qt::Key_unknown, Qt::KeyboardModifiers modifiers=Qt::NoModifier)
        : mKey(key), mModifiers(modifiers) 
    { initKeyTable(); if ( isUnicode() ) mModifiers &= ~Qt::ShiftModifier; }

    YKey(QChar rep, Qt::KeyboardModifiers modifiers=Qt::NoModifier);


    QString toString() const;
    int fromString(const QString &);

    int modifiers() const { return mModifiers; }

    void addModifier(Qt::KeyboardModifier m) { mModifiers |= m; }
    void setKey(Qt::Key key) { mKey = key; }
    void setKey(QChar ch) { mKey = (Qt::Key) ch.unicode(); mModifiers &= ~Qt::ShiftModifier; }

    int key() const { return mKey; }

    YKey & operator=(const YKey &oth) {
        mKey = oth.mKey;
        mModifiers = oth.mModifiers;
        return *this;
    }
    
    bool isUnicode() const { return mKey <= 0xffff && mKey >= 0; }

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

    static void initKeyTable();
    
private:
    Qt::Key mKey;
    Qt::KeyboardModifiers mModifiers; 

    static QMap<QString, Qt::Key> keyTable;
    static QMap<QString, Qt::Key> aliasTable;
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
