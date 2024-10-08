/****************************************************************************************/
/*  ctypes.h		                                                                    */
/*                                                                                      */
/*  Description: Some basic types defined with clear names for	    	                */
/*               more specific data definitions                                         */
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
/*  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/

#ifndef __CTYPES_HEADER__
#define __CTYPES_HEADER__
 
#include "basetype.h"

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifndef _WINDEF_
typedef int     BOOL;
#endif

#define	Q_PI	3.14159265358979323846
#define	M_PI	3.14159265358979323846f
#define PI_2	(M_PI*2)
#ifndef NAME_MAX

#define NAME_MAX 64
#endif

#endif

 
