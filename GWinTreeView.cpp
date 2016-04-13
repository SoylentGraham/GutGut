/*------------------------------------------------

  GWinTreeView.cpp

	Win32 Tree-view control
	http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/commctls/treeview/treeview.asp

-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GWinTreeView.h"
#include "GWinImageList.h"
#include "GDebug.h"
#include "GMouse.h"

//	globals
//------------------------------------------------



//	Definitions
//------------------------------------------------


GWinTreeItem::GWinTreeItem()
{
	m_HItem			= NULL;
	m_pParent		= NULL;
	m_pOwnerTree	= NULL;
	m_ItemValue[0]	= 0;
	m_ItemValue[1]	= 0;
}


GWinTreeItem::~GWinTreeItem()
{
	//	delete children
	RemoveChildren();

	//	make sure this has been removed from the win32 tree
	if ( m_HItem != NULL )
	{
		GDebug_Break("Item has not been removed from win32 tree\n");
	}

};



GWinTreeItem* GWinTreeItem::AddItem(GWinTreeItem* pNewItem)
{
	//	setup new item
	pNewItem->m_pOwnerTree = m_pOwnerTree;
	pNewItem->m_pParent = this;
	
	//	add to child list
	m_Children.Add( pNewItem );
	
    
	TVITEM tvi; 
	TVINSERTSTRUCT tvins; 

    //tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM; 
	tvi.mask = TVIF_TEXT | TVIF_PARAM; 

	//	set text
    tvi.pszText = "GWinTreeItem";
    tvi.cchTextMax = 0;	//	buffer size of text, not used for setting up an item

	//	set these if we're setting the images
	//tvi.iImage = 0; 
	//tvi.iSelectedImage = 0; 

    //	set application specific id
    tvi.lParam = (LPARAM)0;
    tvins.item = tvi;
	
	//	set previous item handle
	if ( m_Children.Size() == 0 )
		tvins.hInsertAfter = TVI_FIRST;	//	previous item in list
	else
		tvins.hInsertAfter = m_Children.ElementLast()->m_HItem;

	//	set parent item
	tvins.hParent = this->m_HItem;

	//	add to tree control
	pNewItem->m_HItem = TreeView_InsertItem( m_pOwnerTree->m_Hwnd, &tvins );

	if ( pNewItem->m_HItem == NULL )
	{
		GDebug::CheckWin32Error();
	}

/*    // The new item is a child item. Give the parent item a 
    // closed folder bitmap to indicate it now has child items. 
    if (nLevel > 1)
    { 
        hti = TreeView_GetParent(hwndTV, hPrev); 
        tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE; 
        tvi.hItem = hti; 
        tvi.iImage = g_nClosed; 
        tvi.iSelectedImage = g_nClosed; 
        TreeView_SetItem(hwndTV, &tvi); 
    } 
*/

	return pNewItem;
}




GWinTreeItem* GWinTreeItem::FindItem(HTREEITEM HItem)
{
	//	is this the item we want?
	if ( this->m_HItem == HItem )
	{
		return this;
	}

	//	search children for the item
	GWinTreeItem* pItem = NULL;
	for ( int i=0;	i<m_Children.Size();	i++ )
	{
		if ( !m_Children[i] )
			continue;

		pItem = m_Children[i]->FindItem(HItem);
		if ( pItem )
			break;
	}

	return pItem;
}


GWinTreeItem* GWinTreeItem::FindItem(int MatchValue,int ValueIndex)
{
	//	is this the item we want?
	if ( this->m_ItemValue[ValueIndex] == MatchValue )
	{
		return this;
	}

	//	search children for the item
	GWinTreeItem* pItem = NULL;
	for ( int i=0;	i<m_Children.Size();	i++ )
	{
		if ( !m_Children[i] )
			continue;

		pItem = m_Children[i]->FindItem(MatchValue,ValueIndex);
		if ( pItem )
			break;
	}

	return pItem;
}


GWinTreeItem* GWinTreeItem::FindItem(int2 MatchValue)
{
	//	is this the item we want?
	if ( this->m_ItemValue == MatchValue )
	{
		return this;
	}

	//	search children for the item
	GWinTreeItem* pItem = NULL;
	for ( int i=0;	i<m_Children.Size();	i++ )
	{
		if ( !m_Children[i] )
			continue;

		pItem = m_Children[i]->FindItem( MatchValue );
		if ( pItem )
			break;
	}

	return pItem;
}


