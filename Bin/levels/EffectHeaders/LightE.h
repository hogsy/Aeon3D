////////////////////////////////////////////////////////////////////////////////////////
//	LightE.h
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
#ifndef LIGHTE_H
#define LIGHTE_H

#include "Genesis.h"
#include "ActorStart.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_Light.bmp")
typedef struct EM_Light
{
#pragma	GE_Private

#pragma GE_Published
	geVec3d		Position;
	ActorStart	*AStart;
	float		MinRadius;
	float		MaxRadius;
    GE_RGBA		ColorMin;
    GE_RGBA		ColorMax;
	char		*LString;
	float		LTime;
	float		LLoopStyle;

#pragma GE_Origin(Position)			
#pragma GE_DefaultValue( MinRadius, "100.0" )
#pragma GE_DefaultValue( MaxRadius, "150.0" )
#pragma GE_DefaultValue( ColorMin, "255.0 255.0 255.0" )
#pragma GE_DefaultValue( ColorMax, "255.0 255.0 255.0" )
#pragma GE_DefaultValue( LTime, "0.0" )
#pragma GE_DefaultValue( LLoopStyle, "0" )

#pragma GE_Documentation( Position, "Location of effect" )
#pragma GE_Documentation( AStart, "Actor on whose effect bones the lights will attach to (optional)" )
#pragma GE_Documentation( MinRadius, "Minimum light radius" )
#pragma GE_Documentation( MaxRadius, "Maximum light radius" )
#pragma GE_Documentation( ColorMin, "Minimum RGB values for light" )
#pragma GE_Documentation( ColorMax, "Maximum RGB values for light" )
#pragma GE_Documentation( LString, "String used to create a light intensity pattern" )
#pragma GE_Documentation( LTime, "How many second to cycle thru light string" )
#pragma GE_Documentation( LLoopStyle, "0 = Restart, 1 = Back and forth" )

} EM_Light;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
