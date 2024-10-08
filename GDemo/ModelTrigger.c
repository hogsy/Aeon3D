/****************************************************************************************/
/*  ModelTrigger.c                                                                      */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Similar to Door.c - but can only be activated by triggers           */
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
#include <stdlib.h>
#include <assert.h>

#include "modeltrigger.h"
#include "errorlog.h"
#include "objmgr.h"
#include "trigger.h"
#include "genvs.h"

static geBoolean ModelTrigger_Control(geWorld *World, Object *Object, float Time);
static geBoolean Plat_Control(geWorld *World, Object *Object, float Time);
static geBoolean ModelTrigger_Trigger(geWorld *World, Object *Object, void *TargetData, void* data);

//=====================================================================================
//	ModelTrigger_Spawn
//=====================================================================================
geBoolean ModelTrigger_Spawn(geWorld *World, Object *Obj, void *ClassData)
{
	geWorld_Model	*Model;
	geMotion		*Motion;
	gePath			*Path;
	ModelTrigger			*FuncModelTrigger;
	geXForm3d		XForm;
	int32			Flags;

	assert(World != NULL);
	assert(Obj != NULL);

	ObjMgr_SetObjectControlFunction(Obj, ModelTrigger_Control);
	ObjMgr_SetObjectTriggerFunction(Obj, ModelTrigger_Trigger);

	// Setup the ModelTriggers local user data (not passed accross network)
	// Grab the model
	FuncModelTrigger = (ModelTrigger*)ClassData;
	geXForm3d_SetIdentity(&XForm);
	ObjMgr_SetObjectXForm(Obj, &XForm);

	Model = FuncModelTrigger->Model;

	if (!Model)
	{
		geErrorLog_AddString(-1, "ModelTrigger_Spawn:  No model on object.", NULL);
		return GE_FALSE;
	}

	// Set the default xform to time 0 of the animation
	Motion = geWorld_ModelGetMotion(Model);

	if (!Motion)
	{
		geErrorLog_AddString(-1, "ModelTrigger_Spawn:  No motion for model.", NULL);
		return GE_FALSE;
	}

	Path = geMotion_GetPath(Motion, 0);

	if (!Path)
	{
		geErrorLog_AddString(-1, "ModelTrigger_Spawn:  No path for model motion.", NULL);
		return GE_FALSE;
	}

	// Put model at default position
	gePath_Sample(Path, 0.0f, &XForm);
	ObjMgr_SetObjectXForm(Obj, &XForm);

	Flags = ObjMgr_GetObjectFlags(Obj);
	ObjMgr_SetObjectFlags(Obj, Flags | TYPE_MODEL );


	// Register the model with the player if all is ok
	// NOTE - If somthing went bad, then this should not be called!!!

	geWorld_ModelSetUserData(Model, (void*)Obj);
	ObjMgr_SetObjectModel(Obj, Model);
	
	return GE_TRUE;
}

//=====================================================================================
//	ModelTrigger_Trigger
//=====================================================================================
static geBoolean ModelTrigger_Trigger(geWorld *World, Object *Obj, void *TargetData, void* data)
{
	int32 Flags;

	assert(World != NULL);
	assert(Obj != NULL);

	if (ObjMgr_GetObjectState(Obj) == STATE_Opened)
		return GE_TRUE;

	ObjMgr_SetObjectState(Obj, STATE_Opened);
	ObjMgr_SetObjectFrameTime(Obj, 0.0f);

	Flags = ObjMgr_GetObjectFlags(Obj);
	ObjMgr_SetObjectFlags(Obj, Flags | MODEL_OPEN);

	{
	geWorld_Model *Model;
	Model = ObjMgr_GetObjectModel(Obj);

	if (Model == NULL)
	{
		geErrorLog_AddString(-1, "ModelTrigger_Trigger:  Object has no model.", NULL);
		return GE_FALSE;
	}

	geWorld_OpenModel(World, Model, GE_TRUE);
	}

	return GE_TRUE;
	TargetData,data;
}


