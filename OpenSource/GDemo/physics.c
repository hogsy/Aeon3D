/****************************************************************************************/
/*  Physics.c                                                                           */
/*                                                                                      */
/*  Author: Jason Wood, Frank Maddin                                                    */
/*  Description:    Control code for physics system. Converted from GTest               */
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

#include "errorlog.h"
#include "physics.h"
#include "objmgr.h"
#include "genvs.h"

geBoolean PhysicsObject_Spawn(geWorld *World, void* PlayerData, void* Class);
void PhysicsObject_Destroy(geWorld *World, void *PlayerData, void *ClassData);
static geBoolean PhysicsObject_Trigger(geWorld *World, void* PlayerData, void* TargetData, void* data);
static geBoolean PhysicsObject_Control(geWorld *World, void* PlayerData, float Time);

geBoolean PhysicsJoint_Spawn(geWorld *World, void* PlayerData, void* Class);
void PhysicsJoint_Destroy(geWorld *World, void *PlayerData, void *ClassData);

geBoolean PhysicalSystem_Spawn(geWorld *World, void* PlayerData, void* Class);
void PhysicalSystem_Destroy(geWorld *World, void *PlayerData, void *ClassData);
static geBoolean PhysicalSystem_Control(geWorld *World, void* PlayerData, float Time);

//=====================================================================================
//	PhysicsObject_Spawn
//=====================================================================================
geBoolean PhysicsObject_Spawn(geWorld *World, void *PlayerData, void *ClassData)
{
	geWorld_Model						*Model;
	Object									*Player;
	PhysicsObject						*po;
	geVec3d									Mins;
	geVec3d									Maxs;

	assert( World );
	assert( PlayerData );
	assert( ClassData );

	Player = (Object*)PlayerData;

	ObjMgr_SetObjectControlFunction(Player, PhysicsObject_Control);
	ObjMgr_SetObjectTriggerFunction(Player, PhysicsObject_Trigger);
	ObjMgr_SetObjectDestroyFunction(Player, PhysicsObject_Destroy);
	ObjMgr_SetObjectClassData(Player, ClassData);
	
	po = NULL;
	po = (PhysicsObject*)ClassData;

	assert(po);
	Model = po->Model;

	if (!Model)
	{
		geErrorLog_AddString(-1,"PhysicsObject_Spawn:  No model for entity.",NULL);
		return GE_FALSE;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////

	ObjMgr_SetObjectFlags(Player, TYPE_MODEL | TYPE_PHYSOB);

	#define PHYSOB_MAX_DAMPING		(1.f)
	#define PHYSOB_MIN_DAMPING		(0.f)

	if (po->linearDamping < PHYSOB_MIN_DAMPING)
		po->linearDamping = PHYSOB_MIN_DAMPING;

	else if (po->linearDamping > PHYSOB_MAX_DAMPING)
		po->linearDamping = PHYSOB_MAX_DAMPING;

	if (po->angularDamping < PHYSOB_MIN_DAMPING)
		po->angularDamping = PHYSOB_MIN_DAMPING;

	else if (po->angularDamping > PHYSOB_MAX_DAMPING)
		po->angularDamping = PHYSOB_MAX_DAMPING;


	
	geWorld_ModelGetBBox(World, po->Model, &Mins, &Maxs);

	po->stateInfo = gePhysicsObject_Create(&po->Origin,
																po->mass,
																po->isAffectedByGravity,
																po->respondsToForces,
																po->linearDamping,
																po->angularDamping,
																&Mins,
																&Maxs,
																0.01f);
																//po->physicsScale);

	if (po->stateInfo == NULL)
	{
		geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"PhysicsObject_Spawn:  Couldn't Create().",NULL);
		return GE_FALSE;
	}

	ObjMgr_SetObjectUserData(Player, po->stateInfo);

	geWorld_ModelSetUserData(Model, (void*)Player);
	ObjMgr_SetObjectModel(Player, Model);
	
	return GE_TRUE;
}

void PhysicsObject_Destroy(geWorld *World, void *PlayerData, void *ClassData)
{
	Object								*Player;
	PhysicsObject						*po;

	assert( World );
	assert( PlayerData );
	assert( ClassData );

	Player = (Object*)PlayerData;
	po = (PhysicsObject*)ClassData;

	assert(po);
	assert(po->stateInfo);

	gePhysicsObject_Destroy(&po->stateInfo);
}

static geBoolean PhysicsObject_Trigger(geWorld *World, void *PlayerData, Object *Target, void *data)
{
	Object		*Player;
	PhysicsObject* poPtr;
	gePhysicsObject* podPtr;
	GE_Collision* collisionInfo;
	float velMagN;
	geVec3d radiusVector;
	geVec3d force;
	geVec3d poCOM;
	int activeConfigIndex;
	float scale;
	geVec3d Velocity;

	assert( World );
	assert( PlayerData );
	assert( Target );

	Player = (Object*)PlayerData;
	poPtr = (PhysicsObject*)ObjMgr_GetObjectClassData(Player);
	podPtr = (gePhysicsObject*)ObjMgr_GetObjectUserData(Player);

	collisionInfo = (GE_Collision*)data;

	scale = gePhysicsObject_GetPhysicsScale(podPtr);
	activeConfigIndex = gePhysicsObject_GetActiveConfig(podPtr);
	gePhysicsObject_GetLocationInEditorSpace(podPtr, &poCOM, activeConfigIndex);
	geVec3d_Subtract(&collisionInfo->Impact, &poCOM, &radiusVector);

	ObjMgr_GetObjectVelocity(Target, &Velocity);
	velMagN = geVec3d_DotProduct(&Velocity, &collisionInfo->Plane.Normal);

	geVec3d_Scale(&collisionInfo->Plane.Normal, velMagN * 4.f, &force);
	geVec3d_Scale(&radiusVector, scale, &radiusVector);

	gePhysicsObject_ApplyGlobalFrameForce(podPtr, &force, &radiusVector, GE_TRUE, activeConfigIndex);
	
	return GE_TRUE;

}

geVec3d bboxVerts[8] = 
{
	{-1.0f, 1.0f, 1.0f},
	{-1.0f, -1.0f, 1.0f},
	{1.0f, -1.0f, 1.0f},
	{1.0f, 1.0f, 1.0f},
	{-1.0f, 1.0f, -1.0f},
	{-1.0f, -1.0f, -1.0f},
	{1.0f, -1.0f, -1.0f},
	{1.0f, 1.0f, -1.0f}
};

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

static geBoolean PhysicsObject_Control(geWorld *World, void* PlayerData, float Time)
{

	Object* player;
	gePhysicsObject* pod;
	int activeConfigIndex;
	geXForm3d destXForm;

	assert( World );
	player = (Object*)PlayerData;
	assert(player != NULL);

	pod = (gePhysicsObject*)ObjMgr_GetObjectUserData(player);
	assert(pod != NULL);
	activeConfigIndex = gePhysicsObject_GetActiveConfig(pod);

	// update model's xform
	gePhysicsObject_GetXFormInEditorSpace(pod, &destXForm, activeConfigIndex);

	geWorld_SetModelXForm(World, ObjMgr_GetObjectModel(player), &destXForm);
	ObjMgr_SetObjectXForm(player, &destXForm);

	return GE_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//=====================================================================================
//	PhysicsJoint_Spawn
//=====================================================================================
geBoolean PhysicsJoint_Spawn(geWorld *World, void *PlayerData, void *ClassData)
{
	Object			*Player;
	PhysicsJoint	*ij;

	assert(World);
	assert(PlayerData);
	assert(ClassData);

	Player = (Object*)PlayerData;
	
	ij = NULL;
	ij = (PhysicsJoint*)ClassData;

	ObjMgr_SetObjectDestroyFunction(Player, PhysicsJoint_Destroy);
	ObjMgr_SetObjectClassData(Player, ClassData);

	#define JOINT_MIN_ASSEMBLY_RATE		(0.01f)
	#define JOINT_MAX_ASSEMBLY_RATE		(1.f)

	if (ij->assemblyRate < JOINT_MIN_ASSEMBLY_RATE)
		ij->assemblyRate = JOINT_MIN_ASSEMBLY_RATE;

	else if (ij->assemblyRate > JOINT_MAX_ASSEMBLY_RATE)
		ij->assemblyRate = JOINT_MAX_ASSEMBLY_RATE;

	ij->jointData = NULL;
	ij->jointData = gePhysicsJoint_Create(ij->jointType,
																 &ij->Origin,
																 ij->assemblyRate,
																 ij->physicsObject1 ? ij->physicsObject1->stateInfo : NULL,
																 ij->physicsObject2 ? ij->physicsObject2->stateInfo : NULL,
																 0.01f);
																 //ij->physicsScale);
	if (ij->jointData == NULL)
	{
		geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"PhysicsJoint_Spawn:  Couldn't Create().",NULL);
		return GE_FALSE;
	}

	return GE_TRUE;
}

