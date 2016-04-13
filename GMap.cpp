/*------------------------------------------------

  GMap.cpp

	Map enviroment object. contains map objects, submap info
	etc



-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GMap.h"
#include "GDebug.h"
#include "GWorld.h"
#include "GCamera.h"
#include "GAssetList.h"
#include "GStats.h"
#include "GApp.h"
#include "GBinaryData.h"



//	globals
//------------------------------------------------
const u32	GMap::g_Version			= 0x66660003;
const u32	GSubMap::g_Version		= 0x77770004;
const u32	GMapObject::g_Version	= 0x88880004;


//	Definitions
//------------------------------------------------



GMapObject::GMapObject()
{
	m_ShaderRef	= GAssetRef_Invalid;
	m_Mesh		= GAssetRef_Invalid;
	m_Texture	= GAssetRef_Invalid;

	m_Position	= float3(0,0,0);
	m_Rotation	= GQuaternion();
	m_Colour	= float4(1,1,1,1);
	m_Flags		= 0x0;

	m_pShader	= NULL;
}


GMapObject::~GMapObject()
{
}

GMesh* GMapObject::GetMesh()
{
	return GAssets::g_Meshes.Find( m_Mesh );
}

GTexture* GMapObject::GetTexture()
{
	return GAssets::g_Textures.Find( m_Texture );
}


Bool GMapObject::Load(GBinaryData& Data)
{
	/*
	//	read a block of data, update sizes and data pointer
	#define READ_BLOCK( addr, size )					\
	{													\
		if ( (int)DataSize < (int)(size) )				\
		{												\
			GDebug::Print("MapObject is missing data. requested %d, remaining %d\n",(size),DataSize);	\
			return FALSE;								\
		}												\
		memcpy( addr, pData, (size) );					\
		pData += (size);								\
		DataSize -= (size);								\
		DataRead += (size);								\
	}
	*/

	//	read-in the header
	GMapObjectHeader Header;
//	READ_BLOCK( &Header, GDataSizeOf(GMapObjectHeader) );
	if ( !Data.Read( &Header, GDataSizeOf(GMapObjectHeader), "GMapObject" ) )
		return FALSE;

	m_Mesh		= Header.MeshRef;
	m_Texture	= Header.TextureRef;
	m_ShaderRef	= Header.ShaderRef;
	m_Position	= Header.Position;
	m_Rotation	= Header.Rotation;
	m_Colour	= Header.Colour;
	m_Flags		= Header.Flags;

	//#undef READ_BLOCK
	return TRUE;
}


Bool GMapObject::Save(GBinaryData& Data)
{
	//	add header
	GMapObjectHeader Header;
	Header.MeshRef		= m_Mesh;
	Header.TextureRef	= m_Texture;
	Header.ShaderRef	= m_ShaderRef;
	Header.Position		= m_Position;
	Header.Rotation		= m_Rotation;
	Header.Colour		= m_Colour;
	Header.Flags		= m_Flags;

	Data.Write( &Header, GDataSizeOf(GMapObjectHeader) );

	return TRUE;
}


GDrawResult GMapObject::Draw(u32 DrawFlags)
{
	//	grab mesh
	GMesh* pMesh = GetMesh();
	if ( !pMesh )
		return GDrawResult_Error;

	//	setup drawinfo
	GDrawInfo DrawInfo;
	DrawInfo.Flags			= DrawFlags;
	DrawInfo.pLight			= NULL;			//	todo
	DrawInfo.pRotation		= &m_Rotation;
	DrawInfo.Translation	= m_Position;
	DrawInfo.RGBA			= m_Colour;
	DrawInfo.WorldPos		= m_Position;
	DrawInfo.TextureRef		= m_Texture;
	DrawInfo.TextureRef2	= GAssetRef_Invalid;
	DrawInfo.pShader		= m_pShader;

	//	convert any m_Flags to drawinfo flags
	if ( m_Flags & GMapObjectFlags::CullFrontFaces )
		DrawInfo.Flags |= GDrawInfoFlags::CullFrontFaces;	

	GIncCounter( DrawMapObjects, 1 );

	return pMesh->Draw( DrawInfo );
}

GBounds& GMapObject::GetBounds()
{
	static GBounds TmpBounds( 1.f );

	//GDebug_Print("Todo: get new bounds from mesh\n");

	return TmpBounds;
/*
	//	use meshes's bounds
	GMesh* pMesh = GetMesh();
	if ( !pMesh )
		return TmpBounds;

	return pMesh->m_Bounds;
	*/
}




//------------------------------------------------


GSubMap::GSubMap()
{
	//m_MapObjects;
}


GSubMap::~GSubMap()
{
}


