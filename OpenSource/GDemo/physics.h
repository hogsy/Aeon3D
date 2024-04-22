/****************************************************************************************/
/*  Physics.h                                                                           */
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
#ifndef PHYSICS_H
#define PHYSICS_H

#include "genesis.h"
#include "objmgr.h"

#ifdef __cplusplus
extern "C" {
#endif

void PhysicsObject_Destroy(geWorld *World, void *PlayerData, void *ClassData);
void PhysicsJoint_Destroy(geWorld *World, void *PlayerData, void *ClassData);
void PhysicalSystem_Destroy(geWorld *World, void *PlayerData, void *ClassData);

geBoolean Physics_EntityGetPhysicsObjects( geWorld *World, ObjMgr *ObjectList);
geBoolean Physics_EntityGetPhysicsJoints(geWorld *World, ObjMgr *ObjectList);
geBoolean Physics_EntityGetPhysicalSystem(geWorld *World, ObjMgr *ObjectList);
geBoolean Physics_EntityGetForceField(geWorld *World, ObjMgr *ObjectList);

geBoolean PhysicsObject_Spawn(geWorld *World, void* PlayerData, void* Class);
geBoolean PhysicsJoint_Spawn(geWorld *World, void* PlayerData, void* Class);
geBoolean PhysicalSystem_Spawn(geWorld *World, void* PlayerData, void* Class);
geBoolean ForceField_Spawn(geWorld *World, void* PlayerData, void* Class);

#ifdef __cplusplus
}
#endif


#endif
