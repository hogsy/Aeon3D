/****************************************************************************************/
/*  Cam.h                                                                               */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Interface for camera manipulation routines			                */
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

#ifndef __CAM_H__
#define __CAM_H__

#include "objmgr.h"
#include "genesis.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Cam Cam;

void		Cam_SetIndex(Cam *Cam, int Index);
void		Cam_SetFrameTime(Cam *Cam, float FrameTime);
void		Cam_Reset(Cam* Cam);
geMotion**	Cam_GetMotion(Cam* Cam);


Cam			*Cam_Create(char *LevelName, Object *Object);
geBoolean	Cam_CreateMotion(Cam *Cam, char *LevelName);
geBoolean	Cam_SaveMotion(Cam *Cam, char *LevelName);
geBoolean	Cam_SaveMotionDebug(Cam *Cam, char *LevelName);
void		Cam_Destroy(Cam **Cam);
geBoolean	Cam_Update(geWorld *World, Cam *Cam, geXForm3d *XForm, geVec3d *Channels, float ElapsedTime);

#ifdef __cplusplus
}
#endif

#endif