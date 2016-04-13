/*------------------------------------------------

  GWinControl.cpp

	Base class for win32 controls
	

-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GWinControl.h"
#include "GApp.h"
#include "GMouse.h"
#include <winuser.h>	//	updated core SDK


//	globals
//------------------------------------------------
GWinControlList		g_WinControlList;	//	global list of all the created windows controls
LRESULT CALLBACK	Win32CallBack(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );
u32					GWinControl::g_MouseWheelMsg = 0;


//	Definitions
//------------------------------------------------


GWinControl* GWinControlList::FindControl(HWND Hwnd)
{
	//	find control matching hwnd
	for ( int i=0;	i<Size();	i++ )
	{
		if ( ElementAt(i)->m_Hwnd == Hwnd )
			return ElementAt(i);
	}

	//	not found in list
	return NULL;
}



GWinControl::GWinControl()
{
	m_Hwnd			= NULL;
	m_OwnerHwnd		= NULL;
	m_pOwnerControl	= NULL;
	m_ControlID		= 0;
	m_ClientPos		= int2(0,0);
	m_ClientSize	= int2(40,40);
	m_StyleFlags	= 0x0;
}



GWinControl::~GWinControl()
{
	//	destroy control
	Destroy();

	//	null entry in children
	for ( int c=0;	c<m_ChildControls.Size();	c++ )
	{
		if ( m_ChildControls[c]->m_pOwnerControl == this )
		{
			m_ChildControls[c]->m_pOwnerControl = NULL;
		}
	}

	//	remove from parents list
	if ( m_pOwnerControl )
	{
		int Index = m_pOwnerControl->m_ChildControls.FindIndex(this);
		m_pOwnerControl->m_ChildControls.RemoveAt( Index );
	}

}


void GWinControl::PosToScreen(int2& ClientPos)
{
	//	convert client pos to screen
	POINT p;
	p.x = ClientPos.x;
	p.y = ClientPos.y;
	ClientToScreen( m_Hwnd, &p );
	ClientPos.x = p.x;
	ClientPos.y = p.y;
}


void GWinControl::ScreenToPos(int2& ScreenPos)
{
	//	convert screen pos to client pos
	POINT p;
	p.x = ScreenPos.x;
	p.y = ScreenPos.y;
	ScreenToClient( m_Hwnd, &p );
	ScreenPos.x = p.x;
	ScreenPos.y = p.y;
}



Bool GWinControl::Init(GWinControl* pOwner, u32 Flags, u32 ControlID, char* WindowName)
{
	//	check we havent already created a contorl
	if ( m_Hwnd != NULL )
	{
		GDebug_Break("Control already created\n");
		return FALSE;
	}
	
	//	create control
	Flags |= AdditionalStyleFlags();
	m_StyleFlags	= Flags;

//	HMENU hMenu		= (HMENU)ControlID;
	HMENU hMenu		= (HMENU)NULL;
	HINSTANCE hInstance = GApp::g_HInstance;
	HWND OwnerHwnd	= pOwner ? pOwner->m_Hwnd : m_OwnerHwnd;

	//	reset handle
	m_Hwnd = NULL;

	m_ControlID = ControlID;
	m_pOwnerControl = pOwner;

	//	get resulting hwnd, m_Hwnd is set from the WM_CREATE callback
	HWND ResultHwnd = CreateWindowEx( StyleExFlags(), ClassName(), WindowName, StyleFlags(), m_ClientPos.x, m_ClientPos.y, m_ClientSize.x, m_ClientSize.y, OwnerHwnd, hMenu, hInstance, (void*)this );

	//	if control doesnt get a WM_CREATE (if its a standard windows control) call it
	if ( ResultHwnd != NULL && m_Hwnd == NULL )
	{
		OnWindowCreate( this, ResultHwnd );
	}

	//	failed
	if ( m_Hwnd == NULL || ResultHwnd == NULL || m_Hwnd != ResultHwnd )
	{
		GDebug::CheckWin32Error();
		return FALSE;
	}

	//	control has been created
	OnCreate();

	return TRUE;
}

//-------------------------------------------------------------------------
//	callback after a window has been created
//-------------------------------------------------------------------------
/*static*/void GWinControl::OnWindowCreate(GWinControl* pControl,HWND Hwnd)
{
	if ( !pControl )
	{
		GDebug_BreakGlobal("WM_CREATE callback with invalid control pointer\n");
		return;
	}
	
	//	set handle
	pControl->m_Hwnd = Hwnd;

	//	update styles added by windows
	pControl->GetStyleFlags();

	//	set member values now we successfully created the window
	if ( pControl->m_pOwnerControl )
	{
		pControl->m_pOwnerControl->m_ChildControls.Add( pControl );
	}

	//	add to global control list
	g_WinControlList.Add( pControl );
}