Bool GSubMap::Load(GBinaryData& Data)
{
	/*
	//	read a block of data, update sizes and data pointer
	#define READ_BLOCK( addr, size )					\
	{													\
		if ( (int)DataSize < (int)(size) )				\
		{												\
			GDebug::Print("MapObject is missing data. requested %d, remaining %d\n",(size),DataSize);	\
			return FALSE;								\
		}												\
		memcpy( addr, pData, (size) );					\
		pData += (size);									\
		DataSize -= (size);								\
		DataRead += (size);								\
	}
	*/
	
	//	read-in the header
	GSubMapHeader Header;
	//READ_BLOCK( &Header, GDataSizeOf(GSubMapHeader) );
	Data.Read( &Header, GDataSizeOf(GSubMapHeader ) );

	//	make a new list of map objects
	m_MapObjects.Empty();
	int MapObjectRefs = Header.MapObjectCount;

	while ( MapObjectRefs > 0 )
	{
		MapObjectRefs--;
		GAssetRef Ref = GAssetRef_Invalid;
		//READ_BLOCK( &Ref, GDataSizeOf(GAssetRef) );
		if ( !Data.Read( &Ref, GDataSizeOf(GAssetRef), "submap mapobject ref" ) )
			return FALSE;

		m_MapObjects.Add( Ref );
	}

	//	add in object list entries
	m_ObjectInsideList.Resize( Header.ObjectInsideListSize );
	//READ_BLOCK( m_ObjectInsideList.Data(), GDataSizeOf(u32)*m_ObjectInsideList.Size() );
	if ( !Data.Read( m_ObjectInsideList.Data(), m_ObjectInsideList.DataSize(), "Submap ObjectInsideList" ) )
		return FALSE;

	//	add in portals
	m_Portals.Resize( Header.Portals );
	for ( int i=0;	i<m_Portals.Size();	i++ )
	{
		GMapPortalHeader PortalData;
		//READ_BLOCK( &PortalData, GDataSizeOf(GMapPortalHeader) );
		if ( !Data.Read( &PortalData, GDataSizeOf(GMapPortalHeader), "Submap portal header" ) )
			return FALSE;

		m_Portals[i].m_PortalRef		= PortalData.AssetRef;
		m_Portals[i].m_Type				= PortalData.Type;
		m_Portals[i].m_PortalVerts[0]	= PortalData.Verts[0];
		m_Portals[i].m_PortalVerts[1]	= PortalData.Verts[1];
		m_Portals[i].m_PortalVerts[2]	= PortalData.Verts[2];
		m_Portals[i].m_PortalVerts[3]	= PortalData.Verts[3];
		m_Portals[i].m_PortalNormal		= PortalData.Normal;
		m_Portals[i].m_OtherSubmap		= PortalData.Ref1;
		m_Portals[i].m_OtherPortal		= PortalData.Ref2;

		//	temp
		m_Portals[i].m_Texture = GAsset::NameToRef("ABC");
	}

	//#undef READ_BLOCK


	//	temp:
	//	create temp light
	GMapLight Light;
	Light.m_Pos = float4( 3.f, 3.f, 1.f, 0.f );
	m_Lights.Add( Light );



	return TRUE;
}


Bool GSubMap::Save(GBinaryData& Data)
{
	int i;

	//	add header
	GSubMapHeader Header;
	Header.MapObjectCount = m_MapObjects.Size();
	Header.ObjectInsideListSize = m_ObjectInsideList.Size();
	Header.Portals = m_Portals.Size();

	Data.Write( &Header, GDataSizeOf(GSubMapHeader) );

	//	add each reference
	for ( i=0;	i<m_MapObjects.Size();	i++ )
	{
		Data.Write( &m_MapObjects[i], GDataSizeOf(GAssetRef) );
	}

	//	add each ObjectInsideList entry
	for ( i=0;	i<m_ObjectInsideList.Size();	i++ )
	{
		Data.Write( &m_ObjectInsideList[i], GDataSizeOf(u32) );
	}

	//	add each portal
	for ( i=0;	i<m_Portals.Size();	i++ )
	{
		GMapPortalHeader Portal;
		Portal.AssetRef	= m_Portals[i].m_PortalRef;
		Portal.Type		= m_Portals[i].m_Type;
		Portal.Verts[0]	= m_Portals[i].m_PortalVerts[0];
		Portal.Verts[1]	= m_Portals[i].m_PortalVerts[1];
		Portal.Verts[2]	= m_Portals[i].m_PortalVerts[2];
		Portal.Verts[3]	= m_Portals[i].m_PortalVerts[3];
		Portal.Normal	= m_Portals[i].m_PortalNormal;
		Portal.Ref1		= m_Portals[i].m_OtherSubmap;
		Portal.Ref2		= m_Portals[i].m_OtherPortal;

		Data.Write( &Portal, GDataSizeOf(GMapPortalHeader) );
	}


	return TRUE;
}


