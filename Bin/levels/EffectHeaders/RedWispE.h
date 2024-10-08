////////////////////////////////////////////////////////////////////////////////////////
//	RedWispE.h
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
#ifndef REDWISPE_H
#define REDWISPE_H

#include "Genesis.h"
#include "ActorStart.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_RedWisp.bmp")
typedef struct EM_RedWisp
{
#pragma	GE_Private

#pragma GE_Published
	geVec3d		Position;
	ActorStart	*AStart;
	char		*FluffBmp;
	char		*FluffAlphaBmp;
	char		*CenterBmp;
	char		*CenterAlphaBmp;

#pragma GE_Origin(Position)			

#pragma GE_Documentation( Position, "Location of effect" )
#pragma GE_Documentation( AStart, "Actor whose texture the effect will attach to (optional)" )
#pragma GE_Documentation( FluffBmp, "Wisp particle art" )
#pragma GE_Documentation( FluffAlphaBmp, "Wisp particle alpha art" )
#pragma GE_Documentation( CenterBmp, "Wisp center art" )
#pragma GE_Documentation( CenterAlphaBmp, "Wisp center alpha art" )

} EM_RedWisp;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
