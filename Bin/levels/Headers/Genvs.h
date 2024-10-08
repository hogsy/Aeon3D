////////////////////////////////////////////////////////////////////////////////////////
//	Genvs.h
//                                                                                      
//  The contents of this file are subject to the Genesis3D Public License               
//  Version 1.01 (the "License"); you may not use this file except in                   
//  compliance with the License. You may obtain a copy of the License at                
//  http://www.genesis3d.com                                                            
//                                                                                      
//  Software distributed under the License is distributed on an "AS IS"                 
//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                
//  the License for the specific language governing rights and limitations              
//  under the License.                                                                  
//                                                                                      
//  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           
//                                                                                      
////////////////////////////////////////////////////////////////////////////////////////
#ifndef GENVS_H
#define GENVS_H

#include "PhysicsObject.h"
#include "PhysicsJoint.h"
#include "PhysicsSystem.h"

#pragma warning( disable : 4068 )
#ifdef __cplusplus
extern "C" {
#endif

////////////////////////
// PhysicsObject
////////////////////////
typedef struct PhysicsObject PhysicsObject;
#pragma GE_Type("Model.ico")
typedef struct	PhysicsObject
{
#pragma GE_Published
	geWorld_Model	*Model;
	geVec3d			Origin;
	float			mass;
	int				isAffectedByGravity;
	int				respondsToForces;
	float			linearDamping;
	float			angularDamping;
	PhysicsObject	*Next;
	float			physicsScale;
#pragma GE_DefaultValue(mass, "10.0")
#pragma GE_DefaultValue(isAffectedByGravity, "1")
#pragma GE_DefaultValue(respondsToForces, "1")
#pragma GE_DefaultValue(linearDamping, "0.0005")
#pragma GE_DefaultValue(angularDamping, "0.0005")
#pragma GE_DefaultValue(physicsScale, "0.01")

#pragma GE_Private
	gePhysicsObject *stateInfo;
#pragma GE_Origin(Origin)
}	PhysicsObject;

typedef struct PhysicsJoint PhysicsJoint;

////////////////////////
// Joint
////////////////////////
#pragma GE_Type("Item.ico")
typedef struct  PhysicsJoint
{
#pragma GE_Published
		geVec3d				Origin;
		PhysicsObject *		physicsObject1;
		PhysicsObject *		physicsObject2;
		PhysicsJoint *		Next;
		float				assemblyRate;
		int					jointType;
		float				physicsScale;
#pragma GE_DefaultValue(assemblyRate, "0.03")
#pragma GE_DefaultValue(physicsScale, "0.01")

#pragma GE_Private
		gePhysicsJoint *	jointData;
#pragma GE_Origin(Origin)
}   PhysicsJoint;

#pragma GE_Type("Item.ico")

////////////////////////
// PhysicalSystem
////////////////////////
typedef struct PhysicalSystem
{
#pragma GE_Published
	geVec3d					Origin;
	PhysicsObject		*	physicsObjectListHeadPtr;
	PhysicsJoint		*	jointListHeadPtr;
#pragma GE_Private
	gePhysicsSystem		*	physsysData;
#pragma GE_Origin(Origin)
} PhysicalSystem;

////////////////////////
// DeathMatchstart
////////////////////////
#pragma GE_Type("Player.ico")
typedef struct  DeathMatchStart
{
#pragma GE_Published
    geVec3d     Origin;
#pragma GE_Origin(Origin)
}   DeathMatchStart;

#pragma GE_Type("Item.ico")

typedef struct ForceField
{
#pragma GE_Published
	geVec3d				Origin;
	float				radius;
	float				strength;
	int					falloffType;
	int					affectsPlayers;
	int					affectsPhysicsObjects;
#pragma GE_DefaultValue(radius, "50.0")
#pragma GE_DefaultValue(falloffType, "1")
#pragma GE_DefaultValue(affectsPlayers, "1")
#pragma GE_DefaultValue(affectsPhysicsObjects, "1")
#pragma GE_Origin(Origin)
} ForceField;

////////////////////////
// Door
////////////////////////
#pragma GE_Type("Model.ico")
typedef struct  Door
{
#pragma GE_Published
    geWorld_Model   *Model;
    geVec3d			Origin;
	float			Radius;
#pragma GE_Origin(Origin)
#pragma GE_DefaultValue(Radius, "300.0")
}   Door;

////////////////////////
// ModelTrigger
////////////////////////
#pragma GE_Type("Model.ico")
typedef struct  ModelTrigger
{
#pragma GE_Published
    geWorld_Model   *Model;
    geVec3d			Origin;
	char *			TriggerName;
#pragma GE_Origin(Origin)
#pragma GE_DefaultValue(TriggerName, "NULL")
}   ModelTrigger;

////////////////////////
// MovingPlat
////////////////////////
#pragma GE_Type("Model.ico")
typedef struct  MovingPlat
{
#pragma GE_Published
    geWorld_Model	*Model;
    geVec3d			Origin;
#pragma GE_Origin(Origin)
}   MovingPlat;

////////////////////////
// ChangeLevel
////////////////////////
#pragma GE_Type("Player.ico")
typedef struct ChangeLevel
{
#pragma GE_Published
    geWorld_Model	*Model;
	char			*LevelName;
    geVec3d			Origin;
#pragma GE_Origin(Origin)
} ChangeLevel;


#ifdef __cplusplus
}
#endif

#pragma warning( default : 4068 )

#endif
