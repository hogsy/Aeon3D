/****************************************************************************************/
/*  GPCorona.c                                                                          */
/*                                                                                      */
/*  Description:    Corona module - converted from GTEST         		                */
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
#include	<assert.h>

#include	"ErrorLog.h"

#define	BUILD_CORONA
#include "GPCorona.h"
#include "BitmapUtil.h"
#include "ErrorLog.h"

static	geWorld		*CWorld;
static	geEngine	*TheEngine;
static	geBitmap	*CoronaBitmap;

extern	geFloat		EffectScale;

geBoolean Corona_Init(geEngine *Engine, geWorld *World, geVFile *MainFS)
{
	assert(Engine);
	assert(World);
	assert(MainFS);

	CWorld = World;
	TheEngine = Engine;

	CoronaBitmap = BitmapUtil_CreateFromFileName( NULL, "bmp\\corona.bmp", "bmp\\corona_a.bmp", GE_FALSE, GE_FALSE );

	if(!CoronaBitmap)
	{
		geErrorLog_AddString(-1, "Corona_SetWorld:  geBitmapUtil_CreateFromFileAndAlphaNames failed:","Bmp\\Corona.Bmp, Bmp\\Corona_a.Bmp");
		return GE_FALSE;
	}

	if (!geWorld_AddBitmap(World, CoronaBitmap))
	{
		geBitmap_Destroy(&CoronaBitmap);
		geErrorLog_AddString(-1, "Corona_SetWorld:  geWorld_AddBitmap failed", NULL);
		return GE_FALSE;
	}

	if (!geWorld_GetEntitySet(World, "Corona"))
		return GE_TRUE;

	return GE_TRUE;
}

geBoolean Corona_Shutdown(void)
{
	geBoolean	Ret;

	Ret = GE_TRUE;

	if (CoronaBitmap)
	{
		assert(CWorld);

		if (!geWorld_RemoveBitmap(CWorld, CoronaBitmap))
		{
			geErrorLog_AddString(-1, "Corona_ChangeWorld:  geWorld_RemoveBitmap failed", NULL);
			Ret = GE_FALSE;
		}

		geBitmap_Destroy(&CoronaBitmap);
	}

	CWorld = NULL;
	CoronaBitmap = NULL;

	return Ret;
}

geBoolean Corona_ChangeWorld(geWorld *NewWorld)
{
	geBoolean	Ret;

	assert(NewWorld);
	assert(CWorld);
	assert(CoronaBitmap);

	Ret = GE_TRUE;

	if (!geWorld_RemoveBitmap(CWorld, CoronaBitmap))
	{
		geErrorLog_AddString(-1, "Corona_ChangeWorld:  geWorld_RemoveBitmap failed", NULL);
		Ret = GE_FALSE;
	}

	CWorld = NewWorld;

	if (!geWorld_AddBitmap(NewWorld, CoronaBitmap))
	{
		geBitmap_Destroy(&CoronaBitmap);
		geErrorLog_AddString(-1, "Corona_ChangeWorld:  geWorld_AddBitmap failed", NULL);
		Ret = GE_FALSE;
	}

	return Ret;
}

geBoolean Corona_Frame(geWorld *World, const geXForm3d *XForm, geFloat DeltaTime)
{
	geEntity_EntitySet *	Set;
	geEntity *				Entity;
	GE_Collision			Collision;

	assert(World);
	assert(World == CWorld);

	Set = geWorld_GetEntitySet(World, "GPCorona");
	
	if (Set == NULL)
		return GE_TRUE;

	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);

	while	(Entity)
	{
		GPCorona *	C;
		geVec3d		Pos;
		geFloat		DistanceToCorona;
		geVec3d		Delta;
		geFloat		Radius;
		geBoolean	Visible;
		geBoolean	Fading;
		int32		Leaf;

		C = geEntity_GetUserData(Entity);
		
		geWorld_GetLeaf(World, &C->origin, &Leaf);

		if (geWorld_MightSeeLeaf(World, Leaf))
		{

		if	(C->Model)
		{
			geXForm3d	XForm;

			geWorld_GetModelXForm(World, C->Model, &XForm);

			Pos = C->origin;
			if	(C->AllowRotation)
			{
				geVec3d	Center;

				geWorld_GetModelRotationalCenter(World, C->Model, &Center);
				geVec3d_Subtract(&Pos, &Center, &Pos);
				geXForm3d_Transform(&XForm, &Pos, &Pos);
				geVec3d_Add(&Pos, &Center, &Pos);
			}
			else
				geVec3d_Add(&Pos, &XForm.Translation, &Pos);
		}
		else
		{
			Pos = C->origin;
		}

		geVec3d_Subtract(&Pos, &XForm->Translation, &Delta);
		DistanceToCorona = geVec3d_Length(&Delta);

		if	(!geWorld_Collision(World,
								NULL,
								NULL,
								&Pos,
								&XForm->Translation,
								GE_CONTENTS_CANNOT_OCCUPY,
								GE_COLLIDE_MODELS,
								0xffffffff, NULL, NULL, 
								&Collision) &&
			 (DistanceToCorona < C->MaxVisibleDistance))
		{
			Visible = GE_TRUE;
			C->LastVisibleTime = C->LastTime;
		}
		else
		{
			Visible = GE_FALSE;
		}

		Fading = (C->FadeOut && (C->LastTime <= C->LastVisibleTime + C->FadeTime)) ? GE_TRUE : GE_FALSE;
	
		if	(Visible || Fading)
		{
			GE_LVertex	Vert;
			
			Vert.X = Pos.X;
			Vert.Y = Pos.Y;
			Vert.Z = Pos.Z;
			Vert.r = C->Color.r;
			Vert.g = C->Color.g;
			Vert.b = C->Color.b;
			Vert.a = 255.0f;
			Vert.u = Vert.v = 0.0f;

			if	(Visible)
			{
				if	(DistanceToCorona >= C->MaxRadiusDistance)
				{
					Radius = (geFloat)C->MaxRadius;
				}
				else if	(DistanceToCorona <= C->MinRadiusDistance)
				{
					Radius = C->MinRadius;
				}
				else
				{
					geFloat	Slope;

					Slope = (C->MaxRadius - C->MinRadius) / (geFloat)(C->MaxRadiusDistance - C->MinRadiusDistance);
					Radius = C->MinRadius + Slope * (geFloat)(DistanceToCorona - C->MinRadiusDistance);
				}
				C->LastVisibleRadius = Radius;
			}
			else
			{
				assert(Fading == GE_TRUE);
				assert(C->LastTime <= C->LastVisibleTime + C->FadeTime);
				Radius = (1.0f - (C->LastTime - C->LastVisibleTime) / C->FadeTime) * C->LastVisibleRadius;
			}

			geWorld_AddPolyOnce(World,
								&Vert,
								1,
								CoronaBitmap,
								GE_TEXTURED_POINT,
								GE_RENDER_DO_NOT_OCCLUDE_OTHERS | GE_RENDER_DO_NOT_OCCLUDE_SELF,
								Radius * EffectScale);
		}

		C->LastTime += DeltaTime;
		}

		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}

	return GE_TRUE;
}

