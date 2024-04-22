/****************************************************************************************/
/*  DynLight.c                                                                          */
/*                                                                                      */
/*  Description:    Dynamic light module - converted from GTEST  		                */
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
#include	<string.h>
#include	<assert.h>

#define	BUILD_DYNLIGHT
#include	"Dynlight.h"
#include	"ErrorLog.h"

static geBoolean DynLight_SetWorld(geWorld *World, geVFile *Context);

geBoolean DynLight_Init(geEngine *Engine, geWorld *World, geVFile *Context)
{
	assert(World);

	if (!DynLight_SetWorld(World, Context))
	{
		geErrorLog_AddString(-1,"DynLight_Init: DynLight_SetWorld failed.\n",NULL);
		return GE_FALSE;
	}

	return GE_TRUE;
	Engine;
}

geBoolean DynLight_Reset(geWorld *World)
{
	geEntity_EntitySet *	Set;
	geEntity *				Entity;

	assert(World);
 
	Set = geWorld_GetEntitySet(World, "DynamicLight");
	if	(Set == NULL)		 
		return GE_TRUE;

	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	while	(Entity)
	{
		DynamicLight *		Light;

		Light = geEntity_GetUserData(Entity);
		Light->LastTime = 0.0f;

		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}

	return GE_TRUE;
}

geBoolean DynLight_Shutdown(void)
{
	return GE_TRUE;
}

geBoolean DynLight_Frame(geWorld *World, const geXForm3d *XForm, geFloat DeltaTime)
{
	geEntity_EntitySet *	Set;
	geEntity *				Entity;

	assert(World);
	assert(XForm);

	Set = geWorld_GetEntitySet(World, "DynamicLight");
	if	(Set == NULL)
		return GE_TRUE;

	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	while	(Entity)
	{
		DynamicLight *	Light;
		geFloat			Radius;
		geFloat			Percentage;
		int				Index;
		geVec3d			Pos;

		// process this entity only if a dynamic light was created for it
		Light = geEntity_GetUserData(Entity);
		if ( Light->DynLight == NULL )
		{
			Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
			continue;
		}

		if	(Light->Model)
		{
			geXForm3d	XForm;

			geWorld_GetModelXForm(World, Light->Model, &XForm);

			Pos = Light->origin;
			if	(Light->AllowRotation)
			{
				geVec3d	Center;

				geWorld_GetModelRotationalCenter(World, Light->Model, &Center);
				geVec3d_Subtract(&Pos, &Center, &Pos);
				geXForm3d_Transform(&XForm, &Pos, &Pos);
				geVec3d_Add(&Pos, &Center, &Pos);
			}
			else
				geVec3d_Add(&Pos, &XForm.Translation, &Pos);
		}
		else
		{
			Pos = Light->origin;
		}

		assert(Light->RadiusSpeed > Light->LastTime);
		Percentage = Light->LastTime / Light->RadiusSpeed;

		Index = (int)(Percentage * Light->NumFunctionValues);
		assert(Index < Light->NumFunctionValues);
		assert(Light->RadiusFunction[Index] >= 'a' && Light->RadiusFunction[Index] <= 'z');

		if	(Light->InterpolateValues && Index < Light->NumFunctionValues - 1)
		{
			geFloat	Remainder;
			geFloat	InterpolationPercentage;
			int		DeltaValue;
			geFloat	Value;

			Remainder = (geFloat)fmod(Light->LastTime, Light->IntervalWidth);
			InterpolationPercentage = Remainder / Light->IntervalWidth;
			DeltaValue = Light->RadiusFunction[Index + 1] - Light->RadiusFunction[Index];
			Value = Light->RadiusFunction[Index] + DeltaValue * InterpolationPercentage;
			Percentage = ((geFloat)(Value - 'a')) / ((geFloat)('z' - 'a'));
		}
		else
		{
			Percentage = ((geFloat)(Light->RadiusFunction[Index] - 'a')) / ((geFloat)('z' - 'a'));
		}

		Radius = Percentage * (Light->MaxRadius - Light->MinRadius) + Light->MinRadius;

		geWorld_SetLightAttributes(World,
								   Light->DynLight,
								   &Pos,
								   &Light->Color,
								   Radius,
								   Light->CastShadows);

		Light->LastTime = (geFloat)fmod(Light->LastTime + DeltaTime, Light->RadiusSpeed);

		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}

	return GE_TRUE;
}

static geBoolean DynLight_SetWorld(geWorld *World, geVFile *Context)
{
	geEntity_EntitySet *	Set;
	geEntity *				Entity;

	assert(World);

	Set = geWorld_GetEntitySet(World, "DynamicLight");
	if	(Set == NULL)
		return GE_TRUE;

	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	while	(Entity)
	{
		DynamicLight *	Light;

		Light = geEntity_GetUserData(Entity);
		Light->NumFunctionValues = strlen(Light->RadiusFunction);
		assert(Light->NumFunctionValues > 0);
		Light->IntervalWidth = Light->RadiusSpeed / (geFloat)Light->NumFunctionValues;
		Light->DynLight = geWorld_AddLight(World);
		if	(!Light->DynLight)
		{
			geErrorLog_AddString(-1,"DynLight_SetWorld: geWorld_AddLight failed.\n",NULL);
			return GE_FALSE;
		}

		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}

	return GE_TRUE;
	Context;
}
