/****************************************************************************************/
/*  Trigger.c                                                                           */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Processes triggers from event strings           		            */
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
#include <stdlib.h>
#include <assert.h>
#include "errorlog.h"
#include "genesis.h"
#include "string.h"
#include "objmgr.h"
#include "trigger.h"
#include "usertypes.h"
#include "genvs.h"

#include "Effect.h"
#include "EffectM.h"
#include "SPool.h"
#include "TPool.h"
#include "fade.h"

#define MAX_EVENT_ARGS 6

struct TriggerData
{
	ObjMgr *OM;
	SoundPool *SPool;
	TexturePool *TPool;
	void *ESystem;
	void *EManager;
}TriggerData;


geBoolean Trigger_Set(ObjMgr *OM, SoundPool *SPool, TexturePool *TPool, void *ESystem, void *EManager)
{
	TriggerData.OM = OM;
	TriggerData.SPool = SPool;
	TriggerData.TPool = TPool;
	TriggerData.ESystem = ESystem;
	TriggerData.EManager = EManager;

	return GE_TRUE;
}

static geBoolean Trigger_IsStringNull(
	char	*String )	// string to check
{

	// first way
	if ( String == NULL )
	{
		return GE_TRUE;
	}

	// second way
	if ( strlen( String ) < 1 )
	{
		return GE_TRUE;
	}

	// third way
	if ( strnicmp( String, "<null>", 6 ) == 0 )
	{
		return GE_TRUE;
	}

	// fourth way
	if ( strnicmp( String, "NULL", 4 ) == 0 )
	{
		return GE_TRUE;
	}

	// if we got to here then the string is not null
	return GE_FALSE;

}

static void Trigger_ActorMotion(geWorld *World, 
						  TriggerContext ContextType, 
						  void *ContextData, 
						  char *AnimSearchName, 
						  char *ActorTriggerName)
{
	geEntity_EntitySet	*ClassSet;
	geEntity			*Entity;
	ObjMgr *OM;
	Object *Obj;
	ActorAnim *Data;
	geBoolean FoundData = GE_FALSE;
	MotionData MD;
	geActor *Actor;

	assert(World);

	// first look for entity data
	ClassSet = geWorld_GetEntitySet(World, "ActorAnim");
	if (!ClassSet)
		return;

	Entity = NULL;
	while ((Entity = geEntity_EntitySetGetNextEntity(ClassSet, Entity)) != NULL)
		{
		Data = (ActorAnim*)geEntity_GetUserData(Entity);
		if (stricmp(Data->TriggerName, AnimSearchName) == 0)
			{
			FoundData = GE_TRUE;
			break;
			}
		}

	if (!FoundData)
		return;

	OM = TriggerData.OM;
	assert(OM);
	Obj = ObjMgr_GetObjectByTriggerName(World, OM, ActorTriggerName);
	if (Obj == NULL)
	{
		return;
	}

	// see if we have a good motion
	Actor = ObjMgr_GetObjectActor(Obj);
	assert(Actor != NULL);
	if (Trigger_IsStringNull(Data->MotionName))
		{
		MD.Motion = NULL;
		MD.Active = GE_FALSE;
		}
	else
		{
		MD.Motion = geActor_GetMotionByName(geActor_GetActorDef(Actor),Data->MotionName);
		if (MD.Motion == NULL)
			return;
		MD.Active = GE_TRUE;
		}

	MD.BlendAmt = Data->BlendAmount;
	MD.FrameTime = 0.0f;
	MD.Loop = Data->Loop;
	MD.TimeScale = Data->MotionScale;

	// argh - need a define for this
	if (Data->SyncIndex >= 0 && Data->SyncIndex < 4)
		{
		MotionData *OMD;
		OMD = ObjMgr_GetObjectMotionData(Obj, Data->SyncIndex);
		MD.FrameTime = OMD->FrameTime;
		}

	
	{
	// had to do this for situations where an actor started with no motion
	geXForm3d XForm;
	geXForm3d_SetIdentity(&XForm);
	geActor_SetBoneAttachment(Actor, NULL, &XForm);
	}
	

	ObjMgr_SetObjectMotionData(Obj, Data->Index, &MD);
	ObjMgr_SetObjectActive(Obj, GE_TRUE);
}