//-------------------------------------------------------------------------
//	remove a child by index
//-------------------------------------------------------------------------
void GWinTreeItem::RemoveItem(int Index)
{
	GDebug_CheckIndex( Index, 0, m_Children.Size() );

	//	remove from win32 tree, if we havent already removed ourself
	if ( m_HItem != NULL )
	{
		m_Children[Index]->RemoveFromTree();
	}
	else
	{
		//	just NULL anyway, so we dont try to remove it
		if ( m_Children[Index]->m_HItem != NULL )
			m_Children[Index]->m_HItem = NULL;
	}

	//	delete item
	GDelete( m_Children[Index] );

	//	remove from children list
	m_Children.RemoveAt( Index );

}


void GWinTreeItem::RemoveChildren()
{
	while ( m_Children.Size() > 0 )
	{
		RemoveItem( 0 );
	}

	m_Children.Empty();
}


void GWinTreeItem::RecurseFunc(GWinTreeItemFunc Func,void* pData)
{
	//	call for this item
	Func( this, pData );

	//	recurse func to children
	for ( int i=0;	i<m_Children.Size();	i++ )
	{
		m_Children[i]->RecurseFunc( Func, pData );
	}

}

	
void GWinTreeItem::SetString(char* String)
{
	TV_ITEM ItemInfo;
	ItemInfo.pszText = String;
	ItemInfo.hItem = m_HItem;
	ItemInfo.mask = TVIF_TEXT;
    
	/*
	UINT state;
    UINT stateMask;
    int cchTextMax;
    int iImage;
    int iSelectedImage;
    int cChildren;
    LPARAM lParam;
    int iIntegral;
	*/

	if ( ! TreeView_SetItem( m_pOwnerTree->m_Hwnd, &ItemInfo ) )
	{
		GDebug::CheckWin32Error();
	}
	
}


void GWinTreeItem::Expand()
{
	//	MSDN note:
	//	Expanding a node that is already expanded, or collapsing a node that is already collapsed is considered a successful operation and TVM_EXPAND will return a non-zero value. 
	//	Attempting to expand or collapse a node that has no children is considered a failure and TVM_EXPAND will return zero.
	if ( m_Children.Size() == 0 )
		return;

	//	expand the tree from this item
	if ( ! TreeView_Expand( m_pOwnerTree->m_Hwnd, m_HItem, TVE_EXPAND ) )
	{
		GDebug::CheckWin32Error();
	}
}

	

void GWinTreeItem::SetImageIndex(int Image, int SelectedImage)
{
	TV_ITEM ItemInfo;
	ItemInfo.hItem = m_HItem;
	ItemInfo.iImage = Image;
    ItemInfo.iSelectedImage = SelectedImage;
	ItemInfo.mask = TVIF_IMAGE|TVIF_SELECTEDIMAGE;

	if ( ! TreeView_SetItem( m_pOwnerTree->m_Hwnd, &ItemInfo ) )
	{
		GDebug::CheckWin32Error();
	}
}




//------------------------------------------------



GWinTreeView::GWinTreeView()
{
	m_Hwnd		= NULL;
	m_ControlID	= 0;

}


GWinTreeView::~GWinTreeView()
{
	Destroy();
}



void GWinTreeView::Empty()
{
	int i;

	//	remove items from windows
	for ( i=0;	i<m_RootItems.Size();	i++ )
	{
		//	remove from tree
		m_RootItems[i]->RemoveFromTree();
	}

	//	delete items
	for ( i=0;	i<m_RootItems.Size();	i++ )
	{
		GDelete( m_RootItems[i] );
	}

	m_RootItems.Empty();
}


/*
Bool GWinTreeView::Create(HWND OwnerHwnd, HINSTANCE Instance, int ControlID, int4 Dimensions)
{
	m_ControlID		= ControlID;

	//	create control
	u32 ExStyle		= 0x0;
	u32 Style		= WS_VISIBLE | WS_CHILD | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_EDITLABELS | TVS_DISABLEDRAGDROP;
	HWND TreeHwnd	= NULL;
	HMENU hMenu		= (HMENU)ControlID;
	HINSTANCE hInstance = Instance;

	m_Hwnd = CreateWindowEx( ExStyle, WC_TREEVIEW, "Tree", Style, Dimensions.x, Dimensions.y, Dimensions[2], Dimensions[3], OwnerHwnd, hMenu, hInstance, NULL );

	if ( m_Hwnd == NULL )
	{
		GDebug::CheckWin32Error();
		return FALSE;
	}
	
	return TRUE;
}	
*/


