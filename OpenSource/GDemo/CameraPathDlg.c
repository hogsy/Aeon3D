/****************************************************************************************/
/*  CameraPath.c                                                                        */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Maintains a dialog and motions for a camera			                */
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

#define	WIN32_LEAN_AND_MEAN
#pragma warning(disable : 4201 4214 4115)
#include	<windows.h>
#pragma warning(default : 4201 4214 4115)

#include	<stdio.h>
#include	<stdlib.h>
#include	<assert.h>

#include	"genesis.h"
#include	"resrc1.h"
#include	"resource.h"

#include "CameraPathDlg.h"

#include "errorlog.h"
#include "ram.h"

#include "TimeEdit.h"
#include "wmuser.h"
#include "ctypes.h"

#ifndef GENESIS_20
#define GE_ERR_WINDOWS_API_FAILURE 99
#define GE_ERR_SUBSYSTEM_FAILURE   98
#define geErrorLog_IntToString(xx) NULL
#endif

typedef struct CameraPath
{
	
	HWND				hwnd;
	HWND				hwndParent;
	HWND				hwndTimeEdit;
	float				CurrentTime;
	//geMotion *			Motion;
	geMotion **			Motion;
	CP_CB	 			CP_AppCallback;
	TimeEdit *			TE;
	int					CurrentCut;	

	// all current data possible from the current selection
	int					SelType;	// what is selected
	float				SelTime;	// current time
	int					SelIndex;	// index for keys
	char				SelString[512];	// event
	geXForm3d			SelXForm1;	// key data
	geXForm3d			SelXForm2;	// key data
}CameraPath;

typedef enum CPSelType
{
CP_SELTYPE_NONE=0,
CP_SELTYPE_KEY,
CP_SELTYPE_EVENT,
CP_SELTYPE_TIMEBAR,
};

const char *CameraPathDlg_SceneName=NULL;

#define EVENT_TIMECHECK_RANGE 0.001f
#define KEY_TIMECHECK_RANGE 0.00001f

CameraPath *GlobData;

static geBoolean	CameraPath_UpdateDialog(CameraPath *CP);
static void			CameraPath_SafeEventInsertTime(CameraPath *CP, float *InsertTime);
static void			CameraPath_SafeKeyInsertTime(CameraPath *CP, float *InsertTime);
static geBoolean	CameraPath_UpdateMotion(CameraPath *CP);
static geBoolean	CameraPath_InitTimeEdit(CameraPath *CP, TimeEdit *TE);
static geBoolean	CameraPath_GetPaths(CameraPath *CP, int MotionIndex, gePath **Path0, gePath **Path1);

void CameraPath_Destroy(CameraPath **CP);