static void Trigger_ActorToPath(geWorld *World, 
						  TriggerContext ContextType, 
						  void *ContextData, 
						  char *PathTriggerName, 
						  char *ActorTriggerName)
{
	ObjMgr *OM;
	Object *Obj;

	assert(World);

	// Currently does not use PathTriggerName. 
	// This routine can be extended to look for a path if needed.

	OM = TriggerData.OM;
	Obj = ObjMgr_GetObjectByTriggerName(World, OM, ActorTriggerName);
	if (Obj == NULL)
	{
		return;
	}

	assert(OM);

	ObjMgr_SetObjectPathState(Obj, GE_TRUE);

	return;
	PathTriggerName;
}


static geBoolean Trigger_Model(geWorld *World, char *TriggerName)
{
	geEntity_EntitySet *	Set;
	geEntity *				Entity;
	Object	*TObj;
 
	assert(World);

	Set = geWorld_GetEntitySet(World, "ModelTrigger");
	if	(Set == NULL)
		return GE_TRUE;

	Entity = geEntity_EntitySetGetNextEntity(Set, NULL);
	while	(Entity)
	{
		ModelTrigger *Data;
		TriggerP	TriggerFunc;

		Data = geEntity_GetUserData(Entity);

		if (stricmp(Data->TriggerName, TriggerName) == 0)
		{
			TObj = (Object*)geWorld_ModelGetUserData(Data->Model);

			if (TObj)
				{
				TriggerFunc = ObjMgr_GetObjectTriggerFunction(TObj);
				if (TriggerFunc)
					{
					TriggerFunc(World, TObj, NULL, NULL);
					}
				}
		}


		Entity = geEntity_EntitySetGetNextEntity(Set, Entity);
	}

	return GE_TRUE;
}


static void Trigger_GetContextXForm(TriggerContext ContextType, void *ContextData, geXForm3d *XForm)
	{
	assert(ContextData);
	assert(XForm);

	switch (ContextType)
		{
		case TRIGGER_FROM_OBJECT:	
			{
			Object *Obj;
			Obj = (Object*)ContextData;
			ObjMgr_GetObjectXForm(Obj,XForm);
			break;
			}
		case TRIGGER_FROM_XFORM:	
			*XForm = *((geXForm3d*)ContextData);
			break;
		case TRIGGER_FROM_MODEL:	
			*XForm = *((geXForm3d*)ContextData);
			break;
		case TRIGGER_FROM_ENTITY:	
			*XForm = *((geXForm3d*)ContextData);
			break;
		case TRIGGER_FROM_ACTOR:	
			geActor_GetBoneTransform((geActor*)ContextData, NULL, XForm);
			break;
		}
	}


static geBoolean Trigger_PlaySound( geXForm3d *XForm, char *Name, float Min )
{
	SoundPool		*SPool;
	Snd				Sound;
	int				SndNum;
	char			FileName[_MAX_PATH];

	SPool = TriggerData.SPool;

	if (SPool == NULL)
		return GE_TRUE;
	
	memset( &Sound, 0, sizeof( Sound ) );

	strlwr(Name);
	// look for a slash in the name
	if (strstr(Name,"\\") == NULL)
		sprintf(FileName,"wav\\%s",Name);
	else
		strcpy(FileName, Name);

	SndNum = SPool_Add(SPool, NULL, FileName );
	
	if (SndNum < 0)
		{
		// make sure the actor doesn't have events with bad formats
		//geErrorLog_AddString(-1,"Trigger_PlaySound: failed to get sound from sound pool.",FileName);
		return( GE_FALSE );
		}

	Sound.SoundDef = SPool_Get( SPool, SndNum );
	if( Sound.SoundDef == NULL )
	{
		geErrorLog_AddString(-1,"Trigger_PlaySound: failed to get sound def.",FileName);
		return( GE_FALSE );
	}
	Sound.Loop = GE_FALSE;
	
	geVec3d_Copy( &XForm->Translation, &Sound.Pos );
	
	Sound.Min = Min;
	
	if (Effect_New( TriggerData.ESystem, Effect_Sound, &Sound, 0, NULL , 0, NULL) == GE_FALSE)
	{
		geErrorLog_AddString(-1,"Trigger_PlaySound: failed to create new effect.",Name);
		return (GE_FALSE);
	}

	return( GE_TRUE );
}