void GSubMap::PreDrawMapObjects(GCamera* pCamera, GList<u32>& VisibleMapObjects, GList<GPreDrawResult>& Results, int SubMapIndex)
{
	//	minimise cull checks by only checking ones we need to, and dont render items if they have been culled

	//	initialise results
	Results.Resize( m_MapObjects.Size() );
	Results.SetAll( GPreDrawResult_Unknown );

	int i;

	//	do auto inside culling
	//if ( m_ObjectInsideList.Size() && !(DrawFlags & GDrawInfoFlags::DontAutoInsideCull) )
	if ( m_ObjectInsideList.Size() )
	{
		/*
			todo: reverse this order to reduce cull checks?
		*/
		//	go through the object inside list and check culling of an outside
		for ( i=0;	i<m_ObjectInsideList.Size();	i++ )
		{
			//	a is inside b
			u32 a = m_ObjectInsideList[i] & 0xffff;			
			u32 b = (m_ObjectInsideList[i]>>16) & 0xffff;

			//	outside object state unknown, do a cull check
			if ( Results[b] == GPreDrawResult_Unknown )
			{
				//	check culling of map object
				GMapObject* pMapObject = GAssets::g_MapObjects.Find( m_MapObjects[b] );
				GMesh* pMesh = pMapObject ? pMapObject->GetMesh() : NULL;
				
				//	object missing!?
				if ( !pMapObject || !pMesh )
				{
					Results[b] = GPreDrawResult_NoObject;
				}
				else 
				{
					GDebug_Print("Todo: check mesh culling\n");
					/*
					//	check culled state
					if ( pMesh->m_Bounds.IsCulled( pMapObject->m_Position, pCamera ) )
						Results[b] = GPreDrawResult_Culled;
					else
					*/
						//	yes, will be drawn
						Results[b] = GPreDrawResult_Draw;
				}					
			}		

			//	is outside object culled? if so, auto-cull this inside object
			if ( Results[b] == GPreDrawResult_Culled || Results[b] == GPreDrawResult_CulledInside )
			{
				Results[a] = GPreDrawResult_CulledInside;
			}
		}

		/*
			todo: see if we can integrate this with above... 
			
			 or precalc all b's? or recursive (b, inside b, inside b...) testing before handling a's?
		*/
		//	check objects that are inside other objects that shouldnt be shown
		for ( i=0;	i<m_ObjectInsideList.Size();	i++ )
		{
			//	a is inside b
			u32 a = m_ObjectInsideList[i] & 0xffff;			
			u32 b = (m_ObjectInsideList[i]>>16) & 0xffff;

			//	is a already marked to not be drawn?
			if ( Results[a] != GPreDrawResult_Draw && Results[a] != GPreDrawResult_Unknown )
				continue;

			GMapObject* pMapObject = GAssets::g_MapObjects.Find( m_MapObjects[b] );
			GMesh* pMesh = pMapObject ? pMapObject->GetMesh() : NULL;
			
			if ( !pMesh )
				continue;

			GDebug_Print("todo: check mesh inside mesh\n");
			/*
			//	do we want to cull this object, just because its going to be hidden by an object its inside?
			if ( ! pMesh->m_Bounds.Inside( pMapObject->m_Position, pCamera->m_Position ) )
			{
				//	b will be rendered, do we want to not bother rendering a? which is inside it
				if ( Results[b] == GPreDrawResult_Draw || Results[b] == GPreDrawResult_Unknown )
				{
					//	check flags
					if ( !(pMapObject->m_Flags & GMapObjectFlags::ShowObjectsInside) )
					{
						//	cull as outside object(b) will hide a anyway
						Results[a] = GPreDrawResult_IsInside;
					}
				}
			}
			*/
		}
	}

	//	do a cull check on any remaining unchecked objects
	for ( i=0;	i<m_MapObjects.Size();	i++ )
	{
		//	not checked yet
		if ( Results[i] == GPreDrawResult_Unknown )
		{
			//	check culling of map object
			GMapObject* pMapObject = GAssets::g_MapObjects.Find( m_MapObjects[i] );
			GMesh* pMesh = pMapObject ? pMapObject->GetMesh() : NULL;
					
			//	object missing!?
			if ( !pMapObject || !pMesh )
			{
				Results[i] = GPreDrawResult_NoObject;
				continue;
			}
			
			//GDebug_Print("todo: mesh cull test\n");
			Results[i] = GPreDrawResult_Draw;
			/*
			//	check culled state
			if ( pMesh->m_Bounds.IsCulled( pMapObject->m_Position, pCamera ) )
			{
				Results[i] = GPreDrawResult_Culled;
			}
			else
			{
				Results[i] = GPreDrawResult_Draw;
			}
			*/
		}

		//	add to visible list if we're going to be drawing it
		if ( Results[i] == GPreDrawResult_Draw )
		{
			VisibleMapObjects.Add( (SubMapIndex<<16) | i );
		}
	}

}


void GSubMap::PreDrawGameObjects(GCamera* pCamera, GWorld& World, GList<u32>& VisibleGameObjects, GList<GPreDrawResult>& MapObjectPreDrawResults, int SubMapIndex)
{
	//	initialise results

	//	extended info in case we need it
	GList<GPreDrawResult> PreDrawResults;
	PreDrawResults.Resize( World.m_pSubmapObjectList[SubMapIndex].Size() );
	PreDrawResults.SetAll( GPreDrawResult_Unknown );

	/*
		TODO: dont draw game objects INSIDE mapobjects that arent being drawn
	*/

	//	go through each world object and check if its visible
	for ( int i=0;	i<World.m_pSubmapObjectList[SubMapIndex].Size();	i++ )
	{
		//	get the object
		GGameObject* pGameObject = World.m_pSubmapObjectList[SubMapIndex][i];
		if ( !pGameObject )
		{
			//	no object, no draw
			PreDrawResults[i] = GPreDrawResult_NoObject;
			continue;
		}

		//	check culling
		if ( pGameObject->GetBounds().IsCulled( pGameObject->m_Position, pCamera ) )
		{
			PreDrawResults[i] = GPreDrawResult_Culled;
			continue;
		}

		//	must be visible
		PreDrawResults[i] = GPreDrawResult_Draw;

		//	we will draw this object
		VisibleGameObjects.Add( (SubMapIndex<<16) | i );
	}
}


