
#include <cppunit/TestAssert.h>
#include <cppunit/SourceLine.h>
#include <stdio.h>

#include "PhilAsserts.h"

#define TMP_SIZE 1024



void philNotNull( std::string actualExpr, void * ptr,
				long lineNumber, std::string fileName ) {
	if (ptr != NULL) return;
    throw CppUnit::Exception (CppUnit::Message( "pointer " + actualExpr + " is NULL!" ), CppUnit::SourceLine(fileName, lineNumber)); 

}

void philAssert( std::string actualExpr, bool assertion, 
				long lineNumber, std::string fileName ) {
	if (assertion) return;
    throw CppUnit::Exception (CppUnit::Message("'" + actualExpr +  "' was not true!"), CppUnit::SourceLine(fileName, lineNumber)); 

}

void philNotAssert( std::string actualExpr, bool unassertion, 
				long lineNumber, std::string fileName ) {
	if (unassertion == false) return;
    throw CppUnit::Exception (CppUnit::Message("'" + actualExpr +  "' was not false!"), CppUnit::SourceLine(fileName, lineNumber)); 

}



// --------------------------- numbers

void philAssertEquals( std::string actualExpr, long actual, long expected,
						long lineNumber, std::string fileName )
{
	if (expected == actual) return;
	char tmp[TMP_SIZE];
	std::string s;
	s = "'" + actualExpr +  "' is ";
	sprintf( tmp, "%ld (0x%lX)", actual, actual );
	s += tmp;
	s += " instead of expected ";
	sprintf( tmp, "%ld (0x%lX)\n\n", expected, expected );
	s += tmp;
    throw CppUnit::Exception (CppUnit::Message( s ), CppUnit::SourceLine(fileName, lineNumber)); 
}

void philAssertNotEquals( std::string actualExpr, long actual, long notExpected,
						long lineNumber, std::string fileName )
{
	if (notExpected != actual) return;

	char tmp[TMP_SIZE];
	std::string s;
	s = "'" + actualExpr +  "' should not be ";
	sprintf( tmp, "%ld (0x%lX)\n\n", actual, actual );
	s += tmp;
    throw CppUnit::Exception (CppUnit::Message( s ), CppUnit::SourceLine(fileName, lineNumber)); 
}


void philAssertDeltaEquals( std::string actualExpr, long actual, long expected, long delta,
					long lineNumber, std::string fileName )
{
	long val;
	val = expected - actual;
	if (val < 0) val = - val;
	if (val <= delta) return;

	char tmp[TMP_SIZE];
	std::string s;
	s = "'" + actualExpr +  "' is ";
	sprintf( tmp, "%ld (0x%lX)", actual, actual );
	s += tmp;
	s += " and is not in the interval ";
	sprintf( tmp, "[ %ld , %ld ] ", expected - delta, expected + delta );
	s += tmp;
	sprintf( tmp, "%ld +- %ld\n\n", expected, delta );
	s += tmp;
    throw CppUnit::Exception (CppUnit::Message( s ), CppUnit::SourceLine(fileName, lineNumber)); 
}

// ------------------- pointers
//
void philAssertEquals( std::string actualExpr, void * actual, void * expected,
						long lineNumber, std::string fileName )
{
	if (expected == actual) return;

	char tmp[TMP_SIZE];
	std::string s;
	s = "'" + actualExpr +  "' is ";
	sprintf( tmp, "%p", actual );
	s += tmp;
	s += " instead of expected ";
	sprintf( tmp, "%p\n\n", expected );
	s += tmp;
    throw CppUnit::Exception (CppUnit::Message( s ), CppUnit::SourceLine(fileName, lineNumber)); 

}
void philAssertNotEquals( std::string actualExpr, void *  actual, void *  notExpected,
						long lineNumber, std::string fileName )
{
	if (notExpected != actual) return;

	char tmp[TMP_SIZE];
	std::string s;
	s = "'" + actualExpr +  "' should not be ";
	sprintf( tmp, "%p\n\n", actual );
	s += tmp;
    throw CppUnit::Exception (CppUnit::Message( s ), CppUnit::SourceLine(fileName, lineNumber)); 

}

// ------------------- std::string


void philAssertEquals( std::string actualExpr, std::string actual, std::string expected,
						long lineNumber, std::string fileName )
{
	if (expected.compare( actual ) == 0) return;
	std::string s;
	s = "'" + actualExpr +  "' is '" + actual;
	s += "' instead of expected '" + expected + "'\n\n";
    throw CppUnit::Exception (CppUnit::Message( s ), CppUnit::SourceLine(fileName, lineNumber)); 
}

static std::string strToLower( std::string s )
{
	std::string::iterator it;
	std::string lowS;
	for( it = s.begin(); it != s.end(); it++ ) {
		lowS.append( 1, (char) tolower( *it ) );
	}
	return lowS;
}

void philAssertIEquals( std::string actualExpr, std::string actual, std::string expected,
						long lineNumber, std::string fileName )
{
	philAssertEquals( actualExpr, strToLower( actual ), 
								  strToLower( expected ), 
								  lineNumber, fileName );
}

void philAssertNotEquals( std::string actualExpr, std::string actual, 
								std::string notExpected,
						long lineNumber,
                       std::string fileName )
{
	if (notExpected.compare( actual ) != 0) return;
	std::string s;
	s = "'" + actualExpr +  "' should not be  '" + actual + "'\n\n";
    throw CppUnit::Exception (CppUnit::Message( s ), CppUnit::SourceLine(fileName, lineNumber)); 
}

void philAssertContains( std::string expr, std::string s, std::string sub,
						long lineNumber, std::string fileName )
{
	if (s.find( sub ) != std::string::npos) return;
	std::string e;
	e = "'" + expr +  "' is '" + s + "' and does not contain '" + sub + "'\n\n";
	throw CppUnit::Exception( CppUnit::Message( e ), CppUnit::SourceLine(fileName, lineNumber));
}

void philAssertIContains( std::string expr, std::string s, std::string sub,
						long lineNumber, std::string fileName )
{
	philAssertContains( expr, strToLower( s ), strToLower( sub ), lineNumber, fileName );
}
