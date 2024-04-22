/****************************************************************************************/
/*  App.c                                                                               */
/*                                                                                      */
/*  Author: Frank Maddin	                                                            */
/*  Description:  Main module for the Genesis Demo                                      */
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

#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning(disable : 4201 4214 4115)
#include <mmsystem.h>
#pragma warning(default : 4201 4214 4115)

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include "genesis.h"
#include "ram.h"
#include "errorlog.h"

#include "usertypes.h"
#include "genvs.h"
#include "actorpool.h"
#include "modeltrigger.h"

#include "GPcorona.h"
#include "modelctl.h"
#include "dynlight.h"
#include "objmgr.h"
#include "physics.h"

#include "Effect.h"
#include "EffectM.h"
#include "SPool.h"
#include "TPool.h"
#include "door.h"

#include "autoselect.h"
#include "pathpoint.h"
#include "app.h"
#include "trigger.h"
#include "wmuser.h"

#include "CameraPathDlg.h"

#include "Gui.h"
#include "mouse.h"
#include "fade.h"
#include "gutil.h"
#include "GameMgr.h"
#include "cache.h"
#include "cam.h"

#pragma warning (disable:4514)	// unreferenced inline function (caused by Windows)

#define CAM_PLAYBACK GE_TRUE
#define GUI_TIMEOUT 10.0f
#define MAX_WORLDS 10

static char App_LevelList[MAX_WORLDS+1][_MAX_PATH] = 
{
	"levels\\squid.bsp",
	"levels\\heart.bsp",
	"levels\\reactor.bsp",
	"levels\\fire.bsp",
};

static int App_LevelMapping[] = 
{
	-1,	// unchecked background
	-1,	// full sequence
	3,	// hellfire
	2,	// reactor
	1,	// heart
	0,	// seafloor
	-1	// exit

};

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

typedef struct App_WorldData App_WorldData;
typedef struct App App;

typedef geBoolean SpawnF(geWorld *World, Object *Obj, void *ClassData);
typedef SpawnF *SpawnFP;

static void App_MovePos(App_WorldData *wd, Object *Obj, geVec3d *Pos, geXForm3d *XForm, float Time);
static void App_Error(const char *Msg, ...);
static void App_DisplayHourGlass(App *App);

static geBoolean App_EntitySetupObjects(App_WorldData *wd, ActorPool *ActorPool);
static Object	*App_EntityGetCameraObject(App_WorldData *wd, ActorPool *ActorPool, geXForm3d *XForm);
static geBoolean App_EntityResetObjects(App_WorldData *wd);
static geBoolean App_EntityResetPhysics(App_WorldData *wd, char *ClassType, SpawnFP SpawnFunc);
static void App_SaveErrorLog(void);

// global to this function
static App *GlobApp;

// loan global - left over from GTest
geFloat		EffectScale = 0.3f;

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
typedef	struct
{
	geSound_System	*SoundSystem;
	TexturePool		*TPool;
	SoundPool		*SPool;
	EffectSystem	*ESystem;
	EffectManager	*EManager;

} Resource;

Resource	App_Resource;

typedef struct App_WorldData
{
	char			LevelName[MAX_PATH];
	geWorld			*World;
	ObjMgr			*ObjectList;
	Object			*CameraObject;
	CameraPath		*CameraPathEditor;
	gePath			*CameraPathPoint;
	geXForm3d		MyXForm;
	geVec3d			LocalPos;
	geBoolean		Attached;
	float			FieldOfView;
	geVec3d			Rot;
	Cam	*			Cam;
	geBoolean		FlyControl;

}App_WorldData;

