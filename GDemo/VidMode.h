/****************************************************************************************/
/*  VidMode.h                                                                           */
/*                                                                                      */
/*  Author: Mike Sandige                                                                */
/*  Description:                                                                        */
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
#ifndef VIDMODE_H
#define VIDMODE_H

#include "basetype.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef int32 VidMode;		// changed this from an enum with minimal impact on existing data structures

void VidMode_GetResolution (const VidMode V, int *Width, int *Height);
geBoolean VidMode_SetResolution (VidMode *V, int Width, int Height);


#ifdef __cplusplus
}
#endif

#endif