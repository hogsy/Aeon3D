/****************************************************************************************/
/*  ObjMgr.c                                                                            */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    General object routines             				                */
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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "genesis.h"
#include "ObjMgr.h"
#include "Ram.h"
#include "errorlog.h"
#include "DList.h"
#include "trigger.h"
#include "ActorStart.h"

typedef struct ObjMgr
{
	DListP	  List;
} ObjMgr;

#define MAX_MOTIONS 4

typedef struct Object
{
	char				*Name;
	geBoolean			Active;
	float				Time;
	geXForm3d			XForm;						
	geActor				*Actor;
	char				*MotionBone;
	MotionData			MotionData[MAX_MOTIONS];

	// stuff to support doors
	geWorld_Model		*Model;
	int32				Flags;
	ControlP			Control;
	TriggerP			Trigger;
	int32				State;
	float				FrameTime;

	// added for physics objects
	void				*userData;
	void				*ClassData;
	geVec3d				Velocity;
	DestroyP			Destroy;

	geMotion			*Path;
	geBoolean			PathLoop;
	float   			PathFrameTime;
	geBoolean			PathActive;
} Object;

ObjMgr *ObjMgr_Create( void )
{
	ObjMgr *OM;

	OM = GE_RAM_ALLOCATE_STRUCT( ObjMgr );
	if (OM == NULL)
		{
			// post error
			return NULL;
		}
	OM->List = DList_Create(0, sizeof(Object*) );
	if (OM->List == NULL )
		{
			geRam_Free(OM);
			// post error
			return NULL;
		}
	return OM;
}

void ObjMgr_ListDestroyCB( void* NodeData, void* Context )
{
	Object **O;

	assert(NodeData);

	O = (Object **)NodeData;

	if ((*O)->MotionBone)
		geRam_Free((*O)->MotionBone);

	if ((*O)->Name)
		geRam_Free((*O)->Name);

	if ((*O)->Actor)
		geActor_Destroy(&(*O)->Actor);

	geRam_Free( *O );
	*O = NULL;
	Context;	// just warning removal
}				

void ObjMgr_Destroy( ObjMgr **pOM )
{
	ObjMgr *OM;
	assert( pOM );
	assert( *pOM );
	OM = *pOM;

	DList_Destroy( &(OM->List), ObjMgr_ListDestroyCB, NULL );
	geRam_Free( OM );
	*pOM = NULL;
}

void ObjMgr_FreeWorldData( geWorld *World, ObjMgr *pOM )
{
	Object *Obj;
	int Node, Count, i;

	Node = 0;
	Count = DList_NodeN(pOM->List);
	for( i = 0; i < Count; i++ )
	{
		DList_GetNext( pOM->List, &Node, (void *)&Obj );
		if (Obj->Destroy)
			{
			Obj->Destroy(World, Obj, Obj->ClassData);
			}
	}
}

void ObjMgr_ResetObject(Object *Obj)
{
	geXForm3d_SetIdentity(&Obj->XForm);
	Obj->Active = GE_TRUE;
	memset(Obj->MotionData, 0, sizeof(Obj->MotionData));
	Obj->MotionData[0].Motion = NULL;
	Obj->MotionData[0].BlendAmt = 1.0;
	Obj->MotionData[0].FrameTime = 0.0;
	Obj->MotionData[0].TimeScale = 1.0;
	Obj->MotionData[0].Loop = GE_TRUE;
	Obj->MotionData[0].Active = GE_TRUE;

	Obj->Time = 0;
	Obj->Control = NULL;
	Obj->Trigger = NULL;
	Obj->State = 0;
	Obj->Model = NULL;
	Obj->Flags = 0;
	Obj->FrameTime = 0;
	Obj->userData = NULL;
	Obj->ClassData = NULL;
	Obj->Velocity.X = 0.0f;
	Obj->Velocity.Y = 0.0f;
	Obj->Velocity.Z = 0.0f;
	Obj->Path = NULL;
	Obj->PathLoop = GE_FALSE;
	Obj->PathFrameTime = 0.0f;
	Obj->PathActive = GE_FALSE;
	Obj->Destroy = NULL;
}

