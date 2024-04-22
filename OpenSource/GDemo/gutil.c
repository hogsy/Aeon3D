/****************************************************************************************/
/*  gutil.c                                                                             */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Misc utility routines for gdemo       				                */
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
#include <windows.h>
#include <math.h>
#include "assert.h"
#include "genesis.h"
#include "gutil.h"
#include "ctypes.h"

static void SubLarge(LARGE_INTEGER *start, LARGE_INTEGER *end, LARGE_INTEGER *delta)
{
	_asm {
		mov ebx,dword ptr [start]
		mov esi,dword ptr [end]

		mov eax,dword ptr [esi+0]
		sub eax,dword ptr [ebx+0]

		mov edx,dword ptr [esi+4]
		sbb edx,dword ptr [ebx+4]

		mov ebx,dword ptr [delta]
		mov dword ptr [ebx+0],eax
		mov dword ptr [ebx+4],edx
	}
}


// return TRUE if the specified key is down
geBoolean GUtil_IsKeyDown (int KeyCode, HWND hWnd)
{
	if (GetFocus() == hWnd)
	{
		if (GetAsyncKeyState(KeyCode) & 0x8000)
		{
			return GE_TRUE;
		}
	}

	return GE_FALSE;
}

//=====================================================================================
//	GetCmdLine
//=====================================================================================
char *GUtil_GetCmdLine(char *CmdLine, char *Data, BOOL flag)
{
	int32 dp = 0;
	char ch;

	assert(CmdLine);

	for	(;;)
	{
		ch = *CmdLine++;
		if (ch == ' ')
			continue;
		if (ch == '-' && !flag) 
			break;
		if (flag && ch != ' ')
		{
			CmdLine--;
			break;
		}
		if (ch == 0 || ch == '\n')
			break;
	}

	if (ch == 0 || ch == '\n')
		return FALSE;

	while (ch != ' ' && ch != '\n' && ch != 0)
	{
		ch = *CmdLine++;
		Data[dp++] = ch;
	}

	Data[dp-1] = 0;

	return CmdLine;
		
}

static	BOOL	GUtil_ShiftKeyPressed(void)
{
	if	(GetKeyState(VK_SHIFT) & 0x8000)
	{
		return TRUE;
	}

	return FALSE;
}


void GUtil_GetEulerAngles2(const geXForm3d *M, geVec3d *Angles)
{
	geFloat cy;
	assert( M      != NULL );
	assert( Angles != NULL );
	
	// order of angles x,y,z

	cy = (geFloat)sqrt(M->AX*M->AX + M->BX*M->BX);

	if (cy > 16 * 0.001)
		{
			Angles->X = (geFloat)atan2(M->CY,M->CZ);
			Angles->Y = (geFloat)atan2(-M->CX,cy);
			Angles->Z = (geFloat)atan2(M->BX,M->AX);
		}
	else
		{
			Angles->X = (geFloat)atan2(-M->BZ,M->BY);
			Angles->Y = (geFloat)atan2(-M->CX,cy);
			Angles->Z = 0;
		}

	assert( geVec3d_IsValid(Angles)!=GE_FALSE);
}


void GUtil_SetEulerAngles2(geXForm3d *M, const geVec3d *Angles)
{
	geXForm3d XM, YM, ZM;							            

	assert( M      != NULL );
	assert( geVec3d_IsValid(Angles)!=GE_FALSE);
	
	// order of angles x,y,z
	geXForm3d_SetXRotation(&XM,Angles->X);
	geXForm3d_SetYRotation(&YM,Angles->Y);
	geXForm3d_SetZRotation(&ZM,Angles->Z);
	
	geXForm3d_Multiply(&YM, &XM, M);
	geXForm3d_Multiply(&ZM, M,   M);
}


void GUtil_OrientFromXForm(geXForm3d *XForm, geVec3d *Pos, geVec3d *Rot)
{
	*Pos = XForm->Translation;
	GUtil_GetEulerAngles2(XForm, Rot);
}


void GUtil_GetOrientationFromEulerAngles2(geXForm3d *O, const geVec3d *Angles)
{
 	geVec3d In;
	geVec3d Up = { 0, 1, 0 };
	geVec3d Left;
	geXForm3d M;

	// get the rotation matrix that defines the In vector
	GUtil_SetEulerAngles2(&M, Angles);

	// get the In vector (In is normalized)
	geXForm3d_GetIn(&M, &In);

	// make Left (Up and In are normalized, but most likely aren'tperpendicular)
	geVec3d_CrossProduct(&Up, &In, &Left);

	// normalize Left
	geVec3d_Normalize(&Left);

	// create the real Up
	geVec3d_CrossProduct(&In, &Left, &Up);

	// create the orientation transform
	geXForm3d_SetFromLeftUpIn(O, &Left, &Up, &In);
}

