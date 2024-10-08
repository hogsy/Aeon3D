/****************************************************************************************/
/*  Door.c                                                                              */
/*                                                                                      */
/*  Description:    Door control routines               				                */
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
#include <assert.h>

#include "door.h"
#include "errorlog.h"
#include "objmgr.h"
#include "trigger.h"
#include "genvs.h"

static geBoolean Door_Control(geWorld *World, Object *Object, float Time);
static geBoolean Plat_Control(geWorld *World, Object *Object, float Time);
static geBoolean Door_Trigger(geWorld *World, Object *Object, void *TargetData, void* data);
geBoolean Door_Spawn(geWorld *World, Object *Obj, void *ClassData);

//=====================================================================================
//	Door_Spawn
//=====================================================================================
geBoolean Door_Spawn(geWorld *World, Object *Obj, void *ClassData)
{
	geWorld_Model	*Model;
	geMotion		*Motion;
	gePath			*Path;
	Door			*FuncDoor;
	geXForm3d		XForm;
	int32			Flags;

	assert(World != NULL);
	assert(Obj != NULL);

	ObjMgr_SetObjectControlFunction(Obj, Door_Control);
	ObjMgr_SetObjectTriggerFunction(Obj, Door_Trigger);
	ObjMgr_SetObjectClassData(Obj, ClassData);

	// Setup the doors local user data (not passed accross network)
	// Grab the model
	FuncDoor = (Door*)ClassData;
	geXForm3d_SetIdentity(&XForm);
	ObjMgr_SetObjectXForm(Obj, &XForm);

	Model = FuncDoor->Model;

	if (Model == NULL)
	{
		geErrorLog_AddString(-1, "Door_Spawn: entity has no model.", NULL);
		return GE_FALSE;
	}

	// Set the default xform to time 0 of the animation
	Motion = geWorld_ModelGetMotion(Model);
	if (Motion == NULL)
	{
		geErrorLog_AddString(-1, "Door_Spawn: failed to get model motion.", NULL);
		return GE_FALSE;
	}

	Path = geMotion_GetPath(Motion, 0);
	assert(Path);

	// Put model at default position
	gePath_Sample(Path, 0.0f, &XForm);
	ObjMgr_SetObjectXForm(Obj, &XForm);

	Flags = ObjMgr_GetObjectFlags(Obj);
	ObjMgr_SetObjectFlags(Obj, Flags | TYPE_MODEL | TYPE_TOUCH);

	geWorld_ModelSetUserData(Model, (void*)Obj);
	ObjMgr_SetObjectModel(Obj, Model);
	
	return GE_TRUE;
}

//=====================================================================================
//	Door_Trigger
//=====================================================================================
static geBoolean Door_Trigger(geWorld *World, Object *Obj, void *TargetData, void* data)
{
	int32 Flags;
	Object *TObj;

	assert(World != NULL);
	assert(Obj != NULL);
	assert(TargetData != NULL);

	TObj = TargetData;

	{
	geWorld_Model *Model;
	geVec3d DoorPos;
	Door		*DoorData;
	geXForm3d	XForm;
	float		dist;

	DoorData = ObjMgr_GetObjectClassData(Obj);
	assert(DoorData);
	ObjMgr_GetObjectXForm(TObj, &XForm);
	Model = ObjMgr_GetObjectModel(Obj);
	assert(Model);
	geWorld_GetModelRotationalCenter(World, Model, &DoorPos);
	dist = geVec3d_DistanceBetween(&DoorPos, &XForm.Translation);
	if (dist > DoorData->Radius)
		return GE_TRUE;
	}

	if (ObjMgr_GetObjectState(Obj) == STATE_Opened)
		return GE_TRUE;

	ObjMgr_SetObjectState(Obj, STATE_Opened);
	ObjMgr_SetObjectFrameTime(Obj, 0.0f);

	Flags = ObjMgr_GetObjectFlags(Obj);
	ObjMgr_SetObjectFlags(Obj, Flags | MODEL_OPEN);

	{
	geWorld_Model *Model;
	Model = ObjMgr_GetObjectModel(Obj);
	assert(Model != NULL);
	geWorld_OpenModel(World, Model, GE_TRUE);
	}

	return GE_TRUE;
	TargetData,data;
}


//=====================================================================================
//	Door_Control
//=====================================================================================
static geBoolean Door_Control(geWorld *World, Object *Obj, float Time)
{
	geWorld_Model	*Model;
	geMotion		*Motion;
	float			StartTime, EndTime, NewTime;
	geXForm3d		DestXForm;
	gePath			*Path;
	int32			Flags;
	float			StartFrameTime, EndFrameTime;

	assert(World != NULL);
	assert(Obj != NULL);

	if (ObjMgr_GetObjectState(Obj) != STATE_Opened)
		return GE_TRUE;

	Model = ObjMgr_GetObjectModel(Obj);

	assert(Model);

	Motion = geWorld_ModelGetMotion(Model);
	assert(Motion);

	StartFrameTime = ObjMgr_GetObjectFrameTime(Obj);
	NewTime = EndFrameTime = StartFrameTime + Time;

	Path = geMotion_GetPath(Motion, 0);
	assert(Path);

	geMotion_GetTimeExtents(Motion, &StartTime , &EndTime);

	if (NewTime >= EndTime)		// Played through, done...
	{
		NewTime = StartTime;
		ObjMgr_SetObjectState(Obj, STATE_Closed);
		Flags = ObjMgr_GetObjectFlags(Obj);
		Flags &= ~MODEL_OPEN;
		ObjMgr_SetObjectFlags(Obj, Flags);

		geWorld_OpenModel(World, Model, GE_FALSE);
	}

	// Get the xform for the current time
	gePath_Sample(Path, NewTime, &DestXForm);

	Trigger_ProcessEvents(World, TRIGGER_FROM_MODEL, &DestXForm, Motion, StartFrameTime, EndFrameTime);

	geWorld_SetModelXForm(World, ObjMgr_GetObjectModel(Obj), &DestXForm);
	ObjMgr_SetObjectXForm(Obj, &DestXForm);
	ObjMgr_SetObjectFrameTime(Obj, NewTime);

	return GE_TRUE;
}