static void Trigger_EffectSpout1(geWorld *World, TriggerContext ContextType, void *ContextData, 
						  char *ActorName, char *EntityNameForData)
{
	EM_Spout spout;
	EffectManager *em;
	geBoolean FoundData = GE_FALSE;
	geEntity_EntitySet	*ClassSet;
	geEntity			*Entity;
	EM_Spout			*Data;

	char *EntitySearchName = NULL;

	assert(World);

	if (EntityNameForData && EntityNameForData[0])
		{
		EntitySearchName = EntityNameForData;
		}
	else
	if (ContextType == TRIGGER_FROM_OBJECT || ContextType == TRIGGER_FROM_OBJECT_XFORM)
		{
		Object *Obj;
		ActorStart *ClassData;

		Obj = ContextData;
		ClassData = ObjMgr_GetObjectClassData(Obj);
		assert(ClassData);

		if (ClassData->TriggerName[0] && stricmp(ClassData->TriggerName, "NULL") != 0)
			EntitySearchName = ClassData->TriggerName;
		}

	if (EntitySearchName)
		{
		// first look for entity data
		ClassSet = geWorld_GetEntitySet(World, "EM_Spout");
		if (ClassSet)
			{
			Entity = NULL;
			while ((Entity = geEntity_EntitySetGetNextEntity(ClassSet, Entity)) != NULL)
				{
				Data = (EM_Spout*)geEntity_GetUserData(Entity);
				if (stricmp(Data->TriggerName, EntitySearchName) == 0)
					{
					FoundData = GE_TRUE;
					break;
					}
				}
			}
		}


	if (FoundData)
		{
		memcpy(&spout, Data, sizeof(EM_Spout));
		spout.TriggerName = NULL;
		}
	else
		{
		return;
		}

	em = TriggerData.EManager;

	if (ActorName && ActorName[0])
		{
		int Count;
		Object *Obj;
		ObjMgr *OM;
		int i;
		char *cp;

		OM = TriggerData.OM;
		assert(OM);

		Count = ObjMgr_ObjectCount(OM);
		for (i = 0; i < Count; i++)
			{
			Obj = ObjMgr_Iterate(OM, i);
			cp = ObjMgr_GetObjectTriggerName(Obj);

			if (Trigger_IsStringNull(cp) == GE_TRUE)
				continue;

			if (stricmp(ActorName,cp) == 0)
				{
				spout.Actor = ObjMgr_GetObjectActor(Obj);
				assert(spout.Actor);
				EffectM_Create( em, EffectM_Spout, &spout, sizeof(spout) );
				}
			}
		}
	else
		{
		geXForm3d *XForm,XF;

		if (!spout.AStart)
			{
			switch (ContextType)
				{
				case TRIGGER_FROM_XFORM:	
				case TRIGGER_FROM_MODEL:	
				case TRIGGER_FROM_ENTITY:	
					XForm = ContextData;
					spout.Position = XForm->Translation;
					geXForm3d_GetEulerAngles(XForm, &spout.Angles);
					break;
				case TRIGGER_FROM_ACTOR:	
					spout.Actor = ContextData;
					break;
				case TRIGGER_FROM_OBJECT:	
					{
					Object *Obj;
					geActor *Actor;

					Obj = (Object*)ContextData;
					Actor = ObjMgr_GetObjectActor(Obj);
					if (Actor)
						{
						spout.Actor = ObjMgr_GetObjectActor(Obj);
						}
					else
						{
						ObjMgr_GetObjectXForm(Obj,&XF);
						spout.Position = XF.Translation;
						}
					}
					break;
				case TRIGGER_FROM_OBJECT_XFORM:	
					{
					geActor *Actor;
					geXForm3d XForm;

					ContextType = TRIGGER_FROM_XFORM;
					Actor = ObjMgr_GetObjectActor((Object*)ContextData);
					geActor_GetBoneTransform(Actor, "EFFECT_BONE01", &XForm);

					spout.Position = XForm.Translation;
					spout.Angles.Y -= 90;
					spout.MaxScale += 1.5f;
					break;
					}
				}
			}

		EffectM_Create( em, EffectM_Spout, &spout, sizeof(spout) );
		}
}

