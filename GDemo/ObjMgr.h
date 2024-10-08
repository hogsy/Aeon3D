/****************************************************************************************/
/*  ObjMgr.h                                                                            */
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

#ifndef __OBJMGR_H__
#define __OBJMGR_H__

#include "genesis.h"
#include "actor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Object Object;
typedef struct ObjMgr ObjMgr;

typedef struct MotionData
{
	geMotion	*Motion;
	float		BlendAmt;
	float		TimeScale;
	float		FrameTime;
	geBoolean   Loop;
	geBoolean	Active;
}MotionData;

typedef geBoolean Control(geWorld*, Object *, float);
typedef Control *ControlP;

typedef geBoolean Trigger(geWorld*, Object *, Object *, void *);
typedef Trigger *TriggerP;

typedef void Destroy(geWorld*, void *, void *);
typedef Destroy *DestroyP;

#define STATE_Closed 0
#define STATE_Opened 1

#define MODEL_OPEN (1<<0)
#define TYPE_MODEL (1<<1)
#define TYPE_TOUCH (1<<2)
#define TYPE_PHYSOB (1<<3)

//External Interface
///////////////////////////////////////////////////
ObjMgr *	ObjMgr_Create( void );
void		ObjMgr_ListDestroyCB( void* NodeData, void* Context );
void		ObjMgr_Destroy( ObjMgr **pOM );
Object *	ObjMgr_AddObject(ObjMgr *OM, geWorld *World, char *Name, geActor *Actor, geMotion *Motion, char *MotionBone, geXForm3d *XForm);
geBoolean	ObjMgr_Process( geWorld *World, ObjMgr *OM, float Time );
void		ObjMgr_GetObjectXForm(  Object* Obj, geXForm3d *XForm);
void		ObjMgr_SetObjectXForm(  Object* Obj, geXForm3d *XForm);
geBoolean	ObjMgr_ObjectHasMotion(  Object* Obj);
Object *	ObjMgr_GetObjectByName(geWorld *World, ObjMgr* OM, char *ActorName);
geActor *	ObjMgr_GetObjectActor(  Object* Obj);

geBoolean	ObjMgr_SetObjectActive(Object *Obj, geBoolean State);
geBoolean	ObjMgr_SetObjectMotionByName(Object *Obj, char *MotionName);

geBoolean	ObjMgr_SetMotionScale(Object *Obj, float Scale);

geBoolean	ObjMgr_AddObjectsToWorld(geWorld *World, ObjMgr *OM);
geBoolean	ObjMgr_RemoveObjectsFromWorld(geWorld *World, ObjMgr *OM);


geBoolean		ObjMgr_SetObjectModel(Object *Obj, geWorld_Model *Model);
geWorld_Model *	ObjMgr_GetObjectModel(Object *Obj);
geBoolean		ObjMgr_SetObjectFlags(Object *Obj, int32 Flags);
int32			ObjMgr_GetObjectFlags(Object *Obj);
geBoolean		ObjMgr_SetObjectState(Object *Obj, int32 State);
int32			ObjMgr_GetObjectState(Object *Obj);
geBoolean		ObjMgr_SetObjectFrameTime(Object *Obj, float FrameTime);
float			ObjMgr_GetObjectFrameTime(Object *Obj);

geBoolean	ObjMgr_SetObjectControlFunction(Object *Obj, ControlP ControlFunction);
geBoolean	ObjMgr_SetObjectTriggerFunction(Object *Obj, TriggerP TriggerFunction);

TriggerP	ObjMgr_GetObjectTriggerFunction(Object *Obj);

geBoolean	ObjMgr_SetObjectUserData(Object *Obj, void *UserData);
geBoolean	ObjMgr_SetObjectClassData(Object *Obj, void *ClassData);
geBoolean	ObjMgr_SetObjectVelocity(Object *Obj, geVec3d *Velocity);
void *		ObjMgr_GetObjectUserData(Object *Obj);
void *		ObjMgr_GetObjectClassData(Object *Obj);
void		ObjMgr_GetObjectVelocity(Object *Obj, geVec3d *Velocity);

void		ObjMgr_SetObjectDestroyFunction(Object *Obj, DestroyP DestroyFunction);
DestroyP	ObjMgr_GetObjectDestroyFunction(Object *Obj);

geBoolean	ObjMgr_SetObjectMotionLoop(Object *Obj, int32 State);

geBoolean	ObjMgr_SetObjectPathMotion(Object *Obj, geMotion *Path, geBoolean Loop);
geBoolean	ObjMgr_GetObjectPathMotion(Object *Obj, geMotion **Path, geBoolean *Loop);

geBoolean	ObjMgr_SetObjectMotionData(Object *Obj, int MotionIndex, MotionData *MotionData);
MotionData *ObjMgr_GetObjectMotionData(Object *Obj, int MotionIndex);
Object *	ObjMgr_GetObjectByTriggerName(geWorld *World, ObjMgr* OM, char *ActorName);
void		ObjMgr_FreeWorldData( geWorld *World, ObjMgr *pOM );

geBoolean	ObjMgr_SetObjectPathState(Object *Obj, geBoolean State);
geBoolean	ObjMgr_GetObjectPathState(Object *Obj);

Object *	ObjMgr_GetObjectByActor(ObjMgr* OM, geActor *Actor);
Object *	ObjMgr_GetObjectByClassData(ObjMgr* OM, void *);
void		ObjMgr_ResetObject(Object *Obj);
geBoolean	ObjMgr_SetObjectMotion(Object *Obj, geMotion *Motion);
int			ObjMgr_ObjectCount( ObjMgr *OM );
Object *	ObjMgr_Iterate( ObjMgr *OM, int Index);
char*		ObjMgr_GetObjectTriggerName(Object *Obj);

geBoolean	ObjMgr_SetObjectPathFrameTime(Object *Obj, float FrameTime);
float		ObjMgr_GetObjectPathFrameTime(Object *Obj);

geBoolean	ObjMgr_ObjectGetContents(geWorld *World, Object *Obj, float Time);

#ifdef __cplusplus
}
#endif

#endif