typedef struct App
{
	char			*Name;
	HWND			hWnd;
	HINSTANCE		hInstance;
	BOOL			Running;
	VidMode			VidMode;
	int32			CWidth;
	int32			CHeight;
	geFloat			Velocity;
	geVec3d	Maxs;
	geVec3d	Mins;
	geCamera		*Camera;
	geBoolean		ShowHelp;
	geBoolean		ShowStats;
	geBoolean		DoFrameRateCounter;
	geBoolean		NextLevel;
	geEngine		*Engine;
	App_WorldData	WorldList[MAX_WORLDS];
	ActorPool		*ActorPool;
	Gui				*AppGui;
	float			GuiTime;
	geBoolean		GuiTimeDisable;
	geBoolean		GuiAutoPlay;
	int				GuiCurBg;
	App_WorldData	*wd;	// current world data

	// benchmark timing stuff
	float			BenchTotalRunTime;
	int				BenchTotalFrames;
	geBoolean		BenchTimingOn;

	// mouse
	float			MouseSpeedX;
	float			MouseSpeedY;
	int				MouseFlags;

	// timing
	SYSTEMTIME		SystemTime;
	LARGE_INTEGER	Freq, OldTick, CurTick;
	float			WorldTime;

	geBoolean		ResetTime;
	geBoolean		ManualPick;
	geBoolean		MapOverride;
	geBoolean		ScreenClearFlag;
	geBoolean		InAWindow;
	geBoolean		SoftwareMode;
	geBoolean		CameraPathPause;
	geBoolean		CameraAllowEdit;
	geBoolean		CameraPathPlayback;

	geVFile			*MainFS;  

	ModeList *		DriverModeList;
	int				ChangeDisplaySelection;
	int				DriverModeListLength;
	geBoolean		ChangingDisplayMode;	
					
	char			Dir[_MAX_PATH];

	geBitmap *		TimerBmp;
	geBitmap *		CreditsBmp;

	float			UserGamma;
}App;

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_Allocate()
//
//	Get memory for application.
//
////////////////////////////////////////////////////////////////////////////////////////
static App *App_Allocate(void)
{
	App *App;

	App = geRam_Allocate(sizeof(struct App));

	if (App == NULL)
	{
		geErrorLog_AddString(GE_ERR_MEMORY_RESOURCE, "Could not get memory for application.", NULL);
		return NULL;
	}

	memset(App, 0, sizeof(*App));

	App->Name = "GenesisDemo";
	App->hWnd;
	App->hInstance;
	App->Running		= GE_TRUE;
	App->Name		= "Genesis Demo";
	App->Velocity = 20.0f;
	geVec3d_Set(&App->Maxs, 5.0f,  5.0f,  5.0f );
	geVec3d_Set(&App->Mins, -5.0f, -5.0f, -5.0f );
	App->Engine = NULL;
	App->DoFrameRateCounter = GE_FALSE;
	App->NextLevel = GE_FALSE;
	App->ResetTime = GE_TRUE;
	App->CameraPathPlayback = CAM_PLAYBACK;
	App->UserGamma = 1.0f;

	return App;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_Destroy()
//
//	Free app memory.
//
////////////////////////////////////////////////////////////////////////////////////////
static void App_Destroy(App **App)
{
	assert(*App);
	geRam_Free(*App);
	*App = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_SetupCameraFromXForm()
//
//	Setup Camera from transform.
//
////////////////////////////////////////////////////////////////////////////////////////
static void App_SetupCameraFromXForm(App_WorldData *wd, geCamera *Camera, geXForm3d *XForm)
{
	geRect Rect;

	assert(wd != NULL);
	assert(Camera != NULL);
	assert(XForm != NULL);

	geCamera_SetWorldSpaceXForm(Camera, XForm);
	geCamera_GetClippingRect(Camera, &Rect);
	geCamera_SetAttributes(Camera, wd->FieldOfView, &Rect);
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_ChangeWorld()
//
//	Transition worlds - move all data over.
//
////////////////////////////////////////////////////////////////////////////////////////
static void App_ChangeWorld(App_WorldData *owd, App_WorldData *nwd)
{
	assert(owd != NULL);
	assert(nwd != NULL);

	geEngine_RemoveWorld(GlobApp->Engine, owd->World);
	owd->Attached = FALSE;

	geEngine_AddWorld(GlobApp->Engine, nwd->World);
	nwd->Attached = TRUE;

	if (ObjMgr_RemoveObjectsFromWorld(owd->World, owd->ObjectList) == GE_FALSE)
	{
		App_Error("App_ChangeWorld: ObjMgr_RemoveObjectsFromWorld failed");
	}
	if (ObjMgr_AddObjectsToWorld(nwd->World, nwd->ObjectList) == GE_FALSE)
	{
		App_Error("App_ChangeWorld: ObjMgr_AddObjectsToWorld failed");
	}

	Corona_ChangeWorld(nwd->World);

	if (App_EntityResetObjects(nwd) == GE_FALSE)
	{
		App_Error("App_ChangeWorld: App_EntityResetObjects failed");
	}

	if (App_EntityResetPhysics(nwd, "PhysicsObject", PhysicsObject_Spawn) == GE_FALSE)
	{
		App_Error("App_ChangeWorld: App_EntityResetPhysics failed");
	}
	if (App_EntityResetPhysics(nwd, "PhysicsJoint", PhysicsJoint_Spawn) == GE_FALSE)
	{
		App_Error("App_ChangeWorld: App_EntityResetPhysics failed");
	}
	if (App_EntityResetPhysics(nwd, "PhysicalSystem", PhysicalSystem_Spawn) == GE_FALSE)
	{
		App_Error("App_ChangeWorld: App_EntityResetPhysics failed");
	}

 	if ( App_Resource.EManager != NULL )
	{
		EffectM_DestroyManager( &App_Resource.EManager );
	}

	if (App_Resource.SoundSystem)
	{
		Effect_DeleteAll(App_Resource.ESystem);
		if (App_Resource.SPool)
			SPool_Destroy( &( App_Resource.SPool ) );
		geSound_DestroySoundSystem( App_Resource.SoundSystem );

		App_Resource.SoundSystem = geSound_CreateSoundSystem( GlobApp->hWnd );
		
		if ( App_Resource.SoundSystem != NULL )
		{
			App_Resource.SPool = SPool_Create( App_Resource.SoundSystem );
			Effect_SystemChangeSoundSystem( App_Resource.ESystem, App_Resource.SoundSystem);
			Cache_Wav(GlobApp->Dir, App_Resource.SPool);
			geSound_SetMasterVolume( App_Resource.SoundSystem, 0.0f);
		}
	}

	Effect_ChangeWorld( App_Resource.ESystem, nwd->World );

	TPool_ChangeWorld( App_Resource.TPool, nwd->World );

	App_Resource.EManager = EffectM_CreateManager( App_Resource.ESystem, App_Resource.TPool, App_Resource.SPool, NULL, GlobApp->SoftwareMode );
	if ( App_Resource.EManager == NULL )
	{
		App_Error( "App_ChangeWorld: Could not create effect manager.\n" );
	}

	EffectM_LoadWorldEffects( App_Resource.EManager, nwd->World );

	Trigger_Set(nwd->ObjectList, App_Resource.SPool, App_Resource.TPool, App_Resource.ESystem, App_Resource.EManager);
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_CameraPathCallback()
//
//	Receives data and commands from the CameraPath.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean App_CameraPathCallback(int Type, float Time, geXForm3d *XF1, geXForm3d *XF2)
{
	static geXForm3d SaveXForm;

	switch (Type)
	{
	case CAMERA_PUT_CUT_INDEX:
		{
		App_WorldData *wd;

		wd = GlobApp->wd;

		Cam_SetIndex(wd->Cam, (int)Time);
		break;
		}
	case CAMERA_PUT_CUT_TIME:
		{
		App_WorldData *wd;

		wd = GlobApp->wd;

		Cam_SetFrameTime(wd->Cam, Time);
		break;
		}
	case CAMERA_GET_DATA:
		{
		App_WorldData *wd;

		wd = GlobApp->wd;

		assert(wd != NULL);
		assert(wd->CameraObject);

		if (XF1 != NULL)
			{
			GUtil_BuildXFormFromRotationOrderXY(&wd->Rot, &wd->LocalPos, XF1);
			SaveXForm = *XF1;
			}

		if (XF2 != NULL)
			{
			geXForm3d_SetIdentity(XF2);
			XF2->Translation.X = 2.0;  // field of view default
			}
		break;
		}

	case CAMERA_PUT_DATA:
		{
		App_WorldData *wd;
		geVec3d angles;

		wd = GlobApp->wd;

		assert(wd != NULL);
		assert(wd->CameraObject);

		if (XF1 != NULL)
			{
			wd->MyXForm = *XF1;
			wd->LocalPos = wd->MyXForm.Translation;
			GUtil_GetEulerAngles2(&wd->MyXForm, &angles);

			wd->Rot = angles;

			ObjMgr_SetObjectXForm(wd->CameraObject, XF1);
			}

		if (XF2 != NULL)
			{
			if (XF2->Translation.X >= 0.5f && XF2->Translation.X <= 20.0f)
				wd->FieldOfView = XF2->Translation.X;
			else
				wd->FieldOfView = 2.0;
			}

		break;
		}
	}

	return GE_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_WndProc()
//
//	Message handling for the app.
//
////////////////////////////////////////////////////////////////////////////////////////
long App_WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch(iMessage)
	{
		case WM_ACTIVATEAPP:
		{
			if(GlobApp && GlobApp->Engine)
			{
				geEngine_Activate(GlobApp->Engine, wParam);
			}
			break;
		}

		case WM_CAMERA_EDIT_SAVE:
		{
			App_WorldData *wd = GlobApp->wd;
			Cam_SaveMotionDebug(wd->Cam, wd->LevelName);
			break;
		}

		case WM_CAMERA_EDIT_QUIT:
		{
			App_WorldData *wd = GlobApp->wd;

			assert(wd);
			Mouse_SetForControl();
			Cam_SaveMotion(wd->Cam, wd->LevelName);
			CameraPath_Destroy(&wd->CameraPathEditor);

			if ((int)(lParam) == GE_FALSE)
				{
				App_Error("Camera Motion Editor error occured.");
				break;
				}
			break;
		}

		case WM_QUIT:
			break;

		case WM_MOVE:
		case WM_MOVING:
			if (!GlobApp || !GlobApp->Engine)
				break;
			geEngine_UpdateWindow(GlobApp->Engine);
			break;

		case WM_MOUSEMOVE:
		{
			// let the gui know about the new cursor position
			if ( ( GlobApp != NULL ) && ( GlobApp->AppGui != NULL ) )
			{
				if (GlobApp->GuiTime > 1.0f)
					GlobApp->GuiTimeDisable = GE_TRUE;

				Gui_SetCursorPos( GlobApp->AppGui, LOWORD(lParam), HIWORD(lParam) );
			}
			return FALSE;
		}

		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_LBUTTONDOWN:
		{
			switch ( iMessage )
			{
				case WM_LBUTTONDOWN:
				{
					if (GlobApp == NULL)
						break;

					if (GlobApp->wd->CameraPathEditor)
						{
						Mouse_SetCenter();
						}

					// check if a new level needs to be launched
					if ( ( GlobApp != NULL ) && ( GlobApp->AppGui != NULL ) && ( Gui_IsActive( GlobApp->AppGui ) == GE_TRUE ) )
					{

						// locals
						// get currently active bg
						GlobApp->GuiCurBg = Gui_GetActiveBg( GlobApp->AppGui );

						// exit game...
						if ( GlobApp->GuiCurBg == 6 )
						{
							GlobApp->Running = FALSE;
							break;
						}
						// ...or just start full sequence
						else if ( GlobApp->GuiCurBg == 1 )
						{

							// locals
							App_WorldData *nwd;

							App_DisplayHourGlass(GlobApp);
							// set new world as first world
							nwd = &GlobApp->WorldList[0];
							assert( nwd->World != NULL );
							GlobApp->ResetTime = GE_TRUE;
							if (App_Resource.SoundSystem)
								geSound_SetMasterVolume( App_Resource.SoundSystem, 0.0f);
							App_ChangeWorld(GlobApp->wd, nwd);
							GlobApp->wd = nwd;

							// deactivate gui
							Gui_SetActiveStatus( GlobApp->AppGui, GE_FALSE );
						}
						// ...or make other world adjustment
						else if ( ( GlobApp->GuiCurBg > 0 ) && ( App_LevelMapping[GlobApp->GuiCurBg] >= 0 ) )
						{

							// locals
							App_WorldData *nwd;
							
							App_DisplayHourGlass(GlobApp);
							// set new world
							nwd = &GlobApp->WorldList[App_LevelMapping[GlobApp->GuiCurBg]];
							assert( nwd->World != NULL );
							GlobApp->ResetTime = GE_TRUE;
							if (App_Resource.SoundSystem)
								geSound_SetMasterVolume( App_Resource.SoundSystem, 0.0f);
							App_ChangeWorld(GlobApp->wd, nwd);
							GlobApp->wd = nwd;

							// deactivate gui
							Gui_SetActiveStatus( GlobApp->AppGui, GE_FALSE );
						}
					}

					//
					GlobApp->MouseFlags |= 1;
					lParam = VK_LBUTTON;

					break;
				}
				case WM_RBUTTONDOWN:
					GlobApp->MouseFlags |= 2;
					lParam = VK_RBUTTON;
					break;
				case WM_LBUTTONUP:
					GlobApp->MouseFlags &= ~1;
					lParam = VK_LBUTTON;
					break;
				case WM_RBUTTONUP:
					GlobApp->MouseFlags &= ~2;
					lParam = VK_RBUTTON;
					break;
			}
		}

		case WM_KEYDOWN:
		{
			if (!GlobApp)
				break;

			switch (wParam)
			{
				case 'I':
					{
					// break out of camera movement
					if (GlobApp->CameraPathPlayback == GE_TRUE)	
						{
						App_WorldData *wd = GlobApp->wd;

						GlobApp->CameraPathPause = GE_TRUE;
						GlobApp->CameraPathPlayback = GE_FALSE;

						Fade_Reset();
						GUtil_OrientFromXForm(&wd->MyXForm, &wd->LocalPos, &wd->Rot);

						if (wd->CameraPathEditor)
							Mouse_SetForEdit();
						else
							Mouse_SetForControl();
						}

					break;
					}

				case 'P':
					{
					App_WorldData *wd = GlobApp->wd;

					if (!GlobApp->CameraAllowEdit)
						break;

					GlobApp->CameraPathPlayback = GE_TRUE;
					Cam_Reset(wd->Cam);
					}
					break;

				// play from cut/time
				case 'C':
					{
					App_WorldData *wd = GlobApp->wd;

					if (!GlobApp->CameraAllowEdit)
						break;

					GlobApp->CameraPathPlayback = GE_TRUE;
					}
					break;

				case 'S':
					{
					App_WorldData *wd = GlobApp->wd;

					if (!GlobApp->CameraAllowEdit)
						break;

					if (wd->CameraPathEditor)
						Mouse_SetForEdit();
					else
						Mouse_SetForControl();

					GlobApp->CameraPathPlayback = GE_FALSE;
					wd->FieldOfView = 2.0f;
					Cam_SetFrameTime(wd->Cam, 0.0f);
					}
					break;

				case 'Z':
					{
					App_WorldData *wd = GlobApp->wd;

					if (!GlobApp->CameraAllowEdit)
						break;

					// allow only one copy to be up at a time
					if (wd->CameraPathEditor != NULL)
						break;

					Mouse_SetForEdit();
					wd->CameraPathEditor = CameraPath_Create(App_CameraPathCallback, Cam_GetMotion(wd->Cam));

					if (wd->CameraPathEditor == NULL)
						{
						App_Error("Unable to create camera motion.");
						}

					if (CameraPathDlg_Show(GlobApp->hInstance, GlobApp->hWnd, wd->CameraPathEditor, "Testo") == GE_FALSE)
						{
						App_Error("Unable to open camera edit dialog.");
						}
					}
					break;

				case VK_SPACE :
					GlobApp->NextLevel = TRUE;
					break;
				case VK_ESCAPE :
				{
					if ( GlobApp != NULL )
					{

						if (GlobApp->CameraPathPause == GE_TRUE)
							{
							GlobApp->CameraPathPause = GE_FALSE;
							GlobApp->CameraPathPlayback = GE_TRUE;
							Mouse_SetForPlayback();
							break;
							}

						// process it this way if we have a gui...
						if ( GlobApp->AppGui != NULL )
						{

							// if the gui is active then just exit the app...
							if ( Gui_IsActive( GlobApp->AppGui ) == GE_TRUE )
							{
								GlobApp->Running = FALSE;
								GlobApp->BenchTimingOn = GE_FALSE;
								GlobApp->BenchTotalRunTime = 0.0f;
							}
							// ...otherwise return to gui
							else
							{
								Gui_SetActiveStatus( GlobApp->AppGui, GE_TRUE );
								if (App_Resource.SoundSystem)
									geSound_SetMasterVolume( App_Resource.SoundSystem, 0.0f);
								GlobApp->ResetTime = GE_TRUE;
								if ( App_Resource.EManager != NULL )
								{
									EffectM_DestroyManager( &App_Resource.EManager );
								}
							}
						}
						// ...ot this way if we don't havea gui
						else
						{
							GlobApp->Running = FALSE;
						}
					}
					break;
				}
				case VK_F12 :
					GUtil_ScreenShot(GlobApp->Engine);
					break;
				case 'F' :
					GlobApp->DoFrameRateCounter = !GlobApp->DoFrameRateCounter;
					geEngine_EnableFrameRateCounter(GlobApp->Engine, GlobApp->DoFrameRateCounter);
					break;
			}
			break;
		}

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc (hWnd, iMessage, wParam, lParam);
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_ShutdownAll()
//
//	Kill everything.
//
////////////////////////////////////////////////////////////////////////////////////////
static void App_ShutdownAll()
{
	App_WorldData *wd;

	Corona_Shutdown();
	ModelCtl_Shutdown();
	DynLight_Shutdown();

	// kill the gui
	if ( GlobApp->AppGui != NULL )
	{
		Gui_Destroy( &( GlobApp->AppGui ) );
	}

	if (App_Resource.EManager != NULL)
	{
		EffectM_DestroyManager( &App_Resource.EManager );
	}

	for (wd = GlobApp->WorldList; wd->World; wd++)
	{
		Cam_Destroy(&wd->Cam);

		if (wd->Attached)
		{
			geEngine_RemoveWorld(GlobApp->Engine, wd->World);
		}
	}

	for (wd = GlobApp->WorldList; wd->World; wd++)
	{
		if (wd->ObjectList)
		{
			ObjMgr_FreeWorldData( wd->World, wd->ObjectList );
			ObjMgr_Destroy(&wd->ObjectList);
		}

		if (wd->World)
		{
			geWorld_Free (wd->World);
		}
	}

	if (GlobApp->ActorPool)
	{
		ActorPool_Destroy(&GlobApp->ActorPool);
	}

	if (GlobApp->Camera)
	{
		geCamera_Destroy (&GlobApp->Camera);
	}

	if (GlobApp->Engine)
	{
		geEngine_Free (GlobApp->Engine);
	}

	if (App_Resource.SoundSystem)
	{
		geSound_DestroySoundSystem( App_Resource.SoundSystem );
	}

	if (GlobApp)
	{
		App_Destroy(&GlobApp);
	}

	App_SaveErrorLog();
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_Shutdown()
//
//	Exit clean and shutdown all.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean App_Shutdown(App *App)
{
	float WorldTime;

	assert(App);

	// display credits screen
	{
		// locals
		geBoolean		Result;
		geBitmap_Info	BmpInfo;
		int				x, y;

		// prepare loading bmp
		if ( App->CreditsBmp != NULL )
		{
			Result = geBitmap_GetInfo( App->CreditsBmp, &BmpInfo, NULL );
			if ( Result == GE_TRUE )
			{
				assert( App->CWidth > 0 );
				assert( App->CHeight > 0 );
				x = ( App->CWidth - BmpInfo.Width ) / 2;
				y = ( App->CHeight - BmpInfo.Height ) / 2;
				assert( App->Engine != NULL );
				assert( App->Camera != NULL );
				geEngine_BeginFrame( App->Engine, App->Camera, GE_TRUE );
				Result = geEngine_DrawBitmap( App->Engine, App->CreditsBmp, NULL, x, y );
				assert( Result == GE_TRUE );
				geEngine_EndFrame( App->Engine );
			}
			geBitmap_Destroy( &App->CreditsBmp );
		}
	}
	

	QueryPerformanceCounter(&App->OldTick);
	WorldTime = 0.0f;

	while (WorldTime < 12.0f)
		{
		float			ElapsedTime;

		// timing
		GUtil_CalcElapsedTime(&App->Freq, &App->OldTick, &App->CurTick, &ElapsedTime);
		WorldTime += ElapsedTime;

		if (WorldTime > 0.3f && GUtil_IsKeyDown(VK_ESCAPE, App->hWnd))
			break;
		}


	App_ShutdownAll();

	return GE_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_MovePos()
//
//	Processes movement input.
//
////////////////////////////////////////////////////////////////////////////////////////
static void App_MovePos(App_WorldData *wd, Object *Obj, geVec3d *Pos, geXForm3d *XForm, float Time)
{
	geVec3d	LVect, UVect, InVect;
	geWorld *World;
	geVec3d	Pos2 = *Pos;

	assert(wd != NULL);
	assert(Obj != NULL);
	assert(Pos != NULL);
	assert(XForm != NULL);
	World = wd->World;

	assert(World);
	assert(GlobApp);

	geXForm3d_GetLeft(XForm, &LVect);
	geXForm3d_GetUp(XForm, &UVect);
	geXForm3d_GetIn(XForm, &InVect);

	if (GUtil_UpIsDown(XForm) || wd->Rot.Z > (M_PI/2.0f) || wd->Rot.Z < (-M_PI/2.0f))
	{
		GlobApp->MouseSpeedX = -GlobApp->MouseSpeedX;
	}

	wd->Rot.Y += GlobApp->MouseSpeedX;
	wd->Rot.X += GlobApp->MouseSpeedY;

	GlobApp->MouseSpeedX = 0.0f;
	GlobApp->MouseSpeedY = 0.0f;

	if (GUtil_IsKeyDown(VK_UP, GlobApp->hWnd))	
		geVec3d_AddScaled(Pos, &InVect,  GlobApp->Velocity*Time*20, &Pos2);
	
	if (GUtil_IsKeyDown(VK_DOWN, GlobApp->hWnd))	
		geVec3d_AddScaled(Pos, &InVect, -GlobApp->Velocity*Time*20, &Pos2);

	// Right and left keys turn around
	// If shift key held, then they strafe
	// Note that because the Shift key is also used to turn off clipping,
	// no world clipping is performed when strafing.
	if (GUtil_IsKeyDown(VK_LEFT, GlobApp->hWnd))
	{
		if (GUtil_IsKeyDown(VK_SHIFT, GlobApp->hWnd))
			geVec3d_AddScaled(Pos, &LVect, GlobApp->Velocity, &Pos2);
		else 
			wd->Rot.Y += 0.1f * Time * 20;
	}

	if (GUtil_IsKeyDown(VK_RIGHT, GlobApp->hWnd))
	{
		if (GUtil_IsKeyDown(VK_SHIFT, GlobApp->hWnd))
			geVec3d_AddScaled(Pos, &LVect, -GlobApp->Velocity, &Pos2);
		else 
			wd->Rot.Y -= 0.1f * Time * 20;
	}

	// if the Shift key isn't down, then clip to the world
	// If holding the shift, then we can walk through walls.
	if (!GUtil_IsKeyDown(VK_SHIFT, GlobApp->hWnd))
	{
		GE_Collision Collide;
		if	(!geWorld_Collision (World, &GlobApp->Mins, &GlobApp->Maxs, Pos, &Pos2, GE_CONTENTS_SOLID_CLIP, GE_COLLIDE_ALL, USER_ALL, NULL, NULL, &Collide) == GE_FALSE)
		{
			if (Collide.Model)
			{
				Object		*Target;
				TriggerP	Trigger;
				geVec3d		Velocity;

				Target = (Object*)geWorld_ModelGetUserData(Collide.Model);
				if (Target && (ObjMgr_GetObjectFlags(Target) & TYPE_PHYSOB))
				{
					geVec3d_Scale(&InVect, 500.f, &Velocity);
					ObjMgr_SetObjectVelocity(Obj, &Velocity);

					Trigger = ObjMgr_GetObjectTriggerFunction(Target);
					if (Trigger)
						Trigger(World, Target, Obj, (void*)&Collide);
				}
			}
		}
		else
		{
			*Pos = Pos2;
		}
	}
	else
	{
		*Pos = Pos2;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_LoadLightStyles()
//
//	Load and set light style from file if it exists.
//
////////////////////////////////////////////////////////////////////////////////////////
static void App_LoadLightStyles(App_WorldData *wd, char *LevelName)
{
	char FileName[_MAX_FNAME];
	char Buffer[_MAX_FNAME];
	geVFile *LightFile;
	int i,ret;
	char *ptr;

	assert(wd != NULL);
	assert(LevelName != NULL);

	// Set some light type defaults
	geWorld_SetLTypeTable(wd->World, 0, "z");
	geWorld_SetLTypeTable(wd->World, 1, "mmnmmommommnonmmonqnmmo");
	geWorld_SetLTypeTable(wd->World, 2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
	geWorld_SetLTypeTable(wd->World, 3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");
	geWorld_SetLTypeTable(wd->World, 4, "mamamamamama");
	geWorld_SetLTypeTable(wd->World, 5, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	geWorld_SetLTypeTable(wd->World, 6, "nmonqnmomnmomomno");
	geWorld_SetLTypeTable(wd->World, 7, "mmmaaaabcdefgmmmmaaaammmaamm");
	geWorld_SetLTypeTable(wd->World, 8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
	geWorld_SetLTypeTable(wd->World, 9, "aaaaaaaazzzzzzzz");
	geWorld_SetLTypeTable(wd->World, 10,"mmamammmmammamamaaamammma");
	geWorld_SetLTypeTable(wd->World, 11,"abcdefghijklmnopqrrqponmlkjihgfedcba");

	strcpy(FileName, LevelName);
	strlwr(FileName);
	ptr = strstr(FileName, ".bsp");
	memcpy(ptr,".lmp",strlen(".lmp"));

	LightFile = geVFile_OpenNewSystem (NULL, GE_VFILE_TYPE_DOS, FileName, NULL, GE_VFILE_OPEN_READONLY);

	if (LightFile == NULL)
		return;

	for (i = 0; i < 10; i++)
		{
		ret = geVFile_GetS(LightFile, Buffer, _MAX_FNAME);
		if (ret == 0)
			break;

		ptr = strchr(Buffer,10);
		*ptr = '\0';
		Buffer[69] = '\0';

		geWorld_SetLTypeTable(wd->World, i, Buffer);
		}

	if (LightFile)
		geVFile_Close (LightFile);
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	App_SpawnFromEntity()
//
//	Converts entity data into objects.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean App_SpawnFromEntity( App_WorldData *wd, char *ClassType, SpawnFP SpawnFunc)
{
	geEntity_EntitySet	*ClassSet;
	geEntity			*Entity;
	void				*EntityData;
	Object				*Obj;

	assert(wd);
	assert(ClassType);
	assert(SpawnFunc);

	// Look for the class name in the world
	ClassSet = geWorld_GetEntitySet(wd->World, ClassType);
	if (!ClassSet)
		return GE_TRUE;

	Entity = NULL;

	while ((Entity = geEntity_EntitySetGetNextEntity(ClassSet, Entity)) != NULL)
	{
		EntityData = geEntity_GetUserData(Entity);

		Obj = ObjMgr_AddObject(wd->ObjectList, wd->World, ClassType, NULL, NULL, NULL, NULL);
		if (Obj == NULL)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE, "App_SpawnFromEntity: Unable to add an object.", ClassType);
			return GE_FALSE;
			}

		if (SpawnFunc(wd->World, Obj, EntityData) == GE_FALSE)
		{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE, "App_SpawnFromEntity: Failed to spawn.", ClassType);
			return GE_FALSE;
		}
	}

	return(GE_TRUE);
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	App_DisplayHourGlass()
//
//	Display small Loading... screen.
//
////////////////////////////////////////////////////////////////////////////////////////
void App_DisplayHourGlass(App *App)
{
	geRect Rect;
	geBitmap_Info	BmpInfo;

	if (geBitmap_GetInfo( App->TimerBmp, &BmpInfo, NULL ) == GE_FALSE)
		return;

	Rect.Left = 0;
	Rect.Top = 0;
	Rect.Right = BmpInfo.Width;
	Rect.Bottom = BmpInfo.Height;

	if (geEngine_BeginFrame(App->Engine, App->Camera, GE_TRUE) == GE_FALSE)
	{
		App_Error("App_DisplayHourGlass: geEngine_BeginFrame failed");
	}

	if (geEngine_DrawBitmap (App->Engine, App->TimerBmp, &Rect, 
			(App->CWidth - BmpInfo.Width)/2,  (App->CHeight - BmpInfo.Height)/2) == GE_FALSE)
	{
		App_Error("App_DisplayHourGlass: geEngine_DrawBitmap failed");
	}

	if (geEngine_EndFrame(App->Engine) == GE_FALSE)
	{
		App_Error("App_DisplayHourGlass: geEngine_EndFrame failed");
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_DisplayLoadingScreen()
//
//	Display large Loading... screen.
//
////////////////////////////////////////////////////////////////////////////////////////
void App_DisplayLoadingScreen(App *App)
{
	// locals
	geBitmap		*LoadingBmp;
	geBoolean		Result;
	geBitmap_Info	BmpInfo;
	int				x, y;

	assert(App);

	// prepare loading bmp
	LoadingBmp = geBitmap_CreateFromFileName( NULL, "bmp\\loading.bmp" );
	if ( LoadingBmp != NULL )
	{
		Result = geBitmap_GetInfo( LoadingBmp, &BmpInfo, NULL );
		if ( Result == GE_TRUE )
		{
			assert( App->CWidth > 0 );
			assert( App->CHeight > 0 );
			x = ( App->CWidth - BmpInfo.Width ) / 2;
			y = ( App->CHeight - BmpInfo.Height ) / 2;
			assert( App->Engine != NULL );
			assert( App->Camera != NULL );

			if (geEngine_AddBitmap( App->Engine, LoadingBmp ) == GE_FALSE)
			{
				App_Error("App_DisplayLoadingScreen: geEngine_AddBitmap failed");
			}

			if (geEngine_BeginFrame( App->Engine, App->Camera, GE_TRUE ) == GE_FALSE)
			{
				App_Error("App_DisplayLoadingScreen: geEngine_BeginFrame failed");
			}

			Result = geEngine_DrawBitmap( App->Engine, LoadingBmp, NULL, x, y );
			if (Result == GE_FALSE)
			{
				App_Error("App_DisplayLoadingScreen: geEngine_DrawBitmap failed");
			}

			if (geEngine_EndFrame( App->Engine ) == GE_FALSE)
			{
				App_Error("App_DisplayLoadingScreen: geEngine_EndFrame failed");
			}
		}
		geBitmap_Destroy( &LoadingBmp );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_ParseCommandLine()
//
//	Process command line string.
//
////////////////////////////////////////////////////////////////////////////////////////
static void App_ParseCommandLine(App *App, char *CmdLine, char *Data)
{
	int i;

	// get command line data
	for	(i=0;;)
	{	
		if ((CmdLine = GUtil_GetCmdLine (CmdLine, Data, FALSE)) == NULL)
		{
			break;
		}
		
		if (!stricmp(Data, "Map"))
		{
			// Get the demo name
			if ((CmdLine = GUtil_GetCmdLine (CmdLine, Data, TRUE)) == NULL)
			{
				App_Error("No map name specified on command line.");
			}
			
			if (i == 0)
				memset(App_LevelList, 0, sizeof(App_LevelList));
			
			App->MapOverride = GE_TRUE;
			strcpy(App_LevelList[i++], Data);
		}
		else if (!stricmp(Data, "Gamma"))
		{
			CmdLine = GUtil_GetCmdLine (CmdLine, Data, TRUE);
			App->UserGamma = (float)atof(Data);
		}
		else if (!stricmp(Data, "Cam"))
		{
			App->CameraPathPlayback = !App->CameraPathPlayback;
		}
		else if (!stricmp(Data, "CamEdit"))
		{
			GlobApp->CameraAllowEdit = GE_TRUE;
		}
		else if (!stricmp(Data, "Stats"))
		{
			App->DoFrameRateCounter = GE_TRUE;
		}
		else if (!stricmp(Data,"PickMode"))
		{
			App->ManualPick=GE_TRUE;
		}
		else if (!stricmp(Data,"timedemo"))
		{
			App->BenchTimingOn = GE_TRUE;
		}
		else
		{
			App_Error("Unknown Option: %s.", Data);
		}
	}
}	



////////////////////////////////////////////////////////////////////////////////////////
//
//	App_LoadWorlds()
//
//	Load all levels into memory at once.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean App_LoadWorlds(App *App)
{
	int i;

	assert(App);

	//
	//	Load Level info - load all data that you can into memory
	//
	for (i = 0; App_LevelList[i][0]; i++)
	{
		App_WorldData *wd = &App->WorldList[i];
		strcpy(wd->LevelName, App_LevelList[i]);
		
		//
		//	Create a World
		//
		{
		geVFile *WorldFile;
		
		WorldFile = geVFile_OpenNewSystem (NULL, GE_VFILE_TYPE_DOS, wd->LevelName, NULL, GE_VFILE_OPEN_READONLY);
		if (WorldFile != NULL)
		{
			wd->World = geWorld_Create (WorldFile);
			geVFile_Close (WorldFile);
		}
		}
		
		if (wd->World == NULL)
		{
			App_Error("Could not create the world %s.\n", wd->LevelName);
		}
		
		App_LoadLightStyles(wd, wd->LevelName);
		
		wd->ObjectList = ObjMgr_Create();
		
		if (wd->ObjectList == NULL)
		{
			App_Error("Could not create object manager.");
		}
		
		wd->CameraObject = App_EntityGetCameraObject(wd, App->ActorPool, &wd->MyXForm);
		
		if (wd->CameraObject == NULL)
		{
			App_Error("Could not add camera object.");
		}

		wd->Cam = Cam_Create(wd->LevelName, wd->CameraObject);

		if (wd->Cam == NULL)
		{
			App_Error("Could not get camera motion.");
		}
		
		// these entity loaders could be combined similar to GTEST
		if (App_EntitySetupObjects(wd, App->ActorPool) == GE_FALSE)
		{
			App_Error("Could not set up objects.");
		}

		if (App_SpawnFromEntity(wd, "PhysicsObject", PhysicsObject_Spawn) == GE_FALSE)
		{
			App_Error("Could not set up class PhysicsObjects.");
		}
		if (App_SpawnFromEntity(wd, "PhysicsJoint", PhysicsJoint_Spawn) == GE_FALSE)
		{
			App_Error("Could not set up class PhysicsJoints.");
		}
		if (App_SpawnFromEntity(wd, "PhysicalSystem", PhysicalSystem_Spawn) == GE_FALSE)
		{
			App_Error("Could not set up class PhysicalSystem.");
		}
		if (App_SpawnFromEntity(wd, "ModelTrigger", ModelTrigger_Spawn) == GE_FALSE)
		{
			App_Error("Could not set up class ModelTrigger.");
		}
		if (App_SpawnFromEntity(wd, "Door", Door_Spawn) == GE_FALSE)
		{
			App_Error("Could not set up class Door.");
		}
		
		ModelCtl_Init();
		DynLight_Init(App->Engine, wd->World, NULL);
		
		wd->LocalPos = wd->MyXForm.Translation;
	}

	return GE_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_Create()
//
//	All setup necessary to run start using the App structure.
//
////////////////////////////////////////////////////////////////////////////////////////
#pragma warning (disable:4100)
App *App_Create(HINSTANCE hInstance, HWND hWnd, char *CmdLine)
{
	App* App;
	char			Data[_MAX_PATH];
	int Ret;

	if ((App = App_Allocate()) == NULL)
	{
		App_Error("Unable to create the application.");
	}

	GlobApp = App;
	App->hInstance = hInstance;
	App->hWnd = hWnd;

	App->wd = &App->WorldList[0];
	
	GetSystemTime(&App->SystemTime);
	
	*App->wd->LevelName = 0;
	
	App->hInstance = hInstance;
	
	Ret = GetCurrentDirectory(sizeof(GlobApp->Dir), GlobApp->Dir);
	if (Ret == 0)
	{
		App_Error("App_Create: GetCurrentDirectory failed.");
	}

	
	App->MainFS = geVFile_OpenNewSystem(NULL,
		GE_VFILE_TYPE_DOS,
		GlobApp->Dir,
		NULL,
		GE_VFILE_OPEN_READONLY | GE_VFILE_OPEN_DIRECTORY);
	
	if (App->MainFS == NULL)
	{
		App_Error("App_Create: geVFile_OpenNewSystem failed.");
	}

	App_ParseCommandLine(App, CmdLine, Data);

	if (App_LevelList[0][0] == 0)
	{
		App_Error("No map name specified on command line.\n");
	}

	// get window size
	{
	RECT Rect;   
	Ret = GetClientRect(App->hWnd, &Rect);      // handle to window
	if (Ret == 0)
	{
		App_Error("App_Create: GetClientRect failed.");
	}
	App->CWidth = Rect.right;
	App->CHeight = Rect.bottom; 
	}
	
	geXForm3d_SetIdentity (&App->wd->MyXForm);
	
	//
	//	Create a engine
	//
	App->Engine = geEngine_Create(App->hWnd, App->Name, ".");
	
	if (!App->Engine)
	{
		App_Error("Could not initialize the Genesis engine.\n");
	}
	
	// 'cause we don't know the initial state...
	geEngine_EnableFrameRateCounter(App->Engine, App->DoFrameRateCounter);
	
	//
	//	Mode Selection
	//
	App->DriverModeList = ModeList_Create(App->Engine, &App->DriverModeListLength);
	if (App->DriverModeList == NULL)
	{
		App_Error("Failed to create a list of available drivers - Make sure the driver dll's are in the right directory.");
	}
	
	AutoSelect_SortDriverList(App->DriverModeList, App->DriverModeListLength);
	
	GameMgr_PickMode(App->hWnd, hInstance, App->Engine, App->ChangingDisplayMode, App->ManualPick,
		App->DriverModeList, App->DriverModeListLength, &App->ChangeDisplaySelection, &App->InAWindow);

	if (GameMgr_GetModeData(&App->VidMode, &App->Camera, &App->CWidth, &App->CHeight) == GE_FALSE)	
	{
		App_Error("Failed to get mode data.");
	}

	geEngine_SetGamma(App->Engine, App->UserGamma);
	
	// set software mode flag if required
	if ( App->DriverModeList[App->ChangeDisplaySelection].DriverType == MODELIST_TYPE_SOFTWARE )
	{
		App->SoftwareMode = GE_TRUE;
	}

	Mouse_Set(App->hWnd, App->CWidth, App->CHeight, App->InAWindow);
	Fade_SetRect(App->CWidth, App->CHeight);

	if (App->CameraPathPlayback == GE_TRUE)
		Mouse_SetForPlayback();
	else
		Mouse_SetForControl();

	App_DisplayLoadingScreen(App);

	// prepare loading bmp
	App->TimerBmp = geBitmap_CreateFromFileName( NULL, "bmp\\timer.bmp" );
	if ( App->TimerBmp != NULL )
	{
		geEngine_AddBitmap( App->Engine, App->TimerBmp );
	}

	// prepare loading bmp
	App->CreditsBmp = geBitmap_CreateFromFileName( NULL, "bmp\\credits.bmp" );
	if ( App->CreditsBmp != NULL )
	{
		geEngine_AddBitmap( App->Engine, App->CreditsBmp );
	}

	//
	//	Sound Pool Init
	//
	App_Resource.SPool = NULL;
	App_Resource.SoundSystem = geSound_CreateSoundSystem( App->hWnd );
	
	if ( App_Resource.SoundSystem != NULL )
	{
		App_Resource.SPool = SPool_Create( App_Resource.SoundSystem );
		if ( App_Resource.SPool == NULL )
		{
			App_Error( "Could not create sound pool.\n" );
		}

		geSound_SetMasterVolume( App_Resource.SoundSystem, 0.0f);
		Cache_Wav(GlobApp->Dir, App_Resource.SPool);
	}

	//
	//	Actor Def Pool Init
	//
	App->ActorPool = ActorPool_Create();
	
	if ( App->ActorPool == NULL )
	{
		App_Error( "Could not create actor pool.\n" );
	}
	
	if (App_LoadWorlds(App) == GE_FALSE)
	{
		App_Error( "App_LoadWorlds failed.\n" );
	}
	
	Corona_Init(App->Engine, App->wd->World, App->MainFS);
	
	if (geEngine_AddWorld (App->Engine, App->wd->World) == GE_FALSE)
	{
		App_Error ("Could not add world to engine.\n");
	}
	App->wd->Attached = TRUE;
	
	ObjMgr_AddObjectsToWorld(App->wd->World, App->wd->ObjectList);
	
	// create texture pool
	App_Resource.TPool = TPool_Create( App->wd->World );
	if ( App_Resource.TPool == NULL )
	{
		App_Error( "Could not create texture pool.\n" );
	}
	
	// create effect system
	App_Resource.ESystem = Effect_SystemCreate( App->Engine, App->wd->World, App_Resource.SoundSystem, App->Camera );
	if ( App_Resource.ESystem == NULL )
	{
		App_Error( "Could not create effect system.\n" );
	}
	
	// create effect manager
	App_Resource.EManager = EffectM_CreateManager( App_Resource.ESystem, App_Resource.TPool, App_Resource.SPool, NULL, App->SoftwareMode );
	if ( App_Resource.EManager == NULL )
	{
		App_Error( "Could not create effect manager.\n" );
	}
	
	// load up all world effects into the effect manager
	EffectM_LoadWorldEffects( App_Resource.EManager, App->wd->World );

	// Load in all textures that will be used
	Cache_Bmp(App_Resource.TPool);

	Trigger_Set(App->wd->ObjectList, App_Resource.SPool, App_Resource.TPool, App_Resource.ESystem, App_Resource.EManager);

	App->ResetTime = GE_TRUE;
	QueryPerformanceFrequency(&App->Freq);
	QueryPerformanceCounter(&App->OldTick);
	App->WorldTime = 0;
	
	if (App->CameraPathPlayback == GE_TRUE)
		Mouse_SetForPlayback();
	else
		Mouse_SetForControl();

	// create gui
	if ( App->MapOverride == GE_FALSE )
	{
		App->AppGui = Gui_Create( App->Engine, App->CWidth, App->CHeight );
		Gui_PrepareFrame(App->AppGui, App->Camera);
		Mouse_ShowWinCursor();
	}

	if (App->InAWindow == GE_TRUE && App->AppGui != NULL)
	{
		Gui_ShowCursor(App->AppGui, GE_FALSE);
	}

	App->ScreenClearFlag = GE_TRUE;
	if (App->SoftwareMode)
		App->ScreenClearFlag = GE_FALSE;

	return App;
}
#pragma warning (default:4100)


////////////////////////////////////////////////////////////////////////////////////////
//
//	App_Error()
//
//	Report fatal errors.
//
////////////////////////////////////////////////////////////////////////////////////////
static void App_Error(const char *Msg, ...)
{
	va_list		ArgPtr;
    char		TempStr[500];
    char		TempStr2[500];
	FILE		*f;

	assert(Msg);

	va_start (ArgPtr, Msg);
    vsprintf (TempStr, Msg, ArgPtr);
	va_end (ArgPtr);

	App_ShutdownAll();

	f = fopen("GDemo.Log", "wt");

	if (f)
	{
		int32		i, NumErrors;

		NumErrors = geErrorLog_Count();

		fprintf(f, "Error#:%3i, Code#:%3i, Info: %s\n", NumErrors, 0, TempStr);

		for (i=0; i<NumErrors; i++)
		{
			geErrorLog_ErrorClassType	Error;
			char						*String;

			if (geErrorLog_Report(NumErrors-i-1, &Error, &String))
			{
				fprintf(f, "Error#:%3i, Code#:%3i, Info:%s\n", NumErrors-i-1, Error, String);
			}
		}

		fclose(f);
		
		sprintf(TempStr2, "%s\nPlease refer to GDemo.Log for more info.", TempStr);

		MessageBox(0, TempStr2, "** GDemo System Error **", MB_OK);
		WinExec( "Notepad GDemo.Log", SW_SHOW );
	}

	_exit(1);

}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_SaveErrorLog()
//
//	Report any non fatal errors.
//
////////////////////////////////////////////////////////////////////////////////////////
static void App_SaveErrorLog(void)
{
	FILE		*f;

	f = fopen("GDemo2.Log", "wt");

	if (f)
	{
		int32		i, NumErrors;

		NumErrors = geErrorLog_Count();

		for (i=0; i<NumErrors; i++)
		{
			geErrorLog_ErrorClassType	Error;
			char						*String;

			if (geErrorLog_Report(NumErrors-i-1, &Error, &String))
			{
				fprintf(f, "Error#:%3i, Code#:%3i, Info:%s\n", NumErrors-i-1, Error, String);
			}
		}

		fclose(f);
	}

}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_EntityGetCameraObject()
//
//	Create a camera object.
//
////////////////////////////////////////////////////////////////////////////////////////
static Object *App_EntityGetCameraObject( App_WorldData *wd, ActorPool *ActorPool, geXForm3d *XForm)
{
	geEntity_EntitySet	*ClassSet;
	geEntity			*Entity;
	CameraStart			*StartData;
	geActor_Def			*ActorDef;
	geActor				*Actor;
	geMotion			*Motion;
	Object				*Obj;
	geVec3d				Angles;
	
	assert(wd != NULL);
	assert(ActorPool != NULL);
	assert(wd->World != NULL);
	
	// Currently only set up to use one camera start per world
	
	// Look for the class name in the world
	ClassSet = geWorld_GetEntitySet(wd->World, "CameraStart");
    if (!ClassSet)
	{
		// camera not found - set to 0,0,0
		geXForm3d_SetIdentity(XForm);
		Obj = ObjMgr_AddObject(wd->ObjectList, wd->World, "cam", NULL, NULL, "", XForm);
		if (Obj == NULL)
		{
			App_Error("App_EntityGetCameraObject: Unable to add an object");
		}
		wd->FieldOfView = 2.0;
		return(Obj);
	}
	
	Entity = NULL;
	
	while ((Entity = geEntity_EntitySetGetNextEntity(ClassSet, Entity)) != NULL)
	{
		StartData = (CameraStart*)geEntity_GetUserData(Entity);
		
		Actor = NULL;
		Motion = NULL;
		
		geXForm3d_SetIdentity(XForm);

		Angles = StartData->angles;
		Angles.X *= (M_PI/180.0f);
		Angles.Y -= 90.0f;
		Angles.Y *= (M_PI/180.0f);
		Angles.Z *= (M_PI/180.0f);
		geXForm3d_SetEulerAngles(XForm, &wd->Rot);
		
		XForm->Translation = StartData->origin;
		if (StartData->FieldOfView <= 1.0f || StartData->FieldOfView >= 20.0f)
			StartData->FieldOfView = 2.0;
		wd->FieldOfView = StartData->FieldOfView;
		
		// is the camera on an actor?
		if (StartData->Name[0])
		{
			ActorDef = ActorPool_Get(GlobApp->ActorPool, StartData->Name);
			
			if (ActorDef == NULL)
			{
				App_Error("Unable to load actor %s.",StartData->Name);
			}
			
			Actor = geActor_Create(ActorDef);
			
			if (Actor == NULL)
			{
				App_Error("Unable to create actor %s.",StartData->Name);
			}
			
			if (StartData->Motion[0] && StartData->MotionBone[0])
			{
				Motion = geActor_GetMotionByName(ActorDef, StartData->Motion);
				
				if (Motion == NULL)
				{
					App_Error("Unable to get motion %s, %s.",StartData->Name, StartData->Motion);
				}
				
				strupr(StartData->MotionBone); // bones are all upper case
				geActor_GetBoneTransform( Actor, StartData->MotionBone, XForm );
			}
		}
		
		Obj = ObjMgr_AddObject(wd->ObjectList, wd->World, StartData->Name, Actor, Motion, StartData->MotionBone, XForm);
		if (Obj == NULL)
		{
			App_Error("App_EntityGetCameraObject: Unable to add an object");
		}
		ObjMgr_SetMotionScale(Obj, StartData->MotionScale);
		ObjMgr_SetObjectClassData(Obj, StartData);
		
		return(Obj);
	}
	
	
	return(NULL);
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_EntitySetupObjects()
//
//	Create actor objects.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean App_EntitySetupObjects(App_WorldData *wd, ActorPool *ActorPool)
{
	geEntity_EntitySet	*ClassSet;
	geEntity			*Entity;
	ActorStart			*ActorStartData;
	geActor_Def			*ActorDef;
	Object				*Obj;
	geMotion			*Motion;
	geXForm3d	XForm;
	geActor *Actor;
	geVec3d	Angles;
	geBoolean DefExists;
	
	assert(wd != NULL);
	assert(ActorPool != NULL);
	assert(wd->World != NULL);
	
	// Look for the class name in the world
	ClassSet = geWorld_GetEntitySet(wd->World, "ActorStart");
	if (!ClassSet)
		return GE_TRUE;
	
	Entity = NULL;
	
	while ((Entity = geEntity_EntitySetGetNextEntity(ClassSet, Entity)) != NULL)
	{
		ActorStartData = (ActorStart*)geEntity_GetUserData(Entity);
		
		if (!ActorStartData->Name[0])
			continue;
		
		DefExists = ActorPool_Exists(ActorPool, ActorStartData->Name);

		// will create a new ActorDef or fetch the current one if it exists
		ActorDef = ActorPool_Get(ActorPool, ActorStartData->Name);

		if (ActorDef == NULL)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE, "App_EntitySetupObjects: Unable to create actor def.", ActorStartData->Name);
			goto ExitErr;
			}

		if (stricmp(ActorStartData->Name, "squid"))
		// make a copy of all materials 
		// this is done so any textures that are changed (morphed) can be reset later
		// Need to add ability to do this only for certain actors from the entity - SQUID for instance
		if (!DefExists)
		{
			geBitmap *Bitmap;
			geFloat Red,Green,Blue;
			geBody *Body;
			char *MaterialName;
			int Count,i,index;
			geBoolean ret;

	 		Body = geActor_GetBody(ActorDef);


			if (Body)
			{
				char NewName[256];
				geBitmap_Info Info, Info2;
				geBitmap *NewBmp;

				Count = geBody_GetMaterialCount(Body);

				for (i = 0; i < Count; i++)
				{
					ret = geBody_GetMaterial(Body, i, &MaterialName,	&Bitmap, &Red, &Green, &Blue);
					assert(ret);

					// create the new bitmap
					NewBmp = Bitmap;
					if (Bitmap != NULL)
					{
						if (geBitmap_GetInfo(Bitmap, &Info, &Info2) == GE_FALSE)
						{
							geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE, "App_EntitySetupObjects: Can't get bitmap info.", ActorStartData->Name);
							goto ExitErr;
						}

						//NewBmp = geBitmap_Create( Info.Width, Info.Height, 1, Info.Format );
						NewBmp = geBitmap_CreateFromInfo(&Info);
						if (NewBmp == NULL)
						{
							geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE, "App_EntitySetupObjects: Can't create new bitmap.", ActorStartData->Name);
							goto ExitErr;
						}

						if (geBitmap_BlitBitmap(Bitmap, NewBmp) == GE_FALSE)
						{
							geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE, "App_EntitySetupObjects: Can't blit bitmap.", ActorStartData->Name);
							goto ExitErr;
						}
					}

					assert(ret);
					strcpy(NewName,"copy_");
					strcat(NewName,MaterialName);

					// associate the new bitmap with the body - bumps the ref up on the bitmap also
					if (geBody_AddMaterial(Body, NewName, NewBmp,  Red,  Green,  Blue, &index) == GE_FALSE)
					{
						geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE, "App_EntitySetupObjects: Can't add material to body.", ActorStartData->Name);
						goto ExitErr;
					}
					assert(index == i+Count);

					// just bumps the ref down one
					geBitmap_Destroy(&NewBmp);
				}
			}
		}
		
		
		assert(ActorDef);
		Actor = geActor_Create(ActorDef);
		
		if (Actor == NULL)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE, "App_EntitySetupObjects: Unable to create actor.", ActorStartData->Name);
			goto ExitErr;
			}

		
		ActorStartData->Actor = Actor;
		geActor_SetScale(Actor, ActorStartData->Scale.X, ActorStartData->Scale.Y, ActorStartData->Scale.Z);
		
		Motion = NULL;
		if (ActorStartData->Motion[0])
		{
			Motion = geActor_GetMotionByName(ActorDef, ActorStartData->Motion);
			if (Motion == NULL)
				{
				geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE, "App_EntitySetupObjects: Unable to find actor motion.", ActorStartData->Motion);
				goto ExitErr;
				}
		}
		
		geXForm3d_SetIdentity(&XForm);
		Angles = ActorStartData->angles;
		Angles.X *= (M_PI/180.0f);
		Angles.Y *= (M_PI/180.0f);
		Angles.Z *= (M_PI/180.0f);
		geXForm3d_SetEulerAngles(&XForm, &Angles);
		XForm.Translation = ActorStartData->origin;
		
		Obj = ObjMgr_AddObject(wd->ObjectList, wd->World, ActorStartData->Name, Actor, Motion, NULL, &XForm);
		if (Obj == NULL)
			{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE, "App_EntitySetupObjects: Unable to add an object.", ActorStartData->Name);
			goto ExitErr;
			}
		ObjMgr_SetMotionScale(Obj, ActorStartData->MotionScale);
		ObjMgr_SetObjectClassData(Obj, ActorStartData);
		ObjMgr_SetObjectMotionLoop(Obj, ActorStartData->MotionLooping);
		ObjMgr_SetObjectActive(Obj, !ActorStartData->InActive);

		if (ActorStartData->RenderBoxOn)
		{
			geXForm3d RXForm;
			geExtBox ExtBox;
			geVec3d vec = {0,0,0};

			// rotate actor like its supposed to be
			geXForm3d_SetXRotation(&RXForm, -(M_PI/2.0f));
			// set the pose based on this xform
			geActor_ClearPose(Actor, &RXForm);

			// set render hint box
			geActor_GetDynamicExtBox( Actor, &ExtBox);
			geExtBox_Scale(&ExtBox, 2.4f, 2.4f, 2.4f);
			geActor_SetRenderHintExtBox(Actor, &ExtBox, NULL);
			// set the pose back to the original xform
			geActor_ClearPose(Actor, &XForm);
		}
		
		if (ActorStartData->Path)
		{
			geBoolean Loop;
			geMotion *Motion = PathPoint_GetMotion(wd->World, ActorStartData->Path, &Loop);
			if (Motion == NULL)
			{
				geErrorLog_AddString(-1, "App_EntitySetupObjects: Unable to create path motion", NULL);
				goto ExitErr;
			}
				
			ObjMgr_SetObjectPathMotion(Obj, Motion, ActorStartData->PathLoop);
			ObjMgr_SetObjectPathState(Obj, ActorStartData->PathActive);
		}
		
	}
	
	return(GE_TRUE);

ExitErr:

	return GE_FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_EntityResetObjects()
//
//	Reset actor objects.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean App_EntityResetObjects(App_WorldData *wd)
{
	geEntity_EntitySet	*ClassSet;
	geEntity			*Entity;
	ActorStart			*ActorStartData;
	geActor_Def			*ActorDef;
	Object				*Obj;
	geMotion			*Motion;
	geXForm3d	XForm;
	geActor *Actor;
	geVec3d	Angles;
	
	assert(wd != NULL);
	assert(wd->World != NULL);
	
	// Look for the class name in the world
	ClassSet = geWorld_GetEntitySet(wd->World, "ActorStart");
	if (!ClassSet)
		return GE_TRUE;
	
	Entity = NULL;
	
	while ((Entity = geEntity_EntitySetGetNextEntity(ClassSet, Entity)) != NULL)
	{
		ActorStartData = (ActorStart*)geEntity_GetUserData(Entity);
		
		if (!ActorStartData->Name[0])
			continue;

		Actor = ActorStartData->Actor;
		assert(Actor);
		geActor_SetScale(Actor, ActorStartData->Scale.X, ActorStartData->Scale.Y, ActorStartData->Scale.Z);

		ActorDef = geActor_GetActorDef(Actor);
		assert(ActorDef);
		
		Motion = NULL;
		if (ActorStartData->Motion[0])
		{
			Motion = geActor_GetMotionByName(ActorDef, ActorStartData->Motion);
			assert(Motion);
		}
		
		geXForm3d_SetIdentity(&XForm);
		Angles = ActorStartData->angles;
		Angles.X *= (M_PI/180.0f);
		Angles.Y *= (M_PI/180.0f);
		Angles.Z *= (M_PI/180.0f);
		geXForm3d_SetEulerAngles(&XForm, &Angles);
		XForm.Translation = ActorStartData->origin;

		// Need to add ability to do this only for certain actors from the entity - SQUID for instance
		// reset all materials to what the def has
		if (stricmp(ActorStartData->Name, "squid"))
		{
			geBitmap *SrcBitmap,*DestBitmap;
			geFloat Red,Green,Blue;
			geBody *Body;
			char *MaterialName;
			int Count,i,j;
			geBoolean ret;

	 		Body = geActor_GetBody(ActorDef);

			if (Body)
			{
				Count = geBody_GetMaterialCount(Body);

				for (i = Count/2, j = 0; i < Count; i++, j++)
				{
					ret = geBody_GetMaterial(Body, i, &MaterialName, &SrcBitmap, &Red, &Green, &Blue);
					assert(ret);
					ret = geBody_GetMaterial(Body, j, &MaterialName, &DestBitmap,  &Red,  &Green,  &Blue);
					assert(ret);

					if (SrcBitmap && DestBitmap)
						{
						if (geBitmap_BlitBitmap(SrcBitmap, DestBitmap) == GE_FALSE)
							{
							assert(0);
							geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE, "App_EntityResetObjects: Unable to blit bitmap.", ActorStartData->Motion);
							goto ExitErr;
							}
						}
				}
			}
		}
		
		Obj = ObjMgr_GetObjectByClassData(wd->ObjectList, ActorStartData);
		ObjMgr_ResetObject(Obj);
		ObjMgr_SetObjectMotion(Obj, Motion);
		ObjMgr_SetObjectXForm(Obj, &XForm);
		ObjMgr_SetMotionScale(Obj, ActorStartData->MotionScale);
		ObjMgr_SetObjectClassData(Obj, ActorStartData);
		ObjMgr_SetObjectMotionLoop(Obj, ActorStartData->MotionLooping);
		ObjMgr_SetObjectActive(Obj, !ActorStartData->InActive);

		if (ActorStartData->Path)
		{
			geBoolean Loop;
			geMotion *Motion = PathPoint_GetMotion(wd->World, ActorStartData->Path, &Loop);
			if (Motion == NULL)
			{
				geErrorLog_AddString(-1, "App_EntityResetObjects: Unable to create path motion", ActorStartData->Motion);
				return GE_FALSE;
			}
				
			//ObjMgr_SetObjectPathMotion(Obj, Motion, GE_TRUE);
			ObjMgr_SetObjectPathMotion(Obj, Motion, ActorStartData->PathLoop);
			ObjMgr_SetObjectPathState(Obj, ActorStartData->PathActive);
		}
		
	}
	
	return GE_TRUE;
ExitErr:

	return GE_FALSE;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	App_EntityResetPhysics()
//
//	Reset physics objects.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean App_EntityResetPhysics(App_WorldData *wd, char *ClassType, SpawnFP SpawnFunc)
{
	geEntity_EntitySet	*ClassSet;
	geEntity			*Entity;
	Object				*Obj;
	
	assert(wd != NULL);
	assert(wd->World != NULL);
	
	// Look for the class name in the world
	ClassSet = geWorld_GetEntitySet(wd->World, ClassType);
	if (!ClassSet)
		return GE_TRUE;
	
	Entity = NULL;
	
	while ((Entity = geEntity_EntitySetGetNextEntity(ClassSet, Entity)) != NULL)
	{
		DestroyP Destroy;
		
		Obj = ObjMgr_GetObjectByClassData(wd->ObjectList, geEntity_GetUserData(Entity));

		if (Obj == NULL)
			continue;

		// destroy physics part
		Destroy = ObjMgr_GetObjectDestroyFunction(Obj);
		if (Destroy == NULL)
			continue;

		Destroy(wd->World, Obj, geEntity_GetUserData(Entity));

		// reset physics part
		ObjMgr_ResetObject(Obj);
		if (SpawnFunc(wd->World, Obj, geEntity_GetUserData(Entity)) == GE_FALSE)
		{
			geErrorLog_AddString(GE_ERR_INTERNAL_RESOURCE, "App_SpawnFromEntity: Failed to spawn.", ClassType);
			return GE_FALSE;
		}

	}
	
	return GE_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	App_Gui()
//
//	Process Gui code.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean App_Gui(App *App)
{
	geBoolean		GuiIsActive;
	float			ElapsedTime;

	assert(App);

	// do gui processing asrequired
	if ( App->AppGui == NULL )
	{
		GuiIsActive = GE_FALSE;
	}
	else
	{
		GuiIsActive = Gui_Frame( App->AppGui, App->Camera );

		// timing stuff
		if ( ( App->BenchTimingOn == GE_TRUE ) && ( App->BenchTotalRunTime > 0.0f ) )
		{
			geEngine_Printf( App->Engine, 0, 0, "Time: %.1f", App->BenchTotalRunTime );
			geEngine_Printf( App->Engine, 0, 14, "Frames: %d", App->BenchTotalFrames );
			geEngine_Printf( App->Engine, 0, 28, "AVG FPS: %.1f", (float)App->BenchTotalFrames / App->BenchTotalRunTime );
		}

		if (GuiIsActive)
		{
			if (!App->GuiTimeDisable)
			{
				if (App->ResetTime == GE_TRUE)
				{
					App->ResetTime = GE_FALSE;
					QueryPerformanceCounter(&App->OldTick);

					// needed to do this for full screen sofware mode
					if (App->CameraPathPlayback == GE_TRUE)
						Mouse_SetForPlayback();
					else
						Mouse_SetForControl();
				}

				GUtil_CalcElapsedTime(&App->Freq, &App->OldTick, &App->CurTick, &ElapsedTime);
				App->GuiTime += ElapsedTime;

				if (App->GuiTime > GUI_TIMEOUT)
				{
					App_WorldData *nwd;

					App_DisplayHourGlass(App);
					// set new world as first world
					nwd = &App->WorldList[0];
					assert( nwd->World != NULL );
					App->GuiTime = 0;
					App->GuiAutoPlay = GE_TRUE;
					App->ResetTime = GE_TRUE;
					if (App_Resource.SoundSystem)
						geSound_SetMasterVolume( App_Resource.SoundSystem, 0.0f);
					App_ChangeWorld(App->wd, nwd);
					App->wd = nwd;

					// deactivate gui
					Gui_SetActiveStatus( App->AppGui, GE_FALSE );
				}
			}
		}
	}

	return GuiIsActive;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_HandleLevelTransition()
//
//	Calls necessary code to change levels.
//
////////////////////////////////////////////////////////////////////////////////////////
static void App_HandleLevelTransition(App *App)
{
	assert(App);

	// handle flag to transition to the next level
	if (App->NextLevel)
	{
		App_WorldData *nwd = App->wd;
		
		App->NextLevel = FALSE;
		
		// increment world
		nwd++;
		if (!nwd->World)
		{
			if (App->GuiAutoPlay)
			{
				// press esc to bring up gui
				PostMessage(App->hWnd,WM_KEYDOWN,VK_ESCAPE,0);
				nwd = App->wd;
			}
			else
			{
				nwd = &App->WorldList[0]; //wrap around
			}
		}
		
		if (nwd != App->wd)
		{
			App_DisplayHourGlass(App);
			App->ResetTime = GE_TRUE;
			if (App_Resource.SoundSystem)
				geSound_SetMasterVolume( App_Resource.SoundSystem, 0.0f);
			App_ChangeWorld(App->wd, nwd);
			App->wd = nwd;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_BenchmarkTiming()
//
//	Processing for GDemo benchmark.
//
////////////////////////////////////////////////////////////////////////////////////////
static void App_BenchmarkTiming(App *App, float *ElapsedTime)
{
	assert(App);

	if ( App->BenchTimingOn == GE_TRUE )
	{
		App->BenchTotalRunTime += *ElapsedTime;
		*ElapsedTime = 0.033f;
		App->BenchTotalFrames++;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_ResetTiming()
//
//	Reset all timing variables.
//
////////////////////////////////////////////////////////////////////////////////////////
static void App_ResetTiming(App *App, float *ElapsedTime)
{
	assert(App);

	App->ResetTime = GE_FALSE;
	Cam_Reset(App->wd->Cam);
	App->WorldTime = 0;
	// start off with a real small value
	*ElapsedTime = 0.001f;
	QueryPerformanceCounter(&App->OldTick);
	if (App_Resource.SoundSystem)
		geSound_SetMasterVolume( App_Resource.SoundSystem, 1.0f);

	// timing stuff
	App->BenchTotalRunTime  = 0.0f;
	App->BenchTotalFrames = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_Frame()
//
//	Move and draw from application data.
//
////////////////////////////////////////////////////////////////////////////////////////
static geBoolean App_Frame(App *App, float ElapsedTime)
{
	App_WorldData *wd;

	assert(App);

	wd = App->wd;

	// fixes the sluggish mouse syndrome
	geEngine_Printf(App->Engine, 0, 0, " ");

	// handle mouse input
	if (!wd->CameraPathEditor || (wd->CameraPathEditor && (App->MouseFlags & 1)))
	{
		int Width, Height;
		VidMode_GetResolution(App->VidMode, &Width, &Height);
		Mouse_GetInput(&App->MouseSpeedX, &App->MouseSpeedY);
	}
	
	if (App->CameraPathPlayback)
	{
		geVec3d Channels;

		if (Cam_Update(wd->World, wd->Cam, &wd->MyXForm, &Channels, ElapsedTime) == GE_FALSE)
		{
			if (GlobApp->AppGui && GlobApp->GuiCurBg >= 2 )
			{
				// press escape key
				PostMessage(GlobApp->hWnd,WM_KEYDOWN,VK_ESCAPE,0);
			}
			else
			{
				GlobApp->NextLevel = GE_TRUE;
			}
		}
		else
		{
			wd->FieldOfView = Channels.X;
		}

	}
	else
	{
		// otherwise use input from mouse/keyboard
		GUtil_BuildXFormFromRotationOrderXY (&wd->Rot, &wd->LocalPos, &wd->MyXForm);
		App_MovePos (wd, wd->CameraObject, &wd->LocalPos, &wd->MyXForm, ElapsedTime);
		GUtil_BuildXFormFromRotationOrderXY (&wd->Rot, &wd->LocalPos, &wd->MyXForm);
		// update the objects transform
		ObjMgr_SetObjectXForm(wd->CameraObject, &wd->MyXForm);
	}

	App_SetupCameraFromXForm (wd, App->Camera, &wd->MyXForm);
	
	Corona_Frame(wd->World, &wd->MyXForm, ElapsedTime);
	ModelCtl_Frame(wd->World, &wd->MyXForm, ElapsedTime);
	DynLight_Frame(wd->World, &wd->MyXForm, ElapsedTime);

	// perform an effect manager frame
	EffectM_Update( App_Resource.EManager, ElapsedTime );
	
	ObjMgr_Process(wd->World, wd->ObjectList, ElapsedTime);
	ObjMgr_ObjectGetContents(wd->World, wd->CameraObject, ElapsedTime);

	// perform an effect system frame
	if ( !Effect_SystemFrame( App_Resource.ESystem, ElapsedTime ) )
	{
		App_Error( "Could not perform an effect system frame.\n" );
	}

	if (!geEngine_RenderWorld(App->Engine, wd->World, App->Camera, 0.0f))
	{
		App_Error("Could not render the world.\n");
	}

	if (!App->SoftwareMode)
	{
		Fade_Frame(App->Engine, ElapsedTime);
	}

	return GE_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	App_Update()
//
//	Main application function.
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean App_Update(App *App)
{
	float			ElapsedTime;

	assert(App);

	if (!App->Running)
		return GE_FALSE; // exit app

	if (App_Gui(App) == GE_TRUE)
		return GE_TRUE;	 // continue

	App_HandleLevelTransition(App);

	GUtil_CalcElapsedTime(&App->Freq, &App->OldTick, &App->CurTick, &ElapsedTime);
	App->WorldTime += ElapsedTime;
	
	App_BenchmarkTiming(App, &ElapsedTime);

	if (!geEngine_BeginFrame(App->Engine, App->Camera, App->ScreenClearFlag))
	{
		App_Error("EngineBeginFrame failed.\n");
	}

	// if needed, reset all timing AFTER BeginFrame because
	// the first BeginFrame uploads textures and can be SLOW
	if (App->ResetTime == GE_TRUE)
	{
		App_ResetTiming(App, &ElapsedTime);
	}

	if (App_Frame(App, ElapsedTime) == GE_FALSE)
	{
		App_Error("App_Frame failed.\n");
	}

	if (!geEngine_EndFrame(App->Engine))
	{
		App_Error("EngineEndFrame failed.\n");
	}

	return GE_TRUE;
}

