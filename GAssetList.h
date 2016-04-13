/*------------------------------------------------

  GAssetlist Header file



-------------------------------------------------*/

#ifndef __GASSETLIST__H_
#define __GASSETLIST__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GList.h"
#include "GAsset.h"
#include "GDebug.h"
#include "GMesh.h"
#include "GTexture.h"
#include "GMap.h"
#include "GSkeleton.h"
#include "GShader.h"
#include "GSkin.h"


//	Macros
//------------------------------------------------





//	Types
//------------------------------------------------

class GAssetRefIndex
{
public:
	GAssetRef	Ref;	//	the asset's ref
	int			Index;	//	index in m_Meshes
	
public:
	inline Bool		operator == (GAssetRefIndex& ri)		{	return (Ref==ri.Ref) && (Index==ri.Index);	};
};



template <class TYPE>
class GAssetList
{
private:
	GList<TYPE*>			m_Assets;		//	list of all the asset data in the list
	GList<GAssetRefIndex>	m_AssetRefs;	//	list of meshreferences and their index in our mesh's list, sorted by references

public:
	GAssetList();
	~GAssetList();

	void			Empty();						//	delete all meshes
	inline int		Size()							{	return m_Assets.Size();	};
	Bool			Add(TYPE* pAsset);				//	add a mesh will fail if there are any duplicate references
	Bool			Delete(GAssetRef Ref);			//	deletes the mesh matching the reference
	Bool			Remove(GAssetRef Ref);			//	removes the asset matching this poitner from the list
	Bool			Remove(TYPE* pAsset)			{	int Index;	return ( ( Index = FindIndex(pAsset) ) != -1 ) ? RemoveIndex( Index ) : FALSE;	};
	
	inline TYPE*	Find(const char* pRefName)		{	return Find( GAsset::NameToRef(pRefName) );	};		//	find the mesh matching the reference
	TYPE*			Find(GAssetRef Ref);			//	find the mesh matching the reference
	int				FindIndex(GAssetRef Ref);		//	find the reference index for this reference
	int				FindIndex(TYPE* pAsset);		//	find the index for this asset
	inline TYPE*	GetAsset(int Index)				{	GDebug_CheckIndex(Index,0,Size() );	return (Index<0||Index>=Size()) ? NULL : m_Assets[Index];	};
	void			ChangeAssetRef(GAssetRef OldRef, GAssetRef NewRef);	//	change asset and asset indexes to change the ref

	GAssetRef		GetNextFreeRef();				//	get the next availible asset ref. not for use in realtime

	inline TYPE*	operator[](int Index)			{	return GetAsset(Index);	};

private:
	Bool			DeleteIndex(int Index);			//	deletes the mesh matching the reference
	Bool			RemoveIndex(int Index);			//	removes the mesh matching the reference
	int				FindAssetIndex(GAssetRef Ref);	//	find the mesh index for this reference
	void			ReIndexRefs();					//	update the mesh indexes in the meshrefs
	void			ReSortRefs();					//	resort the asset index list. needs to be called if any asset refs are changed
};



class GMeshList : public GAssetList<GMesh>	
{
};

class GTextureList : public GAssetList<GTexture>	
{
};

class GMapObjectList : public GAssetList<GMapObject>
{
};

class GMapList : public GAssetList<GMap>
{
};

class GSkinList : public GAssetList<GSkin>
{
};

class GSkeletonList : public GAssetList<GSkeleton>
{
};

class GSkeletonAnimList : public GAssetList<GSkeletonAnim>
{
};


namespace GAssets
{
	extern GMeshList			g_Meshes;			//	list of meshes
	extern GTextureList			g_Textures;			//	list of textures
	extern GMapList				g_Maps;				//	list of maps (submap list contained inside each map)
	extern GMapObjectList		g_MapObjects;		//	list of map objects
	extern GSkinList			g_Skins;			//	list of skins
	extern GSkeletonList		g_Skeletons;		//	list of skeletons
	extern GSkeletonAnimList	g_SkeletonAnims;	//	list of skeleton animations
	
