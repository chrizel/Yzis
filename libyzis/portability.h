/* This file is part of the Yzis libraries
*  Copyright (C) 2004 Philippe Fremy <phil@freehackers.org>,
*  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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

#ifndef PORTABILITY_H
#define PORTABILITY_H

#ifdef YZIS_WIN32
 // we are on windows with gcc or msvc

 #include <libintl.h>
 // libintl redefines sprintf and printf
 // but this conflicts with our use of QString::sprintf()
 #undef sprintf
 #undef printf

 // make geteuid work
 #define CHECK_GETEUID( v )  (1)

#endif /* YZIS_WIN32 */

#if defined (YZIS_UNIX) || defined (YZIS_APPLE)
 // we are on unix (or mac?)
 #include <unistd.h>
 #include <dirent.h>
 #include <pwd.h>
 //#include "config.h"
 #include "translator.h"
 #include <libintl.h>

 #define CHECK_GETEUID( v )  (v == geteuid())
#endif /* YZIS_UNIX || YZIS_APPLE */


#ifdef YZIS_WIN32_MSVC
  // windows msvc

 #ifndef S_ISREG
   #define S_ISREG(x) (((x) & S_IFMT) == S_IFREG)
 #endif

 #ifndef S_ISDIR
   #define S_ISDIR(m)  (((m)& S_IFMT) == S_IFDIR)
 #endif

 #ifndef S_IRUSR
   #if S_IREAD
     #define S_IRUSR S_IREAD
   #else
     #define S_IRUSR 00400
   #endif
 #endif

 #ifndef S_IWUSR
   #if S_IWRITE
     #define S_IWUSR S_IWRITE
   #else
     #define S_IWUSR 00200
   #endif
 #endif

#endif /* YZIS_WIN32_MSVC */

/* For an explanation of where this YZIS_FULL_TEMPLATE_EXPORT_INSTANTIATION
 * stuff comes from, see:
 * http://websvn.kde.org/trunk/KDE/kdelibs/kdemacros.h.in?view=markup&pathrev=505607
 * http://lists.trolltech.com/qt-interest/2006-02/thread00180-0.html
 */
#ifdef YZIS_WIN32_MSVC
 #define YZIS_DUMMY_COMPARISON_OPERATOR(C) \
    bool operator==(const C&) const { \
        qWarning(#C"::operator==(const "#C"&) was called"); \
        return false; \
    }
 #define YZIS_DUMMY_QHASH_FUNCTION(C) \
    inline uint qHash(const C) { \
        qWarning("inline uint qHash(const "#C") was called"); \
        return 0; \
    }
#else /* YZIS_WIN32_MSVC */
 #define YZIS_DUMMY_COMPARISON_OPERATOR(C)
 #define YZIS_DUMMY_QHASH_FUNCTION(C)
#endif /* YZIS_WIN32_MSVC */



#endif // PORTABILITY_H

