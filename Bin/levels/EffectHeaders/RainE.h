////////////////////////////////////////////////////////////////////////////////////////
//	RainE.h
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
#ifndef RAINE_H
#define RAINE_H

#include "Genesis.h"
#include "ActorStart.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_Rain.bmp")
typedef struct EM_Rain
{
#pragma	GE_Private
	int			TextureNumber;

#pragma GE_Published
	geVec3d		Position;
	ActorStart	*AStart;
	geVec3d		Gravity;
	float		Radius;
	float		Severity;
	float		DropLife;
    GE_RGBA		ColorMin;
    GE_RGBA		ColorMax;
	char		*BmpName;
	char		*AlphaName;
	char		*TriggerName;

#pragma GE_Origin( Position )
#pragma GE_DefaultValue( Gravity, "0.0 -60.0 0.0" )
#pragma GE_DefaultValue( Radius, "100.0" )
#pragma GE_DefaultValue( Severity, "0.5" )
#pragma GE_DefaultValue( DropLife, "1.0" )
#pragma GE_DefaultValue( TextureNumber, "2" )
#pragma GE_DefaultValue( ColorMin, "255.0 255.0 255.0" )
#pragma GE_DefaultValue( ColorMax, "255.0 255.0 255.0" )
#pragma GE_DefaultValue( TriggerName, "NULL" )

#pragma GE_Documentation( Position, "Location of effect, if it's not hooked to an actor" )
#pragma GE_Documentation( AStart, "Actor that effect is hooked to" )
#pragma GE_Documentation( Gravity, "The velocity applied to each drop each second (world units)" )
#pragma GE_Documentation( Radius, "The redius of the rain coverage aren (world units)" )
#pragma GE_Documentation( Severity, "How severe the rain is, 0.0 being tame, 1.0 being insanity" )
#pragma GE_Documentation( DropLife, "How long a drop lasts (seconds)" )
#pragma GE_Documentation( TextureNumber, "Which texture to use" )
#pragma GE_Documentation( ColorMin, "Minimum RGB for each drop" )
#pragma GE_Documentation( ColorMax, "Maximum RGB for each drop" )
#pragma GE_Documentation( BmpName, "Name of bitmap file to use" )
#pragma GE_Documentation( AlphaName, "Name of alpha bitmap file to use" )
#pragma GE_Documentation( TriggerName, "Name of the associated trigger" )

} EM_Rain;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
