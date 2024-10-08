////////////////////////////////////////////////////////////////////////////////////////
//	SpoutE.h
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
#ifndef SPOUTE_H
#define SPOUTE_H

#include "Genesis.h"
#pragma warning( disable : 4068 )


#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_Spout.bmp")
typedef struct EM_Spout
{
#pragma	GE_Private
	geActor		*Actor;

#pragma GE_Published
	geVec3d		Position;
	geVec3d		Angles;
	ActorStart	*AStart;
	float		ParticleCreateRate;
	float		MinScale;
	float		MaxScale;
	float		MinSpeed;
	float		MaxSpeed;
	float		MinUnitLife;
	float		MaxUnitLife;
	int			SourceVariance;
	int			DestVariance;
    GE_RGBA		ColorMin;
    GE_RGBA		ColorMax;
	geVec3d		Gravity;
	char		*BmpName;
	char		*AlphaName;
	char		*TriggerName;
	float		MinPauseTime;
	float		MaxPauseTime;
	float		TotalLife;
	char		*BoneName;

#pragma GE_Origin( Position )
#pragma GE_Angles(Angles)			
#pragma GE_DefaultValue( ParticleCreateRate, "0.1" )
#pragma GE_DefaultValue( MinScale, "1.0" )
#pragma GE_DefaultValue( MaxScale, "1.0" )
#pragma GE_DefaultValue( MinSpeed, "10.0" )
#pragma GE_DefaultValue( MaxSpeed, "30.0" )
#pragma GE_DefaultValue( MinUnitLife, "3.0" )
#pragma GE_DefaultValue( MaxUnitLife, "6.0" )
#pragma GE_DefaultValue( SourceVariance, "0" )
#pragma GE_DefaultValue( DestVariance, "1" )
#pragma GE_DefaultValue( ColorMin, "255.0 255.0 255.0" )
#pragma GE_DefaultValue( ColorMax, "255.0 255.0 255.0" )
#pragma GE_DefaultValue( Gravity, "0.0 0.0 0.0" )
#pragma GE_DefaultValue( TotalLife, "0.0" )
#pragma GE_DefaultValue( TriggerName, "NULL" )
#pragma GE_DefaultValue( MinPauseTime, "0.0" )
#pragma GE_DefaultValue( MaxPauseTime, "0.0" )
#pragma GE_DefaultValue( BoneName, "EFFECT_BONE01" )

#pragma GE_Documentation( Position, "Location of effect, if it's not hooked to an actor" )
#pragma GE_Documentation( Angles, "Direction in which particles will shoot" )
#pragma GE_Documentation( AStart, "Actor that effect is hooked to" )
#pragma GE_Documentation( ParticleCreateRate, "Every how many seconds to add a new particle" )
#pragma GE_Documentation( MinScale, "Min scale of the textures" )
#pragma GE_Documentation( MaxScale, "Max scale of the textures" )
#pragma GE_Documentation( MinSpeed, "Min speed of the textures" )
#pragma GE_Documentation( MaxSpeed, "Max speed of the textures" )
#pragma GE_Documentation( MinUnitLife, "Min life of each texture" )
#pragma GE_Documentation( MaxUnitLife, "Max life of each texture" )
#pragma GE_Documentation( SourceVariance, "How much to vary spray source point" )
#pragma GE_Documentation( DestVariance, "How much to vary spray dest point" )
#pragma GE_Documentation( ColorMin, "Minimum RGB values for each particle" )
#pragma GE_Documentation( ColorMax, "Maximum RGB values for each particle" )
#pragma GE_Documentation( Gravity, "Gravity vector to apply to each particle" )
#pragma GE_Documentation( BmpName, "Name of bitmap file to use" )
#pragma GE_Documentation( AlphaName, "Name of alpha bitmap file to use" )
#pragma GE_Documentation( TotalLife, "How many seconds this spout lasts. Set to 0 for continuous." )
#pragma GE_Documentation( TriggerName, "Name of the associated trigger" )
#pragma GE_Documentation( MinPauseTime, "Low range of randomly chosen pause time (seconds)" )
#pragma GE_Documentation( MaxPauseTime, "High range of randomly chosen pause time (seconds)" )
#pragma GE_Documentation( BoneName, "Which bone on the actor to hook to" )

} EM_Spout;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
