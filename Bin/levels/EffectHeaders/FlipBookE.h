////////////////////////////////////////////////////////////////////////////////////////
//	FlipbookE.h
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
#ifndef FLIPBOOKE_H
#define FLIPBOOKE_H

#include "Genesis.h"
#include "ActorStart.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_Flipbook.bmp")
typedef struct EM_Flipbook
{
#pragma	GE_Private
	geActor		*Actor;

#pragma GE_Published
	geVec3d		Position;
	ActorStart	*AStart;
	char		*TriggerName;
	char		*BmpNameBase;
	char		*AlphaNameBase;
	int			TextureCount;
    GE_RGBA		Color;
	float		Scale;
	int			CycleStyle;
	float		CycleSpeed;
	char		*WorldBitmapName;

#pragma GE_Origin( Position )
#pragma GE_DefaultValue( TextureCount, "0" )
#pragma GE_DefaultValue( Color, "255.0 255.0 255.0" )
#pragma GE_DefaultValue( Scale, "1.0" )
#pragma GE_DefaultValue( CycleStyle, "0" )
#pragma GE_DefaultValue( CycleSpeed, "0.25" )

#pragma GE_Documentation( Position, "Location of effect, if it's not hooked to an actor" )
#pragma GE_Documentation( AStart, "Actor that effect is hooked to" )
#pragma GE_Documentation( TriggerName, "Name of the associated trigger" )
#pragma GE_Documentation( BmpNameBase, "Base name for all the bitmaps" )
#pragma GE_Documentation( AlphaNameBase, "Base name for all the alpha bitmaps" )
#pragma GE_Documentation( TextureCount, "How many bitmaps in the sequence" )
#pragma GE_Documentation( Color, "Texture color" )
#pragma GE_Documentation( Scale, "What scale to use for the bitmap" )
#pragma GE_Documentation( CycleStyle, "How to cycle the sprites, 0 = sequential, 1 = back/forth, 2 = just one thru" )
#pragma GE_Documentation( CycleSpeed, "How many seconds to wait before going to next image in sequence" )
#pragma GE_Documentation( WorldBitmapName, "If provided, then this world bitmap will be replaced by the cycling bitmaps" )

} EM_Flipbook;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
