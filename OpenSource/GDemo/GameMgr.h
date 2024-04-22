/****************************************************************************************/
/*  GameMgr.h                                                                           */
/*                                                                                      */
/*  Description:    Misc routines mode and window related  				                */
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

#ifndef __GAMEMGR_H__
#define __GAMEMGR_H__

#include <windows.h>
#include "vidmode.h"
#include "genesis.h"
#include "modelist.h"

#ifdef __cplusplus
extern "C" {
#endif

geBoolean GameMgr_PickMode(HWND hwnd, HANDLE hInstance, geEngine *Engine, 
					 geBoolean NoSelection, geBoolean ManualSelection, 
					 ModeList *List, int ListLength, int *ListSelection, geBoolean *WindowMode);
void GameMgr_ResetMainWindow(HWND hWnd, int32 Width, int32 Height);
geBoolean GameMgr_SetDriverAndMode(geDriver *Driver, geDriver_Mode *DriverMode, int Width, int Height);
geBoolean GameMgr_GetModeData(VidMode *, geCamera **, int *Width, int *Height);

#ifdef __cplusplus
}
#endif

#endif