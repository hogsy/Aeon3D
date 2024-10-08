////////////////////////////////////////////////////////////////////////////////////////
//	ActorSettingsE.h
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
#ifndef ACTORSETTINGSE_H
#define ACTORSETTINGSE_H

#include "Genesis.h"
#include "ActorStart.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_ActorSettings.bmp")
typedef struct EM_ActorSettings
{
#pragma	GE_Private

#pragma GE_Published
	geVec3d		Position;
	ActorStart	*AStart;
    GE_RGBA		AmbientLight;
    GE_RGBA		FillLight;
	geVec3d		FillLightNormal;
	geBoolean	PerBoneLighting;
	float		TotalLife;
	char		*LString;
	float		LTime;
	float		LLoopStyle;
	geBoolean	FillNormalActorRelative;

#pragma GE_Origin(Position)			
#pragma GE_DefaultValue( AmbientLight, "25.5 25.5 25.5" )
#pragma GE_DefaultValue( FillLight, "63.75 63.75 63.75" )
#pragma GE_DefaultValue( FillLightNormal, "-0.18 0.91 0.37" )
#pragma GE_DefaultValue( PerBoneLighting, "0" )
#pragma GE_DefaultValue( TotalLife, "0.0" )
#pragma GE_DefaultValue( LTime, "0.0" )
#pragma GE_DefaultValue( LLoopStyle, "0" )
#pragma GE_DefaultValue( FillNormalActorRelative, "1" )

#pragma GE_Documentation( AmbientLight, "Ambient RGB" )
#pragma GE_Documentation( FillLight, "Fill RGB" )
#pragma GE_Documentation( FillLightNormal, "Normal towards light" )
#pragma GE_Documentation( PerBoneLighting, "Better quality but slower lighting" )
#pragma GE_Documentation( TotalLife, "How many second this effect will last" )
#pragma GE_Documentation( LTime, "How many second to cycle thru light string" )
#pragma GE_Documentation( LLoopStyle, "0 = Restart, 1 = Back and forth" )
#pragma GE_Documentation( FillNormalActorRelative, "0 = Fill normal in world frame, 1 = Fill normal in actor frame" )

} EM_ActorSettings;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
