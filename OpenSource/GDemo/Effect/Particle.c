////////////////////////////////////////////////////////////////////////////////////////
//  Particle.c									                                        
//                                                                                      
//  Author:
//		Eli Boling		Created
//		Peter Siamidis	Added anchor point support.
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
#include <memory.h>
#include <assert.h>
#include "genesis.h"
#include "Ram.h"
#include "particle.h"

#define	POLYQ

#define	PARTICLE_USER			( 1 << 0 )
#define	PARTICLE_HASVELOCITY	( 1 << 1 )
#define PARTICLE_HASGRAVITY		( 1 << 2 )
#define PARTICLE_RENDERSTYLE	GE_RENDER_DEPTH_SORT_BF | GE_RENDER_DO_NOT_OCCLUDE_OTHERS

#define	WORLDTOMETERS(w)	((w) / (geFloat)32.0f)
#define METERSTOWORLD(m)	((m) * (geFloat)32.0f)

#define EARTHGRAVITYMAGNITUDE	(((geFloat)9.81f) / (geFloat)1.0f)

typedef struct	Particle
{
	GE_LVertex			 	 ptclVertex;
	geBitmap *		 ptclTexture;
	gePoly *				 ptclPoly;
	unsigned 	 	 		 ptclFlags;
	Particle *			 	 ptclNext;
	Particle *			 	 ptclPrev;
	float					 Scale;
	geVec3d					 Gravity;
	float					Alpha;
	geVec3d					CurrentAnchorPoint;
	const geVec3d			*AnchorPoint;

	union
	{
		struct
		{
			geFloat	 	 ptclTime;
			geFloat		 ptclTotalTime;
			geVec3d	 	 ptclVelocity;
		}	builtin;
		struct
		{
			ParticleCallBack ptclCallBack;
			void *			 ptclUserData;
		}	user;
	}	p;
}	Particle;

typedef	struct	Particle_System
{
	Particle *		psParticles;
	geWorld *		psWorld;
	geFloat			psLastTime;
	geFloat			psQuantumSeconds;
}	Particle_System;

void PARTICLE_CALLINGCONVENTION Particle_SetTexture(Particle *p, geBitmap *Texture)
{
	assert(p->ptclFlags & PARTICLE_USER);
	p->ptclTexture = Texture;
}

GE_LVertex * PARTICLE_CALLINGCONVENTION Particle_GetVertex(Particle *p)
{
	assert(p->ptclFlags & PARTICLE_USER);
	return &p->ptclVertex;
}

void * PARTICLE_CALLINGCONVENTION Particle_GetUserData(Particle  *p)
{
	assert(p->ptclFlags & PARTICLE_USER);
	return p->p.user.ptclUserData;
}

#define	QUANTUMSIZE	(1.0f / 30.0f)

Particle_System * PARTICLE_CALLINGCONVENTION Particle_SystemCreate(geWorld *World)
{
	Particle_System *	ps;

	ps = (Particle_System *)geRam_Allocate(sizeof(*ps));
	if	(!ps)
		return ps;

	memset(ps, 0, sizeof(*ps));
	
	ps->psWorld			= World;
	ps->psQuantumSeconds = QUANTUMSIZE;
	ps->psLastTime = 0.0f;

	return ps;
}

static	void PARTICLE_CALLINGCONVENTION	DestroyParticle(Particle_System *ps, Particle *p)
{
	if	(p->ptclFlags & PARTICLE_USER)
	{
		if	(p->p.user.ptclUserData)
			geRam_Free(p->p.user.ptclUserData);
	}

	if	(p->ptclPoly)
		geWorld_RemovePoly(ps->psWorld, p->ptclPoly);

	geRam_Free(p);
}

void PARTICLE_CALLINGCONVENTION	Particle_SystemDestroy(Particle_System *ps)
{
	Particle *	ptcl;

	ptcl = ps->psParticles;
	while	(ptcl)
	{
		Particle *	temp;

		temp = ptcl->ptclNext;
		DestroyParticle(ps, ptcl);
		ptcl = temp;
	}

	geRam_Free(ps);
}