Bool GWinControl::CreateClass()
{
	WNDCLASS wc;
	ZeroMemory(&wc,sizeof(wc));
	wc.style		= ClassStyle();
	wc.lpfnWndProc	= Win32CallBack; 
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= GApp::g_HInstance;
	wc.hIcon		= GetIconHandle();
	wc.hCursor		= NULL;//LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetBackgroundBrush();
	wc.lpszMenuName	= NULL;
	wc.lpszClassName = ClassName();

	if (!RegisterClass(&wc))
	{
		GDebug::CheckWin32Error();
		return FALSE;
	}

	return TRUE;
}

/*static*/Bool GWinControl::DestroyClass(const char* pClassName)
{
	if ( !UnregisterClass( pClassName, GApp::g_HInstance ) )
	{
		GDebug::CheckWin32Error();
		return FALSE;
	}

	return TRUE;
}

Bool GWinControl::DestroyClass()
{
	return DestroyClass( ClassName() );
}

Bool GWinControl::ClassExists()
{
	WNDCLASS wc;
	return GetClassInfo( GApp::g_HInstance, ClassName(), &wc );
}


void GWinControl::Destroy()
{
	OnDestroy();

	//	destroy the control if we still have a handle
	if ( m_Hwnd )
	{
		if ( ! DestroyWindow( m_Hwnd ) )
		{
			GDebug::CheckWin32Error();
		}
		m_Hwnd = NULL;
	}

	//	remove from global control list
	int Index = g_WinControlList.FindIndex(this);
	if ( Index != -1 )
		g_WinControlList.RemoveAt( Index );
}



void GWinControl::Show(Bool Show)
{
	if ( m_Hwnd )
	{
		ShowWindow( m_Hwnd, Show ? SW_SHOW : SW_HIDE );
		GetStyleFlags();
	}
}

void GWinControl::SetText(char* pText)
{
	if ( m_Hwnd )
	{
		SetWindowText( m_Hwnd, pText );
	}
}


//-------------------------------------------------------------------------
//	redraw window
//-------------------------------------------------------------------------
void GWinControl::Refresh()
{
	if ( m_Hwnd)
	{
		//	invalidate whole window for redrawing
		if ( InvalidateRgn( m_Hwnd, NULL, TRUE ) )
		{
			UpdateWindow( m_Hwnd );
		}
		else
		{
			GDebug::CheckWin32Error();
		}
	}

	//	refresh is manually called, so update scrollbars
	UpdateScrollBars();
}


//-------------------------------------------------------------------------
//	set new pos and dimensions at once
//-------------------------------------------------------------------------
void GWinControl::SetDimensions(int2 Pos, int2 Size)
{
	//	update our pos
	m_ClientPos = Pos;

	//	update our size
	m_ClientSize = Size;

	//	apply the changes
	UpdateDimensions();
}


//-------------------------------------------------------------------------
//	set new position
//-------------------------------------------------------------------------
void GWinControl::Move(int2 Pos)
{
	//	update our pos
	m_ClientPos = Pos;

	//	apply the changes
	UpdateDimensions();
}


//-------------------------------------------------------------------------
//	set new width/height
//-------------------------------------------------------------------------
void GWinControl::Resize(int2 Size)
{
	//	update our size
	m_ClientSize = Size;

	//	apply the changes
	UpdateDimensions();
}


//-------------------------------------------------------------------------
//	update window dimensions to current client size settings
//-------------------------------------------------------------------------
void GWinControl::UpdateDimensions()
{
	if ( m_Hwnd )
	{
		if ( !MoveWindow( m_Hwnd, m_ClientPos.x, m_ClientPos.y, m_ClientSize.x, m_ClientSize.y, TRUE ) )
		{
			GDebug::CheckWin32Error();
		}
	}
}


