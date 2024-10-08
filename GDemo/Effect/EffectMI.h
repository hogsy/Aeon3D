////////////////////////////////////////////////////////////////////////////////////////
//  EffectMI.h									                                        
//                                                                                      
//  Author: Peter Siamidis                                                              
//  Description:    
//			Effect manager interface definition.
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
#ifndef EFFECTMI_H
#define EFFECTMI_H

#include "EffectM.h"
#include "IndexList.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	EffectManager struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct EffectManager
{
	EffectSystem	*ESystem;		// effect system the manager uses
	TexturePool		*TPool;			// texture pool the manager uses
	SoundPool		*SPool;			// sound pool the manager uses
	IndexList		*EffectList;	// list of active effects
	void			*Context;		// context data
	geBoolean		SoftwareMode;	// whether or not output is using software mode

} EffectManager;


////////////////////////////////////////////////////////////////////////////////////////
//	Root list struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	EffectM_Type	Type;				// type of effect
	int32			EffectCount;		// how many individual effects make up this effect
	int32			*EffectList;		// id list of those effects
	float			TimeDelta;			// seconds elapsed since last update
	void			*Custom;			// custom data
	geBoolean		Terminate;			// termination flag
	void			*UserData;			// user data
	int32			UserDataSize;		// size of user data

} EffectM_Root;


////////////////////////////////////////////////////////////////////////////////////////
//	EffectMI interface
////////////////////////////////////////////////////////////////////////////////////////
typedef geBoolean	( *EffectMI_Create )( EffectManager *Manager, EffectM_Root *Root );
typedef void		( *EffectMI_Destroy )( EffectManager *Manager, EffectM_Root *Root );
typedef geBoolean	( *EffectMI_Update )( EffectManager *Manager, EffectM_Root *Root );
typedef	struct EffectM_Interface
{
	EffectMI_Create		Create;
	EffectMI_Destroy	Destroy;
	EffectMI_Update		Update;

}	EffectM_Interface;


#ifdef __cplusplus
	}
#endif

#endif