void GSubMap::GetVisiblePortals(GCamera* pCamera, GList<u32>& VisiblePortals, int SubMapIndex)
{
	//	build list of visible portals
	/*
	 TODO
	*/
	for ( int i=0;	i<m_Portals.Size();	i++ )
	{
		u32 NewPortal = ((u32)i);
		NewPortal |= ((u32)SubMapIndex)<<16;
		
		VisiblePortals.Add( NewPortal );
	}
}


void GSubMap::Draw(u32 DrawFlags)
{
	GIncCounter(DrawSubMaps,1);

	//	lists for predraw stuff
	GList<GPreDrawResult> MapObjectPreDrawResults;
	GList<u32> VisiblePortals;
	GList<u32> VisibleMapObjects;

	//	specify submap index as 0, so all elements in these u32's will just be indexes
	int SubmapIndex = 0;
	PreDrawMapObjects( GCamera::g_pActiveCamera, VisibleMapObjects, MapObjectPreDrawResults, SubmapIndex );
	GetVisiblePortals( GCamera::g_pActiveCamera, VisiblePortals, SubmapIndex );

	int i;
	
	//	render map objects
	for ( i=0;	i<VisibleMapObjects.Size();	i++ )
	{
		int MapObjIndex = VisibleMapObjects[i];
		GMapObject* pMapObject = GAssets::g_MapObjects.Find( m_MapObjects[ MapObjIndex ] );
		if ( pMapObject )
		{
			u32 ObjectDrawFlags = 0x0;

			//	if we've already done a cull test dont do any more
			if ( MapObjectPreDrawResults[MapObjIndex] != GPreDrawResult_Unknown )
				ObjectDrawFlags |= GDrawInfoFlags::DontCullTest;

			GDrawResult Result = pMapObject->Draw( DrawFlags|ObjectDrawFlags );

			if ( Result == GDrawResult_Drawn )
			{
				GIncCounter(DrawMapObjects,1);
			}
		}
	}

	//	render portals
	for ( i=0;	i<VisiblePortals.Size();	i++ )
	{
		int PortalIndex = VisiblePortals[i];
		GMapPortal& Portal = m_Portals[ PortalIndex ];

		if ( DrawFlags & GDrawInfoFlags::DebugPortals )
		{
			Portal.DebugDraw();
		}
	}

}



void GSubMap::DrawShadow(u32 DrawFlags)
{
	/*
	//	lists for predraw stuff
	GList<GPreDrawResult> MapObjectPreDrawResults;
	GList<u32> VisiblePortals;
	GList<u32> VisibleMapObjects;

	//	specify submap index as 0, so all elements in these u32's will just be indexes
	int SubmapIndex = 0;
	PreDrawMapObjects( GCamera::g_pActiveCamera, VisibleMapObjects, MapObjectPreDrawResults, SubmapIndex );
	GetVisiblePortals( GCamera::g_pActiveCamera, VisiblePortals, SubmapIndex );

	int i;
	
	//	render map objects
	if ( ( DrawFlags & GDrawInfoFlags::DisableMapObjShadows ) == 0x0 )
	{
		for ( i=0;	i<VisibleMapObjects.Size();	i++ )
		{
			int MapObjIndex = VisibleMapObjects[i];
			GMapObject* pMapObject = GAssets::g_MapObjects.Find( m_MapObjects[ MapObjIndex ] );
			if ( pMapObject )
			{
				u32 ObjectDrawFlags = 0x0;

				//	if we've already done a cull test dont do any more
				if ( MapObjectPreDrawResults[MapObjIndex] != GPreDrawResult_Unknown )
					ObjectDrawFlags |= GDrawInfoFlags::DontCullTest;

				//	draw shadow
				GMapLight MapLight;
				GetLight( MapLight, pMapObject->m_Position );
				GDrawResult Result = pMapObject->DrawShadow( DrawFlags|ObjectDrawFlags, &MapLight );
			}
		}
	}
	*/
}


void GSubMap::GenerateBounds(Bool Force)
{
	BuildObjectInsideList();
}


void GSubMap::BuildObjectInsideList()
{
	//	empty the current list
	m_ObjectInsideList.Empty();

	GDebug_Print("todo: object inside object test\n");
/*
	int a;
	int b;
	
	//	compare each mapobject to each other
	for ( a=0;	a<m_MapObjects.Size();	a++ )
	{
		GMapObject* pMapObjectA = GAssets::g_MapObjects.Find( m_MapObjects[a] );
		if ( !pMapObjectA )
			continue;

		GMesh* pMeshA = pMapObjectA->GetMesh();
		if ( !pMeshA )
			continue;

		if ( !pMeshA->m_Bounds.IsValid() )
			continue;

		//	check against other objects
		for ( b=0;	b<m_MapObjects.Size();	b++ )
		{
			if ( a == b )
				continue;

			GMapObject* pMapObjectB = GAssets::g_MapObjects.Find( m_MapObjects[b] );
			if ( !pMapObjectB )
				continue;

			GMesh* pMeshB = pMapObjectB->GetMesh();
			if ( !pMeshB )
				continue;

			if ( !pMeshB->m_Bounds.IsValid() )
				continue;

			//	is a inside b?
			if ( pMeshB->m_Bounds.Inside( pMapObjectB->m_Position, pMeshA->m_Bounds, pMapObjectA->m_Position ) )
			{
				u32 AinBValue = ((u32)a) | (((u32)b)<<16);
				m_ObjectInsideList.Add( AinBValue );
			}

		}
	}
*/
}

	
Bool GSubMap::ObjectInsideObject(int ObjectA, int ObjectB)
{
	//	check indexes
	GDebug_CheckIndex( ObjectA, 0, m_MapObjects.Size() );
	GDebug_CheckIndex( ObjectB, 0, m_MapObjects.Size() );

	for ( int i=0;	i<m_ObjectInsideList.Size();	i++ )
	{
		u32 a = m_ObjectInsideList[i] & 0xffff;
		u32 b = (m_ObjectInsideList[i]>>16) & 0xffff;

		//	there IS an entry saying A is inside B
		if ( (int)a == ObjectA && (int)b == ObjectB )
			return TRUE;

		//	any following checks will have a larger a than we're looking for
		if ( (int)a > ObjectA )
			return FALSE;
	}

	//	no matches
	return FALSE;
}


