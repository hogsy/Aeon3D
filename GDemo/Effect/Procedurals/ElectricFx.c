/****************************************************************************************/
/*  ElectricFx.c                                                                        */
/*                                                                                      */
/*  Author: Peter Siamidis                                                              */
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

#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <Assert.h>
#include "Procedural.h"
#include "Ram.h"

#define		PI		(3.14159f)
#define		PI_2	(PI*2)

#define		ELECTRICFX_MAX_VECS		128

//====================================================================================
//====================================================================================
typedef struct
{
	int32		x, y;
} ElectricFx_Vec2d;

typedef struct Procedural
{
	geBitmap			*Bitmap;

	int32				Width;
	int32				Height;
	int32				Size;
	int32				WMask;
	int32				HMask;

	uint8				*ZBuffer;				
	uint8				*Smtex;

	uint16				CLut[256];

	int32				FxType;

	int32				NumVecs;
	ElectricFx_Vec2d	Vecs[ELECTRICFX_MAX_VECS];

	float				Rotation;

} Procedural;

void ElectricFx_Destroy(Procedural *Proc);
geBoolean ElectricFx_Animate(Procedural *Fx, float ElapsedTime);

void ElectricFx_Shade(Procedural *Fx);

void ElectricFx_Update(Procedural *Fx, float Time);

geBoolean ElectricFx_ApplyToBitmap(Procedural *Fx);

void ElectricFx_BuildRGBLuts(Procedural *Fx);