static geBoolean CameraPath_UpdateMotion(CameraPath *CP)
{
	geBoolean ret;

	assert(CP);

	switch (CP->SelType)
	{
	case CP_SELTYPE_NONE:
		break;
	case CP_SELTYPE_KEY:
		{
		float FieldOfView;
		geXForm3d NewXForm, BaseXForm;
		char NewString[512];
		int PosCount, PosIndex;
		gePath *Path0, *Path1;
		float Time;
		geVec3d Pos,Angles;
		int ret;

		if (CameraPath_GetPaths(CP, CP->CurrentCut, &Path0, &Path1) == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CameraPath_UpdateMotion: Unable to get paths.", NULL);
			return GE_FALSE;
			}

		PosIndex = gePath_GetKeyframeIndex(Path0, GE_PATH_TRANSLATION_CHANNEL, CP->SelTime);

		// err check
		PosCount = gePath_GetKeyframeCount(Path0, GE_PATH_TRANSLATION_CHANNEL);
		assert(PosIndex >= 0 && PosIndex < PosCount);

		// get current translation
		gePath_GetKeyframe(Path0, PosIndex, GE_PATH_TRANSLATION_CHANNEL, &Time, &BaseXForm);

		///////////////////////////////////////////////////////////////////////////////
		// Rotation set from dialog - into NewXForm

		if (GetWindowText(GetDlgItem(CP->hwnd, IDC_ROTX), NewString, sizeof(NewString)) == 0)
		{
			if (GetLastError()!=0)
				{
				geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateMotion:  GetWindowText failed",geErrorLog_IntToString(GetLastError));
				return GE_FALSE;
				}
		}
		Angles.X = (float)atof(NewString);

		if (GetWindowText(GetDlgItem(CP->hwnd, IDC_ROTY), NewString, sizeof(NewString)) == 0)
		{
			if (GetLastError()!=0)
				{
				geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateMotion:  GetWindowText failed",geErrorLog_IntToString(GetLastError));
				return GE_FALSE;
				}
		}
		Angles.Y = (float)atof(NewString);

		if (GetWindowText(GetDlgItem(CP->hwnd, IDC_ROTZ), NewString, sizeof(NewString)) == 0)
		{
			if (GetLastError()!=0)
				{
				geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateMotion:  GetWindowText failed",geErrorLog_IntToString(GetLastError));
				return GE_FALSE;
				}
		}
		Angles.Z = (float)atof(NewString);

		Angles.X *= (M_PI/180);
		Angles.Y *= (M_PI/180);
		Angles.Z *= (M_PI/180);
		geXForm3d_SetIdentity(&NewXForm);
		geXForm3d_SetEulerAngles(&NewXForm, &Angles);

		///////////////////////////////////////////////////////////////////////////////
		// Translation set from dialog - into NewXForm

		if (GetWindowText(GetDlgItem(CP->hwnd, IDC_POSX), NewString, sizeof(NewString)) == 0)
		{
			if (GetLastError()!=0)
				{
				geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateMotion:  GetWindowText failed",geErrorLog_IntToString(GetLastError));
				return GE_FALSE;
				}
		}
		Pos.X = (float)atof(NewString);

		if (GetWindowText(GetDlgItem(CP->hwnd, IDC_POSY), NewString, sizeof(NewString)) == 0)
		{
			if (GetLastError()!=0)
				{
				geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateMotion:  GetWindowText failed",geErrorLog_IntToString(GetLastError));
				return GE_FALSE;
				}
		}
		Pos.Y = (float)atof(NewString);

		if (GetWindowText(GetDlgItem(CP->hwnd, IDC_POSZ), NewString, sizeof(NewString)) == 0)
			if (GetLastError()!=0)
				{
				geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateMotion:  GetWindowText failed",geErrorLog_IntToString(GetLastError));
				return GE_FALSE;
				}
		Pos.Z = (float)atof(NewString);

		///////////////////////////////////////////////////////////////////////////////
		// Modify both channels and update the app

		NewXForm.Translation = Pos;
		ret = gePath_ModifyKeyframe(Path0, PosIndex, GE_PATH_TRANSLATION_CHANNEL, &NewXForm);
		assert(ret);
		gePath_ModifyKeyframe(Path0, PosIndex, GE_PATH_ROTATION_CHANNEL, &NewXForm);
		assert(ret);

		// tell the app about the change
		CP->CP_AppCallback(CAMERA_PUT_DATA, -1.0f, &NewXForm, NULL);

		///////////////////////////////////////////////////////////////////////////////
		// Field of view (Path 1) set from dialog

		if (GetWindowText(GetDlgItem(CP->hwnd, IDC_USER1), NewString, 512) == 0)
			if (GetLastError()!=0)
				{
				geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateMotion:  GetWindowText failed",geErrorLog_IntToString(GetLastError));
				return GE_FALSE;
				}

		FieldOfView = (float)atof(NewString);

		PosIndex = gePath_GetKeyframeIndex(Path1, GE_PATH_TRANSLATION_CHANNEL, CP->SelTime);

		// err check
		PosCount = gePath_GetKeyframeCount(Path1, GE_PATH_TRANSLATION_CHANNEL);
		assert(PosIndex >= 0 && PosIndex < PosCount);

		// get current translation
		gePath_GetKeyframe(Path1, PosIndex, GE_PATH_TRANSLATION_CHANNEL, &Time, &BaseXForm);

		//modify the X component and modify the keyframe
		BaseXForm.Translation.X = FieldOfView;
		gePath_ModifyKeyframe(Path1, PosIndex, GE_PATH_TRANSLATION_CHANNEL, &BaseXForm);

		// tell the app about the change
		CP->CP_AppCallback(CAMERA_PUT_DATA, -1.0f, NULL, &BaseXForm);
		break;
		}
	case CP_SELTYPE_EVENT:
		{
		float Time;
		char NewString[512];

		Time = CP->SelTime;

		// get edit box text
		if (GetWindowText(GetDlgItem(CP->hwnd, IDC_EVENTTEXT), NewString, 512) == 0)
			if (GetLastError()!=0)
				{
				geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateMotion:  GetWindowText failed",geErrorLog_IntToString(GetLastError));
				return GE_FALSE;
				}

		strcpy(CP->SelString, NewString);

		ret = geMotion_DeleteEvent(CP->Motion[CP->CurrentCut], Time);
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CameraPath_UpdateMotion: Unable to delete an event.", NULL);
			return GE_FALSE;
			}

		// put edit box text back in
		CameraPath_SafeEventInsertTime(CP, &Time);
		ret = geMotion_InsertEvent(CP->Motion[CP->CurrentCut], Time, NewString);
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CameraPath_UpdateMotion: Unable to insert an event.", NULL);
			return GE_FALSE;
			}

		if (CameraPath_UpdateDialog(CP) == GE_FALSE)
			{
			geErrorLog_AddString(-1,"CameraPath_UpdateMotion: Unable to update dialog.", NULL);
			return GE_FALSE;
			}
			
		break;
		}
	}

	return GE_TRUE;
}

static geBoolean CameraPath_ClearDialogText(CameraPath *CP)
{
	assert(CP != NULL);
	SetWindowText(GetDlgItem(CP->hwnd, IDC_EVENTTEXT), "");

	SetWindowText(GetDlgItem(CP->hwnd, IDC_POSX), "");
	SetWindowText(GetDlgItem(CP->hwnd, IDC_POSY), "");
	SetWindowText(GetDlgItem(CP->hwnd, IDC_POSZ), "");

	SetWindowText(GetDlgItem(CP->hwnd, IDC_ROTX), "");
	SetWindowText(GetDlgItem(CP->hwnd, IDC_ROTY), "");
	SetWindowText(GetDlgItem(CP->hwnd, IDC_ROTZ), "");

	SetWindowText(GetDlgItem(CP->hwnd, IDC_USER1), "");
	SetWindowText(GetDlgItem(CP->hwnd, IDC_USER2), "");
	SetWindowText(GetDlgItem(CP->hwnd, IDC_USER3), "");

	return GE_TRUE;
}