//-------------------------------------------------------------------------
//	calcs the window's overall size to accomadate the client size
//-------------------------------------------------------------------------
void GWinControl::ResizeClientArea(int2 ClientSize)
{
	RECT ClientRect;
	ClientRect.top		= 0;
	ClientRect.left		= 0;
	ClientRect.bottom	= ClientSize.y;
	ClientRect.right	= ClientSize.x;

	if ( !AdjustWindowRectEx( &ClientRect, StyleFlags(), HasMenu(), StyleExFlags() ) )
	{
		GDebug::CheckWin32Error();
		return;
	}

	//	set new window size
	Resize( int2( ClientRect.right-ClientRect.left, ClientRect.bottom-ClientRect.top ) );
}


void GWinControl::GetStyleFlags()
{
	m_StyleFlags = 0x0;

	//	use GetWindowLongPtr in newer platform SDK
	#if(WINVER >= 0x0500)
		m_StyleFlags |= GetWindowLongPtr( m_Hwnd, GWL_STYLE );
		m_StyleFlags |= GetWindowLongPtr( m_Hwnd, GWL_EXSTYLE );
	#else
		m_StyleFlags |= GetWindowLong( m_Hwnd, GWL_STYLE );
		m_StyleFlags |= GetWindowLong( m_Hwnd, GWL_EXSTYLE );
	#endif
}
	
void GWinControl::SetNewStyleFlags(u32 Flags)
{
	m_StyleFlags = Flags;

	if ( !SetWindowLong( m_Hwnd, GWL_STYLE, StyleFlags() ) )
		GDebug::CheckWin32Error();

	if ( !SetWindowLong( m_Hwnd, GWL_EXSTYLE, StyleExFlags() ) )
		GDebug::CheckWin32Error();

	HWND WindowOrder = HWND_NOTOPMOST;
	if ( StyleExFlags() & GWinControlFlags::AlwaysOnTop )
		WindowOrder = HWND_TOPMOST;

	//	update/refresh window
	if ( !SetWindowPos( m_Hwnd, WindowOrder, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE ) )
		GDebug::CheckWin32Error();

}

void GWinControl::SetStyleFlags(u32 Flags)
{
	//	add flags to our current set
	SetNewStyleFlags( m_StyleFlags|Flags );
}

void GWinControl::ClearStyleFlags(u32 Flags)
{
	//	remove flags from our current flags
	SetNewStyleFlags( m_StyleFlags&(~Flags) );
}


GMenuSubMenu* GWinControl::GetChildSubMenu(HMENU HMenu, GWinControl** ppControl)	
{
	GMenuSubMenu* pSubMenu = NULL;
	*ppControl = NULL;

	for ( int i=0;	i<m_ChildControls.Size();	i++ )
	{
		pSubMenu = m_ChildControls[i]->GetSubMenu( HMenu );
		if ( pSubMenu )
		{
			*ppControl = m_ChildControls[i];
			break;
		}
	}

	return pSubMenu;
}



GMenuItem* GWinControl::GetChildMenuItem(u16 ItemID, GWinControl** ppControl)	
{
	GMenuItem* pMenuItem = NULL;
	*ppControl = NULL;

	for ( int i=0;	i<m_ChildControls.Size();	i++ )
	{
		pMenuItem = m_ChildControls[i]->GetMenuItem( ItemID );
		if ( pMenuItem )
		{
			*ppControl = m_ChildControls[i];
			break;
		}
	}

	return pMenuItem;
}


