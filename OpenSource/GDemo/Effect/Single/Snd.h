/****************************************************************************************/
/*  Snd.h										                                        */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
/*  Description:    Plays a sound														*/
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
#ifndef SND_H
#define SND_H

#include "Genesis.h"
#include "EffectI.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Data which can be modified
////////////////////////////////////////////////////////////////////////////////////////
#define SND_POS		( 1 << 0 )


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
extern	Effect_Interface	Snd_Interface;
typedef struct
{
	int			TypeID;				// RESERVED, this MUST be the first item in the struct
	geSound		*Sound;				// RESERVED, pointer to the active sound
	geBoolean	Paused;				// RESERVED, whether or not the sound is paused
	geFloat		LastVolume;			// RESERVED, its volume the last time it was modified
	geFloat		LastPan;			// RESERVED, its pan the last time it was modified
	geSound_Def	*SoundDef;			// sound def to play from
	geVec3d		Pos;				// location of the sound
	geFloat		Min;				// min distance whithin which sound is at max volume
	geBoolean	Loop;				// whether or not to loop it
	geBoolean	IgnoreObstructions;	// if obstructions should be ignored when compting sound data

} Snd;



#ifdef __cplusplus
	}
#endif

#endif
