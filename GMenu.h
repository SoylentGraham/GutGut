/*------------------------------------------------

  GMenu Header file



-------------------------------------------------*/

#ifndef __GMENU__H_
#define __GMENU__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GList.h"
//#include "GApp.h"



//	Macros
//------------------------------------------------
namespace GMenuItemFlags
{
	const u32	Divider		= 1<<0;	//	makes this a divider
	const u32	Disabled	= 1<<1;	//	disabled item
	const u32	Greyed		= 1<<2;	//	greyed, but not disabled?
	const u32	Bold		= 1<<3;	//	bold text
	const u32	Checked		= 1<<4;	//	checked menu item
	const u32	RightAlign	= 1<<5;	//	text aligned to the right
};

	


//	Types
//------------------------------------------------

class GMenuItem;
class GMenuSubMenu;
class GMenu;
class GApp;





class GMenuSubMenu
{
	friend GMenuItem;
	friend GMenu;

public:
	int2				m_MenuValue;
	//	list vars
	GMenu*				m_pParentMenu;	//	if no parentmenu, we're a popup
	GMenuItem*			m_pParentItem;	//	if no parentitem, we're a submenu of the main parent menu
	GList<GMenuItem*>	m_Items;

private:
	//	win menu vars
	HMENU				m_HMenu;


public:
	GMenuSubMenu();
	~GMenuSubMenu();

	void				SetParentMenu(GMenu* pMenu,Bool RootMenu);									//	set this submenu's owner
	inline void			SetPopupMenu()							{	SetParentMenu(NULL,FALSE);	};	//	set this submenu to be the root of a popup menu
	void				Popup(int2 Pos);															//	manually popup this menu

	//	list related
	GMenuItem*			Add(const char* pString);
	inline void			Insert(GMenuItem* pMenu, int Index)		{	m_Items.Insert(Index,pMenu);	};
	void				AddDivider();

	void				Remove(int Index);
	void				Remove(GMenuItem* pMenu)				{	Remove( m_Items.FindIndex( pMenu ) );	};
	void				RemoveAll();

/*
	GMenuItem*			ItemAtIndex(u16 Index);
	GMenuItem*			FindItemOfID( u16 ItemID );
*/
	//	null any pointers matching these
	void				NullPointers(GMenuItem* pItem);
	void				NullPointers(GMenuSubMenu* pSubMenu);

	GMenuSubMenu*		GetSubMenu(HMENU handle);
	GMenuItem*			GetMenuItem(u16 MenuID);
	
	GMenuSubMenu*		GetSubMenuWithValue0( int Value0 );
	GMenuItem*			GetMenuItemWithValue0( int Value0 );

	Bool				ContainsMenuItem(GMenuItem* pItem);
	Bool				ContainsSubMenu(GMenuSubMenu* pMenu);
	
	inline HMENU		MenuHandle()							{	return m_HMenu;	};

	void				Destroy();

private:
	inline void			AddHead(GMenuItem* pMenu)				{	Insert(pMenu,0);	};			
	inline void			AddTail(GMenuItem* pMenu)				{	m_Items.Add(pMenu);	};

};







class GMenuItem
{
	friend GMenuSubMenu;
	friend GMenu;

public:
	int2				m_MenuValue;
	GMenu*				m_pParentMenu;	//	reference
	GMenuSubMenu*		m_pParent;		//	reference
	GMenuSubMenu*		m_pSubMenu;		//	owned by this
	u32					m_Flags;		//	GMenuItemFlags

private:
	//	win menu vars
	u16					m_MenuID;
	const char*			m_pString;	//	for now, constant strings only

public:
	GMenuItem();
	~GMenuItem();

	//	list related
	int					Index()									{	return !m_pParent ? -1 : m_pParent->m_Items.FindIndex(this);	};
	GMenuItem*			FindItemID(u16 ItemID);
	void				DeleteSubMenu();
	GMenuSubMenu*		AddSubMenu();

	void				NullPointers(GMenuItem* pItem);
	void				NullPointers(GMenuSubMenu* pSubMenu);

	inline void			SetItemFlags(u32 Flags)		{	m_Flags = Flags;	ApplyMenuFlags();	};
	void				ApplyMenuFlags();
	Bool				GetMenuStateChecked();
	
	GMenuSubMenu*		GetSubMenu(HMENU handle);
	GMenuItem*			GetMenuItem(u16 MenuID);

	GMenuSubMenu*		GetSubMenuWithValue0( int Value0 );
	GMenuItem*			GetMenuItemWithValue0( int Value0 );

	Bool				ContainsMenuItem(GMenuItem* pItem);
	Bool				ContainsSubMenu(GMenuSubMenu* pMenu);

private:
	void				ApplyMenuItemSettings( MENUITEMINFO* ItemInfo );
	void				SetupNewMenuItemInfo( MENUITEMINFO* ItemInfo, const char* pString );	//	for new items
	void				GetMenuItemSettings( MENUITEMINFO* ItemInfo, char* pBuffer, u32 BufferSize );
	
};


class GMenu
{
	friend GMenuItem;
	friend GMenuSubMenu;

public:
	GApp*			m_pOwnerApp;

private:
	GMenuSubMenu*	m_pRootMenu;
	static u16		g_MenuCount;	//	menu counter for individual menu id's

public:
	GMenu();
	~GMenu();

	void					Init(GApp* pApp);
	void					Shutdown();
	inline Bool				Initalised()	{	return m_pRootMenu != NULL;	};

	void					Refresh();
	void					NullPointers(GMenuItem* pItem);	//	null any pointers matching these
	void					NullPointers(GMenuSubMenu* pSubmenu);
	inline GMenuSubMenu*	RootMenu()		{	return m_pRootMenu;	};

	GMenuSubMenu*			GetSubMenu(HMENU handle);
	GMenuItem*				GetMenuItem(u16 MenuID);

	GMenuSubMenu*			GetSubMenuWithValue0( int Value0 );
	GMenuItem*				GetMenuItemWithValue0( int Value0 );

};




//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------



#endif

