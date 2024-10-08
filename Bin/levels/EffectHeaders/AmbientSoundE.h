////////////////////////////////////////////////////////////////////////////////////////
//	AmbientSoundE.h
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
#ifndef AMBIENTSOUNDE_H
#define AMBIENTSOUNDE_H

#include "Genesis.h"
#include "ActorStart.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_AmbientSound.bmp")
typedef struct EM_AmbientSound
{
#pragma	GE_Private
	int			SoundNumber;

#pragma GE_Published
	geVec3d		Position;
	ActorStart	*AStart;
	float		MinDistance;
	char		*SoundName;
	char		*TriggerName;
	geBoolean	IgnoreObstructions;
	geBoolean	LoopSound;

#pragma GE_Origin( Position )
#pragma GE_DefaultValue( SoundNumber, "0" )
#pragma GE_DefaultValue( MinDistance, "500.0" )
#pragma GE_DefaultValue( TriggerName, "NULL" )
#pragma GE_DefaultValue( IgnoreObstructions, "0" )
#pragma GE_DefaultValue( LoopSound, "1" )

#pragma GE_Documentation( Position, "Location of effect, if it's not hooked to an actor" )
#pragma GE_Documentation( AStart, "Actor that effect is hooked to" )
#pragma GE_Documentation( SoundNumber, "Which sound to use" )
#pragma GE_Documentation( MinDistance, "Min distance within which sound is at max volume" )
#pragma GE_Documentation( TriggerName, "Name of the associated trigger" )
#pragma GE_Documentation( IgnoreObstructions, "If obstructions should be ignored when computing the sound data" )
#pragma GE_Documentation( LoopSound, "Whether or not sound should loop" )

} EM_AmbientSound;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
