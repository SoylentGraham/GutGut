/*------------------------------------------------

  GWindow.cpp

	Win32 window

-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GWindow.h"
#include "GDisplay.h"
#include "GApp.h"

//	globals
//------------------------------------------------



//	Definitions
//------------------------------------------------


GWindow::GWindow()
{
	m_pOwnerApp = NULL;
}


GWindow::~GWindow()
{
}


Bool GWindow::Init(GWinControl* pOwner, u32 Flags, u32 ControlID, char* WindowName)
{
	//	window needs to create class first
	if ( ! CreateClass() )
	{
		return FALSE;
	}

	return GWinControl::Init( pOwner, Flags, ControlID, WindowName );
}

void GWindow::OnMove()								
{
	if ( App() )
	{
		App()->OnMove();
	}
};


void GWindow::OnResize()								
{
	if ( App() )
	{
		App()->OnResize();
	}
}


void GWindow::OnMenuPopup( GMenuSubMenu* pSubMenu )	
{
	if ( App() )
	{
		App()->OnMenuPopup(pSubMenu);	
	}
}


void GWindow::OnMenuClick( GMenuItem* pMenuItem )		
{
	if ( App() )
	{
		App()->OnMenuClick(pMenuItem);
	}
}


GMenuSubMenu* GWindow::GetSubMenu( HMENU HMenu )
{	
	if ( App() )
	{
		GMenuSubMenu* pSubMenu = App()->GetSubMenu(HMenu);

		if ( pSubMenu )
			return pSubMenu;
	}

	return GWinControl::GetSubMenu( HMenu );
}


GMenuItem* GWindow::GetMenuItem( u16 ItemID )
{	
	if ( App() )
	{
		GMenuItem* pMenuItem = App()->GetMenuItem(ItemID);

		if ( pMenuItem )
			return pMenuItem;
	}

	return GWinControl::GetMenuItem( ItemID );
}














GOpenglWindow::GOpenglWindow()
{
	m_HDC	= NULL;
	m_HGLRC	= NULL;
}


GOpenglWindow::~GOpenglWindow()
{
}


Bool GOpenglWindow::Init(GWinControl* pOwner, u32 Flags, u32 ControlID, char* WindowName)
{
	//	create window
	if ( !GWindow::Init( pOwner, Flags, ControlID, WindowName ) )
	{
		return FALSE;
	}

	if ( GDisplay::g_OpenglInitialised )
	{
		GDebug_Break("Opengl already initialised\n");
		return FALSE;
	}

	//	create rendering stuff

	//	make the pixel format descriptor
	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),		// Size Of This Pixel Format Descriptor
		1,									// Version Number
		PFD_DRAW_TO_WINDOW |				// Format Must Support Window
		PFD_SUPPORT_OPENGL |				// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,					// Must Support Double Buffering
		PFD_TYPE_RGBA,						// Request An RGBA Format
	//	16,									// Select Our Color Depth
		24,									// Select Our Color Depth
		0, 0, 0, 0, 0, 0,					// Color Bits Ignored
		0,									// No Alpha Buffer
		0,									// Shift Bit Ignored
		0,									// No Accumulation Buffer
		0, 0, 0, 0,							// Accumulation Bits Ignored
		16,									// 16Bit Z-Buffer (Depth Buffer)  
		1,									//	use stencil buffer
		0,									// No Auxiliary Buffer
		PFD_MAIN_PLANE,						// Main Drawing Layer
		0,									// Reserved
		0, 0, 0								// Layer Masks Ignored
	};
			
	u32 PixelFormat=0;

	//	get the hdc
	if ( ! ( m_HDC = GetDC( m_Hwnd ) ) )
	{
		GDebug_Break("Failed to get HDC\n");
		return FALSE;
	}

	//	check we can use this pfd
	if ( ! ( PixelFormat = ChoosePixelFormat( m_HDC, &pfd ) ) )
	{
		GDebug::Print("Failed to choose pixel format %d\n",PixelFormat);
		GDebug_Break("Failed to choose pixel format\n");
		return FALSE;
	}

	//	set it to the pfd
	if ( ! SetPixelFormat( m_HDC, PixelFormat, &pfd ) )
	{
		GDebug_Break("Failed to set pixel format\n");
		return FALSE;
	}

	//	make and get the windows gl context for the hdc
	if ( ! ( m_HGLRC = wglCreateContext( m_HDC ) ) )
	{
		GDebug_Break("Failed to create context\n");
		return FALSE;
	}

	//	set the active HGLRC for this HDC
	if ( ! wglMakeCurrent( m_HDC, m_HGLRC ) )
	{
		GDebug_Break("Failed wglMakeCurrent\n");
		return FALSE;
	}

	GDisplay::g_OpenglInitialised = TRUE;

	glDisable( GL_LIGHTING );

	glDisable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading

	//	setup the texture alpha blending mode
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	//glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
	//glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );

	//	enable sphere map generation
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);


	GDebug::Print("Device %s\n", glGetString( GL_RENDERER ) );
	GDebug::Print("Driver %s %s\n", glGetString( GL_VENDOR ), glGetString( GL_VERSION ) );
	GDebug::Print("\n-------------\n");
/*
	//	print off extensions first	
	GDebug::Print("Extensions...\n");
	GDebug::Print( (char*)glGetString( GL_EXTENSIONS ) );
	GDebug::Print("\n-------------\n");
*/



	return TRUE;
}