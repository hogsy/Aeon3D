////////////////////////////////////////////////////////////////////////////////////////
//	ColorCycleE.h
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
#ifndef COLORCYCLEE_H
#define COLORCYCLEE_H

#include "Genesis.h"
#include "ActorStart.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_ColorCycle.bmp")
typedef struct EM_ColorCycle
{
#pragma	GE_Private
	geActor		*Actor;

#pragma GE_Published
	geVec3d		Position;
	ActorStart	*AStart;
	int			BlendStages;
	float		DelayToNextStage;
	char		*BitmapToAttachTo;
	char		*BmpWithEndPalette;
	geBoolean	Loop;
	char		*TriggerName;

#pragma GE_Origin(Position)			
#pragma GE_DefaultValue( BlendStages, "8" )
#pragma GE_DefaultValue( DelayToNextStage, "1.0" )
#pragma GE_DefaultValue( Loop, "1" )
#pragma GE_DefaultValue( TriggerName, "NULL" )

#pragma GE_Documentation( Position, "Location of effect" )
#pragma GE_Documentation( AStart, "Actor whose texture the effect will attach to (optional)" )
#pragma GE_Documentation( BlendStages, "How many different stages of blending there will be" )
#pragma GE_Documentation( DelayToNextStage, "How many seconds to wait before going to next stage" )
#pragma GE_Documentation( BitmapToAttachTo, "Which world or actor bitmap the effect will attach to" )
#pragma GE_Documentation( BmpWithEndPalette, "Bitmap which contains end palette" )
#pragma GE_Documentation( Loop, "Whether or not to keep looping the morph animation" )
#pragma GE_Documentation( TriggerName, "Name of the associated trigger" )

} EM_ColorCycle;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif