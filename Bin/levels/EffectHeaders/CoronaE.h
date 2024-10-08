////////////////////////////////////////////////////////////////////////////////////////
//	CoronaE.h
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
#ifndef CORONAE_H
#define CORONAE_H

#include "Genesis.h"
#include "ActorStart.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_Corona.bmp")
typedef struct EM_Corona
{
#pragma	GE_Private
	int			TextureNumber;

#pragma GE_Published
	geVec3d		Position;
	ActorStart	*AStart;
	float		FadeTime;
    float		MinRadius;
    float		MaxRadius;
	float		MaxRadiusDistance;
	float		MinRadiusDistance;
	float		MaxVisibleDistance;
    GE_RGBA		Color;
	int			LightType;
	char		*LightTypeSignal;
	char		*BmpName;
	char		*AlphaName;
	char		*TriggerName;
	char		*BoneName;


#pragma GE_Origin( Position )
#pragma GE_DefaultValue( TextureNumber, "0" )
#pragma GE_DefaultValue( FadeTime, "0.15" )
#pragma GE_DefaultValue( MinRadius, "1.0" )
#pragma GE_DefaultValue( MaxRadius, "20.0" )
#pragma GE_DefaultValue( MaxRadiusDistance, "1000.0" )
#pragma GE_DefaultValue( MinRadiusDistance, "100.0" )
#pragma GE_DefaultValue( MaxVisibleDistance, "2000.0" )
#pragma GE_DefaultValue( Color, "255.0 255.0 255.0" )
#pragma GE_DefaultValue( LightType, "-1" )
#pragma GE_DefaultValue( TriggerName, "NULL" )
#pragma GE_DefaultValue( BoneName, "EFFECT_BONE01" )

#pragma GE_Documentation( Position, "Location of effect, if it's not hooked to an actor" )
#pragma GE_Documentation( AStart, "Actor that effect is hooked to" )
#pragma GE_Documentation( TextureNumber, "Which texture to use" )
#pragma GE_Documentation( FadeTime, "How long the fade takes to drop to zero visibility (seconds)" )
#pragma GE_Documentation( MinRadius, "Minimum radius corona will ever drop to (texels)" )
#pragma GE_Documentation( MaxRadius, "Maximum radius corona will ever increase to (texels)" )
#pragma GE_Documentation( MaxRadiusDistance, "Above this distance, corona is capped at MaxRadius (world units)" )
#pragma GE_Documentation( MinRadiusDistance, "Below this distance, corona is capped at MinRadius (world units)" )
#pragma GE_Documentation( MaxVisibleDistance, "Maximum distance corona is visible at (world units)" )
#pragma GE_Documentation( Color, "Texture color" )
#pragma GE_Documentation( LightType, "World light type that the corona will monitor to determine if it should go on" )
#pragma GE_Documentation( LightTypeSignal, "Letter above which the corona will go on" )
#pragma GE_Documentation( BmpName, "Name of bitmap file to use" )
#pragma GE_Documentation( AlphaName, "Name of alpha bitmap file to use" )
#pragma GE_Documentation( TriggerName, "Name of the associated trigger" )
#pragma GE_Documentation( BoneName, "Which bone on the actor to hook to" )

} EM_Corona;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