static geBoolean geBitmapUtil_SmoothBits(geBitmap_Info *pInfo,void *FmBits,void *ToBits,int radius)
{
int last,bpp,x,y,w,h,s;

	assert(FmBits && ToBits);
	assert(radius > 0);

	if ( radius > 3 ) radius = 3;

	bpp = gePixelFormat_BytesPerPel(pInfo->Format);

	w = pInfo->Width;
	h = pInfo->Height;
	s = pInfo->Stride;

	last = (h-1)*s + (w-1);

	switch(bpp)
	{
		case 0:
			return GE_FALSE;
		case 1:
		{
		uint8 *pSrc,*pDst;

			pSrc = FmBits;
			pDst = ToBits;

			switch(radius)
			{
			case 1:
				
					// first line
					*pDst++ = (uint8)( (pSrc[1] + pSrc[s])>>1 );
					pSrc++;
					for(x=w-1;x--;)
					{
						*pDst++ = (uint8)( (pSrc[-1] + pSrc[1] + pSrc[s]+ pSrc[s-1])>>2 );
						pSrc++;
					}
					pDst += (s-w);
					pSrc += (s-w);

				for(y=h-2;y--;)
				{
					// middle lines
					
					x = w;

					#ifdef DONT_USE_ASM //{

					while(x--)
					{
						*pDst++ = (pSrc[-1] + pSrc[1] + pSrc[s] + pSrc[-s])>>2;
						pSrc++;
					}

					pDst += (s-w);
					pSrc += (s-w);

					#else //}{

					__asm 
					{
						mov esi, pSrc
						mov edi, pDst
					
						// something like 10 clocks per byte
						
					mainloop:
						xor eax,eax						// load the sum into eax

						movzx edx,BYTE PTR [esi+1]		// port 2, latency 1
						add eax,edx						// port 1, latency 1

						movzx edx,BYTE PTR [esi-1]
						add eax,edx	
							
						mov ecx,esi						// ecx = esi + s
						add ecx,s
						movzx edx,BYTE PTR [ecx]		// eax += (BYTE) [ecx]
						add eax,edx						// we have to use edx cuz of the byte stuff

						mov ecx,esi	
						sub ecx,s
						movzx edx,BYTE PTR [ecx]
						add eax,edx	

						shr eax,2

						mov [edi],al

						inc edi
						inc esi
						dec x

						jnz mainloop
					}

					pDst += s;
					pSrc += s;

					#endif // }
				}
				
					// last line
					for(x=w-1;x--;)
					{
						*pDst++ = (uint8)( (pSrc[-1] + pSrc[1] + pSrc[-s]+ pSrc[-s+1])>>2 );
						pSrc++;
					}
					*pDst++ = (uint8)( (pSrc[-1] + pSrc[-s])>>1 );
					pSrc++;
					pSrc += (s-w);
					pDst += (s-w);
				break;

			case 2:
					// first line
					*pDst++ = (uint8)( (pSrc[1] + pSrc[s])>>1 );
					pSrc++;
					for(x=w-1;x--;)
					{
						*pDst++ = (uint8)( (pSrc[-1] + pSrc[1] + pSrc[s]+ pSrc[s-1])>>2 );
						pSrc++;
					}
					pSrc += (s-w);
					pDst += (s-w);

				for(y=h-2;y--;)
				{
					*pDst++ = (uint8)( (pSrc[-1] + pSrc[1] + pSrc[s] + pSrc[-s])>>2 );
					pSrc++;

					#ifdef DONT_USE_ASM //{

					for(x=w-2;x--;)
					{
						*pDst++ = (pSrc[-1] + pSrc[1] + pSrc[s] + pSrc[-s] +
									pSrc[s+1] + pSrc[s-1] + pSrc[-s+1] + pSrc[-s-1])>>3;
						pSrc++;
					}

					#else //}{

					x = w-2;

					__asm 
					{
						mov esi, pSrc
						//lea esi, [pSrc]
						mov edi, pDst
						//lea edi, [pDst]
						
					mainloop2:
						xor eax,eax						// load the sum into eax

						movzx edx,BYTE PTR [esi+1]		// eax += (BYTE) esi[1]
						add eax,edx						// we have to use edx cuz of the byte stuff

						movzx edx,BYTE PTR [esi-1]
						add eax,edx	

						mov ecx,esi						// ecx = esi + s
						add ecx,s
						movzx edx,BYTE PTR [ecx-1]		
						add eax,edx						
						movzx edx,BYTE PTR [ecx]		
						add eax,edx
						movzx edx,BYTE PTR [ecx+1]
						add eax,edx						

						mov ecx,esi	
						sub ecx,s
						movzx edx,BYTE PTR [ecx-1]
						add eax,edx	
						movzx edx,BYTE PTR [ecx]
						add eax,edx	
						movzx edx,BYTE PTR [ecx+1]
						add eax,edx	

						shr eax,3

						mov [edi],al

						inc edi
						inc esi
						dec x

						jnz mainloop2
					}

					pDst += w-2;
					pSrc += w-2;

					#endif //}

					*pDst++ = (uint8)( (pSrc[-1] + pSrc[1] + pSrc[s] + pSrc[-s])>>2 );
					pSrc++;
					pSrc += (s-w);
					pDst += (s-w);
				}
					// last line
					for(x=w-1;x--;)
					{
						*pDst++ = (uint8)( (pSrc[-1] + pSrc[1] + pSrc[-s]+ pSrc[-s+1])>>2 );
						pSrc++;
					}
					*pDst++ = (uint8)( (pSrc[-1] + pSrc[-s])>>1 );
					pSrc++;
					pSrc += (s-w);
					pDst += (s-w);
				break;
			case 3:
			default:
				// this code is deprecated

				pSrc += s + s;
				pDst += s + s;
				for(y=h-4;y--;)
				{
					pSrc += 2;
					pDst += 2;
					for(x=w-4;x--;)
					{
						*pDst++ = (uint8)( (pSrc[-1] + pSrc[1] + pSrc[-2] + pSrc[2] + 
									pSrc[s] + pSrc[-s] + pSrc[s+s] + pSrc[-s-s] +
									pSrc[s+1] + pSrc[s-1] + pSrc[-s+1] + pSrc[-s-1] +
									pSrc[s+2] + pSrc[s-2] + pSrc[-s+2] + pSrc[-s-2])>>4 );
						pSrc++;
					}
					pSrc += (s-w) + 2;
					pDst += (s-w) + 2;
				}
				pSrc += s + s;
				pDst += s + s;
				break;
			}
			
			assert( pSrc == ((uint8 *)FmBits + h*s) );
			assert( pDst == ((uint8 *)ToBits + h*s) );
			break;
		}


		case 2:
		case 3:
		case 4:
			// can't just use a simple blender for 2-3-4-byte pixel formats;
			// must decompose pixels to RGBA and blend
			//geErrorLog_AddString(-1,"geBitmapUtil_SmoothBits : only implemented for 1-byte data", NULL);
			return GE_FALSE;
	}

	return GE_TRUE;
}







