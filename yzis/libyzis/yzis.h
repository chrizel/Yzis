#ifndef YZIS_H
#define YZIS_H
/**
 * yzis.h
 *
 * Main include file for the yzis project
 *
 */


#include <stdio.h>	// printf();
#include <stdlib.h>	// exit();


#define VERSION_MAJOR	0
#define VERSION_MINOR	0
#define VERSION_PATCH	1

#define VERSION_CHAR	"0.0.1"


extern FILE * debugstr;

// top-modern debug system
#define debug(format, arg...)	{fprintf(debugstr, "Yzis Debug :" __FILE__ "( %d ): " format "\n" , __LINE__,  ## arg); fflush(debugstr);} 
#define warning(format, arg...)	{fprintf(debugstr, "Yzis Warning :" __FILE__ "( %d ): " format "\n" , __LINE__,  ## arg); fflush(debugstr);} 
#define error(format, arg...)	{fprintf(debugstr, "Yzis Error :" __FILE__ "( %d ): " format "\n" , __LINE__, ## arg); fflush(debugstr);} 
#define panic(format, arg...)	{ fprintf(debugstr, "Yzis Panic :" __FILE__ "( %d ): " format "\n" , __LINE__, ## arg); exit(-1); fflush(debugstr);}

#define yz_assert(cond,msg, arg...)  if (!(cond)) error("ys_assert failed : "msg,## arg);

#endif // YZIS_H
