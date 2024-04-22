/****************************************************************************************/
/*  App.h                                                                               */
/*                                                                                      */
/*  Author: Frank Maddin	                                                            */
/*  Description:  Main module interface                                                 */
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

#ifndef APP_H
#define APP_H

#include "getypes.h"
#include "windows.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct App App;

App*				App_Create(HINSTANCE, HWND, char *CmdLine);
geBoolean			App_Update(App *);
geBoolean			App_Shutdown(App *);
long				App_WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
}
#endif

#endif
