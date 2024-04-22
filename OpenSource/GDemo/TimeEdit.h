/****************************************************************************************/
/*  TimeEdit.h                                                                          */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Timeline editor for motions								            */
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
#ifndef TIME_EDIT_H
#define TIME_EDIT_H

#pragma warning(disable : 4201 4214 4115)
#include <windows.h>
#pragma warning(default : 4201 4214 4115)

#include "basetype.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TimeEdit TimeEdit;
typedef geBoolean (*TE_CB)(void *CBdata, int Type, float Time, float *TimeChange);

typedef enum TE_TYPE
{
	TE_DEL_KEY, 
	TE_INS_KEY, 
	TE_MOVE_KEY, 
	TE_DEL_EVENT, 
	TE_INS_EVENT, 
	TE_MOVE_EVENT, 
	TE_TIME_CHG, 
	TE_SEL_NONE,
	TE_SEL_KEY,
	TE_SEL_EVENT,
	TE_SEL_TIME,
	TE_INTERNAL_ERROR, // for reporting errors that do not originate from the controlling dialog
};

void		TimeEdit_Clear(TimeEdit* TE);
geBoolean	TimeEdit_InitAddKey(TimeEdit* TE, float Time);
geBoolean	TimeEdit_InitAddEvent(TimeEdit* TE, float Time);
void		TimeEdit_Destroy(TimeEdit *TE);

HWND		TimeEdit_CreateWindow(	HANDLE hInstance,
									HWND hwndParent,
									TE_CB,
									void *,
									char *AppName, 
									int32 Width, 
									int32 Height);


#ifdef __cplusplus
}
#endif

#endif

