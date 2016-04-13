/*------------------------------------------------

  GMouse.cpp

	Mouse input type


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GMouse.h"
#include "GApp.h"
#include "GWin32.h"
#include "GDebug.h"


//	globals
//------------------------------------------------
GMouse	g_Mouse;



//	Definitions
//------------------------------------------------

GMouse::GMouse()
{
	m_Pos		= int2(0,0);
	m_PosChange	= int2(0,0);
	AllocButtons( GMouse::MaxButtons );
}



GMouse::~GMouse()
{

}



void GMouse::Init()
{
	//	update pos and init change
	POINT p;
	GetCursorPos(&p);
	ScreenToClient( GApp::g_pApp->Hwnd(), &p );

	m_Pos.x = p.x;
	m_Pos.y = p.y;
	m_PosChange.x = 0;
	m_PosChange.y = 0;
}




void GMouse::Update()
{
	//	ignore input if game isnt main window
	if ( !GWin32::IsActiveWindow() )
	{
		//	reset input values
		UpdateButton( GMouse::Left, FALSE );
		UpdateButton( GMouse::Right, FALSE );
		UpdateButton( GMouse::Middle, FALSE );
		UpdateButton( GMouse::WheelUp, FALSE );
		UpdateButton( GMouse::WheelDown, FALSE );
		m_PosChange = int2(0,0);
	}
	else
	{
		//	update pos
		POINT p;
		GetCursorPos(&p);
		ScreenToClient( GApp::g_pApp->Hwnd(), &p );

		int2 lastpos = m_Pos;
		m_Pos.x = p.x;
		m_Pos.y = p.y;
		m_PosChange.x = m_Pos.x - lastpos.x;
		m_PosChange.y = m_Pos.y - lastpos.y;

		//	update buttons
		UpdateButton( GMouse::Left,		GetAsyncKeyState( VK_LBUTTON ) >> 8 );
		UpdateButton( GMouse::Right,	GetAsyncKeyState( VK_RBUTTON ) >> 8 );
		UpdateButton( GMouse::Middle,	GetAsyncKeyState( VK_MBUTTON ) >> 8 );

		//	wheel state is updated by messages when it happens
		UpdateButton( GMouse::WheelUp, m_WheelScrolledUp );
		UpdateButton( GMouse::WheelDown, m_WheelScrolledDown );
	}


	m_WheelScrolledUp = FALSE;
	m_WheelScrolledDown = FALSE;
}