//====================================================================================
//	ElectricFx_Create
//====================================================================================
Procedural *ElectricFx_Create(geBitmap **Bitmap, const char *StrParms)
{
	Procedural	*Proc;

	assert(Bitmap);
	//assert(ParmStart);	// Unremark this when implemented!!!!!

	Proc = GE_RAM_ALLOCATE_STRUCT(Procedural);

	if (!Proc)
		goto ExitWithError;

	memset(Proc, 0, sizeof(*Proc));

	if (*Bitmap)
	{
		Proc->Bitmap = *Bitmap;

		geBitmap_CreateRef(Proc->Bitmap);
	}
	else
	{
		// Must make bitmap for tha caller!!!
		goto ExitWithError;
	}

	if (!geBitmap_ClearMips(Proc->Bitmap))
		goto ExitWithError;

	{
		// We need to change the format of this bitmap to a 4444 in the world (hope he doesn't mind ;)
		gePixelFormat	Format;
		geBitmap_Info			Info;

		Format = GE_PIXELFORMAT_16BIT_4444_ARGB;

		if (!geBitmap_SetFormat(Proc->Bitmap, Format, GE_TRUE, 255, NULL))
			goto ExitWithError;

		if (!geBitmap_GetInfo(Proc->Bitmap, &Info, NULL))
			goto ExitWithError;

		Proc->Width = Info.Width;
		Proc->Height = Info.Height;
		Proc->Size = Proc->Width*Proc->Height;

		Proc->WMask = Proc->Width-1;
		Proc->HMask = Proc->Height-1;

		Proc->ZBuffer = GE_RAM_ALLOCATE_ARRAY(uint8, Proc->Width*Proc->Height);
		if (!Proc->ZBuffer)
			goto ExitWithError;

		Proc->Smtex = GE_RAM_ALLOCATE_ARRAY(uint8, Proc->Width*Proc->Height);
		if (!Proc->Smtex)
			goto ExitWithError;
	}

	// Clear the buffers out
	memset(Proc->ZBuffer,0, Proc->Size*sizeof(uint8));
	memset(Proc->Smtex, 0, Proc->Size*sizeof(uint8));

	{
		


	#if 0
		Proc->NumVecs = 1;
		Proc->FxType = 3;

		Proc->Vecs[0].x = Proc->Width>>1;
		Proc->Vecs[0].y = Proc->Height>>1;
	#else
		Proc->NumVecs = 4;
		Proc->FxType = 0;

		Proc->Vecs[0].x = 1;
		Proc->Vecs[0].y = 1;

		Proc->Vecs[1].x = Proc->Width-1;
		Proc->Vecs[1].y = Proc->Height-1;

		Proc->Vecs[2].x = Proc->Width-1;
		Proc->Vecs[2].y = 1;
		Proc->Vecs[3].x = 1;
		Proc->Vecs[3].y = Proc->Height-1;
	#endif

		ElectricFx_BuildRGBLuts(Proc);
	}

	if (!ElectricFx_Animate(Proc, 0.1f))
	{
		goto ExitWithError;
	}

	return Proc;

	ExitWithError:
	{
		if (Proc)
			ElectricFx_Destroy(Proc);


		return NULL;
	}

	// get rid of warnings
	StrParms;
}

