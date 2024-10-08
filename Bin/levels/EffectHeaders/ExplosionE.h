////////////////////////////////////////////////////////////////////////////////////////
//	ExplosionE.h
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
#ifndef EXPLOSIONE_H
#define EXPLOSIONE_H

#include "Genesis.h"
#include "ActorStart.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_Explosion.bmp")
typedef struct EM_Explosion
{
#pragma	GE_Private
	int			TextureNumber;

#pragma GE_Published
	geVec3d		Position;
	geVec3d		Angles;
	ActorStart	*AStart;
	char		*BmpName;
	char		*AlphaName;
	char		*TriggerName;

#pragma GE_Origin(Position)			
#pragma GE_Angles(Angles)			
#pragma GE_DefaultValue( TriggerName, "NULL" )

#pragma GE_Documentation( Position, "Location of effect" )
#pragma GE_Documentation( Angles, "Direction in which explosion will shoot" )
#pragma GE_Documentation( AStart, "Actor that effect is hooked to" )
#pragma GE_Documentation( BmpName, "Name of bitmap file to use" )
#pragma GE_Documentation( AlphaName, "Name of alpha bitmap file to use" )
#pragma GE_Documentation( TriggerName, "Name of the associated trigger" )

} EM_Explosion;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif