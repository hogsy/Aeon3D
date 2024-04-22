/****************************************************************************************/
/*  gutil.h                                                                             */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Misc utility routines for gdemo       				                */
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
#ifndef __GUTIL_H__
#define __GUTIL_H__

#include "genesis.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	USER_ALL	0xffffffff

geBoolean	GUtil_IsKeyDown (int KeyCode, HWND hWnd);
char *  	GUtil_GetCmdLine(char *CmdLine, char *Data, BOOL flag);
BOOL		GUtil_ShiftKeyPressed(void);
void		GUtil_GetEulerAngles2(const geXForm3d *M, geVec3d *Angles);
void		GUtil_SetEulerAngles2(geXForm3d *M, const geVec3d *Angles);
void		GUtil_OrientFromXForm(geXForm3d *XForm, geVec3d *Pos, geVec3d *Rot);
void		GUtil_GetOrientationFromEulerAngles2(geXForm3d *O, const geVec3d *Angles);
void		GUtil_ScreenShot(geEngine *Engine);

void		GUtil_CalcElapsedTime(LARGE_INTEGER *Freq, LARGE_INTEGER *OldTick, LARGE_INTEGER *CurTick, float *ElapsedTime);

geBoolean	GUtil_UpIsDown(geXForm3d *XForm);
int			GUtil_UpIsDown2(geFloat XRot,geFloat YRot, geFloat ZRot);
geBoolean	GUtil_Collide(geWorld *World, const geVec3d *Front, const geVec3d *Back, GE_Collision *Collision);

void		GUtil_BuildXFormFromRotationOrderXY(geVec3d *Rot, geVec3d *Pos, geXForm3d *XForm);

#ifdef __cplusplus
}
#endif


#endif