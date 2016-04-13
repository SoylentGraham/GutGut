/*------------------------------------------------

  GWinTreeView Header file



-------------------------------------------------*/

#ifndef __GWINTREEVIEW__H_
#define __GWINTREEVIEW__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GWin32.h"
#include "GList.h"
#include "GWinControl.h"



//	Macros
//------------------------------------------------




//	Types
//------------------------------------------------
class GWinTreeView;
class GWinTreeItem;
class GWinImageList;


//	function pointer for recursive item calls. Function(Item,ExtraData)
typedef void (*GWinTreeItemFunc)(GWinTreeItem*,void*);




class GWinTreeItem
{
	friend GWinTreeView;

public:
	HTREEITEM				m_HItem;
	GWinTreeItem*			m_pParent;
	GList<GWinTreeItem*>	m_Children;
	int2					m_ItemValue;

private:
	GWinTreeView*			m_pOwnerTree;

public:
	GWinTreeItem();
	virtual ~GWinTreeItem();

	//	child items
	GWinTreeItem*			AddItem(GWinTreeItem* pNewItem);
	GWinTreeItem*			FindItem(HTREEITEM HItem);					//	match an item with a HItem
	GWinTreeItem*			FindItem(int MatchValue,int ValueIndex);	//	match an item by its value
	GWinTreeItem*			FindItem(int2 MatchValue);					//	match an item by both its values
	void					RemoveItem(int Index);
	void					RemoveChildren();
	void					RecurseFunc(GWinTreeItemFunc Func,void* pData=NULL);
	void					SetString(char* String);
	void					Expand();

	//	useful stuff
	inline GWinTreeView*	GetOwner();			//	returns m_pOwnerTree, but checks the pointer first
	inline void				SelectInTree();		//	makes this item the selected one in the tree
	inline int				GetIndex()			{	return !m_pParent ? -1 : m_pParent->m_Children.FindIndex( this );	};
	inline void				RemoveFromTree();	
	inline void				RemoveFromParentList();
	inline void				DeleteItem();	
	void					SetImageIndex(int Image, int SelectedImage);	//	sets images for item
	inline void				SetImageIndex(int Image)	{	SetImageIndex( Image, Image );	};
	inline void				SetImageCallBackIndex()		{	SetImageIndex(I_IMAGECALLBACK);	};	//	sets up so when image is requested the window gets a message to set

};






class GWinTreeView : public GWinControl
{
public:
	GList<GWinTreeItem*>	m_RootItems;

public:
	GWinTreeView();
	~GWinTreeView();

	//	Tree control
	void				Empty();		//	remove all items
	GWinTreeItem*		AddRootItem(GWinTreeItem* pItem);
	void				RecurseFunc(GWinTreeItemFunc Func,void* pData=NULL);

	GWinTreeItem*		FindItem(HTREEITEM HItem);					//	match an item with a HItem
	GWinTreeItem*		FindItem(int MatchValue,int ValueIndex);	//	match an item by its value
	GWinTreeItem*		FindItem(int2 MatchValue);					//	match an item by both its values

	//	win32 control
	virtual void		Destroy();
	void				SelectItem(GWinTreeItem* pItem);
	GWinTreeItem*		SelectedItem()						{	return FindItem( TreeView_GetSelection( m_Hwnd ) );	};
	void				RemoveFromTree(GWinTreeItem* pItem);	//	removes item from the win32 tree
	virtual int			HandleNotifyMessage(u32 message, NMHDR* pNotifyData);	//	message has been sent to this control matching the hwnd
	virtual const char*	ClassName()							{	return WC_TREEVIEW;	};	//	what class this window creates
	virtual u32			AdditionalStyleFlags()				{	return GWinControlFlags::TreeView_StyleFlags;	};		//	extra style flags for this particular type of control
	virtual u32			DefaultFlags()						{	return GWinControlFlags::TreeView_StyleFlags | GWinControlFlags::Visible | GWinControlFlags::Child;	};		//	extra style flags for this particular type of control
	Bool				SetImageList(GWinImageList* pImageList);	//	pass NULL to unset image list
	virtual Bool		OnButtonDown(int MouseButton, int2 Pos);	//	control got a right click notify message. return TRUE if we handle it

	//	win32 tree callbacks
	virtual void		Selected(NMTREEVIEW* pNotifyData)				{	};
	virtual void		RightClicked(GWinTreeItem* pItem, int2 Pos)							{	};	//	pos is in client space
	virtual Bool		AllowItemEdit(GWinTreeItem* pItem, NMTVDISPINFO* pEditInfo )		{	return TRUE;	}
	virtual Bool		FinishItemEdit(GWinTreeItem* pItem, NMTVDISPINFO* pEditInfo )		{	return TRUE;	}
	
};





//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------
inline GWinTreeView* GWinTreeItem::GetOwner()
{
	//	check for the owner variable that should always be present
	if ( ! m_pOwnerTree )
	{
		GDebug::Break("WinTree item doesnt have an owner set!\n");
	}
	return m_pOwnerTree;
}

	
inline void	GWinTreeItem::SelectInTree()		
{	
	GetOwner()->SelectItem(this);	
}


inline void	GWinTreeItem::RemoveFromTree()	
{
	m_pOwnerTree->RemoveFromTree(this);	
}


inline void	GWinTreeItem::RemoveFromParentList()
{
	int Index;
	if ( m_pParent )
	{
		Index = m_pParent->m_Children.FindIndex(this);
		m_pParent->m_Children.RemoveAt(Index);
	}
	else if ( m_pOwnerTree )
	{
		Index = m_pOwnerTree->m_RootItems.FindIndex(this);
		m_pOwnerTree->m_RootItems.RemoveAt(Index);
	}		
}

inline void	GWinTreeItem::DeleteItem()
{
	RemoveFromTree();
	RemoveFromParentList();
	delete this;
}



#endif

