////////////////////////////////////////////////////////////////////////////////////////
//	LightStyleE.h
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
#ifndef LIGHTSTYLEE_H
#define LIGHTSTYLEE_H

#include "Genesis.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_LightStyle.bmp")
typedef struct EM_LightStyle
{
#pragma	GE_Private

#pragma GE_Published
	geVec3d		Position;
	char		*S1;
	char		*S2;
	float		MorphTime;
	int			StyleToUse;
	geBoolean	Loop;
	char		*TriggerName;
	int			TriggerCount;

#pragma GE_Origin(Position)			
#pragma GE_DefaultValue( S1, "z" )
#pragma GE_DefaultValue( S2, "a" )
#pragma GE_DefaultValue( MorphTime, "0.0" )
#pragma GE_DefaultValue( StyleToUse, "0" )
#pragma GE_DefaultValue( Loop, "0" )
#pragma GE_DefaultValue( TriggerName, "NULL" )
#pragma GE_DefaultValue( TriggerCount, "-1" )

#pragma GE_Documentation( S1, "First light string" )
#pragma GE_Documentation( S2, "Second light string" )
#pragma GE_Documentation( MorphTime, "How many seconds to go from S1 to S2" )
#pragma GE_Documentation( StyleToUse, "Which light style slot this effect will use" )
#pragma GE_Documentation( Loop, "Whether or not to loop light style back and forth" )
#pragma GE_Documentation( TriggerName, "Name of trigger starts the light morph (optional)" )
#pragma GE_Documentation( TriggerCount, "Number of times this will trigger. -1 is infinte" )

} EM_LightStyle;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
