/*------------------------------------------------

  GApp Header file



-------------------------------------------------*/

#ifndef __GAPP__H_
#define __GAPP__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GCamera.h"
#include "GMenu.h"
#include "GStats.h"
#include "GWin32.h"
#include "GDisplay.h"
#include "GString.h"


//	Macros
//------------------------------------------------
#define g_App	(GApp::g_pApp)

namespace GAppFlags
{
	const u32	QuitApp			= 1<<0;
	const u32	SyncFrames		= 1<<1;
	const u32	Win32Wait		= 1<<2;	//	if set, we dont do an update every frame, we wait for windows messages

	const u32	Default			= 0x0;
};




//	Types
//------------------------------------------------
class GMenuItem;
class GMenuSubMenu;
class GWinControl;
class GWinTreeView;
class GWinTreeItem;




class GApp
{
public:
	static GApp*			g_pApp;
	static HINSTANCE		g_HInstance;

	static GString			g_AppPath;							//	base path for files (usually exe's path)
	static GString			g_AppFilename;						//	app's original exe/dll filename
	static GString			g_AppParams;						//	program arguments
	static Bool				SetAppPath(GString& Path);			//	pass in exe or dll file and this will extract the path
	static Bool				SetAppFilename(GString& Filename);	//	set app's filename
	static Bool				SetAppParams(GString& Params);		//	set app's params
	
public:
	u32						m_AppFlags;					//	GAppFlags
	GStats					m_Stats;
	GDisplay				m_Display;
	float					m_SlowMotionModifier;		//	1.f is regular FrameDelta, use to speedup/slowdown physics

protected:
	GWindow*				m_pWindow;

public:
	GApp();
	~GApp();

	Bool					Init();
	Bool					DllInit();					//	init without the display
	Bool					Update();
	void					Draw();
	inline void				QuitGame()					{	m_AppFlags |= GAppFlags::QuitApp;	};
	void					Shutdown();
	inline float			FrameDelta()				{	return m_Stats.m_FrameDelta * m_SlowMotionModifier;	};	//	sum in a second adds to 1
	inline float			FrameDelta60()				{	return FrameDelta() * 60.f;	};							//	sum in a second adds to 60

	//	game code
	virtual Bool			GameInit()					{	return TRUE;	};				//	first initialisation
	virtual Bool			GameDisplayInit()			{	return TRUE;	};				//	post display creation initialisation (Create controls etc)
	virtual Bool			GameUpdate()				{	return TRUE;	};				//	regular update
	virtual void			GameDraw()					{	};
	virtual void			GameDestroy()				{	};
	virtual const char*		GameName()					{	return "GutGut App";	};		//	basic description, used for title bars etc
	virtual const char*		GameDescription()			{	return "Base GutGut App";	};	//	longer description, used for DLL descriptions

	//	win32 
	inline GWindow*			Window()					{	return m_pWindow;	};
	virtual Bool			CreateAppWindow()			{	m_pWindow = new GAppWindow;	return TRUE;	};
	inline HWND				Hwnd()						{	return Window()->Hwnd();	};
	virtual void			OnMove()					{	};									//	called after resize, m_ClientSize has been updated
	virtual void			OnResize()					{	};									//	called after resize, m_ClientSize has been updated
	virtual int				HandleMessage(u32 message, WPARAM wParam, LPARAM lParam)	{	return 0;	};	//	message has been sent to this control matching the hwnd
	virtual void			OnMenuClick(GMenuItem* pMenuItem)		{	};
	virtual void			OnMenuPopup(GMenuSubMenu* pSubMenu)		{	};
	virtual GMenuSubMenu*	GetSubMenu( HMENU HMenu )				{	return NULL;	};
	virtual GMenuItem*		GetMenuItem( u16 ItemID )				{	return NULL;	};
};








//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------



#endif