void GUtil_ScreenShot(geEngine *Engine)
	{
	int num,end_num,next_num;
	geVFile * FileBase;
	geVFile_Finder * Finder;
	char FName[1024];

	// find the last saved position
	FileBase = geVFile_OpenNewSystem(NULL,GE_VFILE_TYPE_DOS,".",NULL,GE_VFILE_OPEN_READONLY|GE_VFILE_OPEN_DIRECTORY);
	if ( ! FileBase )
		return;

	Finder = geVFile_CreateFinder(FileBase,"ss*.bmp");
	if ( ! Finder )
		{
		geVFile_Close(FileBase);
		return;
		}

	end_num = -1;
	num = 0;
	while( geVFile_FinderGetNextFile(Finder) )
		{
		geVFile_Properties Properties;
		geVFile_FinderGetProperties(Finder,&Properties);

		strlwr(Properties.Name);
		sscanf(Properties.Name,"ss%04d.bmp",&num);

		if (num > end_num)
			end_num = num;
		}

	next_num = end_num + 1;

	geVFile_DestroyFinder(Finder);
	geVFile_Close(FileBase);

	sprintf(FName,"ss%04d.bmp",next_num);
	geEngine_ScreenShot(Engine, FName);
	}

void GUtil_CalcElapsedTime(LARGE_INTEGER *Freq, LARGE_INTEGER *OldTick, LARGE_INTEGER *CurTick, float *ElapsedTime)
{
	LARGE_INTEGER DeltaTick;

	#define FRAME_TIME_CAP (1.0f/6)

	// timing
	QueryPerformanceCounter(CurTick);
	SubLarge(OldTick, CurTick, &DeltaTick);
	*OldTick = *CurTick;
	if (DeltaTick.LowPart > 0)
		*ElapsedTime =  1.0f / (((float)Freq->LowPart / (float)DeltaTick.LowPart));
	else 
		*ElapsedTime = 0.001f;

	if (*ElapsedTime > FRAME_TIME_CAP)
		*ElapsedTime = FRAME_TIME_CAP;
}


geBoolean	GUtil_UpIsDown(geXForm3d *XForm)
{
	return (XForm->BY < 0.0f) ? GE_TRUE : GE_FALSE;
}

int	GUtil_UpIsDown2(geFloat XRot,geFloat YRot, geFloat ZRot)
{
	int Flip =0;

	if ((XRot> M_PI/2.0f) || (XRot< -M_PI/2.0f))
		Flip ++;

	if ((YRot> M_PI/2.0f) || (YRot< -M_PI/2.0f))
		Flip ++;

	return (Flip);
}


geBoolean	GUtil_Collide(geWorld *World, const geVec3d *Front, const geVec3d *Back, GE_Collision *Collision)
{
	geVec3d		Direction;
	geVec3d		InBetween;
	geFloat	Length;
	geFloat	InBetweenLength;

	assert(World != NULL);
	assert(Front != NULL);
	assert(Back != NULL);
	assert(Collision != NULL);

	geVec3d_Subtract(Back, Front, &Direction);
	Length = geVec3d_Normalize(&Direction);
	geVec3d_Scale(&Direction, 5.0f, &Direction);
	geVec3d_Copy(Front, &InBetween);
	InBetweenLength = 5.0f;
	while	(InBetweenLength < Length)
	{
		if	(geWorld_Collision(World, NULL, NULL, Front, &InBetween, GE_CONTENTS_SOLID_CLIP, GE_COLLIDE_ALL, USER_ALL, NULL, NULL, Collision))
			return GE_TRUE;
		geVec3d_AddScaled(Front, &Direction, InBetweenLength, &InBetween);
		InBetweenLength += 5.0f;
	}

	return geWorld_Collision(World, NULL, NULL, Front, Back, GE_CONTENTS_SOLID_CLIP, GE_COLLIDE_ALL, USER_ALL, NULL, NULL, Collision);
}

//=====================================================================================
//	GUtil_BuildXFormFromRotationOrderXY
//=====================================================================================
void GUtil_BuildXFormFromRotationOrderXY(geVec3d *Rot, geVec3d *Pos, geXForm3d *XForm)
{
	assert(Pos != NULL);
	assert(XForm != NULL);

	GUtil_SetEulerAngles2(XForm, Rot);
	geXForm3d_Translate(XForm, Pos->X, Pos->Y, Pos->Z);
}

