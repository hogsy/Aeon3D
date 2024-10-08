////////////////////////////////////////////////////////////////////////////////////////
//	ProceduralE.h
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
#ifndef PROCEDURALE_H
#define PROCEDURALE_H

#include "Genesis.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_Procedural.bmp")
typedef struct EM_Procedural
{
#pragma	GE_Private

#pragma GE_Published
	geVec3d		Position;
	char		*TextureName;
	char		*ProceduralName;
	char		*Parameters;
	char		*TriggerName;

#pragma GE_Origin(Position)			
#pragma GE_DefaultValue( TriggerName, "NULL" )

#pragma GE_Documentation( Position, "Not used" )
#pragma GE_Documentation( TextureName, "Name of world texture to apply procedural to" )
#pragma GE_Documentation( ProceduralName, "Name of procedural to use" )
#pragma GE_Documentation( Parameters, "Procedural parameters" )
#pragma GE_Documentation( TriggerName, "Name of the associated trigger" )

} EM_Procedural;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
