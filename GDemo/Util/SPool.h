/****************************************************************************************/
/*  SPool.h																				*/
/*                                                                                      */
/*  Description:    A poor mans sound pool												*/
/*                                                                                      */
/*  The contents of this file are subject to the Genesis3D Public License               */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.genesis3d.com                                                            */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#ifndef SPOOL_H
#define SPOOL_H

#include "genesis.h"

#ifdef __cplusplus
	extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////////////
//	Sound pool pointer
////////////////////////////////////////////////////////////////////////////////////////
typedef struct	SoundPool	SoundPool;


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

//	Create the sound pool.
//
////////////////////////////////////////////////////////////////////////////////////////
SoundPool * SPool_Create(
	geSound_System	*SoundSystem );	// sound system to use

//	Destroy the sound pool.
//
////////////////////////////////////////////////////////////////////////////////////////
void SPool_Destroy(
	SoundPool	**SPool );		// sound pool to zap

//	Get a sound from the sound pool.
//
////////////////////////////////////////////////////////////////////////////////////////
geSound_Def * SPool_Get(
	SoundPool	*SPool,	// sound pool to retrieve it from
	int			Num );	// sound that we want

//	Add a sound to the sound pool.
//
////////////////////////////////////////////////////////////////////////////////////////
int SPool_Add(
	SoundPool	*SPool,		// sound pool to add sound to
	geVFile		*File,		// file system to use
	char		*Name );	// sound name


#ifdef __cplusplus
	}
#endif
#pragma warning ( default : 4068 )

#endif
