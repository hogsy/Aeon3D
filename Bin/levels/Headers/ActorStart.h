////////////////////////////////////////////////////////////////////////////////////////
//	ActorStart.h
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
#ifndef ACTORSTART_H
#define ACTORSTART_H

#include "Genesis.h"
#pragma warning( disable : 4068 )

typedef struct PathPoint PathPoint;

#ifdef __cplusplus
	extern "C" {
#endif

#pragma GE_Type("ActorAnim.ico")
typedef struct ActorAnim
{
#pragma GE_Private
	geMotion	*Motion;

#pragma GE_Published
    geVec3d		origin;
	int			Index;
	char *		MotionName;
	char *		TriggerName;
	float		BlendAmount;
	float		MotionScale;
	geBoolean   Loop;
	int			TriggerCount;
	int			SyncIndex;
#pragma GE_Origin(origin)
#pragma GE_DefaultValue(Index, "0")
#pragma GE_DefaultValue(MotionName, "")
#pragma GE_DefaultValue(TriggerName, "NULL")
#pragma GE_DefaultValue(BlendAmount, "1.0")
#pragma GE_DefaultValue(MotionScale, "1.0")
#pragma GE_DefaultValue(Loop, "1") 
#pragma GE_DefaultValue(TriggerCount, "1") 
#pragma GE_DefaultValue(SyncIndex, "-1") 

#pragma GE_Documentation(Index, "Motions have 0 to 3 indexes.  The first active motion is set and subsequent active motions are blended into this.") 
#pragma GE_Documentation(MotionName, "Motion to play - must exist for the actor.") 
#pragma GE_Documentation(TriggerName, "Used to match Events.")
#pragma GE_Documentation(BlendAmount, "Percent to blend with other motions.") 
#pragma GE_Documentation(MotionScale, "Speed to play the motion") 
#pragma GE_Documentation(Loop, "Loop the motion?  Non looping motions go inactive when finished.") 
#pragma GE_Documentation(TriggerCount, "Number of times this will trigger.") 
#pragma GE_Documentation(SyncIndex, "Sync the frame time to another motion index")
}	ActorAnim;

#pragma GE_Type("PathPoint.ico")
typedef struct	PathPoint
{
#pragma GE_Published
    geVec3d		origin;
	char *		Event;
	float		Time;
	PathPoint * Next;
	geVec3d		angles;
	geBoolean	FirstPoint;
#pragma GE_Angles(angles)
#pragma GE_Origin(origin)
#pragma GE_DefaultValue(Event, "") 
#pragma GE_DefaultValue(FirstPoint, "0")
}	PathPoint;


////////////////////////////////////////////////////////////////////////////////////////
//	ActorStart info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("ActorStart.ico")
typedef struct	ActorStart
{
#pragma GE_Private
	geActor		*Actor;
#pragma GE_Published
	geVec3d		origin;
	char *		Name;
	char *		Motion;
	char *		MotionBone;
	float		MotionScale;
	geBoolean	MotionLooping;
	geVec3d		angles;
	geVec3d		Scale;
	char *		TriggerName;
	PathPoint	*Path;
	int			PathLoop;
	int			PathActive;
	int			InActive;
	int			RenderBoxOn;

#pragma GE_Angles(angles)
#pragma GE_Origin(origin)
#pragma GE_DefaultValue(Name, "") 
#pragma GE_DefaultValue(Motion, "") 
#pragma GE_DefaultValue(MotionBone, "") 
#pragma GE_DefaultValue(MotionLooping, "1") 
#pragma GE_DefaultValue(Scale, "1.0 1.0 1.0") 
#pragma GE_DefaultValue(MotionScale, "1.0") 
#pragma GE_DefaultValue(TriggerName, "NULL")
#pragma GE_DefaultValue(PathActive, "0")
#pragma GE_DefaultValue(InActive, "0")
#pragma GE_DefaultValue(PathLoop, "0")
#pragma GE_DefaultValue(RenderBoxOn, "1") 

#pragma GE_Documentation(Name, "Actor file name NOT including the path and NO .ACT extension") 
#pragma GE_Documentation(Motion, "Starting motion name of the actor.") 
#pragma GE_Documentation(MotionBone, "If set then translation info is taken off of the bone of the actor.") 
#pragma GE_Documentation(MotionLooping, "Loop the current motion?") 
#pragma GE_Documentation(Scale, "Size the actor.") 
#pragma GE_Documentation(MotionScale, "Speed of motion playback.  Global for motions of the same actor.") 
#pragma GE_Documentation(TriggerName, "Used to match an effects TriggerName.")
#pragma GE_Documentation(Path, "Point to the a path.  Must be the first path point.")
#pragma GE_Documentation(PathActive, "Start out traversing path? 0 = False, 1 = True.")
#pragma GE_Documentation(PathLoop, "Loop the path? 0 = False, 1 = True.")
#pragma GE_Documentation(RenderBoxOn, "1 = On - use box, 0 = Off - draws all the time")

}	ActorStart;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