static void Trigger_EffectAmbient(geWorld *World, TriggerContext ContextType, void *ContextData, 
						  char *ActorName, char *EntityNameForData)
{
	EffectManager *em;
	geBoolean FoundData = GE_FALSE;
	geEntity_EntitySet	*ClassSet;
	geEntity			*Entity;
	EM_AmbientSound			*Data;
	EM_AmbientSound amb;

	char *EntitySearchName = NULL;

	assert(World);

	if (EntityNameForData && EntityNameForData[0])
		{
		EntitySearchName = EntityNameForData;
		}
	else
	if (ContextType == TRIGGER_FROM_OBJECT || ContextType == TRIGGER_FROM_OBJECT_XFORM)
		{
		Object *Obj;
		ActorStart *ClassData;

		Obj = ContextData;
		ClassData = ObjMgr_GetObjectClassData(Obj);
		assert(ClassData);

		if (ClassData->TriggerName[0] && stricmp(ClassData->TriggerName, "NULL") != 0)
			EntitySearchName = ClassData->TriggerName;
		}

	if (EntitySearchName)
		{
		// first look for entity data
		ClassSet = geWorld_GetEntitySet(World, "EM_AmbientSound");
		if (ClassSet)
			{
			Entity = NULL;
			while ((Entity = geEntity_EntitySetGetNextEntity(ClassSet, Entity)) != NULL)
				{
				Data = (EM_AmbientSound*)geEntity_GetUserData(Entity);
				if (stricmp(Data->TriggerName, EntitySearchName) == 0)
					{
					FoundData = GE_TRUE;
					break;
					}
				}
			}
		}


	if (FoundData)
		{
		memcpy(&amb, Data, sizeof(EM_AmbientSound));
		amb.TriggerName = NULL;
		}
	else
		{
		return;
		}

	em = TriggerData.EManager;

	if (ActorName && ActorName[0])
		{
		int Count;
		Object *Obj;
		ObjMgr *OM;
		int i;
		char *cp;

		OM = TriggerData.OM;
		assert(OM);

		Count = ObjMgr_ObjectCount(OM);
		for (i = 0; i < Count; i++)
			{
			Obj = ObjMgr_Iterate(OM, i);
			cp = ObjMgr_GetObjectTriggerName(Obj);

			if (Trigger_IsStringNull(cp) == GE_TRUE)
				continue;

			if (stricmp(ActorName,cp) == 0)
				{
				amb.AStart = ObjMgr_GetObjectClassData(Obj);
				EffectM_Create( em, EffectM_AmbientSound, &amb, sizeof(amb) );
				}
			}
		}
	else
		{
		geXForm3d *XForm, XF;

		if (!amb.AStart)
			{
			switch (ContextType)
				{
				case TRIGGER_FROM_XFORM:	
				case TRIGGER_FROM_MODEL:	
				case TRIGGER_FROM_ENTITY:	
					XForm = ContextData;
					amb.Position = XForm->Translation;
					break;
				case TRIGGER_FROM_ACTOR:
					geActor_GetBoneTransform(ContextData,NULL,&XF);
					amb.Position = XF.Translation;
					break;
				case TRIGGER_FROM_OBJECT_XFORM:	
					break;
				case TRIGGER_FROM_OBJECT:	
					{
					Object *Obj;
					geActor *Actor;

					Obj = (Object*)ContextData;
					assert(Obj);
					Actor = ObjMgr_GetObjectActor(Obj);
					if (Actor)
						{
						amb.AStart = ObjMgr_GetObjectClassData(Obj);
						}
					else
						{
						ObjMgr_GetObjectXForm(Obj,&XF);
						amb.Position = XF.Translation;
						}
					}
					break;
				}
			}

		EffectM_Create( em, EffectM_AmbientSound, &amb, sizeof(amb) );
		}
}

