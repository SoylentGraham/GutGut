/*------------------------------------------------

  GWindow Header file



-------------------------------------------------*/

#ifndef __GWINDOW__H_
#define __GWINDOW__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GWinControl.h"



//	Macros
//------------------------------------------------





//	Types
//------------------------------------------------
class GApp;

class GWindow : public GWinControl
{
protected:
	GApp*				m_pOwnerApp;

public:
	GWindow();
	~GWindow();

	virtual u32				DefaultFlags()						{	return GWinControlFlags::ClientEdge|GWinControlFlags::OverlappedWindow;	};
	inline void				SetOwnerApp(GApp* pApp)				{	m_pOwnerApp = pApp;	};
	inline GApp*			App()								{	return m_pOwnerApp;	};

	virtual const char*		ClassName()							{	return "Window";	};
	virtual Bool			Init(GWinControl* pOwner, u32 Flags, u32 ControlID, char* WindowName);	//	window is overloaded to create class

	virtual void			OnMove();
	virtual void			OnResize();
	virtual void			OnMenuPopup( GMenuSubMenu* pSubMenu );
	virtual void			OnMenuClick( GMenuItem* pMenuItem );
	virtual GMenuSubMenu*	GetSubMenu( HMENU HMenu );
	virtual GMenuItem*		GetMenuItem( u16 ItemID );
};




class GOpenglWindow : public GWindow
{
public:
	HDC				m_HDC;
	HGLRC			m_HGLRC;
	
public:
	GOpenglWindow();
	~GOpenglWindow();
	
	virtual const char*	ClassName()				{	return "OpenGL";	};	//	what class this window creates
	virtual Bool		Init(GWinControl* pOwner, u32 Flags, u32 ControlID, char* WindowName);	//	overloaded to create rendering stuff
	virtual Bool		OnEraseBackground()		{	return FALSE;	};		//	dont erase backgroud over the opegnl display (stops WM_PAINT flicker)
};






//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------




#endif

