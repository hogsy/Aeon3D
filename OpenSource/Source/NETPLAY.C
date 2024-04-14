/****************************************************************************************/
/*  NetPlay.c                                                                           */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: DirectPlay wrapper                                                     */
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
/*  The Original Code is Genesis3D, released March 25, 1999.                            */
/*Genesis3D Version 1.1 released November 15, 1999                            */
/*  Copyright (C) 1999 WildTangent, Inc. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/

#include <assert.h>

#include "netplay.h"
#include "ErrorLog.h"

//#define INIT_GUID

//#define UNICODE   (do not use, not done yet.  Need to fix strings...)

//************************************************************************************
// Misc globals all will need...
//************************************************************************************

LPGUID glpGuid;

SP_DESC       GlobalSP;
SESSION_DESC *GlobalSession;// Global sessions availible
DWORD         gSessionCnt;
BOOL          FoundSP = FALSE;// If the provider was found
BOOL          FoundSession = FALSE;

BOOL   FoundConnection = FALSE;
LPVOID lpConnectionBuffer = NULL;

// ************************************************************************************
//	Misc global functions
// ************************************************************************************

//========================================================================================================
//	InitNetPlay
//	Enumerate the service providers, and everything else...
//========================================================================================================
BOOL InitNetPlay( LPGUID lpGuid )
{
	HRESULT Hr;

	glpGuid = lpGuid;

	FoundSP = FALSE;

#if 0// todo
	Hr = DPlayCreate();

	if (Hr != DP_OK)
	{
		DoDPError(Hr);
		return FALSE;
	}
	
	IDirectPlayX_EnumConnections( g_lpDP, glpGuid, DPEnumConnectionsCallback, NULL, 0);
#endif

	if ( !FoundConnection )
	{
		geErrorLog_AddString( -1, "InitNetPlay:  No connections available.\n", NULL );
		return FALSE;
	}

	return TRUE;
}

//====================================================================================================
//	 NetPlayEnumSession
//====================================================================================================
BOOL NetPlayEnumSession( LPSTR IPAdress, SESSION_DESC **SessionList, int32_t *SessionNum )
{
#if 0// todo

	HRESULT hr;

#	if 0
	char					tempBuf[1024];
	DWORD					tempLng = 1024;
	LPDIRECTPLAYLOBBY2A		lpDPL = NULL;

	// Free the old connection buffer
	if(lpConnectionBuffer ) 
	{
		free( lpConnectionBuffer );
		lpConnectionBuffer = NULL;
	}

	hr = CoCreateInstance(	&CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER,
							&IID_IDirectPlayLobby3A, (LPVOID *) &lpDPL );

	if (hr != DP_OK)
	{
		DoDPError(hr);
		return( FALSE );
	}

 	hr = IDirectPlayLobby_CreateAddress(lpDPL, &DPSPGUID_TCPIP, &DPAID_INet, (LPVOID)IPAdress, strlen(IPAdress), tempBuf, &tempLng);

	if (hr != DP_OK)
	{
		DoDPError(hr);
		return( FALSE );
	}

	if (lpDPL)
	{
		hr = IDirectPlayLobby_Release(lpDPL);
		
		if (hr != DP_OK)
		{
			DoDPError(hr);
			return( FALSE );
		}
		lpDPL = NULL;
	}

	hr = IDirectPlayX_InitializeConnection( g_lpDP, tempBuf, 0);
#	else
	hr = IDirectPlayX_InitializeConnection( g_lpDP, lpConnectionBuffer, 0 );
#	endif

	if ( hr != DP_OK )
	{
		DoDPError( hr );
		return ( FALSE );
	}

	GlobalSession = NULL;
	gSessionCnt = 0;

	hr = DPlayEnumSessions( 0, EnumSession, NULL, 0 );

	*SessionList = GlobalSession;
	*SessionNum = gSessionCnt;

#else

	*SessionList = NULL;
	*SessionNum = 0;

#endif

	return ( TRUE );
}

//==================================================================================================
//	NetPlayCreateSession
//==================================================================================================
BOOL NetPlayCreateSession( LPSTR SessionName, DWORD MaxPlayers )
{
#if 0//todo

	HRESULT Hr;

	assert( g_lpDP );
	assert( lpConnectionBuffer );

	Hr = IDirectPlayX_InitializeConnection( g_lpDP, lpConnectionBuffer, 0 );

	if ( Hr != DP_OK )
	{
		DoDPError( Hr );
		return FALSE;
	}

	Hr = DPlayCreateSession( SessionName, MaxPlayers );

	if ( Hr != DP_OK )
	{
		DoDPError( Hr );
		return FALSE;
	}

	return TRUE;

#else

	return TRUE;

#endif
}

//==================================================================================================
//	NetPlayJoinSession
//==================================================================================================
BOOL NetPlayJoinSession( SESSION_DESC *Session )
{
#if 0//todo

	HRESULT Hr;

	Hr = DPlayOpenSession( &Session->Guid );

	if ( Hr != DP_OK )
	{
		DoDPError( Hr );
		return FALSE;
	}

	return TRUE;

#else

	return TRUE;

#endif
}

//==================================================================================================
//	NetPlayCreatePlayer
//	Creates a player for session
//==================================================================================================
BOOL NetPlayCreatePlayer( LPDPID lppidID, LPTSTR lptszPlayerName, HANDLE hEvent, LPVOID lpData, DWORD dwDataSize, geBoolean ServerPlayer )
{
#if 0//todo

	HRESULT hr = DPERR_GENERIC;
	DPNAME  name;
	DWORD   Flags;

	assert( g_lpDP );

	ZeroMemory( &name, sizeof( name ) );
	name.dwSize = sizeof( DPNAME );

#ifdef UNICODE
	name.lpszShortName = lptszPlayerName;
#else
	name.lpszShortNameA = lptszPlayerName;
#endif

	Flags = 0;

	//if (ServerPlayer)
	//	Flags |= DPPLAYER_SERVERPLAYER;

	hr = IDirectPlayX_CreatePlayer( g_lpDP, lppidID, &name, hEvent, lpData, dwDataSize, Flags );

	if ( hr != DP_OK )
	{
		DoDPError( hr );
		return FALSE;
	}

	return TRUE;

#else

	return TRUE;

#endif
}

//=========================================================================================================
//	NetPlayDestroyPlayer
//=========================================================================================================
BOOL NetPlayDestroyPlayer( DPID pid )
{
#if 0//todo

	HRESULT Hr = DPlayDestroyPlayer( pid );

	if ( Hr != DP_OK )
	{
		DoDPError( Hr );
		return FALSE;
	}

#endif

	return TRUE;
}

//=========================================================================================================
//	NetPlayReceive
//=========================================================================================================
HRESULT NetPlayReceive( LPDPID lpidFrom, LPDPID lpidTo, DWORD dwFlags, LPVOID lpData, LPDWORD lpdwDataSize )
{
#if 0//todo

	HRESULT Hr;
	assert( g_lpDP );

	Hr = IDirectPlayX_Receive( g_lpDP, lpidFrom, lpidTo, dwFlags, lpData, lpdwDataSize );

	if ( Hr != DP_OK )
	{
		if ( Hr != DPERR_NOMESSAGES )
			DoDPError( Hr );
	}

	return Hr;

#else

	return 0;

#endif
}

//=========================================================================================================
//	NetPlaySend
//=========================================================================================================
HRESULT NetPlaySend( DPID idFrom, DPID idTo, DWORD dwFlags, LPVOID lpData, DWORD dwDataSize )
{
#if 0//todo

	HRESULT Hr;

	assert( g_lpDP );

#	if 0
	dwFlags |= DPSEND_ASYNC;
    Hr = IDirectPlayX_SendEx(g_lpDP, idFrom, idTo, dwFlags, lpData, dwDataSize, 0, 0, NULL, NULL);
#	else
	Hr = IDirectPlayX_Send( g_lpDP, idFrom, idTo, dwFlags, lpData, dwDataSize );
#	endif

	if ( Hr != DP_OK )
	{
		if ( Hr == DPERR_PENDING )
			return DP_OK;

		DoDPError( Hr );
	}

	return Hr;

#else

	return 0;

#endif
}

//=========================================================================================================
//	DeInitNetPlay
//=========================================================================================================
BOOL DeInitNetPlay( void )
{
	HRESULT Hr;

	if ( lpConnectionBuffer )
	{
		free( lpConnectionBuffer );
		lpConnectionBuffer = NULL;
	}

	FoundConnection = FALSE;
	FoundSP = FALSE;

#if 0//todo

	Hr = DPlayRelease();

	if ( Hr != DP_OK )
	{
		DoDPError( Hr );
		return FALSE;
	}

#endif

	return TRUE;
}

//====================================================================================================
//	NetPlayGetNumMessages
//====================================================================================================
geBoolean NetPlayGetNumMessages( int32 *NumMsgSend, int32 *NumBytesSend, int32 *NumMsgRec, int32 *NumBytesRec )
{
#if 0//todo

	HRESULT Hr;
	DPID    IdFrom, IdTo;

	IdFrom = IdTo = 0;

	Hr = IDirectPlayX_GetMessageQueue( g_lpDP, IdFrom, IdTo, DPMESSAGEQUEUE_SEND, ( LPDWORD ) NumMsgSend, ( LPDWORD ) NumBytesSend );

	if ( Hr != DP_OK )
	{
		geErrorLog_AddString( -1, "NetPlayGetNumMessages:  IDirectPlayX_GetMessageQueue failed.\n", NULL );
		DoDPError( Hr );
		return GE_FALSE;
	}

	Hr = IDirectPlayX_GetMessageQueue( g_lpDP, IdFrom, IdTo, DPMESSAGEQUEUE_RECEIVE, ( LPDWORD ) NumMsgRec, ( LPDWORD ) NumBytesRec );

	if ( Hr != DP_OK )
	{
		geErrorLog_AddString( -1, "NetPlayGetNumMessages:  IDirectPlayX_GetMessageQueue failed.\n", NULL );
		DoDPError( Hr );
		return GE_FALSE;
	}

#endif

	return GE_TRUE;
}

#if 0//todo

//========================================================================================================
//	DPlayEnumSessions
//========================================================================================================
HRESULT DPlayEnumSessions( DWORD dwTimeout, LPDPENUMSESSIONSCALLBACK2 lpEnumCallback,
                           LPVOID lpContext, DWORD dwFlags )
{
	HRESULT        hr = E_FAIL;
	DPSESSIONDESC2 dpDesc;

	ZeroMemory( &dpDesc, sizeof( dpDesc ) );
	dpDesc.dwSize = sizeof( dpDesc );

	if ( glpGuid )
		dpDesc.guidApplication = *glpGuid;

	if ( g_lpDP )
		hr = IDirectPlayX_EnumSessions( g_lpDP, &dpDesc, dwTimeout, lpEnumCallback,
		                                lpContext, dwFlags );
	return hr;
}

//========================================================================================================
//	Callback for enum session
//========================================================================================================
BOOL WINAPI EnumSession( LPCDPSESSIONDESC2 lpDPSessionDesc, LPDWORD lpdwTimeOut, DWORD dwFlags,
                         LPVOID lpContext )
{
	if ( dwFlags & DPESC_TIMEDOUT )
		return FALSE;// don't try again

	gSessionCnt++;

	if ( GlobalSession )
		GlobalSession = ( SESSION_DESC * ) realloc( GlobalSession, gSessionCnt * sizeof( SESSION_DESC ) );
	else
		GlobalSession = ( SESSION_DESC * ) malloc( sizeof( SESSION_DESC ) );

	GlobalSession[ gSessionCnt - 1 ].Guid = lpDPSessionDesc->guidInstance;

#ifdef UNICODE
	strcpy( GlobalSession[ gSessionCnt - 1 ].SessionName, lpDPSessionDesc->lpszSessionName );
#else
	strcpy( GlobalSession[ gSessionCnt - 1 ].SessionName, lpDPSessionDesc->lpszSessionNameA );
#endif

	return ( TRUE );
}

//====================================================================================================
//	DPlayCreatePlayer
//====================================================================================================
HRESULT DPlayCreatePlayer( LPDPID lppidID, LPTSTR lptszPlayerName, HANDLE hEvent,
                           LPVOID lpData, DWORD dwDataSize )
{
	HRESULT hr = DPERR_GENERIC;
	DPNAME  name;

	assert( g_lpDP );

	ZeroMemory( &name, sizeof( name ) );
	name.dwSize = sizeof( DPNAME );

#ifdef UNICODE
	name.lpszShortName = lptszPlayerName;
#else
	name.lpszShortNameA = lptszPlayerName;
#endif

	hr = IDirectPlayX_CreatePlayer( g_lpDP, lppidID, &name, hEvent, lpData, dwDataSize, DPPLAYER_SERVERPLAYER );

	return hr;
}

//====================================================================================================
//	DPlayDestroyPlayer
//====================================================================================================
HRESULT DPlayDestroyPlayer( DPID pid )
{
#if 0//todo

	HRESULT hr = E_FAIL;

	assert( g_lpDP );

	hr = IDirectPlayX_DestroyPlayer( g_lpDP, pid );

	return hr;

#else

	return 0;

#endif
}

//====================================================================================================
//	DPlayRelease
//====================================================================================================
HRESULT DPlayRelease( void )
{
#if 0//todo

	HRESULT hr = DP_OK;

	if ( g_lpDP )
	{
		IDirectPlayX_Close( g_lpDP );

		hr = IDirectPlayX_Release( g_lpDP );
		g_lpDP = NULL;
	}

	CoUninitialize();

	return hr;

#else

	return 0;

#endif
}

//====================================================================================================
//	DPEnumConnectionsCallback
//====================================================================================================
BOOL FAR PASCAL DPEnumConnectionsCallback(
        LPCGUID   lpguidSP,
        LPVOID    lpConnection,
        DWORD     dwSize,
        LPCDPNAME lpName,
        DWORD     dwFlags,
        LPVOID    lpContext )
{
	LPSTR Str;

	if ( FoundConnection )
		return TRUE;

	Str = lpName->lpszShortNameA;

	// Loop through and try to see if this is the service provider we want (TCP/IP for now...)
	while ( strlen( Str ) > 0 )
	{
		if ( !strnicmp( Str, "TCP", 3 ) )
		//if (!strnicmp(Str, "Serial", 6))
		{
			// Make sure it's deleted
			if ( lpConnectionBuffer )
			{
				free( lpConnectionBuffer );
				lpConnectionBuffer = NULL;
			}

			// make space for Connection Shortcut
			lpConnectionBuffer = ( char * ) malloc( dwSize );
			if ( lpConnectionBuffer == NULL )
				goto FAILURE;

			memcpy( lpConnectionBuffer, lpConnection, dwSize );
			FoundConnection = TRUE;
			break;
		}
		Str++;
	}

FAILURE:
	return ( TRUE );
}

#endif
