/****************************************************************************************/
/*  Fade.c                                                                              */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Fade control routines               				                */
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

#include "genesis.h"
#include "fade.h"

//float		FadeAlpha = 255.0f;
static float		FadeAlpha = 0.0f;
static int			FadeDir = 0;
static float		FadeTime = 2.0f;
static GE_RGBA		FadeRGB = {0,0,0,0};

static int			Width;
static int			Height;

void Fade_SetRect(int RWidth, int RHeight)
{
	Width = RWidth;
	Height = RHeight;
}

void Fade_Reset(void)
{
	FadeDir = 0;
	FadeTime = 0.0f;
}

void Fade_Set(int Dir, float Time)
{
	FadeDir = Dir;
	FadeTime = Time;

	// immediately set the FadeAlpha if time is 0
	if (FadeTime == 0.0f)
	{
		if (FadeDir == -1)
			FadeAlpha = 0.0f; // transparent
		else
		if (FadeDir == 1)
			FadeAlpha = 255.0f;
	}
}

void Fade_Frame(geEngine *Engine, float ElapsedTime)
{	  
	// do fade stuff
	if ( FadeDir != 0 )
	{
		float	FadeDelta;

		if (FadeTime == 0)
			FadeDelta = 255.0f;
		else
			FadeDelta = (float)FadeDir * (255.0f/FadeTime) * ElapsedTime;
			
		FadeAlpha += FadeDelta;

		if ( FadeAlpha > 255.0f ) 
		{
			FadeAlpha = 255.0f;
			FadeDir = 0;
		}
		else if ( FadeAlpha < 0.0f ) 
		{
			FadeAlpha = 0.0f;	// transparent
			FadeDir = 0;
		}

	}

	if (FadeAlpha > 0.0f)
	{
		GE_Rect Rect;
		GE_RGBA	Color;
		Rect.Left = Rect.Top = 0;
		Rect.Right = Width - 1;
		Rect.Bottom = Height - 1;
		Color.r = 0.0f;
		Color.g = 0.0f;
		Color.b = 0.0f;
		Color.a = FadeAlpha;
		geEngine_FillRect( Engine, &Rect, &Color );
	}
}