static geBoolean CameraPath_SetDialogReadOnly(CameraPath *CP, geBoolean ReadOnly)
{
	assert(CP != NULL);

	SendDlgItemMessage(CP->hwnd,IDC_EVENTTEXT,EM_SETREADONLY,(WPARAM)ReadOnly,(LPARAM)0);

	SendDlgItemMessage(CP->hwnd, IDC_POSX, EM_SETREADONLY,(WPARAM)ReadOnly,(LPARAM)0);
	SendDlgItemMessage(CP->hwnd, IDC_POSY, EM_SETREADONLY,(WPARAM)ReadOnly,(LPARAM)0);
	SendDlgItemMessage(CP->hwnd, IDC_POSZ, EM_SETREADONLY,(WPARAM)ReadOnly,(LPARAM)0);

	SendDlgItemMessage(CP->hwnd, IDC_ROTX, EM_SETREADONLY,(WPARAM)ReadOnly,(LPARAM)0);
	SendDlgItemMessage(CP->hwnd, IDC_ROTY, EM_SETREADONLY,(WPARAM)ReadOnly,(LPARAM)0);
	SendDlgItemMessage(CP->hwnd, IDC_ROTZ, EM_SETREADONLY,(WPARAM)ReadOnly,(LPARAM)0);

	SendDlgItemMessage(CP->hwnd, IDC_USER1, EM_SETREADONLY,(WPARAM)ReadOnly,(LPARAM)0);
	SendDlgItemMessage(CP->hwnd, IDC_USER2, EM_SETREADONLY,(WPARAM)ReadOnly,(LPARAM)0);
	SendDlgItemMessage(CP->hwnd, IDC_USER3, EM_SETREADONLY,(WPARAM)ReadOnly,(LPARAM)0);

	return GE_TRUE;
}

static geBoolean CameraPath_UpdateDialog(CameraPath *CP)
{
	assert(CP);

	switch (CP->SelType)
	{
	case CP_SELTYPE_TIMEBAR:
	case CP_SELTYPE_NONE:
		// blank all fields
		CameraPath_SetDialogReadOnly(CP, TRUE);
		CameraPath_ClearDialogText(CP);
		break;
	case CP_SELTYPE_KEY:
		{
		char String[32];
		geVec3d angles;

		// tranlsation
		sprintf(String,"%.3f",CP->SelXForm1.Translation.X);
		if (SetWindowText(GetDlgItem(CP->hwnd, IDC_POSX), String) == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateDialog: SetWindowText failed.", NULL);
			return GE_FALSE;
			}
		sprintf(String,"%.3f",CP->SelXForm1.Translation.Y);
		if (SetWindowText(GetDlgItem(CP->hwnd, IDC_POSY), String) == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateDialog: SetWindowText failed.", NULL);
			return GE_FALSE;
			}
		sprintf(String,"%.3f",CP->SelXForm1.Translation.Z);
		if (SetWindowText(GetDlgItem(CP->hwnd, IDC_POSZ), String) == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateDialog: SetWindowText failed.", NULL);
			return GE_FALSE;
			}

		// rotation
		geXForm3d_GetEulerAngles(&CP->SelXForm1, &angles);
		angles.X *= (180/M_PI);
		angles.Y *= (180/M_PI);
		angles.Z *= (180/M_PI);
		sprintf(String,"%.3f",angles.X);
		if (SetWindowText(GetDlgItem(CP->hwnd, IDC_ROTX), String) == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateDialog: SetWindowText failed.", NULL);
			return GE_FALSE;
			}
		sprintf(String,"%.3f",angles.Y);
		if (SetWindowText(GetDlgItem(CP->hwnd, IDC_ROTY), String) == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateDialog: SetWindowText failed.", NULL);
			return GE_FALSE;
			}
		sprintf(String,"%.3f",angles.Z);
		if (SetWindowText(GetDlgItem(CP->hwnd, IDC_ROTZ), String) == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateDialog: SetWindowText failed.", NULL);
			return GE_FALSE;
			}

		// user translation
		sprintf(String,"%.3f",CP->SelXForm2.Translation.X);
		if (SetWindowText(GetDlgItem(CP->hwnd, IDC_USER1), String) == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateDialog: SetWindowText failed.", NULL);
			return GE_FALSE;
			}
		sprintf(String,"%.3f",CP->SelXForm2.Translation.Y);
		if (SetWindowText(GetDlgItem(CP->hwnd, IDC_USER2), String) == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateDialog: SetWindowText failed.", NULL);
			return GE_FALSE;
			}
		sprintf(String,"%.3f",CP->SelXForm2.Translation.Z);
		if (SetWindowText(GetDlgItem(CP->hwnd, IDC_USER3), String) == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateDialog: SetWindowText failed.", NULL);
			return GE_FALSE;
			}
		
		CameraPath_SetDialogReadOnly(CP, FALSE);
		SendDlgItemMessage(CP->hwnd,IDC_EVENTTEXT,EM_SETREADONLY,(WPARAM)TRUE,(LPARAM)0);

		// for now you can't edit the rotation data
		//SendDlgItemMessage(CP->hwnd, IDC_ROTX, EM_SETREADONLY,(WPARAM)TRUE,(LPARAM)0);
		//SendDlgItemMessage(CP->hwnd, IDC_ROTY, EM_SETREADONLY,(WPARAM)TRUE,(LPARAM)0);
		//SendDlgItemMessage(CP->hwnd, IDC_ROTZ, EM_SETREADONLY,(WPARAM)TRUE,(LPARAM)0);
		break;
		}
	case CP_SELTYPE_EVENT:
		CameraPath_SetDialogReadOnly(CP, TRUE);
		CameraPath_ClearDialogText(CP);

		SendDlgItemMessage(CP->hwnd, IDC_EVENTTEXT, EM_SETREADONLY,(WPARAM)FALSE,(LPARAM)0);
		if (SetWindowText(GetDlgItem(CP->hwnd, IDC_EVENTTEXT), CP->SelString) == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_WINDOWS_API_FAILURE,"CameraPath_UpdateDialog: SetWindowText failed.", NULL);
			return GE_FALSE;
			}
		break;
	}

	return GE_TRUE;
}