void GWinTreeView::Destroy()
{
	//	remove items
	Empty();

	//	destroy contrl
	GWinControl::Destroy();
}





GWinTreeItem* GWinTreeView::AddRootItem(GWinTreeItem* pItem)
{
	//	create and setup new item
	pItem->m_pParent = NULL;
	pItem->m_pOwnerTree = this;
	m_RootItems.Add( pItem );
    
	TVITEM tvi; 
	TVINSERTSTRUCT tvins; 

    //tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM; 
	tvi.mask = TVIF_TEXT | TVIF_PARAM; 

	//	set text
    tvi.pszText = "GWinTreeItem";
    tvi.cchTextMax = 0;	//	buffer size of text, not used for setting up an item

	//	set these if we're setting the images
	//tvi.iImage = 0; 
	//tvi.iSelectedImage = 0; 

    //	set application specific id
    tvi.lParam = (LPARAM)0;
    tvins.item = tvi;
	
	//	set previous item handle
	if ( m_RootItems.Size() == 0 )
		tvins.hInsertAfter = TVI_FIRST;	//	previous item in list
	else
		m_RootItems.ElementLast()->m_HItem;

	//	set parent item
	tvins.hParent = TVI_ROOT;
	//tvins.hParent = item.HTREEITEM	//	parent item

	//	add to tree control
	pItem->m_HItem = TreeView_InsertItem( m_Hwnd, &tvins );

	if ( pItem->m_HItem == NULL )
	{
		GDebug::CheckWin32Error();
	}

/*    // The new item is a child item. Give the parent item a 
    // closed folder bitmap to indicate it now has child items. 
    if (nLevel > 1)
    { 
        hti = TreeView_GetParent(hwndTV, hPrev); 
        tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE; 
        tvi.hItem = hti; 
        tvi.iImage = g_nClosed; 
        tvi.iSelectedImage = g_nClosed; 
        TreeView_SetItem(hwndTV, &tvi); 
    } 
*/
	return pItem;
} 




GWinTreeItem* GWinTreeView::FindItem(HTREEITEM HItem)
{
	GWinTreeItem* pItem = NULL;

	//	find the item from the root items
	for ( int i=0;	i<m_RootItems.Size();	i++ )
	{
		if ( !m_RootItems[i] )
			continue;

		pItem = m_RootItems[i]->FindItem(HItem);
		if ( pItem )
			break;
	}

	return pItem;
}




GWinTreeItem* GWinTreeView::FindItem(int MatchValue,int ValueIndex)
{
	GWinTreeItem* pItem = NULL;

	//	find the item from the root items
	for ( int i=0;	i<m_RootItems.Size();	i++ )
	{
		if ( !m_RootItems[i] )
			continue;

		pItem = m_RootItems[i]->FindItem(MatchValue,ValueIndex);
		if ( pItem )
			break;
	}

	return pItem;
}

	
	
GWinTreeItem* GWinTreeView::FindItem(int2 MatchValue)
{
	GWinTreeItem* pItem = NULL;

	//	find the item from the root items
	for ( int i=0;	i<m_RootItems.Size();	i++ )
	{
		if ( !m_RootItems[i] )
			continue;

		pItem = m_RootItems[i]->FindItem( MatchValue );
		if ( pItem )
			break;
	}

	return pItem;
}