Object *ObjMgr_AddObject(ObjMgr *OM, geWorld *World, char *Name, geActor *Actor, geMotion *Motion, char *MotionBone, geXForm3d *XForm)
{
	Object *Obj;

	assert(World);

	Obj = geRam_Allocate( sizeof( Object ) );
	if( Obj == NULL )
		{
		geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE,"ObjMgr_AddObject: failed to create object struct.",NULL);
		return( NULL );
		}
	
	if (DList_Insert( OM->List, DLIST_HEAD, &Obj ) != GE_TRUE)
		{
		geErrorLog_AddString(-1,"ObjMgr_AddObject: failed to create add object to list.",NULL);
		return NULL;
		}

	// defaults - should call reset function for these
	geXForm3d_SetIdentity(&Obj->XForm);
	Obj->Active = GE_TRUE;
	memset(Obj->MotionData, 0, sizeof(Obj->MotionData));
	Obj->MotionData[0].BlendAmt = 1.0;
	Obj->MotionData[0].FrameTime = 0.0;
	Obj->MotionData[0].TimeScale = 1.0;
	Obj->MotionData[0].Loop = GE_TRUE;
	Obj->MotionData[0].Active = GE_TRUE;
	Obj->Time = 0;
	Obj->Control = NULL;
	Obj->Trigger = NULL;
	Obj->State = 0;
	Obj->Model = NULL;
	Obj->Flags = 0;
	Obj->FrameTime = 0;
	Obj->userData = NULL;
	Obj->ClassData = NULL;
	Obj->Velocity.X = 0.0f;
	Obj->Velocity.Y = 0.0f;
	Obj->Velocity.Z = 0.0f;
	Obj->Path = NULL;
	Obj->PathLoop = GE_FALSE;
	Obj->PathFrameTime = 0.0f;
	Obj->PathActive = GE_FALSE;
	Obj->Destroy = NULL;

	// passed in
	Obj->Actor = Actor;
	Obj->MotionData[0].Motion = Motion;

	if (MotionBone)
		{
		if (MotionBone[0])
			{
			Obj->MotionBone = geRam_Allocate(strlen(MotionBone)+1);
			strcpy(Obj->MotionBone, MotionBone);
			}
		else
			Obj->MotionBone = NULL;
		}
	else
		Obj->MotionBone = NULL;

	if (Name)
		{
		if (Name[0])
			{
			Obj->Name = geRam_Allocate(strlen(Name)+1);
			strcpy(Obj->Name, Name);
			}
		else
			Obj->Name = NULL;
		}
	else
		Obj->Name = NULL;

	if (XForm)
		Obj->XForm = *XForm;

	return(Obj);
}


void ObjMgr_SetObjectXForm(  Object* Obj, geXForm3d *XForm)
{
	assert(Obj);

	Obj->XForm = *XForm;

	return;
}
void ObjMgr_GetObjectXForm(  Object* Obj, geXForm3d *XForm)
{
	assert(Obj);

	*XForm = Obj->XForm;

	return;
}

geActor *ObjMgr_GetObjectActor(  Object* Obj)
{
	assert(Obj);

	return Obj->Actor;
}

geBoolean ObjMgr_ObjectHasMotion(  Object* Obj)
{
	assert(Obj);

	return(Obj->MotionData[0].Motion && Obj->MotionBone);
}

geBoolean ObjMgr_SetObjectActive(Object *Obj, geBoolean State)
{
	assert(Obj);

	Obj->Active = State;
	return(GE_TRUE);
}

geBoolean ObjMgr_SetObjectMotionByName(Object *Obj, char *MotionName)
{
	geActor_Def *ActorDef;

	assert(Obj);
	
	ActorDef = geActor_GetActorDef(Obj->Actor);
	Obj->MotionData[0].Motion = geActor_GetMotionByName(ActorDef, MotionName);

	return(GE_TRUE);
}

geBoolean ObjMgr_SetObjectMotion(Object *Obj, geMotion *Motion)
{
	geActor_Def *ActorDef;

	assert(Obj);
	
	ActorDef = geActor_GetActorDef(Obj->Actor);
	Obj->MotionData[0].Motion = Motion;

	return(GE_TRUE);
}

geBoolean ObjMgr_SetObjectMotionData(Object *Obj, int MotionIndex, MotionData *MotionData)
{
	assert(Obj != NULL);
	assert(MotionIndex >= 0 && MotionIndex < MAX_MOTIONS);
	assert(MotionData != NULL);
	
	memcpy(&Obj->MotionData[MotionIndex], MotionData, sizeof(*MotionData));

	return(GE_TRUE);
}
MotionData *ObjMgr_GetObjectMotionData(Object *Obj, int MotionIndex)
{
	assert(Obj != NULL);
	assert(MotionIndex >= 0 && MotionIndex < MAX_MOTIONS);
	
	return &Obj->MotionData[MotionIndex];
}