	void					ClearAssets();
	void					AddToList(GList<GAsset*>& AssetList);	//	adds all our global assets into this asset list
	int						AddFromList(GList<GAsset*>& AssetList);	//	adds all elements from a list into the global assets
};	



//	Declarations
//------------------------------------------------



//	Inline Definitions
//-------------------------------------------------






template <class TYPE>
GAssetList<TYPE>::GAssetList()
{
}



template <class TYPE>
GAssetList<TYPE>::~GAssetList()
{
	//	remove all Assetes
	Empty();
}



template <class TYPE>
void GAssetList<TYPE>::Empty()
{
	//	remove the first Asset until theyre all gone
	while ( m_Assets.Size() > 0 )
	{
		DeleteIndex(0);
	}
}



template <class TYPE>
Bool GAssetList<TYPE>::Add(TYPE* pAsset)
{
	int i;
	GAssetRef& ref = pAsset->m_AssetRef;
	
	//	find duplicate Asset ref
	if ( FindIndex( ref ) != -1 )
	{
		GDebug::Print("Error inserting Asset into Asset list, duplicate Asset ref: 0x%08x\n", ref );
		return FALSE;
	}

	//	setup new index
	GAssetRefIndex RefIndex;
	RefIndex.Ref = ref;
	RefIndex.Index = -1;
	
	//	add Asset into list
	RefIndex.Index = m_Assets.Add( pAsset );
	if ( RefIndex.Index == -1 )
	{
		GDebug_Break("Error inserting Asset into Asset list");
		return FALSE;
	}
	
	//	find index to insert refindex into (sorted)
	int RefIndexIndex = -1;
	for ( i=0;	i<m_AssetRefs.Size();	i++ )
	{
		if ( ref < m_AssetRefs[i].Ref )
		{
			RefIndexIndex = i;
			break;
		}
	}

	//	if we havent got an index, add onto the end of the list
	if ( RefIndexIndex == -1 )
	{
		RefIndexIndex = m_AssetRefs.Add( RefIndex );
	}
	else
	{
		m_AssetRefs.Insert( RefIndexIndex, RefIndex );
	}

/*
	GDebug::Print("New list of refs:\n");
	for ( i=0;	i<m_AssetRefs.Size();	i++ )
	{
		GDebug::Print("%d: %08x \"%s\" (%d)\n", i, m_AssetRefs[i].Ref, GAsset::RefToName(m_AssetRefs[i].Ref), m_AssetRefs[i].Index );
	}
*/

	return TRUE;
}



template <class TYPE>
Bool GAssetList<TYPE>::Delete(GAssetRef Ref)
{
	int Index = FindIndex( Ref );
	if ( Index == -1 )
		return FALSE;

	return DeleteIndex( Index );
}

//-------------------------------------------------------------------------
//	Finds an asset and removes it from our list without deleting it
//-------------------------------------------------------------------------
template <class TYPE>
Bool GAssetList<TYPE>::Remove(GAssetRef Ref)
{
	int Index = FindIndex( Ref );
	if ( Index == -1 )
		return FALSE;

	return RemoveIndex( Index );
}




template <class TYPE>
TYPE* GAssetList<TYPE>::Find(GAssetRef Ref)
{
	//	find the index
	int Index = FindAssetIndex( Ref );
	if ( Index == -1 )
		return NULL;

	return m_Assets[Index];
}





template <class TYPE>
Bool GAssetList<TYPE>::DeleteIndex(int Index)
{
	if ( Index < 0 || Index >= m_Assets.Size() )
	{
		return FALSE;
	}

	//	grab the Asset's index
	int AssetIndex = m_AssetRefs[Index].Index;

	//	remove the Asset reference
	m_AssetRefs.RemoveAt( Index );

	GDebug_CheckIndex( AssetIndex, 0, m_Assets.Size() );
	
	//	delete the Asset
	GDelete( m_Assets[AssetIndex] );
	m_Assets.RemoveAt( AssetIndex );

	//	we've removed an index from somewhere in the list, the list needs re-indexing!
	ReIndexRefs();


	return TRUE;
}


