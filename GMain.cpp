/*------------------------------------------------

  GMain.cpp

	bootup entry


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GDebug.h"
#include "GApp.h"
#include "GWin32.h"


//	globals
//------------------------------------------------
static Bool	g_DLLInit = FALSE;	//	has the DLL already been initialised?


//	Definitions
//------------------------------------------------

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//	set instance
	GApp::g_HInstance = hInstance;

	//	intialise win32 and debug stuff
	if ( ! GWin32::Init() )	return -1;
	if ( ! GDebug::Init() )	return -1;

	//	mok-a-moka-mok
	if ( GetSystemMetrics(SM_SLOWMACHINE) != 0 )
	{
		GDebug::Print("In soviet russia, your HARDWARE says WINDOWS is slow\n");
	}

	//	initialise app
	if ( !g_App )
	{
		GDebug_BreakGlobal("No app declared");
		return -1;
	}

	GApp::SetAppParams( GString(lpCmdLine) );
/*	if ( strcmp( lpCmdLine, "" ) != 0 )
	{
		GApp::SetAppPath( GString(lpCmdLine) );
		GApp::SetAppFilename( GString(lpCmdLine) );
	}
	else
	{
*/		char ModuleFilenameBuffer[GMAX_STRING];
		GetModuleFileName( hInstance, ModuleFilenameBuffer, GMAX_STRING );
		GApp::SetAppPath( GString(ModuleFilenameBuffer) );
		GApp::SetAppFilename( GString(ModuleFilenameBuffer) );
//	}

	if ( !g_App->Init() )
	{
		GDebug_BreakGlobal("App failed to Init");
		return -1;
	}

	//	just before first update, reset stats
	g_Stats.Init();

	//	loop
	while ( 1 )
	{
		if ( !GWin32::Update() )
			break;

		if ( !g_App->Update() )
			break;
	}

	//	shutdown
	g_App->GameDestroy();
	g_App->Shutdown();
	GDebug::Shutdown();

	return 0;
}


u32 GNearestPower( int Size )
{
	const u32 LowestPower = 2;

	//	handle low/negative numbers
	if ( Size <= LowestPower )
		return LowestPower;

	u32 Size32 = (u32)Size;

	//	find the largest bit. each bit is a power: 1,2,4,8,16..
	u32 Bit = 31;
	while ( (1<<Bit) > LowestPower )
	{
		if ( Size32 & Bit )
		{
			//	return this power bit
			return 1<<Bit;
		}
	}

	return LowestPower;
}




char* ExportString(const char* pString)
{
	//	multiple buffers so we can use exportstring more than once
	static char BufferA[GMAX_STRING];
	static char BufferB[GMAX_STRING];
	static char BufferC[GMAX_STRING];
	static char BufferD[GMAX_STRING];
	static char* g_Buffers[] = {	BufferA,	BufferB,	BufferC,	BufferD	};
	static int ActiveBuffer = 0;
	char* pBuffer = g_Buffers[ (ActiveBuffer++)%ARRAY_SIZE(g_Buffers) ];
	
	if ( pString == pBuffer )
		return pBuffer;

	pBuffer[0] = 0;
	strcpy( pBuffer, pString );
	return pBuffer;
}

//-------------------------------------------------------------------------
//	converts a string to a unicode string
//-------------------------------------------------------------------------
u16* ExportWString(const char* pString)
{
	//	multiple buffers so we can use exportstring more than once
	static u16 WBufferA[GMAX_STRING];
	static u16 WBufferB[GMAX_STRING];
	static u16 WBufferC[GMAX_STRING];
	static u16 WBufferD[GMAX_STRING];
	static u16* g_Buffers[] = {	WBufferA,	WBufferB,	WBufferC,	WBufferD	};
	static int ActiveBuffer = 0;
	u16* pBuffer = g_Buffers[ (ActiveBuffer++)%ARRAY_SIZE(g_Buffers) ];
	
	memset( pBuffer, 0, sizeof(u16) * GMAX_STRING );

	//	copy string
	for ( int c=0;	c<(int)strlen(pString);	c++ )
	{
		pBuffer[c] = (u16)(pString[c]);
	}

	return pBuffer;
}


char* ExportStringf(const char* pString,...)
{
	char TxtOut[GMAX_STRING];
	if ( strlen( pString ) >= GMAX_STRING )
	{
		GDebug_BreakGlobal("Formatted string for ExportString is too long");
	}

	va_list v;
	va_start( v, pString );
	int iChars = vsprintf((char*)TxtOut,pString, v );
	
	return ExportString( TxtOut );
}

char* AppendStringf(char* pOrigString, const char* pString,...)
{
	char TxtOut[GMAX_STRING];
	if ( strlen( pString ) >= GMAX_STRING )
	{
		GDebug_BreakGlobal("Formatted string for ExportString is too long");
	}

	va_list v;
	va_start( v, pString );
	int iChars = vsprintf((char*)TxtOut,pString, v );
	
	return ExportStringf( "%s%s", pOrigString, TxtOut );
}