Bool GSubMap::PositionInside(float3& Position)
{
	//	is this position inside the bounds of any of our mapobjects?
	for ( int i=0;	i<m_MapObjects.Size();	i++ )
	{
		GMapObject* pMapObject = GAssets::g_MapObjects.Find( m_MapObjects[i] );
		if ( !pMapObject )
			continue;

		GMesh* pMesh = pMapObject->GetMesh();
		if ( !pMesh )
			continue;

		GDebug_Print("todo: mesh inside submap test\n");
		return TRUE;	
		/*
		//	is this position inside this mapobject's, object's bounds?
		if ( pMesh->m_Bounds.Inside( pMapObject->m_Position, Position ) )
			return TRUE;
		*/
	}

	return FALSE;
}


float3 GSubMap::GetSubmapCenter()
{
	//	collect the centers of all the map objects
	float3	Center(0,0,0);
	int Divide = 0;

	for ( int i=0;	i<m_MapObjects.Size();	i++ )
	{
		GMapObject* pMapObject = GAssets::g_MapObjects.Find( m_MapObjects[i] );
		if ( !pMapObject )
			continue;

		GBounds& Bounds = pMapObject->GetBounds();

		//	is this position inside this mapobject's, object's bounds?
		Center += ( pMapObject->m_Position + Bounds.m_Offset );
		Divide++;
	}

	//	no centers collected...
	if ( !Divide )
		return Center;

	//	get average center
	Center /= (float)Divide;
	
	return Center;
}


Bool GSubMap::ChangeMapObjectRef(GAssetRef MapObjectRef,GAssetRef NewRef )
{
	int OldIndex = GetMapObjectIndex(MapObjectRef);
	int NewIndex = GetMapObjectIndex(NewRef);

	//	old object never existed!
	if ( OldIndex == -1 )
		return FALSE;

	//	new object already exists!
	if ( NewIndex != -1 )
		return FALSE;

	//	no change
	if ( MapObjectRef == NewRef )
		return FALSE;

	//	change ref
	m_MapObjects[OldIndex] = NewRef;

	//	rebuild inside list to include new mapobject
	BuildObjectInsideList();
	
	return TRUE;
}

void GSubMap::RemoveMapObject(GAssetRef MapObjectRef)
{
	int Index = m_MapObjects.FindIndex( MapObjectRef );

	//	not found, ERROR!
	if ( Index == -1 )
	{
		GDebug_Break("Couldnt find mapobject to remove\n");
		return;
	}
	
	//	remove from list
	m_MapObjects.RemoveAt( Index );

	//	rebuild object-inside-object lists
	BuildObjectInsideList();
}


void GSubMap::AddMapObject(GAssetRef MapObjectRef)
{
	int Index = m_MapObjects.FindIndex( MapObjectRef );

	//	error already exists!
	if ( Index != -1 )
	{
		GDebug_Break("This mapobject already exists in this submap\n");
		return;
	}
	
	//	add to list
	m_MapObjects.Add( MapObjectRef );

	//	rebuild object-inside-object lists
	BuildObjectInsideList();
}




int GSubMap::GetPortalIndex(GAssetRef PortalRef)
{
	for ( int i=0;	i<m_Portals.Size();	i++ )
	{
		if ( m_Portals[i].m_PortalRef == PortalRef )
			return i;
	}

	return -1;
}


GMapPortal* GSubMap::GetPortal(GAssetRef PortalRef)
{
	int PortalIndex = GetPortalIndex(PortalRef);

	return ( PortalIndex == -1 ? NULL : &m_Portals[PortalIndex] );
}


Bool GSubMap::ChangePortalRef(GAssetRef PortalRef,GAssetRef NewRef )
{
	int OldIndex = GetPortalIndex(PortalRef);
	int NewIndex = GetPortalIndex(NewRef);

	//	old portal never existed!
	if ( OldIndex == -1 )
		return FALSE;

	//	new portal already exists!
	if ( NewIndex != -1 )
		return FALSE;

	//	no change
	if ( PortalRef == NewRef )
		return FALSE;

	//	change ref
	m_Portals[OldIndex].m_PortalRef = NewRef;
	
	return TRUE;
}


void GSubMap::DeletePortal(GAssetRef PortalRef)
{
	int Index = GetPortalIndex(PortalRef);
	if ( Index == -1 )
	{
		GDebug_Break("Tried to delete non-existant portal\n");
		return;
	}

	//	remove from list
	m_Portals.RemoveAt( Index );
}