geBoolean ObjMgr_SetMotionScale(Object *Obj, float Scale)
{
	assert(Obj);
	
	Obj->MotionData[0].TimeScale = Scale;
	return(GE_TRUE);
}

geBoolean ObjMgr_SetObjectModel(Object *Obj, geWorld_Model *Model)
{
	assert(Obj);
	Obj->Model = Model;
	return(GE_TRUE);
}
geWorld_Model *ObjMgr_GetObjectModel(Object *Obj)
{
	assert(Obj);
	return(Obj->Model);
}

geBoolean ObjMgr_SetObjectFlags(Object *Obj, int32 Flags)
{
	assert(Obj);
	Obj->Flags = Flags;
	return(GE_TRUE);
}
int32 ObjMgr_GetObjectFlags(Object *Obj)
{
	assert(Obj);
	return(Obj->Flags);
}

geBoolean ObjMgr_SetObjectUserData(Object *Obj, void *UserData)
{
	assert(Obj);
	Obj->userData = UserData;
	return(GE_TRUE);
}
void *ObjMgr_GetObjectUserData(Object *Obj)
{
	assert(Obj);
	return(Obj->userData);
}

geBoolean ObjMgr_SetObjectClassData(Object *Obj, void *ClassData)
{
	assert(Obj);
	Obj->ClassData = ClassData;
	return(GE_TRUE);
}
void *ObjMgr_GetObjectClassData(Object *Obj)
{
	assert(Obj);
	return(Obj->ClassData);
}

geBoolean ObjMgr_SetObjectControlFunction(Object *Obj, ControlP ControlFunction)
{
	assert(Obj);
	Obj->Control = ControlFunction;
	return(GE_TRUE);
}

geBoolean ObjMgr_SetObjectTriggerFunction(Object *Obj, TriggerP TriggerFunction)
{
	assert(Obj);
	Obj->Trigger = TriggerFunction;
	return(GE_TRUE);
}
TriggerP ObjMgr_GetObjectTriggerFunction(Object *Obj)
{
	assert(Obj);
	return(Obj->Trigger);
}

geBoolean ObjMgr_SetObjectState(Object *Obj, int32 State)
{
	assert(Obj);
	Obj->State = State;
	return(GE_TRUE);
}
int32 ObjMgr_GetObjectState(Object *Obj)
{
	assert(Obj);
	return(Obj->State);
}

geBoolean ObjMgr_SetObjectMotionLoop(Object *Obj, int32 State)
{
	assert(Obj);
	Obj->MotionData[0].Loop = State;
	return(GE_TRUE);
}

geBoolean ObjMgr_SetObjectFrameTime(Object *Obj, float FrameTime)
{
	assert(Obj);
	Obj->FrameTime = FrameTime;
	return(GE_TRUE);
}
float ObjMgr_GetObjectFrameTime(Object *Obj)
{
	assert(Obj);
	return(Obj->FrameTime);
}

geBoolean ObjMgr_SetObjectPathFrameTime(Object *Obj, float FrameTime)
{
	assert(Obj);
	Obj->PathFrameTime = FrameTime;
	return(GE_TRUE);
}
float ObjMgr_GetObjectPathFrameTime(Object *Obj)
{
	assert(Obj);
	return(Obj->PathFrameTime);
}

geBoolean ObjMgr_SetObjectVelocity(Object *Obj, geVec3d *Velocity)
{
	assert(Obj);
	Obj->Velocity = *Velocity;
	return(GE_TRUE);
}
void ObjMgr_GetObjectVelocity(Object *Obj, geVec3d *Velocity)
{
	assert(Obj);
	*Velocity = Obj->Velocity;
	return;
}

void ObjMgr_SetObjectDestroyFunction(Object *Obj, DestroyP DestroyFunction)
{
	assert(Obj);
	Obj->Destroy = DestroyFunction;
}
DestroyP ObjMgr_GetObjectDestroyFunction(Object *Obj)
{
	assert(Obj);
	return (Obj->Destroy);
}

geBoolean ObjMgr_SetObjectPathMotion(Object *Obj, geMotion *Path, geBoolean Loop)
{
	assert(Obj);
	Obj->Path = Path;
	Obj->PathLoop = Loop;
	return(GE_TRUE);
}
geBoolean ObjMgr_GetObjectPathMotion(Object *Obj, geMotion **Path, geBoolean *Loop)
{
	assert(Obj);
	*Loop = Obj->PathLoop;
	*Path = Obj->Path;
	return(GE_TRUE);
}

