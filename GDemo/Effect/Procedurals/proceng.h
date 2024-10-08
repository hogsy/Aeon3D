/****************************************************************************************/
/*  ProcEng.h                                                                           */
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
#ifndef PROCENG_H
#define PROCENG_H

#include "Genesis.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	ProcEng			ProcEng;


ProcEng	*	ProcEng_Create(geVFile *CfgFile, geWorld *World);
void		ProcEng_Destroy(ProcEng **pPEng);
geBoolean	ProcEng_AddProcedural(ProcEng *PEng, const char *ProcName, geBitmap **Bitmap, const char * Params);
geBoolean	ProcEng_Animate(ProcEng *PEng, float ElapsedTime);
				// only animates visible procedurals

geBoolean	ProcEng_Minimize(ProcEng *PEng);
				// flush out unused procedurals

#ifdef __cplusplus
}
#endif

#endif
