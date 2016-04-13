/*------------------------------------------------

  GApp.cpp

	Base app type. Handles win32 interaction


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GApp.h"
#include "GWin32.h"
#include "GDebug.h"
#include "GMouse.h"
#include "GKeyboard.h"
#include "GPad.h"
#include "GFile.h"


//	globals
//------------------------------------------------
GApp*		GApp::g_pApp		= NULL;
HINSTANCE	GApp::g_HInstance	= NULL;
GString		GApp::g_AppPath;
GString		GApp::g_AppFilename;
GString		GApp::g_AppParams;

GStatsCounterList	g_StatsCounterList;
GStatsTimerList		g_StatsTimerList;

GDeclareCounter(UpdateCounter);

//	Definitions
//------------------------------------------------

//-------------------------------------------------------------------------
//	sets the base path for files etc
//-------------------------------------------------------------------------
/*static*/Bool GApp::SetAppPath(GString& Path)
{
	GString RealPath;
	if ( !ExtractFilePath( Path, RealPath ) )
		return FALSE;

	g_AppPath = RealPath;
	
	return TRUE;
}

//-------------------------------------------------------------------------
//	sets the app's EXE filename
//-------------------------------------------------------------------------
/*static*/Bool GApp::SetAppFilename(GString& Filename)
{
	g_AppFilename = Filename;
	
	return TRUE;
}

//-------------------------------------------------------------------------
//	sets the app's params
//-------------------------------------------------------------------------
/*static*/Bool GApp::SetAppParams(GString& Params)
{
	g_AppParams = Params;
	
	return TRUE;
}



GApp::GApp()
{
	g_pApp					= this;
	m_AppFlags				= GAppFlags::Default;
	m_pWindow				= NULL;
	m_SlowMotionModifier	= 1.f;
}


GApp::~GApp()
{
}



Bool GApp::Init()
{
	//	can only get the module handle after main() 
	//GApp::g_HInstance = GetModuleHandle(NULL);
	if ( !CreateAppWindow() )
	{
		return FALSE;
	}

	Window()->SetOwnerApp( this );
	m_Display.m_pWindow = (GOpenglWindow*)Window();

	//	init inputs
	g_Keyboard.Init();
	g_Mouse.Init();
	g_Pad.Init();


	//	game initialisation
	if ( !GameInit() )
	{
		return FALSE;
	}

	//	window needs to already have been allocated
	if ( !Window() )
	{
		GDebug_Break("Window needs to already have been created\n");
		return FALSE;
	}

	//	create new window
	if ( ! Window()->Init( NULL, Window()->DefaultFlags(), 0, (char*)GameName() ) )
	{
		return FALSE;
	}

	//	create display, window etc
	if ( !m_Display.Init() )
	{
		return FALSE;
	}

	//	game post-display init
	if ( !GameDisplayInit() )
	{
		return FALSE;
	}

	Window()->Show();
	
	//	make sure everything is sized up
	OnResize();

	return TRUE;
}



Bool GApp::DllInit()
{
	//	game initialisation
	if ( !GameInit() )
	{
		return FALSE;
	}

	return TRUE;
}



void GApp::Shutdown()
{
	//	kill display
	m_Display.Shutdown();
	
	//	shutdown inputs
	g_Keyboard.Shutdown();
	g_Mouse.Shutdown();
	g_Pad.Shutdown();

	//	destroy window
	GDelete( m_pWindow );
}




Bool GApp::Update()
{
	//	flagged to quit
	if ( m_AppFlags & GAppFlags::QuitApp )
		return FALSE;

	//	update update counter
	m_Stats.OnNewFrame();
	GIncCounter(UpdateCounter,1);

	//	update inputs
	g_Keyboard.Update();
	g_Mouse.Update();
	g_Pad.Update();

	//	update stats
	m_Stats.Update();


	//	update game
	if ( !GameUpdate() )
		return FALSE;

	//	draw
	m_Display.Draw();

	return TRUE;
}