static void Trigger_PathTime(geWorld *World, char *ActorName, float Time)
{
	geBoolean FoundData = GE_FALSE;

	char *EntitySearchName = NULL;

	assert(World);

	if (ActorName && ActorName[0])
		{
		int Count;
		Object *Obj;
		ObjMgr *OM;
		int i;
		char *cp;

		OM = TriggerData.OM;
		assert(OM);

		Count = ObjMgr_ObjectCount(OM);
		for (i = 0; i < Count; i++)
			{
			Obj = ObjMgr_Iterate(OM, i);
			cp = ObjMgr_GetObjectTriggerName(Obj);

			if (Trigger_IsStringNull(cp) == GE_TRUE)
				continue;

			if (stricmp(ActorName,cp) == 0)
				{
				ObjMgr_SetObjectPathFrameTime(Obj, Time);
				}
			}
		}
}

static void Trigger_EffectMorph(geWorld *World, TriggerContext ContextType, void *ContextData, 
						  char *ActorName, char *EntityNameForData)
{
	EM_Morph morph;
	EffectManager *em;
	geBoolean FoundData = GE_FALSE;
	geEntity_EntitySet	*ClassSet;
	geEntity			*Entity;
	EM_Morph			*Data;

	char *EntitySearchName;

	// set string for EntitySearchName
	if (EntityNameForData && EntityNameForData[0])
		{
		EntitySearchName = EntityNameForData;
		}
	else
	if (ContextType == TRIGGER_FROM_OBJECT || ContextType == TRIGGER_FROM_OBJECT)
		{
		Object *Obj;
		ActorStart *ClassData;

		Obj = ContextData;
		ClassData = ObjMgr_GetObjectClassData(Obj);
		assert(ClassData);

		if (ClassData->TriggerName[0] && stricmp(ClassData->TriggerName, "NULL") != 0)
			EntitySearchName = ClassData->TriggerName;
		}

	// search for the entity data
	if (EntitySearchName)
		{
		// first look for entity data
		ClassSet = geWorld_GetEntitySet(World, "EM_Morph");
		if (ClassSet)
			{
			Entity = NULL;
			while ((Entity = geEntity_EntitySetGetNextEntity(ClassSet, Entity)) != NULL)
				{
				Data = (EM_Morph*)geEntity_GetUserData(Entity);

				if (stricmp(Data->TriggerName, EntitySearchName) == 0)
					{
					memcpy(&morph, Data, sizeof(EM_Morph));
					morph.TriggerName = NULL;

					em = TriggerData.EManager;

					if (ActorName && ActorName[0])
						{
						int Count;
						Object *Obj;
						ObjMgr *OM;
						int i;
						char *cp;

						OM = TriggerData.OM;
						assert(OM);

						Count = ObjMgr_ObjectCount(OM);
						for (i = 0; i < Count; i++)
							{
							Obj = ObjMgr_Iterate(OM, i);
							cp = ObjMgr_GetObjectTriggerName(Obj);

							if (Trigger_IsStringNull(cp) == GE_TRUE)
								continue;

							if (stricmp(ActorName,cp) == 0)
								{
								morph.Actor = ObjMgr_GetObjectActor(Obj);
								EffectM_Create( em, EffectM_Morph, &morph, sizeof(morph) );
								}
							}
						}
					else
						{
						switch (ContextType)
							{
							case TRIGGER_FROM_ACTOR:	
								morph.Actor = ContextData;
								break;
							case TRIGGER_FROM_OBJECT:	
								morph.Actor = ObjMgr_GetObjectActor((Object*)ContextData);
								break;
							}

						EffectM_Create( em, EffectM_Morph, &morph, sizeof(morph) );
						}

					//break;
					}
				}
			}
		}

}

