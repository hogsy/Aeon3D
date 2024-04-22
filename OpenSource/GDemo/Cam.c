/****************************************************************************************/
/*  Cam.c                                                                               */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Camera manipulation routines        				                */
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

#include <windows.h>
#include <assert.h>
#include <math.h>
#include "genesis.h"
#include "errorlog.h"
#include "ram.h"
#include "cam.h"
#include "trigger.h"
#include "camerapathdlg.h"

#define MAX_CUTS MAX_CAMERA_CUTS

typedef struct Cam
{
	geMotion*		Motion[MAX_CUTS];
	int				Index;
	float			MotionFrameTime;
	Object			*Object;
}Cam;

Cam *Cam_Create(char *LevelName, Object *Object)
{
	Cam *Cam;

	assert(LevelName);
	assert(Object);

	Cam = geRam_Allocate(sizeof(struct Cam));

	if (Cam == NULL)
		{
		geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"Cam_Create:  Couldn't allocate Cam struct",NULL);
		goto Exit;
		}

	Cam->Object = Object;

	if (Cam_CreateMotion(Cam, LevelName) == GE_FALSE)
	{
		geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"Cam_Create:  Couldn't get motions",NULL);
		goto Exit;
	}

	return Cam;

Exit:

	if (Cam)
	{
		geRam_Free(Cam);
	}

	return NULL;
}

geBoolean Cam_CreateMotion(Cam *Cam, char *LevelName)
{
	geVFile *CamFile;
	geMotion *Motion = NULL;
	char *ptr;
	char FileName[_MAX_PATH];
	int i;

	assert(Cam);
	assert(LevelName);

	strcpy(FileName, LevelName);
	strlwr(FileName);
	ptr = strstr(FileName, ".bsp");
	memcpy(ptr,".mot",strlen(".mot"));
	
	memset(Cam->Motion, 0, sizeof(Cam->Motion));

	CamFile = geVFile_OpenNewSystem (NULL, GE_VFILE_TYPE_DOS, FileName, NULL, GE_VFILE_OPEN_READONLY);

	for (i = 0; i < MAX_CUTS; i++)
		{
		if (CamFile)
			Cam->Motion[i] = geMotion_CreateFromFile(CamFile);

		if (Cam->Motion[i] == NULL)
			{
			// I left this here so the old motion files can be read in
			Cam->Motion[i] = geMotion_Create(GE_TRUE);
			
			if (Cam->Motion[i] == NULL)
				{
				geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"GetMotion: Failed to allocate memory for motion.",FileName);
				goto ExitErr;
				}
			}
		}

	if (CamFile)
		geVFile_Close (CamFile);

	return(GE_TRUE);

	ExitErr:

	if (CamFile)
		geVFile_Close (CamFile);

	for (i = 0; i < MAX_CUTS; i++)
		{
		if (Cam->Motion[i])
			{
			geMotion_Destroy(&Cam->Motion[i]);
			}
		}

	return(GE_FALSE);
}

void Cam_SetIndex(Cam *Cam, int Index)
{
	assert(Cam != NULL);
	assert(Index >= 0 && Index < MAX_CUTS);

	Cam->Index = Index;
}

void Cam_SetFrameTime(Cam *Cam, float FrameTime)
{
	assert(Cam != NULL);
	Cam->MotionFrameTime = FrameTime;
}

geMotion** Cam_GetMotion(Cam* Cam)
{
	return (&Cam->Motion[0]);
}

void Cam_Reset(Cam* Cam)
{
	Cam_SetIndex(Cam, 0);
	Cam_SetFrameTime(Cam, 0.0f);
}

geBoolean Cam_SaveMotion(Cam *Cam, char *LevelName)
{
	geVFile *CamFile = NULL;
	char *ptr;
	char FileName[_MAX_PATH];
	int i;
	int ret;

	assert(Cam);
	assert(LevelName);

	if (Cam->Motion[0] == NULL)
		{
		return GE_TRUE;
		}

	strcpy(FileName, LevelName);
	strlwr(FileName);
	ptr = strstr(FileName, ".bsp");
	memcpy(ptr,".mot",strlen(".mot"));

	CamFile = geVFile_OpenNewSystem (NULL, GE_VFILE_TYPE_DOS, FileName, NULL, GE_VFILE_OPEN_CREATE);
	if (CamFile == NULL)
		{
		geErrorLog_AddString(-1,"SaveMotion: VFile open for create failed",NULL);
		goto ExitErr;
		}

	for (i = 0; i < MAX_CUTS; i++)
		{
		assert(Cam->Motion[i]);
		ret = geMotion_WriteToBinaryFile(Cam->Motion[i], CamFile);
		if (ret == GE_FALSE)
			{
			char ds[256];
			sprintf(ds,"Motion # %d = %x",i, Cam->Motion[i]);
			geErrorLog_AddString(-1,"SaveMotion: geMotion_WriteToBinaryFile failed",ds);
			goto ExitErr;
			}
		}

	geVFile_Close (CamFile);

	return GE_TRUE;

	ExitErr:

	if (CamFile)
		geVFile_Close (CamFile);

	return(GE_FALSE);
}