void GWinTreeView::SelectItem(GWinTreeItem* pItem)
{
	if ( !pItem )
	{
		GDebug_Break("Missing item pointer\n");
		return;
	}

	/*	
	flags:
		TVGN_CARET			Sets the selection to the given item. The control's parent window receives the TVN_SELCHANGING and TVN_SELCHANGED notification messages.
		TVGN_DROPHILITE		Redraws the given item in the style used to indicate the target of a drag-and-drop operation.
		TVGN_FIRSTVISIBLE	Ensures that the specified item is visible, and, if possible, displays it at the top of the control's window. Tree-view controls display as many items as will fit in the window. If the specified item is near the bottom of the control's hierarchy of items, it might not become the first visible item, depending on how many items fit in the window.
	*/

	WPARAM Flags = 0x0;
	Flags |= TVGN_CARET;
	//Flags |= TVGN_FIRSTVISIBLE;	//	makes windows think the HItem is invalid!?

	if ( ! TreeView_Select( m_Hwnd, pItem->m_HItem, Flags ) )
	{
		GDebug::CheckWin32Error();
	}

}





Bool GWinTreeView::OnButtonDown(int MouseButton, int2 Pos)	//	control got a right click notify message. return TRUE if we handle it
{
	if ( MouseButton == GMouse::Right )
	{
		//	find the item under the mouse
		POINT p;
		GetCursorPos(&p);
		ScreenToClient( m_Hwnd, &p );

		//	info about where we're looking for the item
		TVHITTESTINFO TestInfo;
		TestInfo.hItem = NULL;	//	unused
		TestInfo.pt.x = p.x;
		TestInfo.pt.y = p.y;
		TestInfo.flags = TVHT_ONITEM|TVHT_ONITEMBUTTON|TVHT_ONITEMICON|TVHT_ONITEMINDENT|TVHT_ONITEMLABEL|TVHT_ONITEMSTATEICON;

		//	find the item
		HTREEITEM Item = TreeView_HitTest( m_Hwnd, &TestInfo );
		
		if ( Item == NULL )
		{
			//	no item under mouse
			return FALSE;
		}

		//	get the item we right clicked on
		GWinTreeItem* pItem = FindItem( Item );

		if ( ! pItem )
		{
			return FALSE;
		}

		int2 Pos( p.x, p.y );
		RightClicked( pItem, Pos );

		return TRUE;
	}

	//	unhandled
	return FALSE;
}





void GWinTreeView::RecurseFunc(GWinTreeItemFunc Func,void* pData)
{
	//	for all root items, call this func
	for ( int i=0;	i<m_RootItems.Size();	i++ )
	{
		if ( !m_RootItems[i] )
			continue;

		m_RootItems[i]->RecurseFunc( Func, pData );
	}

}



	
void GWinTreeView::RemoveFromTree(GWinTreeItem* pItem)
{
	//	remove from win32 tree
	if ( pItem->m_HItem != NULL )
	{
		if ( ! TreeView_DeleteItem( m_Hwnd, pItem->m_HItem ) )
		{
			GDebug::CheckWin32Error();
		}
		pItem->m_HItem = NULL;
	}

	//	NULL all children's HItem, as all the children's HItems would have been destroyed
	for ( int i=0;	i<pItem->m_Children.Size();	i++ )
	{
		pItem->m_Children[i]->m_HItem = NULL;
	}
}


	
int GWinTreeView::HandleNotifyMessage(u32 message, NMHDR* pNotifyData)
{
	switch ( message )
	{
		case TVN_BEGINLABELEDIT:
		{
			//	about to edit a label, return 0 to allow change, 1 to reject editing
			NMTVDISPINFO* pEditInfo = (NMTVDISPINFO*)pNotifyData;
			GWinTreeItem* pItem = FindItem( pEditInfo->item.hItem );
			Bool AllowEdit = AllowItemEdit( pItem, pEditInfo );
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

			GWinTreeItem* pItem = FindItem( pEditInfo->item.hItem );
			return FinishItemEdit( pItem, pEditInfo );
		}
		break;
	
		case TVN_SELCHANGING:
			//	Returns TRUE to prevent the selection from changing
			return FALSE;
			break;

		case TVN_SELCHANGED:
			Selected( (NMTREEVIEW*)pNotifyData );
			break;
	};
	
	return GWinControl::HandleNotifyMessage( message, pNotifyData );
}



Bool GWinTreeView::SetImageList(GWinImageList* pImageList)
{
	//	if NULL, unassign image list
	if ( !pImageList )
	{
		TreeView_SetImageList( m_Hwnd, NULL, TVSIL_NORMAL );
	}
	else
	{
		//	returns handle to prev iamge list
		TreeView_SetImageList( m_Hwnd, pImageList->Handle(), TVSIL_NORMAL );
	}

	return TRUE;
}