static	void PARTICLE_CALLINGCONVENTION	UnlinkParticle(Particle_System *ps, Particle *ptcl)
{
	if	(ptcl->ptclPrev)
		ptcl->ptclPrev->ptclNext = ptcl->ptclNext;

	if	(ptcl->ptclNext)
		ptcl->ptclNext->ptclPrev = ptcl->ptclPrev;

	if	(ps->psParticles == ptcl)
		ps->psParticles = ptcl->ptclNext;
}

void PARTICLE_CALLINGCONVENTION Particle_SystemRemoveAll
(Particle_System *ps)
{
	Particle *	ptcl;

	ptcl = ps->psParticles;
	while	(ptcl)
	{
		Particle *	temp;

		temp = ptcl->ptclNext;
		UnlinkParticle(ps, ptcl);
		DestroyParticle(ps, ptcl);
		ptcl = temp;
	}
}

int32 PARTICLE_CALLINGCONVENTION	Particle_GetCount(Particle_System *ps)
{

	// locals
	Particle	*ptcl;
	int32		TotalParticleCount = 0;

	// ensure valid data
	assert( ps != NULL );

	// count up how many particles are active in this particle system
	ptcl = ps->psParticles;
	while ( ptcl )
	{
		ptcl = ptcl->ptclNext;
		TotalParticleCount++;
	}

	// return the active count
	return TotalParticleCount;

}

