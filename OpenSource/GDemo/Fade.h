/****************************************************************************************/
/*  Fade.h                                                                              */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Fade control routines               				                */
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

#ifndef __FADE_H__
#define __FADE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "genesis.h"
void Fade_Set(int Dir, float Time);
void Fade_Frame(geEngine *Engine, float ElapsedTime);
void Fade_SetRect(int RWidth, int RHeight);
void Fade_Reset(void);

#ifdef __cplusplus
}
#endif

#endif
