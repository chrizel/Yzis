
#ifndef PHIL_ASSERT_H
#define PHIL_ASSERT_H

#include <string>

#include <cppunit/Portability.h>
#include <cppunit/Exception.h>
#include <cppunit/TestAssert.h>

/**
 * This is a set of replacements for the default asserts of CppUnit. This
 * works with CppUnit 1.8.0 . Thank to the architecture of CppUnit, it
 * integrates seemlessly.
 *
 * The differences are:
 *
 * - my asserts are shorter and easier to type because I do not prepend them
 *   by CPPUNIT_ and because I use mainly non capitalised letters. This does
 *   not respect the convention that macros should be in capitalised letters
 *   but it saves me a lot of pain, so it is worthwile.
 *
 * - the order of equality assertion is reversed, because it is more intuitive
 *   for me to have the actual value before the expected one. So
 *   CPPUNIT_ASSERT_EQUALS( 1, a ) becomes checkEquals( a, 1 )
 *
 * - I display the name of the expression being tested inside the failure, not
 *   just the value that did not match.
 *
 * - for int equality asserts, I display the int value in decimal and
 *   hexadecimal
 *
 * - I support native string equality tests, which avoids the following
 *   drawbacks of CppUnit:
 *
 *   	1. With Visual C++, CPPUNIT_ASSERT_EQUAL( std::string("a"), "a" )
 *   	won't compile because the it does not know which template to use.
 *
 *   	2. If you do a
 *
	char * s1 = "abcd";
	char s2[10];
	strcpy( s2, s1);
	CPPUNIT_ASSERT_EQUAL( s1, (char *) s2 );

 * 		CppUnit will report a failure with the unhelpful message :
 *
 * 		"Expected: abcd, but was: abcd."
 *
 * 		This is because CppUnit has unintuitively compared the two pointers
 * 		instead of comparing the strings. My functions know how to cast a
 * 		(char *) to a string
 *
 *
 * - when displaying an error with a string, my strings are in quote, so that
 *   you can see precicely where it begins and finish. No more
 *   your-strings-are-not-equal-but-you-do-not-know-why-because-you-have-
 *   not-seen-this-naughty-space-at-the-end-of-one-of-them.
 *
 * - for string equality asserts, I support case sensitive and insenstive
 *   check.
 *
 * - I support also a assertion to test whether a string contains another one,
 *   with adjustable case sensitivity
 *
 * - if you define QT_DLL, I support tests on QString too
 *
 *
 * This list of macros available are :
 * phCheck( assertion ) for bool
 * phCheckEquals( actual, expected ) for int, long, double, std::string, QString and char *
 * phCheckNotEquals( actual, notExpected ) for the same types as phCheckEquals
 * phCheckDeltaEquals( doubleValue1, doubleValue2 ) for double
 * phCheckIEquals( s1, s2 )  for std::string, QString, (char *)
 * phCheckContains( s1, s2 ) for std::string, QString, (char *)
 * phCheckIContains( s1, s2 )  for std::string, QString, (char *)
 *
 * Send any bug, comment, suggestion, patch to phil@freehackers.org
 *
 */

void philAssert( std::string actualExpr, bool assertion,
				long lineNumber, std::string fileName );

void philNotNull( std::string actualExpr, void * ptr,
				long lineNumber, std::string fileName );

void philNotAssert( std::string actualExpr, bool assertion,
				long lineNumber, std::string fileName );


// numbers
void philAssertEquals( std::string actualExpr, long actual, long expected,
						long lineNumber, std::string fileName );
void philAssertDeltaEquals( std::string actualExpr, long actual, long expected, long delta,
						long lineNumber, std::string fileName );
void philAssertNotEquals( std::string actualExpr, long actual, long notExpected,
						long lineNumber, std::string fileName );

// pointers
void philAssertEquals( std::string actualExpr, void * actual, void * expected,
						long lineNumber, std::string fileName );
void philAssertNotEquals( std::string actualExpr, void *  actual, void *  notExpected,
						long lineNumber, std::string fileName );

// std::string

void philAssertEquals( std::string actualExpr, std::string actual, std::string expected,
						long lineNumber, std::string fileName );
