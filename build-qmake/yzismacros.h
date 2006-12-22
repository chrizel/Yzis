// Normally, this file is adjusted by cmake
// with qmake, we need to provide it as is
// we make the safe choice that gcc does not contain visibility feature


//merely inspired from kdelibs's kdemacros.h.in
#ifndef YZISMACROS_H
#define YZISMACROS_H

// #cmakedefine __YZIS_HAVE_GCC_VISIBILITY

#ifdef __YZIS_HAVE_GCC_VISIBILITY
#define YZIS_NO_EXPORT __attribute__ ((visibility("hidden")))
#define YZIS_EXPORT __attribute__ ((visibility("default")))
#elif defined(_WIN32) || defined(_WIN64)
#define YZIS_NO_EXPORT
#define YZIS_EXPORT __declspec(dllexport)
#else
#define YZIS_NO_EXPORT
#define YZIS_EXPORT
#endif

#endif