//====================================================================================
//	ElectricFx_Destroy
//====================================================================================
void ElectricFx_Destroy(Procedural *Proc)
{
	assert(Proc);

	if (Proc->ZBuffer)
	{
		geRam_Free(Proc->ZBuffer);
		Proc->ZBuffer = NULL;
	}
	
	if (Proc->Smtex)
	{
		geRam_Free(Proc->Smtex);
		Proc->Smtex = NULL;
	}

	if (Proc->Bitmap)
	{
		geBitmap_Destroy(&Proc->Bitmap);
		Proc->Bitmap = NULL;
	}

	geRam_Free(Proc);
}
/*
typedef enum
{
	Fx_None = 0,
	Fx_Line,
	Fx_Sparkle,
	Fx_SpinSparkle
} ElectrixFx_Type;

typedef struct
{
	ElectrixFx_Type	Type;
	char			Str[256];
} ElectrixFx_TypeField;

ElectrixFx_TypeField FxFields[] = 
{
	{Fx_Line, "Fx_Line"},
	{Fx_Sparkle, "Fx_Sparkle"},
	{Fx_SpinSparkle, "Fx_SpinSparkle"}
}

int32 NumFxFields = (sizeof(FxFields)/sizeof(ElectrixFx_TypeField)) - 1;

//====================================================================================
//	ElectricFx_GetFxTypeFromString
//====================================================================================
ElectricFx_Type	ElectricFx_GetFxTypeFromString(const char *Str)
{
	for (i=0; i<NumFxFields; i++)
	{
		if (!stricmp(Str, FxFields[i].Str))
			return FxFields[i].Type;
	}
}
*/

//====================================================================================
//	ElectricFx_Animate
//====================================================================================
geBoolean ElectricFx_Animate(Procedural *Fx, float ElapsedTime)
{
	if (!Fx->Bitmap)
		return GE_TRUE;

	ElectricFx_Update(Fx, ElapsedTime);

	if (!ElectricFx_ApplyToBitmap(Fx))
		return GE_FALSE;

	return GE_TRUE;
}

//====================================================================================
//====================================================================================
static Procedural_Table ElectricFx_Table = 
{
	Cur_Procedurals_Version,
	"ElectricFx",
	ElectricFx_Create,
	ElectricFx_Destroy,
	ElectricFx_Animate
};

/*
//====================================================================================
//====================================================================================
DllExport Procedural_Table *GetProcedural_Table()
{
	return ElectricFx_GetProcedural_Table();
}
*/

//====================================================================================
//====================================================================================
Procedural_Table *ElectricFx_GetProcedural_Table(void)
{
	return &ElectricFx_Table; 
}

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// sgn() - This function is used by Line() to determine the sign of a long //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

int sgn (long a) 
{
	if (a > 0) 
		return +1;
	else if (a < 0) 
		return -1;
	else 
		return 0;
}


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// round() - This function is used by Line() to round a long to the        //
//           nearest integer.                                              //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