static geBoolean CameraPath_GetPaths(CameraPath *CP, int MotionIndex, gePath **Path0, gePath **Path1)
{
	int Count;
	int Cut;

	assert(CP);
	assert(Path0 && *Path0);
	assert(Path1 && *Path1);

	Cut = MotionIndex;
	assert(Cut >= 0 && Cut < MAX_CAMERA_CUTS);
	Count = geMotion_GetPathCount(CP->Motion[Cut]);

	// if a cut does not exist then create an empty one
	if (Count == 0)
		{
		int InsIndex;
		gePath *PathA, *PathB;
		geBoolean Loop = GE_TRUE;
		char String[256];
		int ret;

		// create a path here
		PathA = gePath_Create(GE_PATH_INTERPOLATE_HERMITE, GE_PATH_INTERPOLATE_SQUAD, Loop);
		PathB = gePath_Create(GE_PATH_INTERPOLATE_HERMITE, GE_PATH_INTERPOLATE_SQUAD, Loop);

		sprintf(String,"Cut%d - Path0",CP->CurrentCut);
		ret = geMotion_AddPath(CP->Motion[Cut], PathA, String, &InsIndex);
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to add path to motion.", String);
			return GE_FALSE;
			}
		sprintf(String,"Cut%d - Path1",CP->CurrentCut);
		ret = geMotion_AddPath(CP->Motion[Cut], PathB, String, &InsIndex);
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to add path to motion.", String);
			return GE_FALSE;
			}

		gePath_Destroy(&PathA);
		gePath_Destroy(&PathB);
		}

	*Path0 = geMotion_GetPath(CP->Motion[Cut], 0);
	assert(*Path0);
	*Path1 = geMotion_GetPath(CP->Motion[Cut], 1);
	assert(*Path1);

	return GE_TRUE;
}

static void CameraPath_SafeEventInsertTime(CameraPath *CP, float *InsertTime)
{
	float Time;
	float ThisTime;
	char *EventString;
	int Count;

	assert(CP);
	assert(InsertTime);
	assert(CP->CurrentCut >= 0 && CP->CurrentCut < MAX_CAMERA_CUTS);

	Time = *InsertTime;

	geMotion_SetupEventIterator(CP->Motion[CP->CurrentCut], Time-EVENT_TIMECHECK_RANGE, Time+EVENT_TIMECHECK_RANGE);

	Count = 0;
	while( geMotion_GetNextEvent( CP->Motion[CP->CurrentCut], &ThisTime, &EventString ) )
		{
		Count++;
		}

	if (Count > 0)
		{
		// keep moving to the right until you do not hit an event
		*InsertTime += EVENT_TIMECHECK_RANGE;
		CameraPath_SafeEventInsertTime(CP, InsertTime);
		}

	// return whatever InsertTime is now
	return;
}

static void CameraPath_SafeKeyInsertTime(CameraPath *CP, float *InsertTime)
{
	gePath *Path0;
	geXForm3d TrashXForm;
	int ret;

	assert(CP);
	assert(InsertTime);
	assert(CP->CurrentCut >= 0 && CP->CurrentCut < MAX_CAMERA_CUTS);

	Path0 = geMotion_GetPath(CP->Motion[CP->CurrentCut], 0);
	assert(Path0);

	// note: I tried using GetKeyframeIndex instead but it crashed on an empty path
	geXForm3d_SetIdentity(&TrashXForm);
	ret = gePath_InsertKeyframe(Path0, GE_PATH_ALL_CHANNELS, *InsertTime, &TrashXForm); 

	if (ret == GE_FALSE)
		{
		// keep moving to the right until you do not hit an event
		*InsertTime += KEY_TIMECHECK_RANGE;
		CameraPath_SafeKeyInsertTime(CP, InsertTime);
		}
	else
	if (ret == GE_TRUE)
		{
		int Index;
		int Count;

		Count = gePath_GetKeyframeCount(Path0, GE_PATH_ROTATION_CHANNEL);
		Index = gePath_GetKeyframeIndex(Path0, GE_PATH_ROTATION_CHANNEL, *InsertTime);
		assert(Index >= 0 && Index < Count);

		ret = gePath_DeleteKeyframe(Path0,Index,GE_PATH_ROTATION_CHANNEL); 
		assert(ret);
		ret = gePath_DeleteKeyframe(Path0,Index,GE_PATH_TRANSLATION_CHANNEL); 
		assert(ret);
		}

	// return whatever InsertTime is now
	return;
}