//=====================================================================================
//	ModelTrigger_Control
//=====================================================================================
static geBoolean ModelTrigger_Control(geWorld *World, Object *Obj, float Time)
{
	geWorld_Model	*Model;
	geMotion		*Motion;
	float			StartTime, EndTime, NewTime;
	geXForm3d		DestXForm;
	gePath			*Path;
	int32			Flags;
	float			StartFrameTime, EndFrameTime;
	geVec3d			OrigPos;

	assert(World != NULL);
	assert(Obj != NULL);

	if (ObjMgr_GetObjectState(Obj) != STATE_Opened)
		return GE_TRUE;

	Model = ObjMgr_GetObjectModel(Obj);

	if (Model == NULL)
	{
		geErrorLog_AddString(-1, "ModelTrigger_Control:  Object has no model.", NULL);
		return GE_FALSE;
	}

	Motion = geWorld_ModelGetMotion(Model);

	if (Motion == NULL)
	{
		geErrorLog_AddString(-1, "ModelTrigger_Control:  Model has no motion.", NULL);
		return GE_FALSE;
	}

	StartFrameTime = ObjMgr_GetObjectFrameTime(Obj);
	NewTime = EndFrameTime = StartFrameTime + Time;

	Path = geMotion_GetPath(Motion, 0);
	if (Path == NULL)
	{
		geErrorLog_AddString(-1, "ModelTrigger_Control:  Motion has no path.", NULL);
		return GE_FALSE;
	}

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

	geWorld_GetModelRotationalCenter(World, Model, &OrigPos);

	// Get the xform for the current time
	gePath_Sample(Path, NewTime, &DestXForm);

	{
	geXForm3d XF;

	geWorld_GetModelXForm(World, Model, &XF);
	XF.Translation = OrigPos;
	Trigger_ProcessEvents(World, TRIGGER_FROM_MODEL, &XF, Motion, StartFrameTime, EndFrameTime);
	}

	{
		geVec3d Mins, Maxs, Pos,Pos2;
		GE_Collision Collide;
		geXForm3d XForm;

		ObjMgr_GetObjectXForm(Obj, &XForm);

		Pos = OrigPos;
		//Pos2 = geVec3d_Add(&Pos, &DestXForm.Translation);
		geVec3d_Add(&Pos, &DestXForm.Translation, &Pos2);
		//geWorld_GetModelRotationalCenter(World, Model, &Pos2);
		geWorld_ModelGetBBox(World, Model, &Mins, &Maxs);

		// all of this code is special cased to get the physics bridge to work
		Mins.X /= 3;
		Mins.Y /= 3;
		Mins.Z /= 3;
		Maxs.X /= 3;
		Maxs.Y /= 3;
		Maxs.Z /= 3;

		if	(!geWorld_Collision (World, &Mins, &Maxs, &Pos, &Pos2, GE_CONTENTS_SOLID_CLIP, GE_COLLIDE_ALL, 0xffffffff, NULL, NULL, &Collide) == GE_FALSE)
		{
			if (Collide.Model)
			{
				Object		*Target;
				TriggerP	Trigger;
				geVec3d		Velocity;

				Target = (Object*)geWorld_ModelGetUserData(Collide.Model);
				if (Target && (ObjMgr_GetObjectFlags(Target) & TYPE_PHYSOB))
				{
					Velocity.X = 0;
					Velocity.Y = 3000;
					Velocity.Z = 0;
					ObjMgr_SetObjectVelocity(Obj, &Velocity);

					NewTime = 0.0f;
					gePath_Sample(Path, 0.0f, &DestXForm);
					ObjMgr_SetObjectState(Obj, STATE_Closed);
					Flags = ObjMgr_GetObjectFlags(Obj);
					Flags &= ~MODEL_OPEN;
					ObjMgr_SetObjectFlags(Obj, Flags);
					geWorld_OpenModel(World, Model, GE_FALSE);

					Trigger = ObjMgr_GetObjectTriggerFunction(Target);
					if (Trigger)
						Trigger(World, Target, Obj, (void*)&Collide);
				}
			}
		}
	}

	geWorld_SetModelXForm(World, ObjMgr_GetObjectModel(Obj), &DestXForm);
	ObjMgr_SetObjectXForm(Obj, &DestXForm);
	ObjMgr_SetObjectFrameTime(Obj, NewTime);


	return GE_TRUE;
}

