/*------------------------------------------------

  GGutFile.cpp

	Class for importing and exporting asset data to and from files. Designed just to be
	used as a local variable


	File structure

	GGutFileHeader		Header
	{
		GAssetHeader	Asset block header
		<AssetHeader>
		<AssetData>
	}


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GGutFile.h"
#include "GAsset.h"
#include "GDebug.h"
#include "GWin32.h"
#include "GMesh.h"
#include "GTexture.h"
#include "GFile.h"
#include "GMap.h"
#include "GAssetList.h"


//	globals
//------------------------------------------------
const u32	GGutFile::g_Version		= 0x55550003;
const char*	GGutFile::g_FileExt		= "gut";
const char*	GGutFile::g_FileFilter	= "Gut file (*.gut)\0*.gut\0\0";


//	Definitions
//------------------------------------------------



GGutFile::GGutFile()
{
	//	init header
	m_Header.Reset();

}



GGutFile::~GGutFile()
{
	//	delete the allocated data in the meshes
	RemoveAssets(TRUE);
}



Bool GGutFile::Load(const GString& Filename)
{
	GFile File;
	if ( ! File.Load( Filename ) )
		return FALSE;

	//	we've read in the data, now turn it into data
	if ( ! ImportData( File.m_Data ) )
		return FALSE;



	return TRUE;
}




Bool GGutFile::Save(const GString& Filename)
{
	GFile File;

	//	create data to save
	if ( ! ExportData( File.m_Data ) )
		return FALSE;

	if ( ! File.Save( Filename ) )
		return FALSE;
	
	return TRUE;
}




Bool GGutFile::ImportData(GBinaryData& Data)
{
	//	read a block of data, update sizes and data pointer
	#define READ_BLOCK( addr, size, errstring )					\
	{													\
		if ( (int)DataSize < (int)(size) )				\
		{												\
			GDebug::Print( errstring );					\
			return FALSE;								\
		}												\
		memcpy( addr, pData, size );					\
		pData += size;									\
		DataSize -= size;								\
	}

	
	//	delete current data
	for ( int m=0;	m<m_Assets.Size();	m++ )
	{
		if ( m_Assets[m] )
		{
			delete m_Assets[m];
			m_Assets[m] = NULL;
		}
	}
	m_Assets.Empty();


	//	reset binary data read pos
	Data.ResetRead();

	//	read in file header
	//READ_BLOCK( &m_Header, sizeof( GGutFileHeader ), "File is missing header\n" );
	Data.Read( &m_Header, sizeof( GGutFileHeader ), "GutFile header" );

	//	check header version
	if ( m_Header.Version != GGutFile::g_Version )
	{
		GDebug_Print("Invalid Gutfile version 0x%08x should be 0x%08x\n", m_Header.Version, GGutFile::g_Version );
		return FALSE;
	}

	int AssetsRead = 0;

	//	read in all the data
	while ( Data.DataUnread() > 0 )
	{
		//	seem to have too much data... break out of the loading loop
		if ( AssetsRead >= m_Header.AssetCount )
		{
			GDebug_Print("Excess data in gutfile\n");
			break;
		}

		GAssetHeader AssetHeader;
		//READ_BLOCK( &AssetHeader, sizeof(GAssetHeader), "Asset header missing\n" );
		Data.Read( &AssetHeader, sizeof(GAssetHeader), "Asset header" );

		AssetsRead++;

		//	check we can load this asset type
		if ( !ValidAssetType( AssetHeader.AssetType ) )
		{
			//	cant load this data, skip over it
			Data.Skip(AssetHeader.BlockSize);
			GDebug_Print("Unknown asset type (%d, version 0x%08x)\n",AssetHeader.AssetType, AssetHeader.AssetVersion);
			continue;
		}


		GAsset* pNewAsset = NULL;

		switch ( AssetHeader.AssetType )
		{
			case GAssetMesh:
				pNewAsset = new GMesh;
				break;

			case GAssetTexture:
				pNewAsset = new GTexture;
				break;

			case GAssetObject:
				//	objects no longer supported
				pNewAsset = NULL;
				break;

			case GAssetMap:
				pNewAsset = new GMap;
				break;

			case GAssetMapObject:
				pNewAsset = new GMapObject;
				break;

			case GAssetSkeleton:
				pNewAsset = new GSkeleton;
				break;

			case GAssetSkeletonAnim:
				pNewAsset = new GSkeletonAnim;
				break;

			case GAssetSkin:
				pNewAsset = new GSkin;
				break;

			case GAssetSubMap:
			{
				GDebug_Break("Submaps should not be present on their own, they must be members of a map\n");
			}
			break;

			default:
			{
				GDebug_Break("Unknown asset type, this should have already been handled by ValidAssetType()\n");
			}
			break;
		};

		//	load the new asset
		if ( pNewAsset )
		{
			//	new mesh of the correct type has been created 
			int PreLoadDataPos = Data.GetReadPos();

			if ( ! pNewAsset->LoadAsset( &AssetHeader, Data ) )
			{
				GDebug_Print("Asset Failed to load, skipping over\n");

				//	delete useless data
				delete pNewAsset;

				//	skip over data
				Data.SetReadPos( PreLoadDataPos + AssetHeader.BlockSize );
			}
			else
			{
				//	loaded okay, add to list of data
				m_Assets.Add( pNewAsset );
			}
		}
		else
		{
			//	skip over to next bit of data as we're not reading this section
			Data.Skip( AssetHeader.BlockSize );
		}
	}

	//	finished loading
	#undef READ_BLOCK

	return TRUE;
}






Bool GGutFile::ExportData(GBinaryData& Data)
{
	//	delete current data
	Data.Empty();
	
	//	reset header
	m_Header.Reset();
	m_Header.AssetCount = m_Assets.Size();

	//	add header to file data
	Data.Write( &m_Header, sizeof( GGutFileHeader ) );
	

	//	save the assets into the FileData
	for ( int a=0;	a<m_Assets.Size();	a++ )
	{
		//	setup block header
		GAssetHeader	AssetHeader;
		AssetHeader.AssetType		= m_Assets[a]->AssetType();
		AssetHeader.AssetVersion	= m_Assets[a]->Version();
		AssetHeader.AssetRef		= m_Assets[a]->m_AssetRef;

		//	save data into an array and update the size of the data
		GBinaryData AssetSaveData;
		
		//	failed to save
		if ( !m_Assets[a]->Save( AssetSaveData ) )
		{
			AssetSaveData.Empty();
			GDebug_Print("Failed to make data from %s asset for save file\n", m_Assets[a]->AssetTypeName() );
			continue;
		}

		//	update data size
		AssetHeader.BlockSize = AssetSaveData.Size();

		//	add the asset block's header first
		Data.Write( &AssetHeader, sizeof(GAssetHeader) );

		//	now add the asset data
		Data.Write( AssetSaveData );

		//	raw asset data no longer needed
		AssetSaveData.Empty();
	}

	//	now have new m_Data to save
	return TRUE;
}








void GGutFile::RemoveAssets(Bool Delete)
{
	//	delete all the meshes if we need to
	if ( Delete )
	{
		for ( int a=0;	a<m_Assets.Size();	a++ )
		{
			if ( m_Assets[a] )
			{
				delete m_Assets[a];	
				m_Assets[a] = NULL;
			}
		}
	}

	//	clear the list
	m_Assets.Empty();	
}




void GGutFile::AddAssets(GMeshList* pList)
{
	for ( int i=0;	i<pList->Size();	i++ )
	{
		m_Assets.Add( pList->GetAsset( i ) );
	}
}





void GGutFile::AddAssets(GTextureList* pList)
{
	for ( int i=0;	i<pList->Size();	i++ )
	{
		m_Assets.Add( pList->GetAsset( i ) );
	}
}



void GGutFile::AddAssets(GMapList* pList)
{
	for ( int i=0;	i<pList->Size();	i++ )
	{
		m_Assets.Add( pList->GetAsset( i ) );
	}
}



void GGutFile::AddAssets(GMapObjectList* pList)
{
	for ( int i=0;	i<pList->Size();	i++ )
	{
		m_Assets.Add( pList->GetAsset( i ) );
	}
}



GAsset* GGutFile::FindAsset(GAssetType Type, GAssetRef Ref)
{
	for ( int i=0;	i<m_Assets.Size();	i++ )
	{
		if ( m_Assets[i]->AssetType() == Type && m_Assets[i]->AssetRef() == Ref )
		{
			return m_Assets[i];
		}
	}

	return NULL;
}


//-------------------------------------------------------------------------
//	load all assets from file into global system. returns number of successfully 
//	loaded assets
//-------------------------------------------------------------------------
int GGutFile::LoadAssets()
{
	return GAssets::AddFromList( m_Assets );
}