void PARTICLE_CALLINGCONVENTION	Particle_SystemFrame(Particle_System *ps, geFloat DeltaTime)
{

	geVec3d	AnchorDelta;
//	geFloat	DeltaT;
//	unsigned long	CurrentTime;

//	CurrentTime = timeGetTime();
//	ps->psFrameTime += CurrentTime - ps->psLastTime;

//	DeltaT = Time - ps->psLastTime;

//	while	(ps->psFrameTime >= ps->psQuantumSize)

	// the quick fix to the particle no-draw problem 
	ps->psQuantumSeconds = DeltaTime;
	//while	(DeltaTime > QUANTUMSIZE)
	{

		Particle *	ptcl;

		ptcl = ps->psParticles;
		while	(ptcl)
		{
			if	(ptcl->ptclFlags & PARTICLE_USER)
			{
				assert(ptcl->p.user.ptclCallBack);
				if	((ptcl->p.user.ptclCallBack)(ps, ptcl, ps->psQuantumSeconds) == GE_FALSE)
				{
					Particle *	temp;

					temp = ptcl->ptclNext;
					UnlinkParticle(ps, ptcl);
					DestroyParticle(ps, ptcl);
					ptcl = temp;
					continue;
				}
			}
			else
			{
//extern	GE_Engine *		Engine;
//			GE_EnginePrintf(Engine, 2, 40, "%4.2f %4.2f %4.2f", ptcl->p.builtin.ptclVelocity.X, ptcl->p.builtin.ptclVelocity.Y, ptcl->p.builtin.ptclVelocity.Z);
				ptcl->p.builtin.ptclTime -= ps->psQuantumSeconds;
				if	(ptcl->p.builtin.ptclTime <= 0.0f)
				{
					Particle *	temp;

					temp = ptcl->ptclNext;
					UnlinkParticle(ps, ptcl);
					DestroyParticle(ps, ptcl);
					ptcl = temp;
					continue;
				}
				else
				{

					// locals
					geVec3d		DeltaPos = { 0.0f, 0.0f, 0.0f };

					// apply velocity
					if ( ptcl->ptclFlags & PARTICLE_HASVELOCITY )
					{
						geVec3d_Scale( &( ptcl->p.builtin.ptclVelocity ), ps->psQuantumSeconds, &DeltaPos );
					}

					// apply gravity
					if ( ptcl->ptclFlags & PARTICLE_HASGRAVITY )
					{

						// locals
						geVec3d	Gravity;

						// make gravity vector
						geVec3d_Scale( &( ptcl->Gravity ), ps->psQuantumSeconds, &Gravity );

						// apply gravity to built in velocity and DeltaPos
						geVec3d_Add( &( ptcl->p.builtin.ptclVelocity ), &Gravity, &( ptcl->p.builtin.ptclVelocity ) );
						geVec3d_Add( &DeltaPos, &Gravity, &DeltaPos );
					}
					
					// apply DeltaPos to particle position
					if (	( ptcl->ptclFlags & PARTICLE_HASVELOCITY ) ||
							( ptcl->ptclFlags & PARTICLE_HASGRAVITY ) )
					{
						geVec3d_Add( (geVec3d *)&( ptcl->ptclVertex.X ), &DeltaPos, (geVec3d *)&( ptcl->ptclVertex.X ) );
					}

					// make the particle follow its anchor point if it has one
					if ( ptcl->AnchorPoint != NULL )
					{
						geVec3d_Subtract( ptcl->AnchorPoint, &( ptcl->CurrentAnchorPoint ), &AnchorDelta );
						geVec3d_Add( (geVec3d *)&( ptcl->ptclVertex.X ), &AnchorDelta, (geVec3d *)&( ptcl->ptclVertex.X ) );
						geVec3d_Copy( ptcl->AnchorPoint, &( ptcl->CurrentAnchorPoint ) );
					}

					// destroy the particle if it is in solid space
					/*{

						// locals
						GE_Contents	Contents;
						static		geVec3d		Mins = { -1.0f, -1.0f, -1.0f };
						static		geVec3d		Maxs = {  1.0f,  1.0f,  1.0f };

						// zap it if its in solid space
						if ( GE_WorldGetContents( ps->psWorld, (geVec3d *)&( ptcl->ptclVertex.X ), &Mins, &Maxs, GE_COLLIDE_ALL, 0xffffffff, &Contents ) == GE_TRUE )
						{
							if ( Contents.Contents & GE_CONTENTS_SOLID )
							{

								// locals
								Particle	*temp;
	
								// destroy it
								temp = ptcl->ptclNext;
								UnlinkParticle( ps, ptcl );
								DestroyParticle( ps, ptcl );
								ptcl = temp;
								continue;
							}
						}
					}*/
				}
			}

#ifndef	POLYQ
			// set particle alpha
			assert( ptcl->p.builtin.ptclTotalTime > 0.0f );
			ptcl->ptclVertex.a = ptcl->Alpha * ( ptcl->p.builtin.ptclTime / ptcl->p.builtin.ptclTotalTime );
			geWorld_AddPolyOnce(ps->psWorld,
								&ptcl->ptclVertex,
								1,
								ptcl->ptclTexture,
								GE_TEXTURED_POINT,
								PARTICLE_RENDERSTYLE,
								ptcl->Scale );
#else
			// set particle alpha
			assert( ptcl->p.builtin.ptclTotalTime > 0.0f );
			ptcl->ptclVertex.a = ptcl->Alpha * ( ptcl->p.builtin.ptclTime / ptcl->p.builtin.ptclTotalTime );

			if	(ptcl->ptclPoly)
				gePoly_SetLVertex(ptcl->ptclPoly, 0, &ptcl->ptclVertex);
#endif

			ptcl = ptcl->ptclNext;
		}

		DeltaTime -= QUANTUMSIZE;
	}

	ps->psLastTime += DeltaTime;
}


static	Particle * PARTICLE_CALLINGCONVENTION	CreateParticle(
	Particle_System *	ps,
	geBitmap *	Texture,
	const GE_LVertex *	Vert )
{
	Particle *	ptcl;

	ptcl = (Particle *)geRam_Allocate(sizeof(*ptcl));
	if	(!ptcl)
		return ptcl;

	memset(ptcl, 0, sizeof(*ptcl));

	ptcl->ptclNext = ps->psParticles;
					 ps->psParticles = ptcl;
	if	(ptcl->ptclNext)
		ptcl->ptclNext->ptclPrev = ptcl;
	ptcl->ptclTexture = Texture;
	ptcl->ptclVertex = *Vert;

	return ptcl;
}