GAssetRef GSubMap::GetFreePortalRef()
{
	GAssetRef FreeAssetRef = GAssetRef_Invalid;

	while ( GetPortalIndex( FreeAssetRef ) != -1 )
	{
		IncrementAssetRef( FreeAssetRef );
	}

	return FreeAssetRef;
}


void GSubMap::AddPortal(GMapPortal& NewPortal)
{
	int Index = GetPortalIndex( NewPortal.m_PortalRef );

	//	error already exists!
	if ( Index != -1 )
	{
		GDebug_Break("A portal with this asset ref already exists in this submap\n");
		return;
	}
	
	//	add to list
	m_Portals.Add( NewPortal );
}


//-------------------------------------------------------------------------
//	fill in the light struct for this position. either by the nearest or
//	generating an average light source
//	TODO: merge local lights etc
//-------------------------------------------------------------------------
void GSubMap::GetLight(GMapLight& Light, float3& Pos)
{
	//	no lights in map? use default
	if ( m_Lights.Size() == 0 )
	{
		GMapLight DefaultLight;
		DefaultLight.m_Pos = GCamera::g_pActiveCamera->m_Position;
		Light.Copy( DefaultLight );
	}
	else if ( m_Lights.Size() == 1 )
	{
		//	only one light, use that
		Light.Copy( m_Lights[0] );
	}
	else
	{
		//	get nearest light
		int NearLightIndex = 0;
		float3 PosDiff;
		PosDiff = Pos;
		PosDiff -= m_Lights[0].m_Pos;
		float NearLightDistSq = PosDiff.LengthSq();
		for ( int i=0;	i<m_Lights.Size();	i++ )
		{
			float LightDistSq = (Pos - m_Lights[i].m_Pos).LengthSq();
			if ( LightDistSq < NearLightDistSq )
			{
				NearLightIndex = i;
			}
		}

		//	use nearest light
		Light.Copy( m_Lights[ NearLightIndex ] );
	}

}


//------------------------------------------------




GMap::GMap()
{
}


GMap::~GMap()
{
	//	delete submap assets
	m_SubMaps.Empty();
}


Bool GMap::Load(GBinaryData& Data)
{
	//	read a block of data, update sizes and data pointer
	/*
	#define READ_BLOCK( addr, size )					\
	{													\
		if ( (int)DataSize < (int)(size) )				\
		{												\
			GDebug::Print("MapObject is missing data. requested %d, remaining %d\n",size,DataSize);	\
			return FALSE;								\
		}												\
		memcpy( addr, pData, size );					\
		pData += size;									\
		DataSize -= size;								\
		DataRead += size;								\
	}
	*/

	//	read-in the header
	GMapHeader Header;
	//READ_BLOCK( &Header, GDataSizeOf(GMapHeader) );
	if ( !Data.Read ( &Header, GDataSizeOf(GMapHeader), "Map header" ) )
		return FALSE;
	
	//	make a new list of map objects
	m_SubMaps.Empty();
	int SubMapObjectRefs = Header.SubMapCount;

	while ( SubMapObjectRefs > 0 )
	{
		SubMapObjectRefs--;

		//	sub map data follows directly after our map header
		GAssetHeader AssetHeader;
		//READ_BLOCK( &AssetHeader, GDataSizeOf(GAssetHeader) );
		if ( !Data.Read( &AssetHeader, GDataSizeOf(GAssetHeader), "Submap asset header" ) )
			return FALSE;

		//	check its a submap asset
		if ( AssetHeader.AssetType != GAssetSubMap )
		{
			//	cant load this data, skip over it
			if ( !Data.Skip( AssetHeader.BlockSize ) )
				return FALSE;
			//pData += AssetHeader.BlockSize;
			//DataSize -= AssetHeader.BlockSize;
			//DataRead += AssetHeader.BlockSize;
			GDebug_Print("Expecting submap asset(%d), found asset type (%d, version 0x%08x)\n",GAssetSubMap, AssetHeader.AssetType, AssetHeader.AssetVersion);
			continue;
		}

		GSubMap* pNewSubMap = new GSubMap;
		int PreLoadDataPos = Data.GetReadPos();

		if ( ! pNewSubMap->LoadAsset( &AssetHeader, Data ) )
		{
			GDebug_Print("Failed to load submap in map\n");

			//	delete useless asset
			delete pNewSubMap;

			//	skip over data
			Data.SetReadPos( PreLoadDataPos + AssetHeader.BlockSize );
		}
		else
		{
			//	loaded okay, add to list of data
			m_SubMaps.Add( pNewSubMap );
		}
	}

	//#undef READ_BLOCK
	return TRUE;
}


Bool GMap::Save(GBinaryData& Data)
{
	//	add header
	GMapHeader Header;
	Header.SubMapCount = m_SubMaps.Size();
	Data.Write( &Header, GDataSizeOf(GMapHeader) );

	//	write the asset, like an asset, after our header
	for ( int i=0;	i<m_SubMaps.Size();	i++ )
	{
		GSubMap* pSubMap = m_SubMaps[i];

		//	write an assset header
		GAssetHeader AssetHeader;
		AssetHeader.AssetType		= pSubMap->AssetType();
		AssetHeader.AssetVersion	= pSubMap->Version();
		AssetHeader.AssetRef		= pSubMap->m_AssetRef;
		AssetHeader.BlockSize		= 0;

		//	save submap
		GBinaryData SubmapData;

		if ( ! pSubMap->Save(SubmapData) )
		{
			GDebug_Print("Error saving submap in map\n");
		}

		//	set data size
		AssetHeader.BlockSize = SubmapData.Size();

		//	add asset header, then submap data
		Data.Write( &AssetHeader, GDataSizeOf(GAssetHeader) );
		Data.Write( SubmapData );

		//	delete redundant submap data
		SubmapData.Empty();
	}

	return TRUE;
}