/*
void philAssertEquals( std::string actualExpr, char * actual, char * expected,
						long lineNumber, std::string fileName )
{ philAssertEquals( actualExpr, std::string( actual ), std::string( expected ), lineNumber, fileName ); }
*/

// same as previous, but case insensitive
void philAssertIEquals( std::string actualExpr, std::string actual, std::string expected,
						long lineNumber, std::string fileName );
void philAssertNotEquals( std::string actualExpr, std::string actual, std::string expected,
						long lineNumber, std::string fileName );
void philAssertContains( std::string expr, std::string s, std::string sub,
						long lineNumber, std::string fileName );
void philAssertIContains( std::string expr, std::string s, std::string sub,
						long lineNumber, std::string fileName );


#ifdef QT_DLL
#include <qstring.h>

/* It is necessary to provide QString functions, because VC++ does not cast
 * QString to char * or std::string
 */

inline void philAssertEquals( std::string actualExpr, QString actual, QString expected,
					  long lineNumber, std::string fileName );
inline void philAssertIEquals( std::string actualExpr, QString actual, QString expected,
						long lineNumber, std::string fileName );

inline void philAssertNotEquals( std::string actualExpr, QString actual, QString expected,
						long lineNumber, std::string fileName );

inline void philAssertContains( std::string actualExpr, QString string, QString substring,
						long lineNumber, std::string fileName );

inline void philAssertIContains( std::string actualExpr, QString string, QString substring,
						 long lineNumber, std::string fileName );

inline void philAssertEquals( std::string actualExpr, QString actual, QString expected,
					  long lineNumber, std::string fileName ) {
	philAssertEquals( actualExpr, std::string( actual.latin1() ),
		std::string( expected.latin1() ), lineNumber, fileName );
}

inline void philAssertIEquals( std::string actualExpr, QString actual, QString expected,
						long lineNumber, std::string fileName ) {
	philAssertIEquals( actualExpr, std::string(actual.latin1()), std::string( expected.latin1()), lineNumber, fileName );
}

inline void philAssertNotEquals( std::string actualExpr, QString actual, QString expected,
						long lineNumber, std::string fileName ) {
	philAssertNotEquals( actualExpr, std::string( actual.latin1()), std::string(expected.latin1()), lineNumber, fileName );
}

inline void philAssertContains( std::string actualExpr, QString string, QString substring,
						long lineNumber, std::string fileName ) {
	philAssertContains( actualExpr, std::string(string.latin1()), std::string(substring.latin1()), lineNumber, fileName );
}

inline void philAssertIContains( std::string actualExpr, QString string, QString substring, long lineNumber, std::string fileName ) {
	philAssertIContains( actualExpr, std::string(string.latin1()), std::string(substring.latin1()), lineNumber, fileName );
}

#endif // Qt


#define phCheckNotNull( pointer )	(philNotNull( (#pointer), (pointer),__LINE__,__FILE__))
#define phCheckAssert( actual )	(philAssert( (#actual), (actual),__LINE__,__FILE__))
#define phCheckNotAssert( actual )(philNotAssert( (#actual), ((actual)),__LINE__,__FILE__))

#define phCheckEquals( actual, expected ) \
  (philAssertEquals ( (#actual), (actual),\
    (expected),__LINE__,__FILE__))

#define phCheckIEquals( actual, expected ) \
  (philAssertIEquals ( (#actual), (actual),\
    (expected),__LINE__,__FILE__))

#define phCheckNotEquals( actual, notExpected ) \
  (philAssertNotEquals ( (#actual), (actual),\
    (notExpected),__LINE__,__FILE__))

#define phCheckContains( s, sub ) \
  (philAssertContains ( (#s), (s),\
    (sub),__LINE__,__FILE__))

#define phCheckIContains( s, sub ) \
  (philAssertIContains ( (#s), (s),\
    (sub),__LINE__,__FILE__))

#define phCheckDeltaEquals( actual, expected, delta ) \
  (philAssertDeltaEquals ( (#actual), (actual),\
    (expected), (delta),__LINE__,__FILE__))



#endif  // PHIL_ASSERT_H