static void Trigger_EffectLightStyle(geWorld *World, TriggerContext ContextType, void *ContextData, 
						  char *EntitySearchName)
{
	EM_LightStyle copy;
	EffectManager *em;
	geBoolean FoundData = GE_FALSE;
	geEntity_EntitySet	*ClassSet;
	geEntity			*Entity;
	EM_LightStyle		*Data;

	// search for the entity data
	if (EntitySearchName && EntitySearchName[0])
		{
		// first look for entity data
		ClassSet = geWorld_GetEntitySet(World, "EM_LightStyle");
		if (ClassSet)
			{
			Entity = NULL;
			while ((Entity = geEntity_EntitySetGetNextEntity(ClassSet, Entity)) != NULL)
				{
				Data = (EM_LightStyle*)geEntity_GetUserData(Entity);

				if (stricmp(Data->TriggerName, EntitySearchName) == 0)
					{
					if (Data->TriggerCount == 0)
						continue;

					if (Data->TriggerCount > 0)
						Data->TriggerCount--;

					memcpy(&copy, Data, sizeof(copy));
					copy.TriggerName = NULL;

					em = TriggerData.EManager;
					EffectM_Create( em, EffectM_LightStyle, &copy, sizeof(copy) );
					}
				}
			}
		}

}

static void Trigger_FadeOut(geWorld *World, float Time)
{
	Fade_Set(1, Time);
}

static void Trigger_FadeIn(geWorld *World, float Time)
{
	Fade_Set(-1, Time);
}

static void Trigger_EffectFlipbook(geWorld *World, TriggerContext ContextType, void *ContextData, 
						  char *ActorName, char *EntityNameForData)
{
	EM_Flipbook flipbook;
	EffectManager *em;
	geBoolean FoundData = GE_FALSE;
	geEntity_EntitySet	*ClassSet;
	geEntity			*Entity;
	EM_Flipbook			*Data;

	char *EntitySearchName;

	assert(World);

	// set string for EntitySearchName
	if (EntityNameForData && EntityNameForData[0])
		{
		EntitySearchName = EntityNameForData;
		}
	else
	if (ContextType == TRIGGER_FROM_OBJECT || ContextType == TRIGGER_FROM_OBJECT)
		{
		Object *Obj;
		ActorStart *ClassData;

		Obj = ContextData;
		ClassData = ObjMgr_GetObjectClassData(Obj);
		assert(ClassData);

		if (ClassData->TriggerName[0] && stricmp(ClassData->TriggerName, "NULL") != 0)
			EntitySearchName = ClassData->TriggerName;
		}

	// search for the entity data
	if (EntitySearchName)
		{
		// first look for entity data
		ClassSet = geWorld_GetEntitySet(World, "EM_Flipbook");
		if (ClassSet)
			{
			Entity = NULL;
			while ((Entity = geEntity_EntitySetGetNextEntity(ClassSet, Entity)) != NULL)
				{
				Data = (EM_Flipbook*)geEntity_GetUserData(Entity);
				if (stricmp(Data->TriggerName, EntitySearchName) == 0)
					{
					FoundData = GE_TRUE;
					break;
					}
				}
			}
		}

	if (FoundData)
		{
		memcpy(&flipbook, Data, sizeof(EM_Flipbook));
		flipbook.TriggerName = NULL;
		}
	else
		{
		return;
		}

	em = TriggerData.EManager;

	switch (ContextType)
		{
		case TRIGGER_FROM_ACTOR:	
			flipbook.Actor = ContextData;
			break;
		case TRIGGER_FROM_OBJECT:	
			flipbook.Actor = ObjMgr_GetObjectActor((Object*)ContextData);
			break;
		}

	EffectM_Create( em, EffectM_Flipbook, &flipbook, sizeof(flipbook) );
}

