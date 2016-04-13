/*------------------------------------------------

  GAssetList.cpp

	List of Assets loaded. Provides quick access via AssetRef's (looking up a sorted
	table) and manages Assetes. (no duplicate references, inserting Assetes in order etc)

	templated for additional asset specific functions


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GAssetList.h"


//	globals
//------------------------------------------------

GMeshList			GAssets::g_Meshes;
GTextureList		GAssets::g_Textures;
GMapList			GAssets::g_Maps;
GMapObjectList		GAssets::g_MapObjects;
GSkinList			GAssets::g_Skins;
GSkeletonList		GAssets::g_Skeletons;
GSkeletonAnimList	GAssets::g_SkeletonAnims;


//	Definitions
//------------------------------------------------

void GAssets::ClearAssets()
{
	GAssets::g_Meshes.Empty();
	GAssets::g_Textures.Empty();
	GAssets::g_Maps.Empty();
	GAssets::g_MapObjects.Empty();
	GAssets::g_Skins.Empty();
	GAssets::g_Skeletons.Empty();
	GAssets::g_SkeletonAnims.Empty();
}
	

void GAssets::AddToList(GList<GAsset*>& AssetList)
{
	int i;

	for ( i=0;	i<GAssets::g_Meshes.Size();	i++ )
		AssetList.Add( GAssets::g_Meshes[i] );

	for ( i=0;	i<GAssets::g_Textures.Size();	i++ )
		AssetList.Add( GAssets::g_Textures[i] );

	for ( i=0;	i<GAssets::g_Maps.Size();	i++ )
		AssetList.Add( GAssets::g_Maps[i] );

	for ( i=0;	i<GAssets::g_MapObjects.Size();	i++ )
		AssetList.Add( GAssets::g_MapObjects[i] );

	for ( i=0;	i<GAssets::g_Skins.Size();	i++ )
		AssetList.Add( GAssets::g_Skins[i] );

	for ( i=0;	i<GAssets::g_Skeletons.Size();	i++ )
		AssetList.Add( GAssets::g_Skeletons[i] );

	for ( i=0;	i<GAssets::g_SkeletonAnims.Size();	i++ )
		AssetList.Add( GAssets::g_SkeletonAnims[i] );

}



int GAssets::AddFromList(GList<GAsset*>& AssetList)
{
	int Loaded = 0;
	for ( int i=0;	i<AssetList.Size();	i++ )
	{
			//	add in all the assets from the file into the world
		GAsset* pAsset = AssetList[i];

		#define ADD_ASSET_TO_LIST(ASSETTYPE,TYPE,LIST)	case ASSETTYPE:	if ( GAssets::LIST.Add( (TYPE*)pAsset ) )	Loaded++;	break

		switch ( pAsset->AssetType() )
		{
			ADD_ASSET_TO_LIST( GAssetMesh,			GMesh,			g_Meshes	);
			ADD_ASSET_TO_LIST( GAssetTexture,		GTexture,		g_Textures	);
			ADD_ASSET_TO_LIST( GAssetMap,			GMap,			g_Maps	);
			//ADD_ASSET_TO_LIST( GAssetSubMap,		GSubmap,		g_SubMaps	);
			ADD_ASSET_TO_LIST( GAssetMapObject,		GMapObject,		g_MapObjects	);
			ADD_ASSET_TO_LIST( GAssetSkeleton,		GSkeleton,		g_Skeletons	);
			ADD_ASSET_TO_LIST( GAssetSkin,			GSkin,			g_Skins	);
			ADD_ASSET_TO_LIST( GAssetSkeletonAnim,	GSkeletonAnim,	g_SkeletonAnims );
			default:
				GDebug::Break("Unhandled asset type %d (%s)\n", pAsset->AssetType(), pAsset->AssetTypeName() );
				break;
		}
	}

	return Loaded;
}