int round (long a) 
{
//undone
	return a;
	/*if ( (a - (int)a) < 0.5) 
		return (int)floor(a);
	else 
		return (int)ceil(a);*/
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void ElectricFx_PutZ(Procedural *Fx, int x, int y, int ZVal, float ZAge) 
{
	Fx->ZBuffer[(y&Fx->HMask)*Fx->Width+(x&Fx->WMask)] = (uint8)ZVal;

	// get rid of warnings
	ZAge;
}


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// Line() - This draws a line from a,b to c,d of color col.                //
//          This function will be explained in more detail in tut3new.zip  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
void ElectricFx_ZLine(Procedural *Fx, int a, int b, int c, int d, int32 ZVal, float ZAge) 
{

	long	u,s,v,d1x,d1y,d2x,d2y,m,n;
	int		i;

	u   = c-a;			// x2-x1
	v   = d-b;			// y2-y1
	d1x = sgn(u);		// d1x is the sign of u (x2-x1) (VALUE -1,0,1)
	d1y = sgn(v);		// d1y is the sign of v (y2-y1) (VALUE -1,0,1)
	d2x = sgn(u);		// d2x is the sign of u (x2-x1) (VALUE -1,0,1)
	d2y = 0;
	m   = abs(u);		// m is the distance between x1 and x2
	n   = abs(v);		// n is the distance between y1 and y2

	if (m<=n)			// if the x distance is greater than the y distance
	{     
		d2x = 0;
		d2y = sgn(v);	// d2y is the sign of v (x2-x1) (VALUE -1,0,1)
		m   = abs(v);	// m is the distance between y1 and y2
		n   = abs(u);	// n is the distance between x1 and x2
	}

	s = (int)(m>>1);						// s is the m distance (either x or y) divided by 2

	for (i=round(m); i>0; i--)				// repeat this loop until it
	{ 
											// is = to m (y or x distance)
		ElectricFx_PutZ(Fx, a, b, ZVal, ZAge);// plot a pixel at the original x1, y1

		s += n;								// add n (dis of x or y) to s (dis of x of y)
		if (s >= m)							// if s is >= m (distance between y1 and y2)
		{  
			s -= m;
			a += d1x;
			b += d1y;
		}
		else 
		{
			a += d2x;
			b += d2y;
		}
	}

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void ElectricFx_ZLine2(Procedural *Fx, int x1, int y1, int x2, int y2, int32 ZVal, float ZAge) 
{

	int		x, y, xlength, ylength, dx = 0, dy = 0;
	float	xslope, yslope;

	xlength = abs(x1-x2);

	if ((x1-x2)  < 0) dx = -1;
	if ((x1-x2) == 0) dx =  0;
	if ((x1-x2)  > 0) dx = +1;

	ylength = abs(y1-y2);
	if ((y1-y2)  < 0) dy = -1;
	if ((y1-y2) == 0) dy =  0;
	if ((y1-y2)  > 0) dy = +1;

	if (dy == 0) 
	{
		if (dx < 0)
			for (x=x1; x<x2+1; x++)
				ElectricFx_PutZ(Fx, x, y1, ZVal, ZAge);
		if (dx > 0)
			for (x=x2; x<x1+1; x++)
				ElectricFx_PutZ(Fx, x, y1, ZVal, ZAge);
	}

	if (dx == 0) 
	{
		if (dy < 0)
			for (y=y1; y<y2+1; y++)
				ElectricFx_PutZ(Fx, x1, y, ZVal, ZAge);
		if (dy > 0)
			for (y=y2; y<y1+1; y++)
				ElectricFx_PutZ(Fx, x1, y, ZVal, ZAge);
	}

	if ((xlength != 0) && (ylength != 0)) 
	{
		xslope = (float)xlength/(float)ylength;
		yslope = (float)ylength/(float)xlength;
	}
	else 
	{
		xslope = 0.0f;
		yslope = 0.0f;
	}

	if ((xslope != 0) && (yslope != 0) && (yslope/xslope < 1) && (yslope/xslope > -1)) 
	{
		if (dx < 0)
		{
			for (x=x1; x<x2+1; x++) 
			{
				y = round( (int)yslope*x );
				ElectricFx_PutZ(Fx, x, y, ZVal, ZAge);
			}
		}
		if (dx > 0)
		{
			for (x=x2; x<x1+1; x++) 
			{
				y = round( (int)(yslope*x));
				ElectricFx_PutZ(Fx, x, y, ZVal, ZAge);
			}
		}
	}
	else 
	{
		if (dy < 0)
		{
			for (y=x1; y<x2+1; y++) 
			{
				x = round( (int)(xslope*y));
				ElectricFx_PutZ(Fx, x, y, ZVal, ZAge);
			}
		}
		if (dy > 0)
		{
			for (y=x2; y<x1+1; y++) 
			{
				x = round( (int)(xslope*y));
				ElectricFx_PutZ(Fx, x, y, ZVal, ZAge);
			}
		}
	}

}

//==================================================================================
//	ElectricFx_ElectricZLine_r
//==================================================================================
void ElectricFx_ElectricZLine_r(	Procedural *Fx, 
							int32 x1, int32 y1, 
							int32 x2, int32 y2, 
							int32 ZVal, float ZAge,
							int32 Recursion)
{
	if (Recursion > 0)
	{
		int32	MidX, MidY;

		MidX = (x1+x2)>>1;
		MidY = (y1+y2)>>1;

		MidX += 8 - (rand()&15);
		MidY += 8 - (rand()&15);
		/*
		if (MidX < 0)
			MidX = 0;
		else if (MidX >= Fx->Width)
			MidX = Fx->Width-1;
	
		if (MidY < 0)
			MidY = 0;
		else if (MidY >= Fx->Height)
			MidY = Fx->Height-1;
		*/

		ElectricFx_ElectricZLine_r(Fx, x1, y1, MidX, MidY, ZVal, ZAge, Recursion>>1);
		ElectricFx_ElectricZLine_r(Fx, MidX, MidY, x2, y2, ZVal, ZAge, Recursion>>1);
	}
	else
	{
		ElectricFx_ZLine(Fx, x1, y1, x2, y2, ZVal, ZAge);
	}
}

//==================================================================================
//
//==================================================================================
void ElectricFx_Shade(Procedural *Fx)
{
	uint8	*ZBuffer;
	uint8	*Smtex;
	int		i;

	ZBuffer = Fx->ZBuffer;
	Smtex = Fx->Smtex;

	// Shade the data for the Fx using the z buffer to provide
	// occlusion information.
	for(i=0;i != Fx->Size-5; i++, ZBuffer++)
	{
		int32	Result;
		int32	Val;

		if(*ZBuffer == 0)
			continue;

	#if 1
		Val = (int32)*ZBuffer;

		Result = Val;// - *(ZBuffer-1);

		Smtex[i] = (uint8)( min(Result + Smtex[i], 255) );
		
		if(Val > ZBuffer[1])
			Smtex[i+1] = (uint8)( max(Smtex[i+1]-3,0) ); 
		if(Val > ZBuffer[2])
			Smtex[i+1] = (uint8)( max(Smtex[i+2]-7,0) );
		if(Val > ZBuffer[3])
			Smtex[i+3] = (uint8)( max(Smtex[i+3]-10,0) );
		if(Val > ZBuffer[4])
			Smtex[i+4] = (uint8)( max(Smtex[i+4]-15,0) );
		if(Val > ZBuffer[5])
			Smtex[i+5] = (uint8)( max(Smtex[i+5]-7,0) );
	#else
		Val = *ZBuffer;

		Result = Val;

		Smtex[i] = min(Result + Smtex[i], 255);
	#endif

	}

	// Take the old texture buffer and smooth it, using a basic bilinear filter.
	// Don't worry about previous calculations affecting current ones - we aren't trying
	// to be overly correct here.
	// Could do a linear filter vertically first with a DWORD pointer for speed?
	{
		int   k;
		for(k=0;k<2;k++)
		{
			
			// Be safe and set all the edges to 0, otherwise it will wrap in an annoying fashion
			for(i=0;i<Fx->Width;i++)									// Top
				Smtex[i] = 0;

			for(i=Fx->Width*(Fx->Height-1);i<(Fx->Size);i++)	// Bottom
				Smtex[i] = 0;

			for(i=0;i<Fx->Width*(Fx->Height-1);i+=Fx->Width )	// Left
				Smtex[i] = 0;

			for(i=Fx->Width-1;i<Fx->Size;i+= Fx->Width)		// Right
				Smtex[i] = 0;
			
			{
				geBitmap_Info	SmtexInfo;
				uint8			*SmtexBits;

				SmtexInfo.Width = Fx->Width;
				SmtexInfo.Height = Fx->Height - 1;
				SmtexInfo.Stride = SmtexInfo.Width;
				SmtexInfo.Format = GE_PIXELFORMAT_8BIT_GRAY;
				SmtexBits = Smtex + Fx->Width;

				geBitmapUtil_SmoothBits(&SmtexInfo, SmtexBits, SmtexBits, 1);
			}

		}
	}
}

//==================================================================================
//==================================================================================
void ElectricFx_Update(	Procedural *Fx, float Time)
{
	uint8		*ZBuffer;
	uint8		*Smtex;

	ZBuffer = Fx->ZBuffer;
	Smtex = Fx->Smtex;

	// Zero the destination buffer
	memset(ZBuffer,0, Fx->Size*sizeof(uint8));

	switch (Fx->FxType)
	{
		case 0:
		{
			int32		i;

			for (i=0; i< Fx->NumVecs>>1; i++)
			{
				int32				Val;
				ElectricFx_Vec2d	*pVec1, *pVec2;

				Val = i<<1;

				pVec1 = &Fx->Vecs[Val];
				pVec2 = &Fx->Vecs[Val+1];

				ElectricFx_ElectricZLine_r(Fx, pVec1->x, pVec1->y, pVec2->x, pVec2->y, 54, 1.0f, 16);
			}

			break;
		}

		case 1:
		{
			int32	i;

			for (i=0; i<6; i++)
			{
				int32		x, y;
				float		r;

				r = ((float)i/6.0f)*PI_2;

				x = (int)(cos(r)*62.0f);
				y = (int)(sin(r)*62.0f);

				x+= 63;
				y+= 63;

				ElectricFx_ElectricZLine_r(Fx, 63, 63, x, y, 1, 1.0f, 4);
			}

			break;
		}

		case 2:
		{
			int32	i;

			for (i=0; i<3; i++)
			{
				int32		x, y;
				float		r;

				r = ((float)i/6.0f)*PI_2;

				//r += 2.0f - ((float)myrand()/RAND_MAX)*4.0f;
				r += 2.0f - ((float)rand()/0xffff)*4.0f;

				x = (int)(cos(r)*62.0f);
				y = (int)(sin(r)*62.0f);

				x+= 63;
				y+= 63;

				ElectricFx_ElectricZLine_r(Fx, 63, 63, x, y, 1, 1.0f, 4);
			}

			break;
		}

		case 3:
		{
			int32				j;
			ElectricFx_Vec2d	*pVec;

			pVec = Fx->Vecs;

			for (j=0; j<Fx->NumVecs; j++, pVec++)
			{
				int32	i, x, y;
				float	r;

				for (i=0; i<3; i++)
				{
					float	Val;

					r = Fx->Rotation;
				
					Val = ((float)i/3) * PI_2;

					r += Val;
				
					x = (int)(cos(r)*63.0f);
					y = (int)(sin(r)*63.0f);

					x += pVec->x;
					y += pVec->y;

					ElectricFx_ElectricZLine_r(Fx, pVec->x, pVec->y, x, y, 74, 1.0f, 8);
				}
			}
			break;
		}
	}

	Fx->Rotation += 2.0f*Time;

	if (Fx->Rotation > PI_2)
		Fx->Rotation -= PI_2;

	// Shade the Fx
	ElectricFx_Shade(Fx);
}

//====================================================================================
//====================================================================================
geBoolean ElectricFx_ApplyToBitmap(Procedural *Fx)
{
	geBitmap		*Dest[4];
	int32			m;
	geBitmap_Info	MainInfo;
	uint8			*pSrc;
	uint16			*pCLut;

	assert(Fx->Bitmap);

	if (!geBitmap_GetInfo(Fx->Bitmap, &MainInfo, NULL))
	{
		return GE_FALSE;
	}

	pSrc = Fx->Smtex;

	assert(MainInfo.MaximumMip == 0);

	//assert(MainInfo.Format == GE_PIXELFORMAT_16BIT_4444_ARGB);

	if (MainInfo.Format != GE_PIXELFORMAT_16BIT_4444_ARGB)
		return GE_TRUE;			// Oh well...
	
	if (!geBitmap_LockForWrite(Fx->Bitmap, Dest, 0, MainInfo.MaximumMip))
	{
		return GE_FALSE;
	}

	pCLut = Fx->CLut;
	
	for (m=0; m<= MainInfo.MaximumMip; m++)
	{
		uint16			*pDest16;
		int32			w, h, Extra;
		//int16			*pSrc;
		geBitmap_Info	Info;

		if (!geBitmap_GetInfo(Dest[m], &Info, NULL))
		{
			return GE_FALSE;
		}
		
		Extra = Info.Stride - Info.Width;
		
		pDest16 = geBitmap_GetBits(Dest[m]);

		//pSrc = &WaterData[NPage][0];

		for (h=0; h< Info.Height; h++)
		{
			for (w=0; w< Info.Width; w++)
			{
				uint8	Val;

				Val = (uint8)*pSrc++;

				*pDest16++ = pCLut[Val];
			}

			pDest16 += Extra;
			pSrc += Extra;
		}
		
		if (!geBitmap_UnLock(Dest[m]))
		{
			return GE_FALSE;
		}
	}

	return GE_TRUE;
}

uint16 LerpColor(float c1, float c2, float Ratio)
{
	float	Val;

	Val = c1+(c2 - c1)*Ratio;

	Val *= (1.0f/3.0f);

	if (Val > 15.0f)
		Val = 15.0f;

	return (uint16)( Val );
}

//====================================================================================
//====================================================================================
void ElectricFx_BuildRGBLuts(Procedural *Fx)
{
	int32	i;
	float	a[4], r[4], g[4], b[4];
	float	f[4];
	int32	NumControlPoints, Current;
	
	NumControlPoints = 3;

#if 1
	f[0] = 0.0f;
	a[0] = 0.0f;
	r[0] = 0.0f;
	g[0] = 0.0f;
	b[0] = 0.0f;
	
	f[1] = 30.0f;
	a[1] = 70.0f;
	r[1] = 0.0f;
	g[1] = 30.0f;
	b[1] = 0.0f;

	f[2] = 180.0f;
	a[2] = 255.0f;
	r[2] = 255.0f;
	g[2] = 255.0f;
	b[2] = 25.0f;

#else
	f[0] = 0.0f;
	a[0] = 0.0f;
	r[0] = 0.0f;
	g[0] = 0.0f;
	b[0] = 0.0f;
	
	f[1] = 30.0f;
	a[1] = 70.0f;
	r[1] = 40.0f;
	g[1] = 0.0f;
	b[1] = 0.0f;

	f[2] = 180.0f;
	a[2] = 255.0f;
	r[2] = 255.0f;
	g[2] = 255.0f;
	b[2] = 25.0f;
#endif
	Current = 0;

	for (i=0; i<256; i++)
	{
		float	Ratio;
		int32	Next;

		Next = Current+1;

		if (Next > NumControlPoints-1)
			Next = NumControlPoints-1;

		Ratio = (float)(i-f[Current])/f[Next];

		if (Ratio > 1.0f)
			Ratio = 1.0f;
		else if (Ratio < 0.0f)
			Ratio = 0.0f;

		Fx->CLut[i]  = (uint16)(LerpColor(a[Current], a[Next], Ratio) << 12);
		Fx->CLut[i] |= (LerpColor(r[Current], r[Next], Ratio) << 8);
		Fx->CLut[i] |= (LerpColor(g[Current], g[Next], Ratio) << 4);
		Fx->CLut[i] |= (LerpColor(b[Current], b[Next], Ratio)) ;
		
		if ((float)i >= f[Next])
		{
			Current++;

			if (Current > NumControlPoints-1)
				Current = NumControlPoints-1;
		}
	}
}
