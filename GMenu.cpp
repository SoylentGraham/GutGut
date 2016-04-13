/*------------------------------------------------

  GMenu.cpp

	Windows menu handling classes


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GMenu.h"
#include "GDebug.h"
#include "GApp.h"


//	globals
//------------------------------------------------
u16 GMenu::g_MenuCount = 0;



//	Definitions
//------------------------------------------------



GMenu::GMenu()
{
	//	todo: make this a bit more variable
	m_pOwnerApp = NULL;
	m_pRootMenu = NULL;
}



GMenu::~GMenu()
{
	Shutdown();
}



void GMenu::Init(GApp* pApp)
{
	m_pOwnerApp = pApp;

	if ( Initalised() )
	{
		GDebug_Break("Menu already initialised\n");
		return;
	}

	m_pRootMenu = new GMenuSubMenu;
	m_pRootMenu->SetParentMenu(this,TRUE);
}


void GMenu::Shutdown()
{
	if ( m_pRootMenu )
	{
		delete m_pRootMenu;
		m_pRootMenu = NULL;
	}
}



void GMenu::NullPointers(GMenuItem* pItem)
{
	if ( m_pRootMenu )
		m_pRootMenu->NullPointers( pItem );
}


void GMenu::NullPointers(GMenuSubMenu* pSubmenu)
{
	if ( m_pRootMenu == pSubmenu )
	{
		GDebug::Print("Warning: nulling root menu pointer\n");
		m_pRootMenu = NULL;
	}

	if ( m_pRootMenu )
		m_pRootMenu->NullPointers( pSubmenu );
}

void GMenu::Refresh()
{
	//	refreshes the menu onscreen
	if ( m_pOwnerApp->Window() )
		DrawMenuBar( m_pOwnerApp->Hwnd() );
}


	
GMenuSubMenu* GMenu::GetSubMenu(HMENU handle)
{
	if ( m_pRootMenu )
	{
		return m_pRootMenu->GetSubMenu( handle );
	}
	return NULL;
}


GMenuItem* GMenu::GetMenuItem(u16 MenuID)
{
	if ( m_pRootMenu )
	{
		return m_pRootMenu->GetMenuItem( MenuID );
	}
	return NULL;
}

GMenuSubMenu* GMenu::GetSubMenuWithValue0( int Value0 )
{
	if ( m_pRootMenu )
	{
		return m_pRootMenu->GetSubMenuWithValue0( Value0 );
	}
	return NULL;
}


GMenuItem* GMenu::GetMenuItemWithValue0( int Value0 )
{
	if ( m_pRootMenu )
	{
		return m_pRootMenu->GetMenuItemWithValue0( Value0 );
	}
	return NULL;
}
















GMenuSubMenu::GMenuSubMenu()
{
	m_MenuValue[0]		= 0;
	m_MenuValue[1]		= 0;

	m_pParentMenu		= NULL;
	m_pParentItem		= NULL;
	
	//	win menu vars
	m_HMenu				= 0;


}


GMenuSubMenu::~GMenuSubMenu()
{
	RemoveAll();
}


void GMenuSubMenu::SetParentMenu(GMenu* pMenu,Bool RootMenu)
{
	if ( m_HMenu != NULL )
	{
		GDebug_Break("This menu has already been initialised\n");
	}

	m_pParentMenu = pMenu;

	//	if NULL passed, create a popup menu
	if ( RootMenu )
	{
		m_HMenu = CreateMenu();
		HWND WindowHandle = pMenu->m_pOwnerApp->Hwnd();
		if ( WindowHandle == NULL )
		{
			GDebug_Break("Window handle is NULL when assigning a menu to a window\n");
		}		
		SetMenu( WindowHandle, m_HMenu );
	}
	else
	{
		m_HMenu = CreatePopupMenu();
	}
	
}


void GMenuSubMenu::Remove(int Index)
{
	if ( Index < 0 || Index >= m_Items.Size() )
	{
		GDebug_Break("Index out of bounds\n");
		return;
	}

	delete m_Items.ElementAt(Index);	
	m_Items.ElementAt(Index) = NULL;
	m_Items.RemoveAt(Index);	
	
}



GMenuItem* GMenuSubMenu::Add(const char* pString)
{
	GMenuItem* pNewItem = new GMenuItem;
	pNewItem->m_pString = pString;
	pNewItem->m_pParent = this;
	pNewItem->m_pParentMenu = m_pParentMenu;
	int NewIndex = m_Items.Add( pNewItem );

	
	MENUITEMINFO NewItemInfo;
	pNewItem->SetupNewMenuItemInfo( &NewItemInfo, pString );

	InsertMenuItem(	m_HMenu, NewIndex, TRUE, &NewItemInfo );


	return pNewItem;
}



void GMenuSubMenu::RemoveAll()
{
	for ( int i=0;	i<m_Items.Size();	i++ )
	{
		//	remove from windows menu menu
		RemoveMenu( m_HMenu, m_Items.ElementAt(i)->m_MenuID, MF_BYCOMMAND );

		//	delete item
		delete m_Items.ElementAt(i);
		m_Items.ElementAt(i) = NULL;
	}
	m_Items.Empty();

	if ( m_pParentMenu )
		m_pParentMenu->Refresh();
}

	

void GMenuSubMenu::NullPointers(GMenuItem* pItem)
{
	//	null members
	if ( pItem == m_pParentItem )
		m_pParentItem = NULL;

	//	null items
	int index = m_Items.FindIndex( pItem );
	if ( index != -1 )
	{
		//	dont delete!
		m_Items.ElementAt( index ) = NULL;
		m_Items.RemoveAt( index );
	}

	//	recurse
	for ( int i=0;	i<m_Items.Size();	i++ )
	{
		m_Items.ElementAt(i)->NullPointers( pItem );
	}
}


void GMenuSubMenu::NullPointers(GMenuSubMenu* pSubmenu)
{
	//	recurse
	for ( int i=0;	i<m_Items.Size();	i++ )
	{
		m_Items.ElementAt(i)->NullPointers( pSubmenu );
	}
}


void GMenuSubMenu::AddDivider()							
{
	GMenuItem* pNewItem = Add("-");	
	pNewItem->m_Flags |= GMenuItemFlags::Divider;	
	pNewItem->ApplyMenuFlags();
}


GMenuSubMenu* GMenuSubMenu::GetSubMenu(HMENU handle)
{
	//	found what we're looking for
	if ( m_HMenu == handle )
		return this;

	//	search submenu items
	for ( int i=0;	i<m_Items.Size();	i++ )
	{
		GMenuSubMenu* pMenu = m_Items[i]->GetSubMenu( handle );
		if ( pMenu )
			return pMenu;
	}

	//	nothing found
	return NULL;
}


GMenuItem* GMenuSubMenu::GetMenuItem(u16 MenuID)
{
	//	search submenu items
	for ( int i=0;	i<m_Items.Size();	i++ )
	{
		GMenuItem* pMenu = m_Items[i]->GetMenuItem( MenuID );
		if ( pMenu )
			return pMenu;
	}

	//	nothing found
	return NULL;
}



void GMenuSubMenu::Popup(int2 Pos)
{
	RECT rct = { 0,0,10,10 };
	HWND hwnd = GApp::g_pApp->Hwnd();
	TrackPopupMenu( m_HMenu, 0, Pos.x, Pos.y, 0, hwnd, &rct );	
}



GMenuItem* GMenuSubMenu::GetMenuItemWithValue0( int Value0 )
{
	for ( int i=0;	i<m_Items.Size();	i++ )
	{
		GMenuItem* pItem = m_Items[i]->GetMenuItemWithValue0( Value0 );
		if ( pItem )
			return pItem;
	}

	return NULL;
}


GMenuSubMenu* GMenuSubMenu::GetSubMenuWithValue0( int Value0 )
{
	if ( m_MenuValue[0] == Value0 )
		return this;

	for ( int i=0;	i<m_Items.Size();	i++ )
	{
		GMenuSubMenu* pMenu = m_Items[i]->GetSubMenuWithValue0( Value0 );
		if ( pMenu )
			return pMenu;
	}

	return NULL;
}


void GMenuSubMenu::Destroy()
{
	//	destroy children
	RemoveAll();

	//	 delete menu
	if ( m_HMenu )
	{
		DestroyMenu( m_HMenu );
	}
}


Bool GMenuSubMenu::ContainsMenuItem(GMenuItem* pItem)
{
	for ( int i=0;	i<m_Items.Size();	i++ )
	{
		if ( m_Items[i]->ContainsMenuItem( pItem ) )
			return TRUE;
	}

	return FALSE;
}

Bool GMenuSubMenu::ContainsSubMenu(GMenuSubMenu* pMenu)
{
	if ( this == pMenu )
		return TRUE;
	
	for ( int i=0;	i<m_Items.Size();	i++ )
	{
		if ( m_Items[i]->ContainsSubMenu( pMenu ) )
			return TRUE;
	}

	return FALSE;
}






















GMenuItem::GMenuItem()
{
	m_MenuValue[0]		= 0;
	m_MenuValue[1]		= 0;

	m_pParentMenu		= NULL;
	m_pParent			= NULL;
	m_pSubMenu			= NULL;
	m_pString			= NULL;
	m_MenuID			= GMenu::g_MenuCount++;

	m_Flags				= 0x0;
}



GMenuItem::~GMenuItem()
{
	DeleteSubMenu();
}


void GMenuItem::DeleteSubMenu()
{
	if ( m_pSubMenu )
	{
		delete m_pSubMenu;
		m_pSubMenu = NULL;
	}
}


GMenuSubMenu* GMenuItem::AddSubMenu()
{
	if ( m_pSubMenu )
	{
		GDebug_Break("Submenu already assigned\n");
		return NULL;
	}

	m_pSubMenu = new GMenuSubMenu;
	m_pSubMenu->m_pParentItem = this;
	m_pSubMenu->SetParentMenu(m_pParentMenu,FALSE);

	
	//	set menu item info
	MENUITEMINFO ItemInfo;
	memset( &ItemInfo, 0, sizeof( MENUITEMINFO ) );
	ItemInfo.cbSize = sizeof( MENUITEMINFO );

	//	update sub menu
	ItemInfo.fMask = MIIM_SUBMENU;

	//	MIIM_SUBMENU
	ItemInfo.hSubMenu = m_pSubMenu->m_HMenu;

	ApplyMenuItemSettings( &ItemInfo );

	return m_pSubMenu;
}


void GMenuItem::ApplyMenuItemSettings( MENUITEMINFO* ItemInfo )
{
	int ItemIndex = Index();

	if ( ItemIndex<0 )
	{
		GDebug_Break("this item isnt in the parent list");
		return;
	}

	//	set this menu's info
	int result = SetMenuItemInfo( m_pParent->m_HMenu, (u16)ItemIndex, TRUE, ItemInfo );

	//	error
	if ( result == 0 )
	{
		GDebug::CheckWin32Error();
	}

	if ( m_pParentMenu )
		m_pParentMenu->Refresh();
}




void GMenuItem::SetupNewMenuItemInfo( MENUITEMINFO* ItemInfo, const char* pString )
{
	memset( ItemInfo, 0, sizeof( MENUITEMINFO ) );
	ItemInfo->cbSize = sizeof( MENUITEMINFO );

	ItemInfo->fMask = MIIM_ID | MIIM_STATE | MIIM_TYPE;


	//	MIIM_TYPE
	if ( pString )
	{
		ItemInfo->fType |= MFT_STRING ;
		m_pString = pString;	//	update string
	}

	//	MIIM_STATE
	ItemInfo->fState = MFS_ENABLED;	//	MFS_DEFAULT

	//	MIIM_ID
	ItemInfo->wID = m_MenuID;

	//	if MIIM_TYPE && fType = MFT_STRING
	if ( pString )
	{
		ItemInfo->cch = strlen( m_pString );
		ItemInfo->dwTypeData = (char*)m_pString;
	}

}



void GMenuItem::GetMenuItemSettings( MENUITEMINFO* ItemInfo, char* pBuffer, u32 BufferSize )
{
	int ItemIndex = Index();

	if ( ItemIndex<0 )
	{
		GDebug_Break("this item isnt in the parent list");
		return;
	}

	//	clear out data
	memset( ItemInfo, 0, sizeof( MENUITEMINFO ) );
	ItemInfo->cbSize = sizeof( MENUITEMINFO );

	//	set the struct to everything so we retrieve all details
	ItemInfo->fMask = MIIM_ID | MIIM_STATE | MIIM_TYPE ;

	//	set data for text retrieval
	ItemInfo->cch = BufferSize;
	ItemInfo->dwTypeData = pBuffer;

	//	get the data
	GetMenuItemInfo( m_pParent->m_HMenu, (u16)ItemIndex, TRUE, ItemInfo );

}





void GMenuItem::ApplyMenuFlags()
{
	unsigned int State = 0x0;
	if ( m_Flags & GMenuItemFlags::Bold )		State |= MFS_DEFAULT;
	if ( m_Flags & GMenuItemFlags::Checked )	State |= MFS_CHECKED;	else	State |= MFS_UNCHECKED;
	if ( m_Flags & GMenuItemFlags::Disabled )	State |= MFS_DISABLED;	else	State |= MFS_ENABLED;
	if ( m_Flags & GMenuItemFlags::Greyed )		State |= MFS_GRAYED;

	unsigned int Type = 0x0;
	
	//	"The MFT_BITMAP, MFT_SEPARATOR, and MFT_STRING values cannot be combined with one another"
	Type = MFT_STRING;
	if ( m_Flags & GMenuItemFlags::Divider )	Type |= MFT_MENUBREAK;
	if ( m_Flags & GMenuItemFlags::RightAlign )	Type |= MFT_RIGHTORDER;
	

	//	set menu item info
	MENUITEMINFO ItemInfo;
	memset( &ItemInfo, 0, sizeof( MENUITEMINFO ) );
	ItemInfo.cbSize = sizeof( MENUITEMINFO );

	//	set menu state and type
	ItemInfo.fMask = MIIM_STATE;

	if ( Type != 0x0 )
		ItemInfo.fMask = MIIM_TYPE;

	//	MIIM_STATE
	ItemInfo.fState = State;

	//	MIIM_TYPE
	ItemInfo.fType = Type;

	ApplyMenuItemSettings( &ItemInfo );
}



Bool GMenuItem::GetMenuStateChecked()
{
	//	temp text buffer
	char Text[255] = { 0 };
	MENUITEMINFO ItemInfo;

	//	get data
	GetMenuItemSettings( &ItemInfo, &Text[0], 255 );

	//	check the state
	return ( ItemInfo.fState & MFS_CHECKED );		
}




void GMenuItem::NullPointers(GMenuItem* pItem)
{
	//	recurse
	if ( m_pSubMenu )
	{
		m_pSubMenu->NullPointers(pItem);
	}
}


void GMenuItem::NullPointers(GMenuSubMenu* pSubMenu)
{
	//	null members
	if ( m_pParent == pSubMenu )
		 m_pParent = NULL;
		
	if ( m_pSubMenu == pSubMenu )
		 m_pSubMenu = NULL;

	//	recurse
	if ( m_pSubMenu )
	{
		m_pSubMenu->NullPointers(pSubMenu);
	}
}


GMenuSubMenu* GMenuItem::GetSubMenu(HMENU handle)
{
	//	search submenu
	if ( m_pSubMenu )
	{
		return m_pSubMenu->GetSubMenu( handle );
	}

	//	nothing more to search
	return NULL;
}


GMenuItem* GMenuItem::GetMenuItem(u16 MenuID)
{
	//	found what we're looking for
	if ( m_MenuID == MenuID )
		return this;

	//	search submenu
	if ( m_pSubMenu )
	{
		return m_pSubMenu->GetMenuItem( MenuID );
	}

	//	nothing more to search
	return NULL;
}




GMenuSubMenu* GMenuItem::GetSubMenuWithValue0( int Value0 )
{
	if ( m_pSubMenu )
		return m_pSubMenu->GetSubMenuWithValue0( Value0 );

	return NULL;
}

GMenuItem* GMenuItem::GetMenuItemWithValue0( int Value0 )
{
	if ( m_MenuValue[0] == Value0 )
		return this;

	if ( m_pSubMenu )
		return m_pSubMenu->GetMenuItemWithValue0( Value0 );

	return NULL;
}

Bool GMenuItem::ContainsMenuItem(GMenuItem* pItem)
{
	if ( this == pItem )
		return TRUE;

	if ( m_pSubMenu )
		return m_pSubMenu->ContainsMenuItem( pItem );

	return FALSE;
}

Bool GMenuItem::ContainsSubMenu(GMenuSubMenu* pMenu)
{
	if ( m_pSubMenu )
		return m_pSubMenu->ContainsSubMenu( pMenu );

	return FALSE;
}