int GMap::SubmapOn(float3& Position)
{
	//	todo: redo this

	//	only one submap
	if ( m_SubMaps.Size() == 1 )
		return 0;
	
	//	check each submap
	for ( int i=0;	i<m_SubMaps.Size();	i++ )
	{
		if ( m_SubMaps[i]->PositionInside(Position) )
			return i;
	}

	return -1;
}

int GMap::SubmapNearest(float3& Position)
{
	//	todo: redo this

	
	//	no submaps
	if ( m_SubMaps.Size() < 1 )
		return -1;

	//	only one submap
	if ( m_SubMaps.Size() == 1 )
		return 0;

	//	get the distances from the center of each submap
	GList<float>	SubmapDistances;
	SubmapDistances.Resize( m_SubMaps.Size() );
	int i;
	
	for ( i=0;	i<m_SubMaps.Size();	i++ )
	{
		float3 Offset = Position - m_SubMaps[i]->GetSubmapCenter();
		SubmapDistances[i] = Offset.LengthSq();
	}

	//	get shortest
	int ShortestDist = 0;

	for ( i=1;	i<m_SubMaps.Size();	i++ )
		if ( SubmapDistances[i] < SubmapDistances[ShortestDist] )
			ShortestDist = i;

	return ShortestDist;
}


int GMap::GetSubmapIndex(GAssetRef SubmapRef)
{
	for ( int i=0;	i<m_SubMaps.Size();	i++ )
	{
		if ( m_SubMaps[i]->AssetRef() == SubmapRef )
			return i;
	}

	return -1;
}

GSubMap* GMap::GetSubMap(GAssetRef SubmapRef)
{
	int Index = GetSubmapIndex( SubmapRef );
	if ( Index < 0 )
		return NULL;

	return m_SubMaps[Index];
}


void GMap::GenerateBounds(Bool Force)
{
	//	generate bounds for submaps
	for ( int i=0;	i<m_SubMaps.Size();	i++ )
	{
		m_SubMaps[i]->GenerateBounds(Force);
	}
}

void GMap::Draw(u32 DrawFlags)
{
	int i;

	//	draw submaps
	for ( i=0;	i<m_SubMaps.Size();	i++ )
	{
		m_SubMaps[i]->Draw( DrawFlags );
	}

	//	draw shadows
	for ( i=0;	i<m_SubMaps.Size();	i++ )
	{
		m_SubMaps[i]->DrawShadow( DrawFlags );
	}
}

void GMap::DeleteSubMap(GAssetRef SubMapRef)
{
	int Index = GetSubmapIndex(SubMapRef);

	if ( Index == -1 )
	{
		GDebug_Break("Failed to find submap to delete\n");
		return;
	}

	//	grab pointer from list
	GSubMap* pSubMap = m_SubMaps[Index];
	m_SubMaps.RemoveAt( Index );

	//	now delete
	GDelete( pSubMap );
}


GMapLight* GMap::GetLight(float3& Pos, int Submap)
{
	//	get nearest light
	int SubMapIndex = Submap == -1 ? SubmapOn( Pos ) : Submap;

	//	pos not on a map
	if ( SubMapIndex == -1 )
		return NULL;

	//	grab light
	static GMapLight g_MapLight;
	m_SubMaps[ SubMapIndex ]->GetLight( g_MapLight, Pos );
	return &g_MapLight;
}


//------------------------------------------------

void GMapPortal::MakeCamera(GCamera& PortalCamera,GCamera* pParentCamera)
{
	//	copy near/far values
	PortalCamera.m_NearZ = pParentCamera->m_NearZ;
	PortalCamera.m_FarZ = pParentCamera->m_FarZ;	//	1000

	//	
	PortalCamera.m_CameraFlags = GCameraFlags::Clear | GCameraFlags::ClearZ;
	PortalCamera.m_ClearColour = float4( 1.f, 0.f, 0.f, 1.f );

	//	set default viewport
	PortalCamera.m_Viewport = pParentCamera->m_Viewport;

	//	set viewport to the texture's dimensions if this portal gets rendered to a texture
	if ( m_Type == GPortal_Mirror )
	{
		GTexture* pTexture = GAssets::g_Textures.Find( m_Texture );
		if ( pTexture )
			PortalCamera.m_Viewport = int4( 0, 0, pTexture->m_Size.x, pTexture->m_Size.y ); 
	}

	/*
		TODO: need to invert direction depending on which side our parent camera is on.
	*/

	//	reflect camera direction off the normal
	float3 PortalNormal = pParentCamera->GetFowardVector().Normal();

	if ( m_Type == GPortal_Mirror )
	{
		float3 CameraDir = pParentCamera->GetFowardVector().Normal();
		CameraDir.Reflect( m_PortalNormal );
		PortalNormal = CameraDir;
		PortalNormal.Normalise();
	}

	//	set camera lookat
	PortalCamera.m_LookAt = PortalCamera.m_Position + PortalNormal;

	//	set center of new camera to the center of our portal
	PortalCamera.m_Position = PortalCenter();

	//	todo: work out w/h from lengths to edges
	//	temp: find longest length to a corner
	float LongestLenSq = ( PortalCamera.m_Position - m_PortalVerts[0] ).LengthSq();
	for ( int i=1;	i<4;	i++ )
	{
		float LenSq = ( PortalCamera.m_Position - m_PortalVerts[i] ).LengthSq();
		if ( LenSq > LongestLenSq )
			LongestLenSq = LenSq;
	}

//	float PortalWidth = sqrtf(LongestLenSq * 0.5f);
//	float PortalHeight = sqrtf(LongestLenSq * 0.5f);
	float PortalWidth = 1.f;
	float PortalHeight = 1.f;

	//	setup orthographic projection to world space dimensions
	PortalCamera.m_OrthoMin.x = -PortalWidth;
	PortalCamera.m_OrthoMin.y = -PortalHeight;
	PortalCamera.m_OrthoMax.x = PortalWidth;
	PortalCamera.m_OrthoMax.y = PortalHeight;

	//	calculate culling plane
	PortalCamera.CalcFrustumPlanes( 4, &m_PortalVerts[0], pParentCamera->m_Position );

}