//=====================================================================================
//=====================================================================================
void PhysicsJoint_Destroy(geWorld *World, void *PlayerData, void *ClassData)
{
	Object			*Player;
	PhysicsJoint	*ij;

	assert( World );
	assert( PlayerData );
	assert( ClassData );

	Player = (Object*)PlayerData;
	
	ij = NULL;
	ij = (PhysicsJoint*)ClassData;
	
	assert(ij->jointData);

	gePhysicsJoint_Destroy(&ij->jointData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//=====================================================================================
//	PhysicalSystem_Spawn
//=====================================================================================
geBoolean PhysicalSystem_Spawn(geWorld *World, void *PlayerData, void *ClassData)
{
	Object							*Player;
	PhysicalSystem					*ips;
	PhysicsObject					*Object;
	PhysicsJoint					*PhysicsJoint;
	geXForm3d						XForm;

	assert(World != NULL);
	assert(PlayerData != NULL);
	assert(ClassData != NULL);

	Player = PlayerData;

	ObjMgr_SetObjectControlFunction(Player, PhysicalSystem_Control);
	ObjMgr_SetObjectDestroyFunction(Player, PhysicalSystem_Destroy);

	ObjMgr_SetObjectClassData(Player, ClassData);

	geXForm3d_SetIdentity(&XForm);
	ObjMgr_SetObjectXForm(Player, &XForm);

	ips = NULL;
	ips = (PhysicalSystem*)ClassData;	

	ips->physsysData = NULL;

	ips->physsysData = gePhysicsSystem_Create();
	if (ips->physsysData == NULL)
	{
		geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"PhysicalSystem_Spawn:  Couldn't Create().",NULL);
		return GE_FALSE;
	}
	Object = ips->physicsObjectListHeadPtr;
	while	(Object)
	{
		if	(gePhysicsSystem_AddObject(ips->physsysData, Object->stateInfo) == GE_FALSE)
		{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"PhysicalSystem_Spawn:  Couldn't AddObject().",NULL);
			return GE_FALSE;
		}
		Object = Object->Next;
	}

	PhysicsJoint = ips->jointListHeadPtr;
	while	(PhysicsJoint)
	{
		if	(gePhysicsSystem_AddJoint(ips->physsysData, PhysicsJoint->jointData) == GE_FALSE)
		{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"PhysicalSystem_Spawn:  Couldn't AddPhysicsJoint().\n",NULL);
			return GE_FALSE;
		}
		PhysicsJoint = PhysicsJoint->Next;
	}

	return GE_TRUE;
}

//=====================================================================================
//=====================================================================================
void PhysicalSystem_Destroy(geWorld *World, void *PlayerData, void *ClassData)
{
	Object				*Player;
	PhysicalSystem		*ips;

	assert( World );
	assert( PlayerData );
	assert( ClassData );

	Player = (Object*)PlayerData;

	ips = (PhysicalSystem*)ClassData;	

	assert(ips);

	assert(ips->physsysData);

	gePhysicsSystem_Destroy(&ips->physsysData);
}

//=====================================================================================
//=====================================================================================
static geBoolean PhysicalSystem_Control(geWorld *World, void* PlayerData, float Time)
{
	Object* player;
	PhysicalSystem* psPtr;

	assert( World );
	assert( PlayerData );

	player = (Object*)PlayerData;
	psPtr = (PhysicalSystem*)ObjMgr_GetObjectClassData(player);

	if (!gePhysicsSystem_Iterate(psPtr->physsysData, Time))
	{
		geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE,"PhysicalSystem_Control: Iterate() failed.\n",NULL);
		return GE_FALSE;
	}	

	return GE_TRUE;
}

