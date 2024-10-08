/****************************************************************************************/
/*  Trigger.h                                                                           */
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
#ifndef TRIGGER_H
#define TRIGGER_H

#include "genesis.h"
#include "objmgr.h"
#include "SPool.h"
#include "TPool.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	TRIGGER_FROM_MODEL = 0,
	TRIGGER_FROM_ACTOR,
	TRIGGER_FROM_ENTITY,
	TRIGGER_FROM_XFORM,
	TRIGGER_FROM_OBJECT,
	TRIGGER_FROM_OBJECT_XFORM,
}	TriggerContext;

geBoolean Trigger_Set(ObjMgr *OM, SoundPool *SPool, TexturePool *TPool, void *ESystem, void *EManager);
void Trigger_ParseEvent(geWorld *World, TriggerContext Type, void *ContextData, char *EventString);
void Trigger_ProcessEvents(geWorld *World, TriggerContext Type, void *ContextData, geMotion *Motion, float StartTime, float EndTime);

#ifdef __cplusplus
}
#endif

#endif