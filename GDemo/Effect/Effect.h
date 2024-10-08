////////////////////////////////////////////////////////////////////////////////////////
//  Effect.h									                                        
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//				API to control special effects.
//				Effect_SystemFrame() must be called once per frame.
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
////////////////////////////////////////////////////////////////////////////////////////
//
//	In order to add a new effect it must:
//	1) have its header listed in here
//	2) have its type listed in the "EffectType" struct
//	3) have its interface listed in the "EffectList" struct
//
////////////////////////////////////////////////////////////////////////////////////////
#ifndef EFFECT_H
#define EFFECT_H


////////////////////////////////////////////////////////////////////////////////////////
//	IF YOU ADD AN EFFECT ITS HEADER MUST BE LISTED HERE
////////////////////////////////////////////////////////////////////////////////////////
#include "Genesis.h"
#include "Spray.h"
#include "Sprite.h"
#include "Snd.h"
#include "ProceduralTexture.h"
#include "Corona.h"
#include "Bolt.h"
#include "Glow.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	Effect death callback
////////////////////////////////////////////////////////////////////////////////////////
typedef void (*EffectDeathCallback)( int ID, void* Context );


////////////////////////////////////////////////////////////////////////////////////////
//	Effect system pointer
////////////////////////////////////////////////////////////////////////////////////////
typedef struct	EffectSystem	EffectSystem;


////////////////////////////////////////////////////////////////////////////////////////
//	Supported effect types (IF YOU ADD AN EFFECT ITS TYPE MUST BE LISTED HERE)
////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	Effect_STARTOFLIST = 0,
	Effect_Spray,
	Effect_Sprite,
	Effect_Sound,
	Effect_ProceduralTexture,
	Effect_Corona,
	Effect_Bolt,
	Effect_Glow

} EffectType;


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

//	Create the effect system.
//
////////////////////////////////////////////////////////////////////////////////////////
EffectSystem * Effect_SystemCreate(
	geEngine		*EngineToUse,	// engine to use
	geWorld			*WorldToUse,	// world to use
	geSound_System	*SoundToUse,	// sound system to use
	geCamera		*CameraToUse );	// camera to use

//	Destroy the effect system.
//
////////////////////////////////////////////////////////////////////////////////////////
void Effect_SystemDestroy(
	EffectSystem	**System );	// effect system to zap

//	Create an effect and returns its unique identifier.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Effect_New(
	EffectSystem		*System,		// effect system to add it to
	EffectType			Type,			// type of effect
	void				*Data,			// effect data
	int					AttachTo,		// identifier of effect to attach to
	EffectDeathCallback	DeathCallback,	// death callback function
	void				*DeathContext,	// context data
	int					*Id );			// where to save effect id

//	Removes an effect.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Effect_Delete(
	EffectSystem	*System,	// effect system to delete it from
	int				ID );		// id of effect to delete

//	Process all current effects.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Effect_SystemFrame(
	EffectSystem	*System,		// effect system to process a frame for
	float			TimeDelta );	// amount of elapsed time

//	Returns true if the effect is still alive, false if it isn't.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Effect_StillExists(
	EffectSystem	*System,	// effect system to search in
	int				ID );		// effect id to verify

//	Modify an effect.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Effect_Modify(
	EffectSystem	*System,	// effect system in which it exists
	EffectType		Type,		// type of effect
	void			*Data,		// new effect data
	int				ID,			// id of effect to modify
	uint32			Flags );	// how to use the new data

//	Pause/unpause an effect.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Effect_SetPause(
	EffectSystem	*System,	// effect system in which it exists
	int				ID,			// id of effect to modify
	geBoolean		Pause );	// new pause state

//	Get a pointer to the engine that this effect system is tied to.
//
////////////////////////////////////////////////////////////////////////////////////////
geEngine * Effect_GetEngine(
	EffectSystem	*ESystem );	// effect system whose engine we want

//	Get a pointer to the world that this effect system is tied to.
//
////////////////////////////////////////////////////////////////////////////////////////
geWorld * Effect_GetWorld(
	EffectSystem	*ESystem );	// effect system whose world we want

//	Get a pointer to the camera that this effect system is tied to.
//
////////////////////////////////////////////////////////////////////////////////////////
geCamera * Effect_GetCamera(
	EffectSystem	*ESystem );	// effect system whose camera we want

//	Destroys all effects.
//
////////////////////////////////////////////////////////////////////////////////////////
void Effect_DeleteAll(
	EffectSystem	*ESystem );	// effect system whose effects will be destroyed

//	Changed the world that the effect system uses
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Effect_ChangeWorld(
	EffectSystem	*ESystem,	// effect system whose world will change
	geWorld			*World );	// the new world that will be used

//	Change the sound system that the effect system uses.
//
////////////////////////////////////////////////////////////////////////////////////////
void Effect_SystemChangeSoundSystem(
	EffectSystem	*System,			// effect system whose sound system will be changed
	geSound_System	*NewSoundSystem );	// new sound system to use


#ifdef __cplusplus
	}
#endif

#endif
