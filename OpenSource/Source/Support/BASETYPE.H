/****************************************************************************************/
/*  BASETYPE.H                                                                          */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description: Basic type definitions and calling convention defines                  */
/*                                                                                      */
/*  The contents of this file are subject to the Genesis3D Public License               */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.genesis3d.com                                                            */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Genesis3D, released March 25, 1999.                            */
/*  Genesis3D Version 1.1 released November 15, 1999                                 */
/*  Copyright (C) 1999 WildTangent, Inc. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#ifndef GE_BASETYPE_H
#define GE_BASETYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/******** The Genesis Calling Conventions ***********/

#if defined( _MSC_VER )
#	define GENESISCC _fastcall
#else
#	define GENESISCC//__attribute__( ( fastcall ) )
#endif

#if	defined(BUILDGENESIS) && defined(GENESISDLLVERSION)
  #define GENESISAPI	_declspec(dllexport)
#else
  #if	defined(GENESISDLLVERSION)
    #define GENESISAPI	_declspec(dllimport)
  #else
    #define GENESISAPI
  #endif
#endif

/******** The Basic Types ****************************/

#include <stdint.h>

typedef signed int geBoolean;
#define GE_FALSE	(0)
#define GE_TRUE		(1)

typedef float geFloat;

#ifndef NULL
#define NULL	((void *)0)
#endif

typedef int32_t 	int32;
typedef int16_t 	int16;
typedef int8_t 		int8;
typedef uint32_t 	uint32;
typedef uint16_t 	uint16;
typedef uint8_t 	uint8;

#ifndef TRUE
#	define TRUE GE_TRUE
#endif
#ifndef FALSE
#	define FALSE GE_FALSE
#endif

#ifndef MAX
#	define MAX( aa, bb ) ( ( aa ) > ( bb ) ? ( aa ) : ( bb ) )
#endif
#ifndef MIN
#	define MIN( aa, bb ) ( ( aa ) < ( bb ) ? ( aa ) : ( bb ) )
#endif

#if !defined( _WIN32 )

#	include <libgen.h>

#	define strnicmp strncasecmp
#	define stricmp  strcasecmp

#if 0//TODO
static inline void _splitpath(
	    const char *path,
	    char *drive,
	    char *dir,
	    char *fname,
	    char *ext )
{
	if ( drive != NULL )
	{
		// uhhhh, this'll do for now...
		drive = "/";
	}

	static char fullname[ 256 ];
	char *c = strrchr( path, '/' );
	if ( c != NULL )
	{
		snprintf( fullname, sizeof( fullname ), "%s", c );
		if ( ext != NULL )
		{
			ext = strrchr( fullname, '.' );
		}
	}

	if ( fname != NULL )
	{

	}
}
#endif

#endif

/******** Macros on Genesis basic types *************/

#define GE_ABS(x)				( (x) < 0 ? (-(x)) : (x) )
#define GE_CLAMP(x,lo,hi)		( (x) < (lo) ? (lo) : ( (x) > (hi) ? (hi) : (x) ) )
#define GE_CLAMP8(x)			GE_CLAMP(x,0,255)
#define GE_CLAMP16(x)			GE_CLAMP(x,0,65536)
#define GE_BOOLSAME(x,y)		( ( (x) && (y) ) || ( !(x) && !(y) ) )

#define GE_EPSILON				((geFloat)0.000797f)
#define GE_FLOATS_EQUAL(x,y)	( GE_ABS((x) - (y)) < GE_EPSILON )
#define GE_FLOAT_ISZERO(x)		GE_FLOATS_EQUAL(x,0.0f)

#define	GE_PI					((geFloat)3.14159265358979323846f)

/****************************************************/

#ifdef __cplusplus
}
#endif

#endif