geBoolean Cam_SaveMotionDebug(Cam *Cam, char *LevelName)
{
	geVFile *CamFile = NULL;
	char *ptr;
	char Temp[_MAX_PATH];
	char FileName[_MAX_PATH];
	int i;
	int ret;
	 int Count = 0;

	assert(Cam);
	assert(LevelName);

	if (Cam->Motion[0] == NULL)
		{
		return GE_TRUE;
		}

	strcpy(Temp, LevelName);
	strlwr(Temp);
	ptr = strstr(Temp, ".bsp");
	ptr[0] = '\0';
	sprintf(FileName,"%s%02d%s",Temp, Count++, ".mot");

	CamFile = geVFile_OpenNewSystem (NULL, GE_VFILE_TYPE_DOS, FileName, NULL, GE_VFILE_OPEN_CREATE);
	if (CamFile == NULL)
		{
		geErrorLog_AddString(-1,"SaveMotion: VFile open for create failed",NULL);
		goto ExitErr;
		}

	for (i = 0; i < MAX_CUTS; i++)
		{
		assert(Cam->Motion[i]);
		ret = geMotion_WriteToBinaryFile(Cam->Motion[i], CamFile);
		if (ret == GE_FALSE)
			{
			char ds[_MAX_PATH];
			sprintf(ds,"Motion # %d = %x",i, Cam->Motion[i]);
			geErrorLog_AddString(-1,"SaveMotion: geMotion_WriteToBinaryFile failed",ds);
			goto ExitErr;
			}
		}

	geVFile_Close (CamFile);

	return GE_TRUE;

	ExitErr:

	if (CamFile)
		geVFile_Close (CamFile);

	return(GE_FALSE);
}

void Cam_Destroy(Cam **Cam)
{
	int i;

	assert(Cam);

	if (*Cam == NULL)
		return;

	for (i = 0; i < MAX_CUTS; i++)
		{
		if ((*Cam)->Motion[i] != NULL)
			geMotion_Destroy(&(*Cam)->Motion[i]);
		}

	geRam_Free(*Cam);

	*Cam = NULL;
}


geBoolean Cam_Update(geWorld *World, Cam *Cam, geXForm3d *XForm, geVec3d *Channels, float ElapsedTime)
{
	assert(World != NULL);
	assert(Cam != NULL);
	assert(XForm != NULL);
	assert(Channels != NULL);

	if (Cam->Motion[Cam->Index] != NULL)
	{
		gePath *Path;

		float Start, End;
		float MotionTimeStart, MotionTimeEnd;
		float SaveTime;

		SaveTime = Cam->MotionFrameTime;
		Cam->MotionFrameTime += ElapsedTime;

		if ((Path = geMotion_GetPath(Cam->Motion[Cam->Index], 0)) == NULL)
		{
			return GE_FALSE;
			//CameraPathPlayback = GE_FALSE;
		}

		gePath_GetTimeExtents(Path, &Start, &End);

		MotionTimeStart = Start + (float)fmod(SaveTime, End);
		MotionTimeEnd = Start + (float)fmod(Cam->MotionFrameTime, End);

		if (MotionTimeEnd < MotionTimeStart)
		{
			Trigger_ProcessEvents(World, TRIGGER_FROM_XFORM, XForm, Cam->Motion[Cam->Index], MotionTimeStart, End);
		}
		else
		{
			Trigger_ProcessEvents(World, TRIGGER_FROM_XFORM, XForm, Cam->Motion[Cam->Index], MotionTimeStart, MotionTimeEnd);
		}

		if (MotionTimeEnd < MotionTimeStart)
			{
			geBoolean HaveKeys;

			// time wrapped
			SaveTime = 0;
			Cam->MotionFrameTime = MotionTimeEnd - Start;

			Cam->Index++;
			Path = geMotion_GetPath(Cam->Motion[Cam->Index], 0);
			assert(Path);

			// see if this path has any key frames
			HaveKeys = gePath_GetKeyframeCount(Path, GE_PATH_ROTATION_CHANNEL) | 
					  gePath_GetKeyframeCount(Path, GE_PATH_TRANSLATION_CHANNEL);

			if (!HaveKeys)
				{
				Cam->Index = 0;
				return GE_FALSE;
				}

			// found a path with keys
			gePath_GetTimeExtents(Path, &Start, &End);

			MotionTimeStart = Start + (float)fmod(SaveTime, End);
			MotionTimeEnd = Start + (float)fmod(Cam->MotionFrameTime, End);

			Trigger_ProcessEvents(World, TRIGGER_FROM_OBJECT, Cam->Object, Cam->Motion[Cam->Index], MotionTimeStart, MotionTimeEnd);
			}

		gePath_Sample(Path, MotionTimeEnd, XForm);

		// grab the other channels from path 1 - field of view
		{
		geXForm3d XForm;
		Path = geMotion_GetPath(Cam->Motion[Cam->Index], 1);
		assert(Path != NULL);
		gePath_Sample(Path, MotionTimeEnd, &XForm);
		Channels->X = XForm.Translation.X;
		}

		// update the objects transform
		ObjMgr_SetObjectXForm(Cam->Object, XForm);
	}

	return GE_TRUE;
}


