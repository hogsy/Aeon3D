////////////////////////////////////////////////////////////////////////////////////////
//	ChaosE.h
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
#ifndef CHAOSE_H
#define CHAOSE_H

#include "Genesis.h"
#include "ActorStart.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_Chaos.bmp")
typedef struct EM_Chaos
{
#pragma	GE_Private
	geActor		*Actor;

#pragma GE_Published
	geVec3d		Position;
	ActorStart	*AStart;
	int			MaxXSway;
	int			MaxYSway;
	float		XStep;
	float		YStep;
	char		*AttachBmp;
	char		*TriggerName;

#pragma GE_Origin(Position)			
#pragma GE_DefaultValue( MaxXSway, "10" )
#pragma GE_DefaultValue( MaxYSway, "10" )
#pragma GE_DefaultValue( XStep, "1.0" )
#pragma GE_DefaultValue( YStep, "1.0" )

#pragma GE_Documentation( AStart, "Actor whose texture the effect will attach to (optional)" )
#pragma GE_Documentation( MaxXSway, "Total horizontal texture pixel sway" )
#pragma GE_Documentation( MaxYSway, "Total vertical texture pixel sway" )
#pragma GE_Documentation( XStep, "Horizontal scroll speed" )
#pragma GE_Documentation( YStep, "Vertical scroll speed" )
#pragma GE_Documentation( AttachBmp, "Name of the textureto attach this effect to" )
#pragma GE_Documentation( AttachBmp, "Name of map bitmap to use" )
#pragma GE_Documentation( TriggerName, "Name of the associated trigger" )

} EM_Chaos;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
