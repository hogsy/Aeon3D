////////////////////////////////////////////////////////////////////////////////////////
//	EffectPosE.h
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
#ifndef EFFECTPOSE_H
#define EFFECTPOSE_H

#include "Genesis.h"
#include "ActorStart.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_EffectPos.bmp")
typedef struct EM_EffectPos
{
#pragma	GE_Private

#pragma GE_Published
	geVec3d			Position;
	geWorld_Model	*Model;
	ActorStart		*AStart;
	char			*BoneName;

#pragma GE_Origin( Position )
#pragma GE_DefaultValue( BoneName, "EFFECT_BONE01" )

#pragma GE_Documentation( Position, "Location data (checked last)" )
#pragma GE_Documentation( Model, "Model to use for location data (optional, checked second)" )
#pragma GE_Documentation( AStart, "Actor on which a bone will represent the position (optional, checked first)" )
#pragma GE_Documentation( BoneName, "Which bone on the actor to use as position (optional)" )

} EM_EffectPos;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
