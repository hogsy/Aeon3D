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
#include <stdio.h>
#include <assert.h>
#include "errorlog.h"
#include "ctypes.h"
#include "genesis.h"
#include "ActorStart.h"

geMotion *PathPoint_GetMotion(geWorld *World, void *FirstPoint, geBoolean *Loop)
{
	PathPoint			*ThisPoint;
	geXForm3d XForm;
	geVec3d	Angles;
	float				Time;
	int					Index;
	geBoolean			Looped, ret;
	int Count;

	gePath				*Path = NULL;
	geMotion			*Motion = NULL;

	assert(World != NULL);
	assert(FirstPoint != NULL);
	assert(Loop != NULL);

	// First look at the list to see if it's looping
	Count = 0;
	ThisPoint = FirstPoint;
	do
	{
	ThisPoint = ThisPoint->Next;
	if (Count++ > 1000)
		{
		geErrorLog_AddString(-1,"PathPoint_GetMotion: Infinite loop traversing path.",NULL);
		goto ExitErr;
		}
	}while(ThisPoint && ThisPoint != FirstPoint);

	if (ThisPoint == FirstPoint)
		*Loop = GE_TRUE;
	else
		*Loop = GE_FALSE;

	// create a path here
	Path = gePath_Create(GE_PATH_INTERPOLATE_HERMITE, GE_PATH_INTERPOLATE_SLERP, *Loop);

	if (Path == NULL)
	{
		geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"PathPoint_GetMotion: Unable to create path.",NULL);
		goto ExitErr;
	}

	ThisPoint = FirstPoint;
	Time = 0.0f;
	Looped = GE_FALSE;

	//move through points setting up the path
	while (1)
	{
		Angles = ThisPoint->angles;

		Angles.X *= (M_PI/180);
		Angles.Y -= 90.0f;
		Angles.Y *= (M_PI/180);
		Angles.Z *= (M_PI/180);
		geXForm3d_SetEulerAngles(&XForm, &Angles);
		XForm.Translation = ThisPoint->origin;

		// add current point to the path
		ret = gePath_InsertKeyframe(Path, GE_PATH_TRANSLATION_CHANNEL|GE_PATH_ROTATION_CHANNEL, 
			Time, &XForm);

		if (ret == GE_FALSE)
		{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"PathPoint_GetMotion: Unable to insert keyframe.",NULL);
			goto ExitErr;
		}
		
		if (Looped)
		{
			break;
		}

		Time += ThisPoint->Time;

		// next point
		ThisPoint = ThisPoint->Next;

		if (!ThisPoint)
			break;

		if (ThisPoint == FirstPoint)
		{
			Looped = TRUE;
		}
	}

	// create motion and add events to it
	//

	Motion = geMotion_Create(GE_TRUE);

	if (Motion == NULL)
	{
		geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"PathPoint_GetMotion: Unable to create motion.",NULL);
		goto ExitErr;
	}

	ret = geMotion_AddPath(Motion, Path, "path", &Index);
	if (ret == GE_FALSE)
	{
		geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"PathPoint_GetMotion: Unable to add path to motion.",NULL);
		goto ExitErr;
	}

	ThisPoint = FirstPoint;
	Time = 0.0f;
	do
	{
		ret = geMotion_InsertEvent(Motion, Time, ThisPoint->Event);
		if (ret == GE_FALSE)
		{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"PathPoint_GetMotion: Unable to add event to motion.",NULL);
			goto ExitErr;
		}
		Time += ThisPoint->Time;
		ThisPoint = ThisPoint->Next;
	}while(ThisPoint && ThisPoint != FirstPoint);

	gePath_Destroy(&Path);

	return (Motion);

ExitErr:

	if (Path != NULL)
	{
		gePath_Destroy(&Path);
	}

	if (Motion != NULL)
	{
		geMotion_Destroy(&Motion);
	}

	return(NULL);
}
