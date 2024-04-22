/****************************************************************************************/
/*  Pathpoint.c                                                                         */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Creates a motion/path from entity points			                */
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
#ifndef PATHPOINT_H
#define PATHPOINT_H

#ifdef __cplusplus
extern "C" {
#endif

geMotion *PathPoint_GetMotion(geWorld *World, void *FirstPoint, geBoolean *);

#ifdef __cplusplus
}
#endif


#endif