//-------------------------------------------------------------------------
//	removes an asset from our list but does NOT delete it
//-------------------------------------------------------------------------
template <class TYPE>
Bool GAssetList<TYPE>::RemoveIndex(int Index)
{
	if ( Index < 0 || Index >= m_Assets.Size() )
	{
		return FALSE;
	}

	//	grab the Asset's index
	int AssetIndex = m_AssetRefs[Index].Index;

	//	remove the Asset reference
	m_AssetRefs.RemoveAt( Index );

	GDebug_CheckIndex( AssetIndex, 0, m_Assets.Size() );
	
	m_Assets.RemoveAt( AssetIndex );

	//	we've removed an index from somewhere in the list, the list needs re-indexing!
	ReIndexRefs();


	return TRUE;
}


//-------------------------------------------------------------------------
//	returns the AssetRef index
//-------------------------------------------------------------------------
template <class TYPE>
int GAssetList<TYPE>::FindIndex(GAssetRef Ref)
{
	//	change this to use the GList sorting functions

	for ( int i=0;	i<m_AssetRefs.Size();	i++ )
	{
		if ( m_AssetRefs[i].Ref == Ref )
			return i;
	}

	//	not found
	return -1;
}

//-------------------------------------------------------------------------
//	returns the AssetRef index
//-------------------------------------------------------------------------
template <class TYPE>
int GAssetList<TYPE>::FindIndex(TYPE* pAsset)
{
	//	find the asset index
	for ( int i=0;	i<m_AssetRefs.Size();	i++ )
	{
		if ( m_Assets[ m_AssetRefs[i].Index ] == pAsset )
		{
			return i;
		}
	}

	return -1;
}




template <class TYPE>
int GAssetList<TYPE>::FindAssetIndex(GAssetRef Ref)
{
	//	get the reference's index
	int RefIndex = FindIndex( Ref );
	if ( RefIndex == -1 )
		return -1;

	//	return the Asset's index
	return m_AssetRefs[RefIndex].Index;
}
	






template <class TYPE>
void GAssetList<TYPE>::ReIndexRefs()
{
	//	todo: speed this up if neccessary
	for ( int r=0;	r<m_AssetRefs.Size();	r++ )
	{
		//	reset index
		m_AssetRefs[r].Index = -1;

		//	find Asset that matches the Assetref
		for ( int m=0;	m<m_Assets.Size();	m++ )
		{
			if ( m_AssetRefs[r].Ref == m_Assets[m]->m_AssetRef )
			{
				m_AssetRefs[r].Index = m;
				break;
			}
		}
	}

}



template <class TYPE>
void GAssetList<TYPE>::ReSortRefs()
{
	//	*TODO*
	//	shouldnt be called too often, shouldnt really ever need to be called in realtime
}



template <class TYPE>
void GAssetList<TYPE>::ChangeAssetRef(GAssetRef OldRef, GAssetRef NewRef)
{
	int Index = FindIndex( OldRef );
	GAssetRefIndex& AssetIndex = m_AssetRefs[Index];

	//	change index's ref
	AssetIndex.Ref = NewRef;

	//	change assets ref
	GAsset* pAsset = m_Assets[ AssetIndex.Index ];
	pAsset->m_AssetRef = NewRef;

	//	asset indexes need resorting
	ReSortRefs();
}




template <class TYPE>
GAssetRef GAssetList<TYPE>::GetNextFreeRef()
{
	GAssetRef NewRef = 0x0;

	for ( int i=0;	i<Size();	i++ )
	{
		if ( m_Assets[i]->m_AssetRef == NewRef )
		{
			NewRef++;
			i=0;
		}
	}

	return NewRef;
}


#endif