static void CameraPath_ExitOnError(CameraPath *CP)
{
	HWND parent, hwnd;

	assert(CP);

	// get rid of time edit
	TimeEdit_Destroy(CP->TE);

	// post a message to the app
	parent = CP->hwndParent;
	hwnd   = CP->hwnd;
	//CameraPath_Destroy(&CP);
	EndDialog(hwnd, 1);

	// the last thing is to send the message
	PostMessage(parent,WM_CAMERA_EDIT_QUIT,0,GE_FALSE);
}

static geBoolean CP_CalledByTimeEdit(void *CallbackData, int Type, float Time, float *TimeChange)
{
	gePath *Path0;
	gePath *Path1;
	CameraPath *CP;
	geBoolean ret;

	assert(CallbackData);

	CP = (void *)CallbackData;

	// first handle any error
	if (Type == TE_INTERNAL_ERROR)
		{
		CameraPath_ExitOnError(CP);
		return GE_TRUE;
		}

	ret = CameraPath_GetPaths(CP, CP->CurrentCut, &Path0, &Path1);
	if (ret == GE_FALSE)
		{
		geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to get paths.", NULL);
		CameraPath_ExitOnError(CP);
		return GE_FALSE;
		}

	assert(Path0);
	assert(Path1);

	CP->SelType = CP_SELTYPE_NONE;

	// currently combines rotataion and translation channels
	// in the future we will split these out

	switch (Type)
	{
	case TE_DEL_KEY:
		{
		int RotIndex, PosIndex;
		int RotCount, PosCount;

		RotCount = gePath_GetKeyframeCount(Path0, GE_PATH_ROTATION_CHANNEL);
		PosCount = gePath_GetKeyframeCount(Path0, GE_PATH_TRANSLATION_CHANNEL);

		// Path0
		RotIndex = gePath_GetKeyframeIndex(Path0, GE_PATH_ROTATION_CHANNEL, Time);
		PosIndex = gePath_GetKeyframeIndex(Path0, GE_PATH_TRANSLATION_CHANNEL, Time);

		assert(PosIndex >= 0 && PosIndex < PosCount);
		assert(RotIndex >= 0 && RotIndex < RotCount);

		ret = gePath_DeleteKeyframe(Path0,RotIndex,GE_PATH_ROTATION_CHANNEL); 
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to delete keyframe.", NULL);
			CameraPath_ExitOnError(CP);
			return GE_FALSE;
			}
		ret = gePath_DeleteKeyframe(Path0,PosIndex,GE_PATH_TRANSLATION_CHANNEL); 
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to delete keyframe.", NULL);
			CameraPath_ExitOnError(CP);
			return GE_FALSE;
			}

		// Path1
		PosCount = gePath_GetKeyframeCount(Path1, GE_PATH_TRANSLATION_CHANNEL);

		assert(PosIndex >= 0 && PosIndex < PosCount);

		PosIndex = gePath_GetKeyframeIndex(Path1, GE_PATH_TRANSLATION_CHANNEL, Time);

		ret = gePath_DeleteKeyframe(Path1,PosIndex,GE_PATH_TRANSLATION_CHANNEL); 
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to delete keyframe.", NULL);
			CameraPath_ExitOnError(CP);
			return GE_FALSE;
			}
		break;
		}

	case TE_INS_KEY:
		{
		geXForm3d XFormA, XFormB;

		CP->CP_AppCallback(CAMERA_GET_DATA, -1.0f, &XFormA, &XFormB);

		// pick a safe time
		*TimeChange = Time;
		CameraPath_SafeKeyInsertTime(CP, TimeChange);
		Time = *TimeChange;

		// inserts a keyframe at a specific time.
		ret = gePath_InsertKeyframe(Path0, GE_PATH_ALL_CHANNELS, Time, &XFormA); 
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to insert a keyframe.", NULL);
			CameraPath_ExitOnError(CP);
			return GE_FALSE;
			}
		ret = gePath_InsertKeyframe(Path1, GE_PATH_TRANSLATION_CHANNEL, Time, &XFormB); 
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to insert a keyframe.", NULL);
			CameraPath_ExitOnError(CP);
			return GE_FALSE;
			}

		break;
		}

	case TE_SEL_KEY:
		{
		geXForm3d RotXForm, PosXForm, XForm, XForm2;
		int RotIndex, PosIndex;
		float TrashTime;
		int RotCount, PosCount;

		CP->SelType = CP_SELTYPE_KEY;
		CP->SelTime = Time;

		// Path0
		//
		RotCount = gePath_GetKeyframeCount(Path0, GE_PATH_ROTATION_CHANNEL);
		PosCount = gePath_GetKeyframeCount(Path0, GE_PATH_TRANSLATION_CHANNEL);

		RotIndex = gePath_GetKeyframeIndex(Path0, GE_PATH_ROTATION_CHANNEL, Time);
		PosIndex = gePath_GetKeyframeIndex(Path0, GE_PATH_TRANSLATION_CHANNEL, Time);
		assert(PosIndex >= 0 && PosIndex < PosCount);
		assert(RotIndex >= 0 && RotIndex < RotCount);

		gePath_GetKeyframe(Path0, RotIndex, GE_PATH_ROTATION_CHANNEL, &TrashTime, &RotXForm);
		gePath_GetKeyframe(Path0, PosIndex, GE_PATH_TRANSLATION_CHANNEL, &TrashTime, &PosXForm);

		geXForm3d_Copy(&RotXForm, &XForm); // combine rotation and translation
		XForm.Translation = PosXForm.Translation;

		CP->SelType = CP_SELTYPE_KEY;
		CP->SelTime = Time;
		CP->SelXForm1 = XForm;

		// Path1
		//
		PosCount = gePath_GetKeyframeCount(Path1, GE_PATH_TRANSLATION_CHANNEL);

		PosIndex = gePath_GetKeyframeIndex(Path1, GE_PATH_TRANSLATION_CHANNEL, Time);
		assert(PosIndex >= 0 && PosIndex < PosCount);

		gePath_GetKeyframe(Path1, PosIndex, GE_PATH_TRANSLATION_CHANNEL, &TrashTime, &XForm2);

		// give the data back to the app
		CP->CP_AppCallback(CAMERA_PUT_DATA, -1.0f, &XForm, &XForm2);

		CP->SelXForm2 = XForm2;
		break;
		}

	case TE_MOVE_KEY:
		{
		geXForm3d RotXForm, PosXForm, XForm;
		int RotIndex, PosIndex;
		float TrashTime;
		int RotCount, PosCount;

		// Path0
		//
		RotCount = gePath_GetKeyframeCount(Path0, GE_PATH_ROTATION_CHANNEL);
		PosCount = gePath_GetKeyframeCount(Path0, GE_PATH_TRANSLATION_CHANNEL);

		RotIndex = gePath_GetKeyframeIndex(Path0, GE_PATH_ROTATION_CHANNEL, Time);
		PosIndex = gePath_GetKeyframeIndex(Path0, GE_PATH_TRANSLATION_CHANNEL, Time);
		assert(PosIndex >= 0 && PosIndex < PosCount);
		assert(RotIndex >= 0 && RotIndex < RotCount);

		gePath_GetKeyframe(Path0, RotIndex, GE_PATH_ROTATION_CHANNEL, &TrashTime, &RotXForm);
		gePath_GetKeyframe(Path0, PosIndex, GE_PATH_TRANSLATION_CHANNEL, &TrashTime, &PosXForm);

		geXForm3d_Copy(&RotXForm, &XForm); // combine rotation and translation
		XForm.Translation = PosXForm.Translation;

		ret = gePath_DeleteKeyframe(Path0,RotIndex,GE_PATH_ROTATION_CHANNEL); 
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to delete keyframe.", NULL);
			CameraPath_ExitOnError(CP);
			return GE_FALSE;
			}
		ret = gePath_DeleteKeyframe(Path0,PosIndex,GE_PATH_TRANSLATION_CHANNEL); 
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to delete keyframe.", NULL);
			CameraPath_ExitOnError(CP);
			return GE_FALSE;
			}

		// pick a safe time
		CameraPath_SafeKeyInsertTime(CP, TimeChange);

		ret = gePath_InsertKeyframe(Path0, GE_PATH_ALL_CHANNELS, *TimeChange, &XForm); 
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to insert a keyframe.", NULL);
			CameraPath_ExitOnError(CP);
			return GE_FALSE;
			}

		// give the data back to the app
		CP->CP_AppCallback(CAMERA_PUT_DATA, -1.0f, &XForm, &XForm);

		// Path1
		//
		PosCount = gePath_GetKeyframeCount(Path1, GE_PATH_TRANSLATION_CHANNEL);

		PosIndex = gePath_GetKeyframeIndex(Path1, GE_PATH_TRANSLATION_CHANNEL, Time);
		assert(PosIndex >= 0 && PosIndex < PosCount);

		gePath_GetKeyframe(Path1, PosIndex, GE_PATH_TRANSLATION_CHANNEL, &TrashTime, &PosXForm);

		geXForm3d_Copy(&RotXForm, &XForm); // combine rotation and translation
		XForm.Translation = PosXForm.Translation;

		ret = gePath_DeleteKeyframe(Path1,PosIndex,GE_PATH_TRANSLATION_CHANNEL); 
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to delete keyframe.", NULL);
			CameraPath_ExitOnError(CP);
			return GE_FALSE;
			}

		ret = gePath_InsertKeyframe(Path1, GE_PATH_TRANSLATION_CHANNEL, *TimeChange, &XForm); 
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to insert a keyframe.", NULL);
			CameraPath_ExitOnError(CP);
			return GE_FALSE;
			}

		break;
		}

	case TE_TIME_CHG:
		{
		geXForm3d XFormA, XFormB;
		float Start, End;

		gePath_GetTimeExtents(Path0, &Start, &End);
		if (Time >= Start && Time <= End)
			{
			gePath_Sample(Path0, Time, &XFormA);
			CP->CP_AppCallback(CAMERA_PUT_DATA, Time, &XFormA, NULL);
			}

		gePath_GetTimeExtents(Path1, &Start, &End);
		if (Time >= Start && Time <= End)
			{
			gePath_Sample(Path1, Time, &XFormB);
			CP->CP_AppCallback(CAMERA_PUT_DATA, -1.0f, NULL, &XFormB);
			}

		CP->SelType = CP_SELTYPE_TIMEBAR;
		CP->SelTime = Time;

		CP->CP_AppCallback(CAMERA_PUT_CUT_TIME, Time, NULL, NULL);
		break;
		}

	case TE_SEL_EVENT:
		{
		float ThisTime;
		char *EventString;
		char TempString[512];
		int Count;

		geMotion_SetupEventIterator(CP->Motion[CP->CurrentCut], Time-EVENT_TIMECHECK_RANGE, Time+EVENT_TIMECHECK_RANGE);

		Count = 0;
		while( geMotion_GetNextEvent( CP->Motion[CP->CurrentCut], &ThisTime, &EventString ) )
			{
			Count++;
			strcpy(TempString, EventString);
			}

		assert(Count > 0 && Count <= 1);

		CP->SelType = CP_SELTYPE_EVENT;
		CP->SelTime = Time;
		strcpy(CP->SelString, TempString);
		break;
		}

	case TE_DEL_EVENT:
		ret = geMotion_DeleteEvent(CP->Motion[CP->CurrentCut], Time);
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to delete an event.", NULL);
			CameraPath_ExitOnError(CP);
			return GE_FALSE;
			}
		break;

	case TE_INS_EVENT:

		// real string comes from the data portion of the CameraPath?
		*TimeChange = Time;
		CameraPath_SafeEventInsertTime(CP, TimeChange);
		ret = geMotion_InsertEvent(CP->Motion[CP->CurrentCut], *TimeChange, "");
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to insert an event.", NULL);
			CameraPath_ExitOnError(CP);
			return GE_FALSE;
			}
		break;

	case TE_MOVE_EVENT:
		{
		float ThisTime;
		char *EventString;
		char TempString[512];
		int Count;

		geMotion_SetupEventIterator(CP->Motion[CP->CurrentCut], Time-EVENT_TIMECHECK_RANGE, Time+EVENT_TIMECHECK_RANGE);

		Count = 0;
		while( geMotion_GetNextEvent( CP->Motion[CP->CurrentCut], &ThisTime, &EventString ) )
			{
			Count++;
			strcpy(TempString, EventString);
			}

		assert(Count > 0 && Count <= 1);

		ret = geMotion_DeleteEvent(CP->Motion[CP->CurrentCut], Time);
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to delete an event.", NULL);
			CameraPath_ExitOnError(CP);
			return GE_FALSE;
			}

		CameraPath_SafeEventInsertTime(CP, TimeChange);
		ret = geMotion_InsertEvent(CP->Motion[CP->CurrentCut], *TimeChange, TempString);
		if (ret == GE_FALSE)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to insert an event.", NULL);
			CameraPath_ExitOnError(CP);
			return GE_FALSE;
			}

		//CP->SelType = CP_SELTYPE_EVENT;
		//CP->SelTime = *TimeChange;
		//strcpy(CP->SelString, TempString);
		break;
		}

	default:
		assert(0);
	}

	if (CameraPath_UpdateDialog(CP) == GE_FALSE)
		{
		geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to update dialog.", NULL);
		CameraPath_ExitOnError(CP);
		return FALSE;
		}

	return GE_TRUE;
}

