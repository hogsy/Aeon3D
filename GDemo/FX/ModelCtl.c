/****************************************************************************************/
/*  ModelCtl.c                                                                          */
/*                                                                                      */
/*  Description:    Model control module - converted from GTEST  		                */
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
#include	<math.h>
#include	<assert.h>

#define	BUILD_MODELCTL
#include	"modelctl.h"
#include	"trigger.h"
#include	"ErrorLog.h"

geBoolean ModelCtl_Init(void)
{
	return GE_TRUE;
}

geBoolean ModelCtl_Reset(geWorld *World)
{
	geEntity_EntitySet *	Set;
	geEntity *				Entity;

	assert(World);

	Set = geWorld_GetEntitySet(World, "ModelController");
	if	(Set == NULL)
	{
		// not an error - there may be no ModelControllers in the level
		return GE_TRUE;
	}

	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	while	(Entity)
	{
		ModelController *		Controller;

		Controller = geEntity_GetUserData(Entity);
		Controller->LastTime = 0.0f;

		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}

	return GE_TRUE;
}

geBoolean ModelCtl_Shutdown(void)
{
	return GE_TRUE;
}

geBoolean ModelCtl_Frame(geWorld *World, const geXForm3d *XForm, geFloat DeltaTime)
{
	geEntity_EntitySet *	Set;
	geEntity *				Entity;

	assert(World);
	assert(XForm);

	Set = geWorld_GetEntitySet(World, "ModelController");
	if	(Set == NULL)
	{
		// not an error - there may be no ModelControllers in the level
		return GE_TRUE;
	}

	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	while	(Entity)
	{
		ModelController *	Controller;
		geMotion *			Motion;
		gePath *			Path;
		geFloat				StartTime;
		geFloat				EndTime;
		geFloat				PrevTime;
		geXForm3d			XForm;

		Controller = geEntity_GetUserData(Entity);
		if (Controller == NULL)
		{
			geErrorLog_AddString(-1,"ModelCtl_Frame: Entity has no user data.",NULL);
		}
		Motion = geWorld_ModelGetMotion(Controller->Model);
		if (Motion == NULL)
		{
			geErrorLog_AddString(-1,"ModelCtl_Frame: Model has no motion.",NULL);
		}
		Path = geMotion_GetPath(Motion, 0);
		if (Path == NULL)
		{
			geErrorLog_AddString(-1,"ModelCtl_Frame: Motion has no path.",NULL);
		}
		gePath_GetTimeExtents(Path, &StartTime, &EndTime);
		gePath_Sample(Path, StartTime + Controller->LastTime, &XForm);

		geWorld_SetModelXForm(World, Controller->Model, &XForm);

		PrevTime = Controller->LastTime;
		Controller->LastTime = (geFloat)fmod(PrevTime + DeltaTime, EndTime - StartTime);

		if (Controller->LastTime < PrevTime)
			{
			//Motion looped
			// end segment
			Trigger_ProcessEvents(World, TRIGGER_FROM_MODEL, &XForm, Motion, PrevTime, EndTime);
			// start segment
			Trigger_ProcessEvents(World, TRIGGER_FROM_MODEL, &XForm, Motion, StartTime, Controller->LastTime);
			}
		else
			{
			Trigger_ProcessEvents(World, TRIGGER_FROM_MODEL, &XForm, Motion, PrevTime, Controller->LastTime);
			}	

		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}

	return GE_TRUE;
}
