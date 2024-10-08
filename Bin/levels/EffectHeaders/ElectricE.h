////////////////////////////////////////////////////////////////////////////////////////
//	ElectricE.h
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
#ifndef ELECTRICE_H
#define ELECTRICE_H

#include "Genesis.h"
#include "EffectPosE.h"
#pragma warning( disable : 4068 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma GE_Type("EM_Electric.bmp")
typedef struct EM_Electric
{
#pragma	GE_Private

#pragma GE_Published
	geVec3d			Position;
	EM_EffectPos	*Source;
	EM_EffectPos	*Dest;
	char			*BmpName;
	char			*AlphaName;
	GE_RGBA			Color;
	float			Offset;
	int				SegmentLength;
	float			SegmentWidth;
	int				BoltSegmentLimit;
	float			MinPauseTime;
	float			MaxPauseTime;
	float			MinFireTime;
	float			MaxFireTime;
	char			*SoundName;
	float			SoundMinDistance;

#pragma GE_Origin(Position)			
#pragma GE_DefaultValue( Color, "255.0 255.0 255.0" )
#pragma GE_DefaultValue( Offset, "0.1" )
#pragma GE_DefaultValue( SegmentLength, "30" )
#pragma GE_DefaultValue( SegmentWidth, "20.0" )
#pragma GE_DefaultValue( BoltSegmentLimit, "100" )
#pragma GE_DefaultValue( MinPauseTime, "0.0" )
#pragma GE_DefaultValue( MaxPauseTime, "0.0" )
#pragma GE_DefaultValue( MinFireTime, "0.0" )
#pragma GE_DefaultValue( MaxFireTime, "0.0" )
#pragma GE_DefaultValue( SoundMinDistance, "500.0" )

#pragma GE_Documentation( Source, "Bolt source" )
#pragma GE_Documentation( Dest, "Bolt dest" )
#pragma GE_Documentation( BmpName, "Name of bitmap file to use" )
#pragma GE_Documentation( AlphaName, "Name of alpha bitmap file to use" )
#pragma GE_Documentation( Color, "Color of the bolt" )
#pragma GE_Documentation( Offset, "How spikey the bolt will be" )
#pragma GE_Documentation( SegmentLength, "Length of each bolt segment" )
#pragma GE_Documentation( SegmentWidth, "Width of each botl segment" )
#pragma GE_Documentation( BoltSegmentLimit, "Maximum number of bolt segments to use" )
#pragma GE_Documentation( MinPauseTime, "Minimum seconds to pause between electric shots" )
#pragma GE_Documentation( MaxPauseTime, "Maximum seconds to pause between electric shots" )
#pragma GE_Documentation( MinFireTime, "Minimum seconds that electric shots will last" )
#pragma GE_Documentation( MaxFireTime, "Maximum seconds that electric shots will last" )
#pragma GE_Documentation( SoundMinDistance, "Min distance within which sound is at max volume" )

} EM_Electric;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