float3 GMapPortal::PortalCenter()
{
	//	add all verts and divide
	float3 Center = m_PortalVerts[0] + m_PortalVerts[1] + m_PortalVerts[2] + m_PortalVerts[3];
	Center /= 4.f;
	return Center;	
}


void GMapPortal::DebugDraw()
{
/*
	g_Display->DebugLine( m_PortalVerts[0], GDisplay::g_DebugColour, m_PortalVerts[1] );
	g_Display->DebugLine( m_PortalVerts[1], GDisplay::g_DebugColour, m_PortalVerts[3] );
	g_Display->DebugLine( m_PortalVerts[3], GDisplay::g_DebugColour, m_PortalVerts[2] );
	g_Display->DebugLine( m_PortalVerts[2], GDisplay::g_DebugColour, m_PortalVerts[0] );

	g_Display->DebugLine( m_PortalVerts[1], GDisplay::g_DebugColour, m_PortalVerts[2] );
	g_Display->DebugLine( m_PortalVerts[0], GDisplay::g_DebugColour, m_PortalVerts[3] );
*/
	g_Display->DebugLine( m_PortalVerts[0], GDisplay::g_DebugColour, m_PortalVerts[1] );
	g_Display->DebugLine( m_PortalVerts[1], GDisplay::g_DebugColour, m_PortalVerts[2] );
	g_Display->DebugLine( m_PortalVerts[2], GDisplay::g_DebugColour, m_PortalVerts[3] );
	g_Display->DebugLine( m_PortalVerts[3], GDisplay::g_DebugColour, m_PortalVerts[0] );

	//	draw normal
	float3 Center = PortalCenter();
	g_Display->DebugLine( Center, GDisplay::g_DebugColour, Center + m_PortalNormal );
	g_Display->DebugLine( Center, GDisplay::g_DebugColour, Center - m_PortalNormal );
}


//-------------------------------------------------------------------------
//	Genreates the portal's vert data from a mesh. 
//	currently just grabs the first 4 verts. could do with improvement if we start to use it seriously
//-------------------------------------------------------------------------
void GMapPortal::GenerateFromMesh(GAssetRef MeshRef)
{
	GMesh* pMesh = GAssets::g_Meshes.Find( MeshRef );

	//	cant find mesh
	if ( !pMesh )
	{
		return;
	}

	//	grab first 4 verts
	if ( pMesh->VertCount() < 4 )
	{
		GDebug::Print("Error, mesh doesnt have enough verts(%d) to make up a portal(4)\n", pMesh->VertCount() );
		return;
	}

	static int base = 0;
	
	m_PortalVerts[0] = pMesh->m_Verts[0+base];
	m_PortalVerts[1] = pMesh->m_Verts[1+base];
	m_PortalVerts[2] = pMesh->m_Verts[2+base];
	m_PortalVerts[3] = pMesh->m_Verts[3+base];

	base += 1;
	if ( base >= pMesh->VertCount() )
		base = 0;

	//	generate normal
	m_PortalNormal = CalcNormal( m_PortalVerts[0], m_PortalVerts[1], m_PortalVerts[2] );

}


void GMapPortal::ReOrderVerts(int VertNo)
{
	static int g_Cycle = 0;

	if ( g_Cycle == VertNo )
	{
		g_Cycle ++;
		if ( g_Cycle == 4 )
			g_Cycle = 0;
	}

	//	swap vertno with cycle no
	float3 tmp = m_PortalVerts[VertNo];
	m_PortalVerts[VertNo] = m_PortalVerts[g_Cycle];
	m_PortalVerts[g_Cycle] = tmp;
}


void GMapPortal::MoveVerts(float3& Movement)
{
	for ( int i=0;	i<4;	i++ )
	{
		m_PortalVerts[i] += Movement;
	}
}


//------------------------------------------------

GMapLight::GMapLight()
{
	m_Pos		= float4( 0.f, 0.f, 0.f, 0.f );
	m_Colour	= float4( 0.5f, 0.5f, 0.5f, 1.f );
	m_Strength	= 0.5f;
}


