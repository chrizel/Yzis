#ifndef YZIS_H
#define YZIS_H
/**
 * yzis.h
 *
 * Main include file for the yzis project
 *
 */


#include <stdio.h>
#include <stdlib.h>


#define VERSION_MAJOR	0
#define VERSION_MINOR	0
#define VERSION_PATCH	1

#define VERSION_CHAR	"0.0.1"

#define USE_THIS 0

#if USE_THIS
extern FILE * debugstr;

/* top-modern debug system */
#define debug(format, arg...)	{fprintf(debugstr, "Yzis Debug :" __FILE__ "( %d ): " format "\n" , __LINE__,  ## arg); fflush(debugstr);} 
#define warning(format, arg...)	{fprintf(debugstr, "Yzis Warning :" __FILE__ "( %d ): " format "\n" , __LINE__,  ## arg); fflush(debugstr);} 
#define error(format, arg...)	{fprintf(debugstr, "Yzis Error :" __FILE__ "( %d ): " format "\n" , __LINE__, ## arg); fflush(debugstr);} 
#define panic(format, arg...)	{fprintf(debugstr, "Yzis Panic :" __FILE__ "( %d ): " format "\n" , __LINE__, ## arg); fflush(debugstr); exit(-1); }

#define yz_assert(cond,msg, arg...)  if (!(cond)) error("ys_assert failed : " msg,## arg);

#define yz_printfirst(text,line)	debug( text "%c%c%c%c%c", line[0], line[1], line[2], line[3], line[4], line[5])

#else

#define debug(format, arg...)	 
#define warning(format, arg...)	 
/*#define error(format, arg...)	 */ /* these are function names in QT :) */
#define panic(format, arg...)	

#define yz_assert(cond,msg, arg...)  if (!(cond)) debug("ys_assert failed : " msg,## arg);

#define yz_printfirst(text,line)	debug( text "%c%c%c%c%c", line[0], line[1], line[2], line[3], line[4], line[5])

#endif
#endif /* YZIS_H */