//-------------------------------------------------------------------------
//	update window's scrollbar info if applicable
//-------------------------------------------------------------------------
void GWinControl::UpdateScrollBars()
{
	//	only needed if window is setup
	if ( !Hwnd() )
		return;

	//	
	SCROLLINFO ScrollInfo;
	ScrollInfo.cbSize = sizeof(SCROLLINFO);

	//	update vert scroll bar
	if ( StyleFlags() & GWinControlFlags::VertScroll )
	{
		int Min = 0;
		int Max = 0;
		int Jump = 0;
		int Pos = 0;
		Bool Enable = GetVertScrollProperties( Min, Max, Jump, Pos );

		ScrollInfo.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
		if ( Enable )
		{
			ScrollInfo.nMin		= Min;
			ScrollInfo.nMax		= Max;
			ScrollInfo.nPage	= Jump;
			ScrollInfo.nPos		= Pos;
			ScrollInfo.fMask |= SIF_DISABLENOSCROLL;	//	disable if invalid/useless properties (eg. min==max)
		}
		else
		{
			//	remove scroll bar by invalidating properies and NOT setting SIF_DISABLENOSCROLL flag
			ScrollInfo.nMin		= 0;
			ScrollInfo.nMax		= 0;
			ScrollInfo.nPage	= 0;
			ScrollInfo.nPos		= 0;
			ScrollInfo.fMask &= ~SIF_DISABLENOSCROLL;
		}

		SetScrollInfo( Hwnd(), SB_VERT, &ScrollInfo, TRUE );
	}

	//	update horz scroll bar
	if ( StyleFlags() & GWinControlFlags::HorzScroll )
	{
		int Min = 0;
		int Max = 0;
		int Jump = 0;
		int Pos = 0;
		Bool Enable = GetHorzScrollProperties( Min, Max, Jump, Pos );

		ScrollInfo.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
		if ( Enable )
		{
			ScrollInfo.nMin		= Min;
			ScrollInfo.nMax		= Max;
			ScrollInfo.nPage	= Jump;
			ScrollInfo.nPos		= Pos;
			ScrollInfo.fMask |= SIF_DISABLENOSCROLL;	//	disable if invalid/useless properties (eg. min==max)
		}
		else
		{
			//	remove scroll bar by invalidating properies and NOT setting SIF_DISABLENOSCROLL flag
			ScrollInfo.nMin		= 0;
			ScrollInfo.nMax		= 0;
			ScrollInfo.nPage	= 0;
			ScrollInfo.nPos		= 0;
			ScrollInfo.fMask &= ~SIF_DISABLENOSCROLL;
		}

		SetScrollInfo( Hwnd(), SB_HORZ, &ScrollInfo, TRUE );
	}
}

//-------------------------------------------------------------------------
//	load a resource and turn it into a brush
//-------------------------------------------------------------------------
HBRUSH GWinControl::GetBrushFromResource(int Resource)
{
	HBITMAP HBitmap = NULL;
	HBRUSH HBrush = NULL;
	
	//	didnt load from external file, try loading internal resource
	HBitmap = (HBITMAP)LoadImage( GApp::g_HInstance, MAKEINTRESOURCE(Resource), IMAGE_BITMAP, 0, 0, 0x0 );
	if ( HBitmap ) 
	{
		HBrush = CreatePatternBrush( HBitmap );
		DeleteObject( HBitmap );
		return HBrush;
	}

	return HBrush;
}
	
//-------------------------------------------------------------------------
//	setup a timer
//-------------------------------------------------------------------------
void GWinControl::StartTimer(int TimerID,int Time)
{
	if ( !SetTimer( m_Hwnd, TimerID, Time, NULL ) )
	{
		GDebug::CheckWin32Error();
	}
}

//-------------------------------------------------------------------------
//	stop a registered timer
//-------------------------------------------------------------------------
void GWinControl::StopTimer(int TimerID)
{
	if ( !KillTimer( m_Hwnd, TimerID ) )
	{
		GDebug::CheckWin32Error();
	}
}




Bool GWinControl::HandleMessage(u32 message, WPARAM wParam, LPARAM lParam, u32& Result)
{
	Result = 0;
	return FALSE;
}


int GWinControl::HandleNotifyMessage(u32 message, NMHDR* pNotifyData)
{
	switch ( message )
	{
		case NM_CLICK:		if ( OnButtonDown( GMouse::Left, int2(-1,-1) ) )	return 0;	break;
		case NM_DBLCLK:		if ( OnDoubleClick( GMouse::Left, int2(-1,-1) ) )	return 0;	break;
		case NM_RCLICK:		if ( OnButtonDown( GMouse::Right, int2(-1,-1) ) )	return 0;	break;
		case NM_RDBLCLK:	if ( OnDoubleClick( GMouse::Right, int2(-1,-1) ) )	return 0;	break;

		default:
			//GDebug::Print("Unhandled Notify Message 0x%04x\n",message);		
			break;
/*
		GWinTreeView* pTree = (GWinTreeView*)pControl;
		case TVN_BEGINLABELEDIT:
		{
			//	about to edit a label, return 0 to allow change, 1 to reject editing
			NMTVDISPINFO* pEditInfo = (NMTVDISPINFO*)pNotifyData;
			GWinTreeItem* pItem = pTree->FindItem( pEditInfo->item.hItem );
			Bool AllowEdit = pTree->AllowItemEdit( pItem, pEditInfo );
			return AllowEdit ? 0 : 1;
		}
		break;

		case TVN_ENDLABELEDIT:
		{
			//	editing label has finished. return FALSE to reject change
			NMTVDISPINFO* pEditInfo = (NMTVDISPINFO*)pNotifyData;

			//	if we have a null string, it was cancelled anyway
			if ( pEditInfo->item.pszText == NULL )
				return FALSE;

			GWinTreeItem* pItem = pTree->FindItem( pEditInfo->item.hItem );
			return pTree->FinishItemEdit( pItem, pEditInfo );
		}
		break;

		case TVN_SELCHANGED:
			pTree->Selected( (GWinTreeView*)pControl, (NMTREEVIEW*)pNotifyData );
			break;
*/
	};

	return 0;
}






