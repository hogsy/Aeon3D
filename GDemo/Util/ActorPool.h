/****************************************************************************************/
/*  ActorPool.h                                                                         */
/*                                                                                      */
/*  Description:    Manages actor defs									                */
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
#ifndef ACTOR_POOL_DEFINE
#define ACTOR_POOL_DEFINE

#include "actor.h"
#include "ctypes.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct ActorPool ActorPool;

ActorPool* ActorPool_Create(void);
BOOL ActorPool_Add( ActorPool* pActorPool, char* ActorName );
geActor_Def * ActorPool_Get( ActorPool* pActorPool, char* ActorName );
BOOL ActorPool_Release( ActorPool* pActorPool, char* ActorName );
void ActorPool_Destroy( ActorPool** hActorPool );
BOOL ActorPool_Exists( ActorPool* pActorPool, char* ActorName );

#ifdef __cplusplus
}
#endif


#endif //ACTOR_POOL_DEFINE