// removes all references to an anchor point
geBoolean PARTICLE_CALLINGCONVENTION	Particle_SystemRemoveAnchorPoint(
	Particle_System	*ps,				// particle system from which this anchor point will be removed
	geVec3d			*AnchorPoint )		// the anchor point to remove
{

	// locals	
	Particle	*ptcl;
	geBoolean	AtLeastOneFound = GE_FALSE;

	// ensure valid data
	assert( ps != NULL );
	assert( AnchorPoint != NULL );

	// eliminate achnor point from all particles in this particle system
	ptcl = ps->psParticles;
	while ( ptcl != NULL )
	{
		if ( ptcl->AnchorPoint == AnchorPoint )
		{
			ptcl->AnchorPoint = NULL;
			AtLeastOneFound = GE_TRUE;
		}
		ptcl = ptcl->ptclNext;
	}

	// all done
	return AtLeastOneFound;
}

void PARTICLE_CALLINGCONVENTION	Particle_SystemAddParticle(
	Particle_System		*ps,
	geBitmap	*Texture,
	const GE_LVertex	*Vert,
	const geVec3d		*AnchorPoint,
	geFloat				Time,
	const geVec3d		*Velocity,
	float				Scale,
	const geVec3d		*Gravity )
{

	// locals
	Particle	*ptcl;

	// create a new particle
	ptcl = CreateParticle( ps, Texture, Vert );
	if ( !ptcl )
	{
		return;
	}

	// setup gravity
	if ( Gravity != NULL )
	{
		geVec3d_Copy( Gravity, &( ptcl->Gravity ) );
		ptcl->ptclFlags |= PARTICLE_HASGRAVITY;
	}

	// setup velocity
	if ( Velocity != NULL )
	{
		geVec3d_Copy( Velocity, &( ptcl->p.builtin.ptclVelocity ) );
		ptcl->ptclFlags |= PARTICLE_HASVELOCITY;
	}

	// setup the anchor point
	if ( AnchorPoint != NULL )
	{
		geVec3d_Copy( AnchorPoint, &( ptcl->CurrentAnchorPoint ) );
		ptcl->AnchorPoint = AnchorPoint;
	}

	// setup remaining data
	ptcl->Scale = Scale;
	ptcl->p.builtin.ptclTime = Time;
	ptcl->p.builtin.ptclTotalTime = Time;
	ptcl->Alpha = Vert->a;

	// add the poly to the world
	#ifdef	POLYQ
		ptcl->ptclPoly = geWorld_AddPoly(	ps->psWorld,
											&ptcl->ptclVertex,
											1,
											ptcl->ptclTexture,
											GE_TEXTURED_POINT,
											PARTICLE_RENDERSTYLE,
											ptcl->Scale );
	#endif
}

Particle * PARTICLE_CALLINGCONVENTION	Particle_SystemAddUserParticle(
	Particle_System *		ps,
	geBitmap *		Texture,
	const GE_LVertex	*	Vert,
	ParticleCallBack		CallBack,
	int						UserDataSize)
{
	Particle *	ptcl;

	ptcl = CreateParticle(ps, Texture, Vert);
	if	(!ptcl)
		return ptcl;

	if	(UserDataSize)
	{
		ptcl->p.user.ptclUserData = geRam_Allocate(UserDataSize);
		if	(!ptcl->p.user.ptclUserData)
		{
			UnlinkParticle(ps, ptcl);
			DestroyParticle(ps, ptcl);
		}
	}

	ptcl->ptclFlags |= PARTICLE_USER;
	ptcl->p.user.ptclCallBack = CallBack;

#ifdef	POLYQ
	ptcl->ptclPoly = geWorld_AddPoly(ps->psWorld,
									&ptcl->ptclVertex,
									1,
									ptcl->ptclTexture,
									GE_TEXTURED_POINT,
									PARTICLE_RENDERSTYLE,
									1.0f );
#endif

	return ptcl;
}

void PARTICLE_CALLINGCONVENTION Particle_SystemReset(Particle_System *ps)
{
	ps->psLastTime = 0.0f;
	Particle_SystemRemoveAll(ps);
}