geBoolean ObjMgr_SetObjectPathState(Object *Obj, geBoolean State)
{
	assert(Obj);
	Obj->PathActive = State;
	return(GE_TRUE);
}
geBoolean ObjMgr_GetObjectPathState(Object *Obj)
{
	assert(Obj);
	return(Obj->PathActive);
}

Object *ObjMgr_GetObjectByName(geWorld *World, ObjMgr* OM, char *ActorName)
{
	int i;
	int Count;
	DList_NodeHandle Node;
	Object *Element;

	assert( World != NULL );
	assert( OM != NULL );
	assert(ActorName != NULL);

	Node = 0;
	Count = DList_NodeN(OM->List);
	for( i = 0; i < Count; i++ )
	{
		DList_GetNext( OM->List, &Node, (void *)&Element );

		if (stricmp(ActorName, Element->Name) == 0)
			return(Element);
	}


	return(NULL);
}

Object *ObjMgr_GetObjectByActor(ObjMgr *OM, geActor *Actor)
{
	int i;
	int Count;
	DList_NodeHandle Node;
	Object *Element;

	assert( OM != NULL );
	assert(Actor != NULL);

	Node = 0;
	Count = DList_NodeN(OM->List);
	for( i = 0; i < Count; i++ )
	{
		DList_GetNext( OM->List, &Node, (void *)&Element );

		if (Element->Actor == Actor)
			return(Element);
	}


	return(NULL);
}

Object *ObjMgr_GetObjectByClassData(ObjMgr *OM, void *ClassData)
{
	int i;
	int Count;
	DList_NodeHandle Node;
	Object *Element;

	assert( OM != NULL );
	assert(ClassData != NULL);

	Node = 0;
	Count = DList_NodeN(OM->List);
	for( i = 0; i < Count; i++ )
	{
		DList_GetNext( OM->List, &Node, (void *)&Element );

		if (Element->ClassData == ClassData)
			return(Element);
	}


	return(NULL);
}

Object *ObjMgr_GetObjectByTriggerName(geWorld *World, ObjMgr* OM, char *ActorName)
{
	int i;
	int Count;
	DList_NodeHandle Node;
	Object *Element;

	assert( World != NULL );
	assert( OM != NULL );
	assert(ActorName != NULL);

	Node = 0;
	Count = DList_NodeN(OM->List);
	for( i = 0; i < Count; i++ )
	{
		DList_GetNext( OM->List, &Node, (void *)&Element );

		if (Element->Actor && Element->ClassData)
			{
			ActorStart *Data = Element->ClassData;
			if (stricmp(ActorName, Data->TriggerName) == 0)
				return(Element);
			}
	}


	return(NULL);
}

char* ObjMgr_GetObjectTriggerName(Object *Obj)
{
	assert( Obj != NULL );

	if (Obj->Actor && Obj->ClassData)
		{
		ActorStart *Data = Obj->ClassData;
		return(Data->TriggerName);
		}

	return(NULL);
}


geBoolean ObjMgr_ObjectGetContents(geWorld *World, Object *Obj, float Time)
{
	GE_Contents		Contents;
	geVec3d			CMins, CMaxs;
	uint32			ColFlags;
	geXForm3d		XForm;
	TriggerP		TriggerFunc;
	
	assert(World);
	assert(Obj);
	
	ColFlags = GE_COLLIDE_MODELS;
	
	// really this is only used for the camera object at the moment
	// could be extended
	// Get a box bigger than the Object for doors, etc...
	CMins.X = 0;
	CMins.Y = 0;
	CMins.Z = 0;
	CMaxs.X = 0;
	CMaxs.Y = 0;
	CMaxs.Z = 0;
	
	CMins.X -= 450;
	CMins.Y -= 10;
	CMins.Z -= 450;
	CMaxs.X += 450;
	CMaxs.Y += 10;
	CMaxs.Z += 450;
	
	ObjMgr_GetObjectXForm(Obj, &XForm);
	
	if (geWorld_GetContents(World, &XForm.Translation, &CMins, &CMaxs, GE_COLLIDE_MODELS, 0,  NULL, NULL, &Contents))
	{
		if (Contents.Model)
		{
			Object	*TObj;
			
			TObj = (Object*)geWorld_ModelGetUserData(Contents.Model);
			
			if (TObj)
			{
				TriggerFunc = ObjMgr_GetObjectTriggerFunction(TObj);
				
				if (TriggerFunc && (ObjMgr_GetObjectFlags(TObj) & TYPE_TOUCH))
				{
					TriggerFunc(World, TObj, Obj, NULL);
				}
			}
		}
	}
	
	return GE_TRUE;
}
	  