static	BOOL	CALLBACK	DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch	(uMsg)
	{
	case	WM_INITDIALOG:
		{
		SendDlgItemMessage(hwndDlg,IDC_CAMERATITLE,WM_SETTEXT,(WPARAM)0,(LPARAM)CameraPathDlg_SceneName);
		return TRUE;
		}
		break;

	case	WM_COMMAND:
		{
		switch (LOWORD(wParam))
			{
			case IDC_CUTLIST:
				{
				switch (HIWORD(wParam))
					{
					case LBN_ERRSPACE:
					case LBN_DBLCLK:
					case LBN_SELCANCEL:
					case LBN_SETFOCUS:
					case LBN_KILLFOCUS:
						break;
					case LBN_SELCHANGE:
						GlobData->CurrentCut = SendDlgItemMessage(hwndDlg,IDC_CUTLIST,LB_GETCURSEL,(WPARAM)0,(LPARAM)0);
						CameraPath_InitTimeEdit(GlobData, GlobData->TE);
						GlobData->CP_AppCallback(CAMERA_PUT_CUT_INDEX, (float)GlobData->CurrentCut, NULL, NULL);
						break;
					}
				return TRUE;
				}
			case IDC_SETBUTTON:
				{
				int ret;
				ret = CameraPath_UpdateMotion(GlobData);
				if (ret == GE_FALSE)
					{
					geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"CP_CalledByTimeEdit: Unable to update motion.", NULL);
					CameraPath_ExitOnError(GlobData);
					return GE_FALSE;
					}

				return TRUE;
				}

			case IDSAVE:
				PostMessage(GlobData->hwndParent,WM_CAMERA_EDIT_SAVE,0,GE_FALSE);
				return TRUE;

			case IDOK:
				TimeEdit_Destroy(GlobData->TE);
				PostMessage(GlobData->hwndParent,WM_CAMERA_EDIT_QUIT,0,GE_TRUE);
				EndDialog(hwndDlg, 1);
				return TRUE;

			case IDCANCEL:
				TimeEdit_Destroy(GlobData->TE);
				PostMessage(GlobData->hwndParent,WM_CAMERA_EDIT_QUIT,0,GE_TRUE);
				EndDialog(hwndDlg, 0);
				return TRUE;
			}
		break;
		}
	}

	return FALSE;
}

