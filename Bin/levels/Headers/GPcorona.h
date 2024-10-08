////////////////////////////////////////////////////////////////////////////////////////
//	GPCorona.h
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
#ifndef	CORONA_H
#define	CORONA_H

#include	"genesis.h"

#pragma warning( disable : 4068 )

#ifdef __cplusplus
extern "C" {
#endif

#pragma GE_Type("corona.bmp")
typedef struct	GPCorona
{
	#pragma	GE_Private
		geFloat			LastTime;
		geFloat			LastVisibleTime;
		geFloat			LastVisibleRadius;
	
	#pragma GE_Published
	    geVec3d			origin;
		int				FadeOut;
		geFloat			FadeTime;
	    float			MinRadius;
	    float			MaxRadius;
		int				MaxVisibleDistance;
		int				MaxRadiusDistance;
		int				MinRadiusDistance;
		int				AllowRotation;
		geWorld_Model *	Model;
	    GE_RGBA			Color;

	#pragma GE_Origin(origin)
	#pragma GE_DefaultValue(FadeOut, "1")
	#pragma GE_DefaultValue(FadeTime, "0.15")
	#pragma GE_DefaultValue(MinRadius, "1.0")
	#pragma GE_DefaultValue(MaxRadius, "20.0")
	#pragma GE_DefaultValue(MaxVisibleDistance, "2000")
	#pragma GE_DefaultValue(MaxRadiusDistance, "1000")
	#pragma GE_DefaultValue(MinRadiusDistance, "100")
	#pragma GE_DefaultValue(AllowRotation, "1")
	#pragma GE_DefaultValue(Color, "255.0 255.0 255.0")

	#pragma GE_Documentation(FadeOut, "Whether to fade out coronas when they pass out of visibility (0 or 1)")
	#pragma GE_Documentation(FadeTime, "How long the fade takes to drop to zero visibility (seconds)")
	#pragma GE_Documentation(MinRadius, "Corona art min scale")
	#pragma GE_Documentation(MaxRadius, "Corona art max scale")
	#pragma GE_Documentation(MaxVisibleDistance, "Maximum distance corona is visible at (texels)")
	#pragma GE_Documentation(MaxRadiusDistance, "Above this distance, corona is capped at MaxRadius (texels)")
	#pragma GE_Documentation(MinRadiusDistance, "Below this distance, corona is capped at MinRadius (texels)")
	#pragma GE_Documentation(AllowRotation, "Permit rotation about model, if one is present (0 or 1)")
	#pragma GE_Documentation(Model, "World model to slave motion of corona to")
}	GPCorona;

geBoolean Corona_Init(geEngine *Engine, geWorld *World, geVFile *MainFS);
geBoolean Corona_Shutdown(void);
geBoolean Corona_Frame(geWorld *World, const geXForm3d *XForm, geFloat DeltaTime);
geBoolean Corona_ChangeWorld(geWorld *NewWorld);

#ifdef __cplusplus
}
#endif

#pragma warning( default : 4068 )

#endif
