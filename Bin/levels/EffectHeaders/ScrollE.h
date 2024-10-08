////////////////////////////////////////////////////////////////////////////////////////
//	ScrollE.h
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
#ifndef SCROLLE_H
#define SCROLLE_H

#include "Genesis.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_Scroll.bmp")
typedef struct EM_Scroll
{
#pragma	GE_Private

#pragma GE_Published
	geVec3d		Position;
	char		*TextureName;
	int			XScroll;
	int			YScroll;

#pragma GE_Origin(Position)			
#pragma GE_DefaultValue( XScroll, "10" )
#pragma GE_DefaultValue( YScroll, "0" )

#pragma GE_Documentation( XScroll, "How many pixels/sec to scroll along X" )
#pragma GE_Documentation( YScroll, "How many pixels/sec to scroll along Y" )

} EM_Scroll;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