static geBoolean CameraPath_InitTimeEdit(CameraPath *CP, TimeEdit *TE)
{
	float Time;
	int RotCount, i;
	geXForm3d Matrix;
	gePath *Path0, *Path1;
	char *EventString;
	geBoolean ret;

	assert(CP);
	assert(TE);

	TimeEdit_Clear(TE);

	ret = CameraPath_GetPaths(CP, CP->CurrentCut, &Path0, &Path1);
	if (ret == GE_FALSE)
		{
		geErrorLog_AddString(-1,"CameraPath_InitTimeEdit: Cannot get paths.",NULL);
		return GE_FALSE;
		}

	assert(Path0);
	assert(Path1);

	RotCount = gePath_GetKeyframeCount(Path0, GE_PATH_ROTATION_CHANNEL);

	for (i = 0; i < RotCount; i++)
		{
		gePath_GetKeyframe(Path0, i, GE_PATH_ROTATION_CHANNEL, &Time, &Matrix);
		if (TimeEdit_InitAddKey(TE, Time) == GE_FALSE)
			{
			geErrorLog_AddString(-1,"CameraPath_InitTimeEdit: Cannot add key.",NULL);
			return GE_FALSE;
			}
		}

	// events
	geMotion_SetupEventIterator(CP->Motion[CP->CurrentCut], 0, 10000.0f);
	while( geMotion_GetNextEvent( CP->Motion[CP->CurrentCut], &Time, &EventString ) )
		{
		if (TimeEdit_InitAddEvent(TE, Time) == GE_FALSE)
			{
			geErrorLog_AddString(-1,"CameraPath_InitTimeEdit: Cannot add key.",NULL);
			return GE_FALSE;
			}
		}


	return GE_TRUE;
}

