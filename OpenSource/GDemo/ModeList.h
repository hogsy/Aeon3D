/****************************************************************************************/
/*  ModeList.h                                                                          */
/*                                                                                      */
/*  Author: Mike Sandige                                                                */
/*  Description:    Builds handy list of available modes                                */
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
#ifndef MODELIST_H
#define MODELIST_H

#include	"genesis.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ModeList_DriverType 
{
	MODELIST_TYPE_GLIDE,
	MODELIST_TYPE_D3D_PRIMARY,
	MODELIST_TYPE_D3D_SECONDARY,
	MODELIST_TYPE_UNKNOWN,
	MODELIST_TYPE_D3D_3DFX,
	MODELIST_TYPE_SOFTWARE,
} ModeList_DriverType;

typedef enum ModeList_Evaluation
{
	MODELIST_EVALUATED_OK,
	MODELIST_EVALUATED_UNDESIRABLE,
	MODELIST_EVALUATED_TRIED_FAILED,
} ModeList_Evaluation;

typedef	struct	ModeList
{
	geDriver * 				Driver;
	geDriver_Mode *			Mode;
	const char *			DriverNamePtr;
	const char *			ModeNamePtr;
	ModeList_DriverType	DriverType;
	int						Width;
	int						Height;
	ModeList_Evaluation	Evaluation;
	geBoolean				InAWindow;
}	ModeList;


void      ModeList_Destroy(ModeList *List);
ModeList *ModeList_Create(geEngine *Engine,int *ListLength);

#ifdef __cplusplus
}
#endif

#endif

