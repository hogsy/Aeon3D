////////////////////////////////////////////////////////////////////////////////////////
//	InvisibleE.h
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
#ifndef INVISIBLEE_H
#define INVISIBLEE_H

#include "Genesis.h"
#include "ActorStart.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_Invisible.bmp")
typedef struct EM_Invisible
{
#pragma	GE_Private

#pragma GE_Published
	geVec3d		Position;
	ActorStart	*AStart;
	float		TotalLife;
	char		*BmpName;
	char		*AlphaName;

#pragma GE_Origin(Position)			
#pragma GE_DefaultValue( TotalLife, "0.0" )

#pragma GE_Documentation( AStart, "Actor which the effect will be applied to" )
#pragma GE_Documentation( TotalLife, "How many second this effect will last" )
#pragma GE_Documentation( BmpName, "Name of bitmap file to use" )
#pragma GE_Documentation( AlphaName, "Name of alpha bitmap file to use" )

} EM_Invisible;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
