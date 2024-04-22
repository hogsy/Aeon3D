////////////////////////////////////////////////////////////////////////////////////////
//  EffectM.h									                                        
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//			Effect manager, designed to group multiple effects into one type of effect.
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
#ifndef EFFECTM_H
#define EFFECTM_H

#include "Genesis.h"
#include "TPool.h"
#include "SPool.h"
#include "Effect.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Settings headers (IF YOU ADD AN EFFECT ITS SETTINGS HEADER MUST BE LISTED HERE)
////////////////////////////////////////////////////////////////////////////////////////
#include "ExplosionE.h"
#include "RainE.h"
#include "CoronaE.h"
#include "SpoutE.h"
#include "AmbientSoundE.h"
#include "ProceduralE.h"
#include "FloatingParticlesE.h"
#include "MorphE.h"
#include "ElectricE.h"
#include "ScrollE.h"
#include "InvisibleE.h"
#include "RedWispE.h"
#include "FlipbookE.h"
#include "ChaosE.h"
#include "LightE.h"
#include "LightStyleE.h"
#include "ColorCycleE.h"
#include "ActorSettingsE.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	List of supported effects (IF YOU ADD AN EFFECT ITS INTERFACE MUST BE LISTED HERE)
////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	EffectM_Explosion,
	EffectM_Rain,
	EffectM_Corona,
	EffectM_Spout,
	EffectM_AmbientSound,
	EffectM_Procedural,
	EffectM_FloatingParticles,
	EffectM_Morph,
	EffectM_Electric,
	EffectM_Scroll,
	EffectM_Invisible,
	EffectM_RedWisp,
	EffectM_Flipbook,
	EffectM_Chaos,
	EffectM_Light,
	EffectM_LightStyle,
	EffectM_ColorCycle,
	EffectM_ActorSettings

} EffectM_Type;



////////////////////////////////////////////////////////////////////////////////////////
//	EffectManager type
////////////////////////////////////////////////////////////////////////////////////////
typedef struct EffectManager	EffectManager;


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

//	Create a new effect manager.
//
////////////////////////////////////////////////////////////////////////////////////////
EffectManager * EffectM_CreateManager(
  	EffectSystem	*ESystem,		// effect system to use
	TexturePool		*TPool,			// texture pool to use
	SoundPool		*SPool,			// sound pool to use
	void			*Context,		// context data
	geBoolean		SoftwareMode );	// whether or not software mode is being used

//	Destroy an effect manager.
//
////////////////////////////////////////////////////////////////////////////////////////
void EffectM_DestroyManager(
	EffectManager	**EManager );	// manager to zap

//	Create a new effect.
//
////////////////////////////////////////////////////////////////////////////////////////
int EffectM_Create(
	EffectManager	*EManager,			// effect manager to add it to
	EffectM_Type	Type,				// type of effect to add
	void			*UserData,			// user data
	int32			UserDataSize );		// size of user data

//	Destroy one effect of an effect manager by id.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean EffectM_Destroy(
	EffectManager	*EManager,	// effect manager that it belongs to
	int				Id );		// effect to destroy

//	Update a effect managers effects.
//
////////////////////////////////////////////////////////////////////////////////////////
int EffectM_Update(
	EffectManager	*EManager,		// manager to update
	float			TimeDelta );	// amount of elased time

//	EffectM_LoadWorldEffects()
//
////////////////////////////////////////////////////////////////////////////////////////
int EffectM_LoadWorldEffects(
	EffectManager	*EManager,	// effect manager that will own them
	geWorld			*World );	// world whose entity effects will be loaded
/*
//	Change the sound pool that the effect manager will use.
//
////////////////////////////////////////////////////////////////////////////////////////
void EffectM_ChangeSoundPool(
	EffectManager	*EManager,	// effect manager that will get a new sound pool
	SoundPool		*SPool );	// the new sound pool
*/

#ifdef __cplusplus
	}
#endif

#endif