CameraPath *CameraPath_Create(CP_CB AppCallback, geMotion **Motion)
{
	CameraPath *CP;
	int i;

	assert(AppCallback);
	assert(Motion && *Motion);

	CP = GE_RAM_ALLOCATE_STRUCT(CameraPath);

	if (CP == NULL)
		{
		geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"CameraPath_Create:  Couldn't create CameraPath object",NULL);
		goto Exit;
		}

	CP->Motion = Motion;
	CP->CP_AppCallback = AppCallback;
	CP->CurrentTime = 0.0f;
	CP->CurrentCut = 0;

	for (i = 0; i < MAX_CAMERA_CUTS; i++)
		{
		gePath *Path0, *Path1;
		if (CameraPath_GetPaths(CP, i, &Path0, &Path1) == GE_FALSE)
			{
			geErrorLog_AddString(-1,"CameraPath_Create:  Couldn't get paths.",NULL);
			goto Exit;
			}
		}

	CP->CurrentCut = 0;

	return((CameraPath *)CP);

	Exit:

	if (CP != NULL)
		{
		geRam_Free(CP);
		}

	return NULL;

}

void CameraPath_Destroy(CameraPath **CP)
{
	assert(CP && *CP);

	if (*CP)
		{
		geRam_Free(*CP);
		}

	*CP = NULL;
}

geBoolean CameraPathDlg_Show(HANDLE hInstance, HWND hwndParent, CameraPath *CP, const char *SceneName)
{
	TimeEdit *TE;
	int i;
	
	assert( hInstance );
	assert( CP != NULL );
	assert( SceneName != NULL );
	
	//InitCommonControls ();
	CameraPathDlg_SceneName = SceneName;
		
	CP->hwndParent = hwndParent;
	CP->hwnd = CreateDialog(hInstance,
					MAKEINTRESOURCE(IDD_CAMERADIALOG),
					hwndParent,
					DlgProc);
	ShowWindow(CP->hwnd,SW_SHOW);
	
	CP->hwndTimeEdit = TimeEdit_CreateWindow(hInstance, hwndParent, CP_CalledByTimeEdit, CP, "Camera Path",600,200);
	TE = (TimeEdit *)GetWindowLong(CP->hwndTimeEdit,0);
	GlobData = CP;
	CP->TE = TE;

	if (CameraPath_InitTimeEdit(CP, TE) == GE_FALSE)
		{
		geErrorLog_AddString(-1,"CameraPath_InitTimeEdit failed.",NULL);
		return GE_FALSE;
		}

	for (i = MAX_CAMERA_CUTS-1; i >= 0; i--)
		{
		char String[64];
		sprintf(String,"Cut %d",i+1);
		SendDlgItemMessage(CP->hwnd,IDC_CUTLIST,LB_INSERTSTRING,(WPARAM)0,(LPARAM)String);
		}

	// select the first cut
	SendDlgItemMessage(CP->hwnd,IDC_CUTLIST,LB_SETCURSEL,(WPARAM)0,(LPARAM)0);

	return GE_TRUE;
}



