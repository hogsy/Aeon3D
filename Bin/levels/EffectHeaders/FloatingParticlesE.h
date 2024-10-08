////////////////////////////////////////////////////////////////////////////////////////
//	FloatingParticlesE.h
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
#ifndef FLOATINGPARTICLESE_H
#define FLOATINGPARTICLESE_H

#include "Genesis.h"
#include "ActorStart.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_FloatingParticles.bmp")
typedef struct EM_FloatingParticles
{
#pragma	GE_Private

#pragma GE_Published
	geVec3d		Position;
	ActorStart	*AStart;
    GE_RGBA		Color;
	float		Scale;
	int			ParticleCount;
	float		Radius;
	float		Height;
	float		XSlant;
	float		ZSlant;
	float		MinSpeed;
	float		MaxSpeed;
	char		*BmpName;
	char		*AlphaName;
	char		*TriggerName;

#pragma GE_Origin( Position )
#pragma GE_DefaultValue( Color, "255.0 255.0 255.0" )
#pragma GE_DefaultValue( Scale, "1.0" )
#pragma GE_DefaultValue( ParticleCount, "10" )
#pragma GE_DefaultValue( Radius, "100" )
#pragma GE_DefaultValue( Height, "100" )
#pragma GE_DefaultValue( XSlant, "0.65" )
#pragma GE_DefaultValue( ZSlant, "0.65" )
#pragma GE_DefaultValue( MinSpeed, "60.0" )
#pragma GE_DefaultValue( MaxSpeed, "150.0" )
#pragma GE_DefaultValue( TriggerName, "NULL" )

#pragma GE_Documentation( Position, "Location of effect, if it's not hooked to an actor" )
#pragma GE_Documentation( AStart, "Actor that effect is hooked to" )
#pragma GE_Documentation( Color, "Texture color" )
#pragma GE_Documentation( Scale, "What scale to use for the bitmap" )
#pragma GE_Documentation( ParticleCount, "How many particles to use" )
#pragma GE_Documentation( Radius, "Radius of particle cylinder" )
#pragma GE_Documentation( Height, "Max height each particle will go from base" )
#pragma GE_Documentation( XSlant, "Upwards slant on X axis" )
#pragma GE_Documentation( ZSlant, "Upwards slant on X axis" )
#pragma GE_Documentation( MinSpeed, "Min speed of each particle" )
#pragma GE_Documentation( MaxSpeed, "Max speed of each particle" )
#pragma GE_Documentation( BmpName, "Name of bitmap file to use" )
#pragma GE_Documentation( AlphaName, "Name of alpha bitmap file to use" )
#pragma GE_Documentation( TriggerName, "Name of the associated trigger" )

} EM_FloatingParticles;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
