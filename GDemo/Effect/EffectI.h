////////////////////////////////////////////////////////////////////////////////////////
//  EffectI.h									                                        
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//			Effect interface definition.
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
#ifndef EFFECTI_H
#define EFFECTI_H

#include "Genesis.h"
#include "Particle.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	Resources available to all effects
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	geEngine		*Engine;	// engine to use
	geWorld			*World;		// world to use
	geSound_System	*Sound;		// sound system to use
	geCamera		*Camera;	// camera to use
	Particle_System	*PS;		// particle system

} EffectResource;


////////////////////////////////////////////////////////////////////////////////////////
//	Effect interface
////////////////////////////////////////////////////////////////////////////////////////
typedef char *		( *EffectI_GetName )( void );
typedef void *		( *EffectI_Create )( EffectResource *Resource, int TypeID );
typedef void		( *EffectI_Destroy )( void **ResourceToZap );
typedef void *		( *EffectI_Add )( void *Resource, void *Data, void *AttachTo );
typedef void		( *EffectI_Remove )( void *Resource, void *Data );
typedef geBoolean	( *EffectI_Process )( void *Resource, float TimeDelta, void *Data );
typedef geBoolean	( *EffectI_Frame )( void *Resource, float TimeDelta );
typedef geBoolean	( *EffectI_Modify )( void *Resource, void *Data, void *NewData, uint32 Flags );
typedef void		( *EffectI_Pause )( void *Resource, void *Data, geBoolean Pause );
typedef	struct Effect_Interface
{
	EffectI_GetName		GetName;	// get name of the effect
	EffectI_Create		Create;		// does all initial setup for an effect type
	EffectI_Destroy		Destroy;	// completely removes an effect type, performs all clean up
	EffectI_Add			Add;		// adds an individual effect
	EffectI_Remove		Remove;		// removes an individual effect
	EffectI_Process		Process;	// performs processing for an individual effect
	EffectI_Frame		Frame;		// performs once-per-frame procesing for all effects of a type
	EffectI_Modify		Modify;		// adjust the effect
	EffectI_Pause		Pause;		// adjust pause state
	int32				Size;		// size of the effects structure

}	Effect_Interface;


#ifdef __cplusplus
	}
#endif

#endif