LRESULT CALLBACK Win32CallBack(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	//	convert mouse wheel registered message
	if ( message == GWinControl::g_MouseWheelMsg && GWinControl::g_MouseWheelMsg != 0 )
		message = WM_MOUSEWHEEL;

	//	handle user messages (WM_USER...WM_APP...inclusive)
	if ( message >= WM_USER && message < 0xC000 && GApp::g_pApp )
	{
		GApp::g_pApp->HandleMessage( message, wParam, lParam );
		return 0;
	}

	//	get control this message is for
	GWinControl* pControl = g_WinControlList.FindControl( hwnd );
	//GDebug_Print("Callback %x. app %x control %x\n", message, GApp::g_pApp, pControl );

	//	check if control wants to handle the message
	if ( pControl )
	{
		u32 ControlHandleResult = 0;
		if ( pControl->HandleMessage( message, wParam, lParam, ControlHandleResult ) )
		{
			return ControlHandleResult;
		}
	}

	//	misc vars
	int VertScroll = -1;

	//	handle windows messages
	switch( message )
	{
		case WM_PARENTNOTIFY:
		{
			if ( pControl )
			{
				switch ( wParam )
				{
					case WM_LBUTTONDOWN:	if ( pControl->OnButtonDown(	GMouse::Left, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;
					case WM_LBUTTONUP:		if ( pControl->OnButtonUp(		GMouse::Left, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;
					case WM_LBUTTONDBLCLK:	if ( pControl->OnDoubleClick(	GMouse::Left, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;

					case WM_RBUTTONDOWN:	if ( pControl->OnButtonDown(	GMouse::Right, int2( LOWORD(lParam), HIWORD(lParam) ) )	)	return 0;	break;
					case WM_RBUTTONUP:		if ( pControl->OnButtonUp(		GMouse::Right, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;
					case WM_RBUTTONDBLCLK:	if ( pControl->OnDoubleClick(	GMouse::Right, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;

					case WM_MBUTTONDOWN:	if ( pControl->OnButtonDown(	GMouse::Middle, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;
					case WM_MBUTTONUP:		if ( pControl->OnButtonUp(		GMouse::Middle, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;
					case WM_MBUTTONDBLCLK:	if ( pControl->OnDoubleClick(	GMouse::Middle, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;
				}
			}
		}
		break;

		case WM_NOTIFY:	//	control notify message
		{
			NMHDR* pNotifyData = (NMHDR*)lParam;
			pControl = g_WinControlList.FindControl( pNotifyData->hwndFrom );
			if ( !pControl )
				return 0;
			return pControl->HandleNotifyMessage( pNotifyData->code, pNotifyData );
		}
		break;

		case WM_MOVE:	//	moved
			if ( pControl )
			{
				pControl->m_ClientPos.x = LOWORD( lParam );
				pControl->m_ClientPos.y = HIWORD( lParam );
				pControl->OnMove();
				return 0;
			}
			break;
	
		case WM_SIZE:	//	resized
			if ( pControl )
			{
				pControl->m_ClientSize.x = LOWORD( lParam );
				pControl->m_ClientSize.y = HIWORD( lParam );
				pControl->OnResize();
				pControl->UpdateScrollBars();
				return 0;
			}
			break;

		case WM_VSCROLL:	if ( VertScroll == -1 )	VertScroll = TRUE;	//	scrolled vertical
		case WM_HSCROLL:	if ( VertScroll == -1 )	VertScroll = FALSE;	//	scrolled horizontal
			if ( pControl && VertScroll != -1 )
			{
				SCROLLINFO ScrollInfo;
				ScrollInfo.cbSize = sizeof( SCROLLINFO );
				ScrollInfo.fMask	= SIF_ALL;
				GetScrollInfo( pControl->Hwnd(), VertScroll?SB_VERT:SB_HORZ, &ScrollInfo );

				//	Save the position for comparison later on
				int CurrentPos = ScrollInfo.nPos;
				int NewPos = CurrentPos;

				switch (LOWORD (wParam))
				{
					case SB_TOP:		NewPos = ScrollInfo.nMin;		break;	//	home keyboard key
					case SB_BOTTOM:		NewPos = ScrollInfo.nMax;		break;	//	end keyboard key
					case SB_LINEUP:		NewPos -= 1;					break;	//	arrow up
					case SB_LINEDOWN:	NewPos += 1;					break;	//	arrow down
					case SB_PAGEUP:		NewPos -= ScrollInfo.nPage;		break;	//	jumped up
					case SB_PAGEDOWN:	NewPos += ScrollInfo.nPage;		break;	//	jumped down
					case SB_THUMBTRACK:	NewPos = ScrollInfo.nTrackPos;	break;	//	dragged to pos
				}

				//	Set the position and then retrieve it.  Due to adjustments
				//	by Windows it may not be the same as the value set
				ScrollInfo.nPos = NewPos;
				ScrollInfo.fMask = SIF_POS;
				SetScrollInfo( pControl->Hwnd(), VertScroll?SB_VERT:SB_HORZ, &ScrollInfo, TRUE );
				GetScrollInfo( pControl->Hwnd(), VertScroll?SB_VERT:SB_HORZ, &ScrollInfo );
				//	If the position has changed, scroll window and update it
				if ( ScrollInfo.nPos != CurrentPos )
				{
					if ( VertScroll )
						pControl->OnScrollVert( ScrollInfo.nPos );
					else
						pControl->OnScrollHorz( ScrollInfo.nPos );
				}

				//	redraw control
				pControl->Refresh();

				return 0;
			}
			break;

		case WM_INITMENUPOPUP:	//	submenu popped up
			if ( pControl )
			{
				GMenuSubMenu* pSubMenu = pControl->GetSubMenu( (HMENU)wParam );
				GWinControl* pMenuControl = pControl;

				//	sub menu not in this control, check the children's menus
				if ( !pSubMenu )
				{
					pSubMenu = pControl->GetChildSubMenu( (HMENU)wParam, &pMenuControl );

					if ( !pSubMenu )
					{
						GDebug::Print("Could not find sub menu\n");
						return 1;
					}
				}

				pMenuControl->OnMenuPopup( pSubMenu );
				return 0;	//	was processed by the app
			}
			break;


		case WM_NCDESTROY:
		case WM_DESTROY:
			//	control destroyed
			break;

		case WM_QUIT:
		case WM_CLOSE:
			if ( !GApp::g_pApp )
			{
				GDebug_Print("No app to destroy\n");
			}
			else
			{
				GApp::g_pApp->QuitGame();
				return 0;
			}
			//	do DefWindProc
			break;


		case WM_CREATE:
		{
			CREATESTRUCT* pCreateStruct = (CREATESTRUCT*)lParam;
			pControl = (GWinControl*)pCreateStruct->lpCreateParams;
			GWinControl::OnWindowCreate( pControl, hwnd );
			return 0;
		}
		break;


		//	window command
		case WM_COMMAND:
		{
			Bool MenuCommand = (HIWORD(wParam) == 0);

			if ( MenuCommand )
			{
				if ( !pControl )
					return 0;
				
				//	get the menu item and the control it belongs to (if any)
				GMenuItem* pMenuItem = pControl->GetMenuItem( LOWORD(wParam) );

				//	child menu not in this control, check the children's menus
				if ( !pMenuItem )
				{
					pMenuItem = pControl->GetChildMenuItem( LOWORD(wParam), &pControl );

					if ( !pMenuItem )
					{
						GDebug::Print("Could not find menu item\n");
						return 1;
					}
				}

				pControl->OnMenuClick( pMenuItem );

				//	was processed by the app
				return 0;
			}
			else
			{
				//	not implemented any other controls yet
			}
		}
		break;
		
		case WM_MOUSEWHEEL:
		{
			//	get the wheel scroll amount
			s16 Scroll = HIWORD( wParam );

			if ( Scroll < 0 )
			{
				g_Mouse.m_WheelScrolledUp = TRUE;
				if ( pControl )	
					if ( pControl->OnButtonDown( GMouse::WheelDown, int2( LOWORD(-1), HIWORD(-1) ) ) )
						return 0;
			}
			else
			{
				g_Mouse.m_WheelScrolledDown = TRUE;
				if ( pControl )	
					if ( pControl->OnButtonDown( GMouse::WheelUp, int2( LOWORD(-1), HIWORD(-1) ) ) )
						return 0;
			}
		}
		break;


		case WM_LBUTTONDOWN:	if ( pControl )	if ( pControl->OnButtonDown(	GMouse::Left, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;
		case WM_LBUTTONUP:		if ( pControl )	if ( pControl->OnButtonUp(		GMouse::Left, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;
		case WM_LBUTTONDBLCLK:	if ( pControl )	if ( pControl->OnDoubleClick(	GMouse::Left, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;

		case WM_RBUTTONDOWN:	if ( pControl )	if ( pControl->OnButtonDown(	GMouse::Right, int2( LOWORD(lParam), HIWORD(lParam) ) )	)	return 0;	break;
		case WM_RBUTTONUP:		if ( pControl )	if ( pControl->OnButtonUp(		GMouse::Right, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;
		case WM_RBUTTONDBLCLK:	if ( pControl )	if ( pControl->OnDoubleClick(	GMouse::Right, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;

		case WM_MBUTTONDOWN:	if ( pControl )	if ( pControl->OnButtonDown(	GMouse::Middle, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;
		case WM_MBUTTONUP:		if ( pControl )	if ( pControl->OnButtonUp(		GMouse::Middle, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;
		case WM_MBUTTONDBLCLK:	if ( pControl )	if ( pControl->OnDoubleClick(	GMouse::Middle, int2( LOWORD(lParam), HIWORD(lParam) ) ) )	return 0;	break;

 
		case WM_PAINT:
			if ( pControl )
			{
				if ( pControl->OnPaint() )
				{
					return 0;
				}
			}
			break;

		case WM_ERASEBKGND:
			if ( pControl )
			{
				if ( !pControl->OnEraseBackground() )
				{
					return 0;
				}
			}
			break;

		case WM_GETMINMAXINFO:
		case WM_NCCREATE:
			//	*need* to handle these with defwndproc
			break;
	
		case WM_SHOWWINDOW:
			if ( pControl )
			{
				if ( wParam == TRUE )
				{
					if ( pControl->OnShow() )
						return 0;
				}
				else
				{
					if ( pControl->OnHide() )
						return 0;
				}
			}
			break;

		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:
			if ( pControl )
			{
				if ( pControl->OnMouseMove( int2( LOWORD(lParam), HIWORD(lParam) ), (message == WM_MOUSEMOVE) ) )
				{
					return 0;
				}
			}
			break;
	
		case WM_TIMER:
			if ( pControl )
			{
				pControl->OnTimer( wParam );
				return 0;
			}
			break;

		case WM_SETCURSOR:
		case WM_NCACTIVATE:
		case WM_GETTEXT:
		case WM_ACTIVATE:
		case WM_ACTIVATEAPP:
		case WM_KILLFOCUS:
		case WM_MOVING:
		case WM_WINDOWPOSCHANGING:
		case WM_WINDOWPOSCHANGED:
		case WM_CAPTURECHANGED:
		case WM_EXITSIZEMOVE:
		case WM_SYNCPAINT:
		case WM_NCPAINT:
		case WM_SETFOCUS:
		case WM_GETICON:
		case WM_MOUSEACTIVATE:
		case WM_SYSCOMMAND:
		case WM_ENTERSIZEMOVE:
		case WM_DROPFILES:
		case WM_NCHITTEST:
/*		case WM_NCLBUTTONDOWN:
		case WM_NCLBUTTONUP:
		case WM_NCLBUTTONDBLCLK:
		case WM_NCRBUTTONDOWN:
		case WM_NCRBUTTONUP:
		case WM_NCRBUTTONDBLCLK:
		case WM_NCMBUTTONDOWN:
		case WM_NCMBUTTONUP:
		case WM_NCMBUTTONDBLCLK:
*/
		case WM_ENTERMENULOOP:
		case WM_EXITMENULOOP:
		case WM_MENUSELECT:
		case WM_INITMENU:
		case WM_ENTERIDLE:
		case WM_CONTEXTMENU:
			//	do DefWindProc
			break;

		default:
			//GDebug_Print("Unhandled WM Message 0x%04x\n",message);		
			break;
	};


	return DefWindowProc(hwnd, message, wParam, lParam) ;
}