geBoolean ObjMgr_ProcessObject(  geWorld *World, ObjMgr *OM, Object* Obj, float Time )
{
	extern geBoolean DebugExtBox;
	geVec3d Pos;
	geXForm3d XForm;
	float StartTime, EndTime, MotionTimeStart, MotionTimeEnd;
	geXForm3d	RXForm;
	float	SaveTime;
	MotionData *md;
	geBoolean FirstMotion,HasMotion;

	assert( World != NULL );
	assert( OM != NULL );

	SaveTime = Obj->Time;
	Obj->Time += Time;

	// move stuff through control
	if (Obj->Control)
		Obj->Control(World, Obj, Time);

	if (Obj->Actor)
	{
		if (Obj->Path && Obj->PathActive)
		{
			gePath *Path;
			geMotion_GetTimeExtents(Obj->Path, &StartTime, &EndTime);
			Path = geMotion_GetPath(Obj->Path, 0);
			SaveTime = Obj->PathFrameTime;
			Obj->PathFrameTime += Time * 1.0f;
			gePath_GetTimeExtents(Path, &StartTime, &EndTime);
			MotionTimeStart = (float)fmod(SaveTime, EndTime);
			MotionTimeEnd	= (float)fmod(Obj->PathFrameTime, EndTime);
			gePath_Sample(Path, MotionTimeEnd, &Obj->XForm);

			if (MotionTimeEnd < MotionTimeStart)
			{
				Trigger_ProcessEvents(World, TRIGGER_FROM_OBJECT, Obj, Obj->Path, StartTime, MotionTimeEnd);
				Trigger_ProcessEvents(World, TRIGGER_FROM_OBJECT, Obj, Obj->Path, MotionTimeStart, EndTime);
			}
			else
			{
				Trigger_ProcessEvents(World, TRIGGER_FROM_OBJECT, Obj, Obj->Path, MotionTimeStart, MotionTimeEnd);
			}

			if (!Obj->PathLoop)
			{
				if (MotionTimeEnd < MotionTimeStart)
					{
					gePath_Sample(Path, EndTime, &Obj->XForm);
					Obj->Path = NULL;
					}
			}

		}
		else
		if (Obj->MotionBone)
		{
			md = &Obj->MotionData[0];
			SaveTime = md->FrameTime;
			md->FrameTime += Time * md->TimeScale;
			geMotion_GetTimeExtents(md->Motion, &StartTime, &EndTime);
			MotionTimeStart = (float)fmod(SaveTime, EndTime);
			MotionTimeEnd	= (float)fmod(md->FrameTime, EndTime);
			geXForm3d_SetIdentity(&RXForm);
			geActor_SetPose(Obj->Actor, md->Motion, MotionTimeEnd, &RXForm);
			geActor_GetBoneTransform(Obj->Actor, Obj->MotionBone, &Obj->XForm);

			if (MotionTimeEnd < MotionTimeStart)
				{
				// motion wrapped
				Trigger_ProcessEvents(World, TRIGGER_FROM_OBJECT, Obj, md->Motion, StartTime, MotionTimeEnd);
				Trigger_ProcessEvents(World, TRIGGER_FROM_OBJECT, Obj, md->Motion, MotionTimeStart, EndTime);
				}
			else
			{
				Trigger_ProcessEvents(World, TRIGGER_FROM_OBJECT, Obj, md->Motion, MotionTimeStart, MotionTimeEnd);
			}

			return GE_TRUE;
		}

		XForm = Obj->XForm;

		// rotate XForm -90 degrees about the X
		// could do the same thing with a root bone attachment
		Pos = XForm.Translation;
		geXForm3d_SetXRotation(&RXForm, -(M_PI/2.0f)); // this resets RXForm before adding the rotation
		geXForm3d_Multiply(&XForm, &RXForm, &XForm);
		XForm.Translation = Pos;

		HasMotion = GE_FALSE;
		FirstMotion = GE_TRUE;
		for (md = &Obj->MotionData[0]; md < &Obj->MotionData[MAX_MOTIONS]; md++)
		{
			if (md->Motion)
			{
				float SaveTime;

				HasMotion = GE_TRUE;

				if (md->Active == GE_FALSE)
					continue;

				SaveTime = md->FrameTime;
				md->FrameTime += Time * md->TimeScale;
				geMotion_GetTimeExtents(md->Motion, &StartTime, &EndTime);
				MotionTimeStart = (float)fmod(SaveTime, EndTime);
				MotionTimeEnd = (float)fmod(md->FrameTime, EndTime);

				if (md->Loop == GE_FALSE && MotionTimeEnd < MotionTimeStart)
				{
					md->Active = GE_FALSE;
					continue;
				}
				
				if (FirstMotion)
				{
					FirstMotion = GE_FALSE;
					geActor_SetPose(Obj->Actor, md->Motion, MotionTimeEnd, &XForm);
				}
				else
				{
					geActor_BlendPose(Obj->Actor, md->Motion, MotionTimeEnd, &XForm, md->BlendAmt);
				}

				if (MotionTimeEnd < MotionTimeStart)
				{
					// motion wrapped
					Trigger_ProcessEvents(World, TRIGGER_FROM_OBJECT, Obj, md->Motion, StartTime, MotionTimeEnd);
					Trigger_ProcessEvents(World, TRIGGER_FROM_OBJECT, Obj, md->Motion, MotionTimeStart, EndTime);
				}
				else
				{
					Trigger_ProcessEvents(World, TRIGGER_FROM_OBJECT, Obj, md->Motion, MotionTimeStart, MotionTimeEnd);
				}
			}
		}

		if (!HasMotion)
		{
			geActor_ClearPose(Obj->Actor, &XForm);
		}
	}

	return GE_TRUE;
}

