/****************************************************************************************/
/*  Mouse.h                                                                             */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Mouse manipulation routines          				                */
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
#ifndef __MOUSE_H__
#define __MOUSE_H__

#include "genesis.h"

#ifdef __cplusplus
extern "C" {
#endif

void Mouse_Set(HWND hWnd, int Width, int Height, geBoolean InAWindow);
void Mouse_ShowWinCursor(void);
void Mouse_HideWinCursor(void);
void Mouse_SetForPlayback(void);
void Mouse_SetForControl(void);
void Mouse_SetForEdit(void);
void Mouse_SetCenter(void);
void Mouse_GetInput(float *DeltaX, float *DeltaY);

#ifdef __cplusplus
}
#endif

#endif