static void Trigger_NewLevel(geWorld *World, char *NewLevelName)
{
}

void Trigger_ParseEvent(geWorld *World, TriggerContext ContextType, void *ContextData, char *EventString)
{
	char *ep;
	geXForm3d XForm;
	int *etype;
	char *semi_ptr = NULL, *bone_ptr = NULL;

	// int32 versions of the id string - reverse characters account for intel endian ordering
	#define PTH		'\0HTP'
	#define MTA		'\0ATM'
	#define WAV		'\0VAW'
	#define MODL	 'LDOM'
	#define ACTR	 'RTCA'
	#define EFCT     'TCFE'
	#define LVL     '\0LVL'
	#define LSTY     'YTSL'
	#define TPTH     'HTPT'
	#define FDIN     'NIDF'
	#define FDOT     'TODF'

	char Arg[MAX_EVENT_ARGS][_MAX_FNAME];

	assert(World);
	assert(EventString);

	// if the event string came from an actor there is always a "; BoneName" attached to the end
	// account for it
	semi_ptr = strstr(EventString,";");
	if (semi_ptr)
		{
		*semi_ptr = '\0';
		bone_ptr = semi_ptr++;
		}

	ep = EventString;
	memset(Arg,0, sizeof(Arg));
	sscanf(ep, "%s %s %s %s %s %s", &Arg[0], &Arg[1], &Arg[2], &Arg[3], &Arg[4], &Arg[5], &Arg[6]);

	etype = (int32*)Arg[0];
	switch (*etype)
		{
		case LSTY:
			Trigger_EffectLightStyle(World, ContextType, ContextData, Arg[1]);
			break;
		case WAV:
			{
			Trigger_GetContextXForm(ContextType, ContextData, &XForm);
			Trigger_PlaySound( &XForm, Arg[1], (float)atof(Arg[2]) );
			break;
			}
		case PTH:
			Trigger_ActorToPath(World, ContextType, ContextData, Arg[1], Arg[2]);
			break;
		case LVL:
			Trigger_NewLevel(World, Arg[1]);
			break;
		case MTA:
			Trigger_ActorMotion(World, ContextType, ContextData, Arg[1], Arg[2]);
			break;
		case MODL:
			Trigger_Model(World, Arg[1]);
			break;
		case FDOT:
			Trigger_FadeOut(World, (float)atof(Arg[1]));
			break;
		case FDIN:
			Trigger_FadeIn(World, (float)atof(Arg[1]));
			break;
		case TPTH:
			Trigger_PathTime(World, Arg[1], (float)atof(Arg[2]));
			break;
		case EFCT:
			if (stricmp(Arg[1], "ambient") == 0)
				{
				Trigger_EffectAmbient(World, ContextType, ContextData, Arg[3], Arg[2]);
				}
			else
			if (stricmp(Arg[1], "spout1") == 0)
				{
				Trigger_EffectSpout1(World, ContextType, ContextData, Arg[3], Arg[2]);
				}
			else
			if (stricmp(Arg[1], "morph") == 0)
				{
				Trigger_EffectMorph(World, ContextType, ContextData, Arg[3], Arg[2]);
				}
			else
			if (stricmp(Arg[1], "ink") == 0)
				{
				if (ContextType == TRIGGER_FROM_OBJECT)
					ContextType = TRIGGER_FROM_OBJECT_XFORM;
				Trigger_EffectSpout1(World, ContextType, ContextData, NULL, NULL);
				}
			break;
		default:
			break;
		}

	return;
}

void Trigger_ProcessEvents(geWorld *World, TriggerContext ContextType, void *ContextData, geMotion *Motion, float StartTime, float EndTime)
{
	float Time;
	char *EventString;

	assert(World);
	assert(Motion);
	assert(StartTime <= EndTime);

	geMotion_SetupEventIterator(Motion, StartTime, EndTime);

	while( geMotion_GetNextEvent( Motion, &Time, &EventString ) )
		{
		Trigger_ParseEvent(World, ContextType, ContextData, EventString);
		}
}