geBoolean ObjMgr_Process( geWorld *World, ObjMgr *OM, float Time )
{
	int i;
	int Count;
	DList_NodeHandle Node;
	Object *Element;

	assert( World != NULL );
	assert( OM != NULL );

	Node = 0;
	Count = DList_NodeN(OM->List);
	for( i = 0; i < Count; i++ )
	{
		DList_GetNext( OM->List, &Node, (void *)&Element );

		if (Element->Active == GE_FALSE )
			continue;

		ObjMgr_ProcessObject(  World, OM, Element, Time );
	}

	return GE_TRUE;
}

int ObjMgr_ObjectCount( ObjMgr *OM )
{
	return(DList_NodeN(OM->List));
}

Object *ObjMgr_Iterate( ObjMgr *OM, int Index)
{
	int i;
	int Count;
	DList_NodeHandle Node;
	Object *Element;

	assert( OM != NULL );

	// very inefficient - rewrite later

	Node = 0;
	Count = DList_NodeN(OM->List);
	for( i = 0; i < Count; i++ )
	{
		DList_GetNext( OM->List, &Node, (void *)&Element );

		if (i == Index)
			return Element;
	}

	return NULL;
}
	
geBoolean ObjMgr_AddObjectsToWorld(geWorld *World, ObjMgr *OM)
{
	int i;
	int Count;
	geBoolean ret;
	DList_NodeHandle Node;
	Object *Element;

	assert( World != NULL );
	assert( OM != NULL );

	Node = 0;
	Count = DList_NodeN(OM->List);
	for( i = 0; i < Count; i++ )
	{
		DList_GetNext( OM->List, &Node, (void *)&Element );
		if (Element->Actor != NULL)
		{
			ret = geWorld_AddActor(World, Element->Actor, GE_ACTOR_RENDER_NORMAL|GE_ACTOR_COLLIDE|GE_ACTOR_RENDER_MIRRORS, 0xffffffff);
			if (ret == GE_FALSE)
			{
				geErrorLog_AddString(-1,"ObjMgr_AddObjectsToWorld: Could not add actor to world.",NULL);
				return GE_FALSE;
			}
		}
	}

	return GE_TRUE;
}

geBoolean ObjMgr_RemoveObjectsFromWorld(geWorld *World, ObjMgr *OM)
{
	int i;
	int Count;
	DList_NodeHandle Node;
	Object *Element;
	geBoolean ret;

	assert( World != NULL );
	assert( OM != NULL );

	Node = 0;
	Count = DList_NodeN(OM->List);
	for( i = 0; i < Count; i++ )
	{
		DList_GetNext( OM->List, &Node, (void *)&Element );
		if (Element->Actor != NULL)
		{
			ret = geWorld_RemoveActor(World, Element->Actor);
			if (ret == GE_FALSE)
			{
				geErrorLog_AddString(-1,"ObjMgr_AddObjectsToWorld: Could not remove actor from world.",NULL);
				return GE_FALSE;
			}
		}
	}

	return GE_TRUE;
}


