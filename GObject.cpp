/*------------------------------------------------

  GObject.cpp

	in-game object type. contains numerous sub objects which 
	define a mesh to draw



-------------------------------------------------*/
/*



//	Includes
//------------------------------------------------
#include "GObject.h"
#include "GDebug.h"
#include "GDisplay.h"
#include "GApp.h"
#include "GAssetList.h"



//	globals
//------------------------------------------------
const u32		GObject::g_Version = 0x44440005;


//	Definitions
//------------------------------------------------


GSubObject::GSubObject()
{
	m_Mesh			= GAssetRef_Invalid;
	m_Texture		= GAssetRef_Invalid;
	m_ShadowMesh	= GAssetRef_Invalid;
	m_CollisionMesh	= GAssetRef_Invalid;
	m_Colour		= float4( 1,1,1,1 );
	
	m_Flags = 0x0;
	m_Position		= float3( 0,0,0 );
	m_Rotation		= GQuaternion( float3(0,1,0), 0);
}




GSubObject::~GSubObject()
{
}


void GSubObject::Set(GAssetRef Mesh, GAssetRef Texture, GAssetRef ShadowMesh, GAssetRef CollisionMesh)
{
	m_Mesh			= Mesh;
	m_Texture		= Texture;
	m_ShadowMesh	= ShadowMesh;
	m_CollisionMesh	= CollisionMesh;

	if ( m_ShadowMesh == GAssetRef_Invalid )
		m_ShadowMesh = Mesh;

	if ( m_CollisionMesh == GAssetRef_Invalid )
		m_CollisionMesh = Mesh;
};




Bool GSubObject::Load(u8* pData,int DataSize,int& DataRead)
{
	DataRead = 0;

	#define READ_VAR(var)						\
	{											\
		if ( DataSize < GDataSizeOfVar( var ) )			\
		{										\
			GDebug::Print("Subobject is missing data. requested %d, remaining %d\n",GDataSizeOfVar(var),DataSize);	\
			return FALSE;						\
		}										\
		memcpy( &var, pData, GDataSizeOfVar( var ) );	\
		pData += GDataSizeOfVar( var );					\
		DataSize -= GDataSizeOfVar( var );				\
		DataRead += GDataSizeOfVar( var );				\
	}											

	READ_VAR( m_Mesh );
	READ_VAR( m_Texture );
	READ_VAR( m_ShadowMesh );
	READ_VAR( m_CollisionMesh );
	READ_VAR( m_Flags );
	READ_VAR( m_Position );
	READ_VAR( m_Rotation );
	READ_VAR( m_Colour );
	
	#undef READ_VAR

	return TRUE;
}



void GSubObject::Save(GList<u8>& SaveData)
{
	#define SAVE_VAR(var)						\
	{											\
		SaveData.Add( (u8*)&var, GDataSizeOfVar(var) / GDataSizeOf(u8) );	\
	}											\

	SAVE_VAR( m_Mesh );
	SAVE_VAR( m_Texture );
	SAVE_VAR( m_ShadowMesh );
	SAVE_VAR( m_CollisionMesh );
	SAVE_VAR( m_Flags );
	SAVE_VAR( m_Position );
	SAVE_VAR( m_Rotation );
	SAVE_VAR( m_Colour );
	
	#undef SAVE_VAR
}






//------------------------------------------------






GObject::GObject()
{
}



GObject::~GObject()
{
	DeleteSubObjects();
}




Bool GObject::Load(u8* pData,int DataSize,int& DataRead)
{
	//	read a block of data, update sizes and data pointer
	#define READ_BLOCK( addr, size )					\
	{													\
		if ( (int)DataSize < (int)(size) )				\
		{												\
			GDebug::Print("Object is missing data. requested %d, remaining %d\n",size,DataSize);	\
			return FALSE;								\
		}												\
		memcpy( addr, pData, size );					\
		pData += size;									\
		DataSize -= size;								\
		DataRead += size;								\
	}

	//	read-in the header
	GObjectHeader Header;
	READ_BLOCK( &Header, GDataSizeOf(GObjectHeader) );
	m_Bounds.m_Offset = Header.BoundsOffset;
	m_Bounds.m_Radius = Header.BoundsRadius;

	//	from the header, allocate data
	for ( int i=0;	i<Header.SubObjectCount;	i++ )
	{
		GSubObject* pSubObject = new GSubObject;
		
		int SubObjectDataRead = 0;
		if ( pSubObject->Load( pData, DataSize, SubObjectDataRead ) )
		{
			AddSubObject( pSubObject );
		}
		
		pData += SubObjectDataRead;
		DataRead += SubObjectDataRead;
		DataSize -= SubObjectDataRead;
	}

	#undef READ_BLOCK

	return TRUE;
}




Bool GObject::Save(GList<u8>& SaveData)
{
	//	add the object header
	GObjectHeader Header;
	Header.SubObjectCount	= m_SubObjects.Size();
	Header.BoundsOffset		= m_Bounds.m_Offset;
	Header.BoundsRadius		= m_Bounds.m_Radius;

	SaveData.Add( (u8*)&Header, GDataSizeOf(GObjectHeader) / GDataSizeOf(u8) );

	//	add each subobject
	for ( int i=0;	i<m_SubObjects.Size();	i++ )
	{
		m_SubObjects[i]->Save( SaveData );
	}

	return TRUE;
}




GSubObject* GObject::AddSubObject(GAssetRef Mesh, GAssetRef Texture, GAssetRef ShadowMesh, GAssetRef CollisionMesh)
{
	//	create new object
	GSubObject* pSubObject = new GSubObject;

	//	setup
	pSubObject->Set( Mesh, Texture, ShadowMesh, CollisionMesh );

	//	add to list
	m_SubObjects.Add( pSubObject );

	return pSubObject;
}



void GObject::DeleteSubObjects()
{
	//	delete elements in sub objects
	for ( int i=0;	i<m_SubObjects.Size();	i++ )
	{
		GDelete( m_SubObjects[i] );
	}

	m_SubObjects.Empty();
}








void GObject::ChangeMeshRef(GAssetRef MeshRef, GAssetRef NewRef )
{
	for ( int i=0;	i<m_SubObjects.Size();	i++ )
	{
		GSubObject* pSubObject = m_SubObjects[i];
		if ( pSubObject->m_Mesh == MeshRef )			pSubObject->m_Mesh = NewRef;
		if ( pSubObject->m_ShadowMesh == MeshRef )		pSubObject->m_ShadowMesh = NewRef;
		if ( pSubObject->m_CollisionMesh == MeshRef )	pSubObject->m_CollisionMesh = NewRef;
	}
}


void GObject::ChangeTextureRef(GAssetRef TextureRef, GAssetRef NewRef )
{
	for ( int i=0;	i<m_SubObjects.Size();	i++ )
	{
		GSubObject* pSubObject = m_SubObjects[i];
		if ( pSubObject->m_Texture == TextureRef )		pSubObject->m_Texture = NewRef;
	}
}




GDrawResult GObject::Draw(GDrawInfo& DrawInfo)
{
	//	modify flags
	DrawInfo.Flags |= GMesh::g_ForceFlagsOn;
	DrawInfo.Flags &= ~GMesh::g_ForceFlagsOff;

	if ( m_SubObjects.Size() == 0 )
		return GDrawResult_Nothing;
	
	GIncCounter( DrawObjects, 1 );

	//	translate to objects position
	g_Display->PushScene();
	g_Display->Translate( DrawInfo.Translation, DrawInfo.pRotation );

	//	setup draw info structure for subobjects
	GDrawInfo SubObjectDrawInfo;

	for ( int i=0;	i<m_SubObjects.Size();	i++ )
	{
		SubObjectDrawInfo = DrawInfo;

		GSubObject* pSubObject = m_SubObjects[i];
		GMesh* pMesh = GAssets::g_Meshes.Find( pSubObject->m_Mesh );
		if ( !pMesh )
			continue;

		/*
		//	outline any selected sub object (unless we're always outlining, in which case dont change flags)
		if ( ( DrawInfoFlags & GDrawInfoFlags::DebugOutline ) == 0x0 )
		{
			if ( i == HighlightSubObjectIndex )
				DrawInfo.Flags |= GDrawInfoFlags::DebugOutline;
			else
				DrawInfo.Flags &= ~GDrawInfoFlags::DebugOutline;
		}
		*/
/*
		//	modify colour
		if ( DrawInfo.Flags & GDrawInfoFlags::ForceColour )
		{
			//	keep colour
			//SubObjectDrawInfo.RGBA = SubObjectDrawInfo.RGBA;
		}
		else if ( DrawInfo.Flags & GDrawInfoFlags::MergeColourMult )
		{
			//	merge with subobject's colour
			SubObjectDrawInfo.RGBA *= pSubObject->m_Colour;
		}
		else if ( DrawInfo.Flags & GDrawInfoFlags::MergeColourAdd )
		{
			//	merge with subobject's colour
			SubObjectDrawInfo.RGBA += pSubObject->m_Colour;
		}
		else if ( DrawInfo.Flags & GDrawInfoFlags::MergeColourSub )
		{
			//	merge with subobject's colour
			SubObjectDrawInfo.RGBA -= pSubObject->m_Colour;
		}
		else
		{
			//	set colour to objects colour
			SubObjectDrawInfo.RGBA = pSubObject->m_Colour;
		}

		//	merge rotations if neccesary
		SubObjectDrawInfo.pRotation = &pSubObject->m_Rotation;

		//	merge out drawinfo with subobject drawinfo
		SubObjectDrawInfo.Translation	= pSubObject->m_Position;
		SubObjectDrawInfo.WorldPos		+= pSubObject->m_Position;
		SubObjectDrawInfo.TextureRef	= pSubObject->m_Texture;
		
		//	draw!
		pMesh->Draw( SubObjectDrawInfo );
	}



	//	dont write debug stuff to depth buffer
	glDisable(GL_DEPTH_TEST);


	//	draw bounds
	if ( DrawInfo.Flags & GDrawInfoFlags::DebugObjectBounds )
	{
		//	draw debug marker at objects position
		GDisplay::DebugPosition( DrawInfo.WorldPos, GDisplay::g_DebugColour, DrawInfo.pRotation ? *DrawInfo.pRotation : GQuaternion() );
		
		m_Bounds.DebugDraw( DrawInfo.WorldPos );
	}

	g_Display->PopScene();

	return GDrawResult_Drawn;
}




GDrawResult GObject::DrawShadow(GDrawInfo& DrawInfo)
{
	//	modify flags
	DrawInfo.Flags |= GMesh::g_ForceFlagsOn;
	DrawInfo.Flags &= ~GMesh::g_ForceFlagsOff;

	if ( m_SubObjects.Size() == 0 )
		return GDrawResult_Nothing;
	
	//	setup draw info structure for subobjects
	GDrawInfo SubObjectDrawInfo;

	for ( int i=0;	i<m_SubObjects.Size();	i++ )
	{
		SubObjectDrawInfo = DrawInfo;
		GSubObject* pSubObject = m_SubObjects[i];

		//	get shadow mesh if availible
		GMesh* pMesh = GAssets::g_Meshes.Find( pSubObject->m_ShadowMesh );
		if ( !pMesh )
		{
			pMesh = GAssets::g_Meshes.Find( pSubObject->m_Mesh );
		}
		
		//	couldnt find any mesh
		if ( !pMesh )
			continue;

		//	merge out drawinfo with subobject drawinfo
		SubObjectDrawInfo.Translation	= pSubObject->m_Position;
		SubObjectDrawInfo.WorldPos		+= pSubObject->m_Position;
		SubObjectDrawInfo.pRotation		= &pSubObject->m_Rotation;
		
		//	draw shadow
		pMesh->DrawShadowNew( SubObjectDrawInfo );
	}

	return GDrawResult_Drawn;
}


void GObject::GenerateBounds(Bool Force)
{
	//	do we need to generate bounds?
	if ( m_Bounds.m_Radius > 0.f && !Force )
		return;

	//	invalidate current bounds
	m_Bounds.m_Radius = 0.f;
	m_Bounds.m_Offset = float3(0,0,0);

	//	find the min max of all the meshes in the object
	float3 Min(0,0,0);
	float3 Max(0,0,0);

	for ( int i=0;	i<m_SubObjects.Size();	i++ )
	{
		GMesh* pMesh = GAssets::g_Meshes.Find( m_SubObjects[i]->m_Mesh );
		if ( !pMesh )
		{
			GDebug::Print("Warning: Mesh not found whilst generating object bounds\n");
			continue;
		}
		
		//	get the meshes min and max bounds
		float3 MeshMin;
		float3 MeshMax;
		pMesh->GetVertexMinMax( MeshMin, MeshMax );
	
		float3 FurthestVert = pMesh->GetFurthestVert();
		FurthestVert += m_SubObjects[i]->m_Position;

		float FurthestLengthSq = FurthestVert.LengthSq();
		if ( FurthestLengthSq > (m_Bounds.m_Radius*m_Bounds.m_Radius) )
			m_Bounds.m_Radius = sqrtf(FurthestLengthSq);

		//	add on the relative vertex offset (should probably rotate too...)
		MeshMin += m_SubObjects[i]->m_Position;
		MeshMax += m_SubObjects[i]->m_Position;

		//	update min and max
		for ( int j=0;	j<3;	j++ )
		{
			if ( MeshMin[j] < Min[j] || i==0 )	Min[j] = MeshMin[j];
			if ( MeshMax[j] > Max[j] || i==0 )	Max[j] = MeshMax[j];
		}
	
		float3 DirToMesh = m_SubObjects[i]->m_Position;
		float LengthToMeshSq = DirToMesh.LengthSq();

		//	add on the meshes own bounds
		if ( pMesh->m_Bounds.IsValid() )
		{
			DirToMesh += pMesh->m_Bounds.m_Offset;
			LengthToMeshSq = DirToMesh.LengthSq() + ( pMesh->m_Bounds.m_Radius*pMesh->m_Bounds.m_Radius );
		}
		
		//	is this length longest so far?
		if ( LengthToMeshSq > m_Bounds.m_Radius )
			m_Bounds.m_Radius = sqrtf(LengthToMeshSq);
	}

	//	the offset is the middle of the minimum and maximum. if its an equal min and max, the middle will be 0,0,0
	m_Bounds.m_Offset.x = (Max.x + Min.x) / 2.f;
	m_Bounds.m_Offset.y = (Max.y + Min.y) / 2.f;
	m_Bounds.m_Offset.z = (Max.z + Min.z) / 2.f;

	//	set the radius to the biggest offset from our middle. (doesnt matter whether we use min or max, middle is in the middle of both)
/*
	m_Bounds.m_Radius = Max.x - m_Bounds.m_Offset.x;

	if ( Max.y - m_Bounds.m_Offset.y > m_Bounds.m_Radius )
		m_Bounds.m_Radius = Max.y - m_Bounds.m_Offset.y;
	
	if ( Max.z - m_Bounds.m_Offset.z > m_Bounds.m_Radius )
		m_Bounds.m_Radius = Max.z - m_Bounds.m_Offset.z;

/*
	//	base bounds on the furthest mesh bounds edge
	for ( int i=0;	i<m_SubObjects.Size();	i++ )
	{
		GMesh* pMesh = GAssets::g_Meshes.Find( m_SubObjects[i]->m_Mesh );
		if ( !pMesh )
			continue;

		float3 DirToMesh = m_SubObjects[i]->m_Position;
		float LengthToMeshSq = DirToMesh.LengthSq();

		//	add on the meshes own bounds
		if ( pMesh->m_Bounds.IsValid() )
		{
			DirToMesh += pMesh->m_Bounds.m_Offset;
			LengthToMeshSq = DirToMesh.LengthSq() + ( pMesh->m_Bounds.m_Radius*pMesh->m_Bounds.m_Radius );
		}
		
		//	is this length longest so far?
		if ( LengthToMeshSq > m_Bounds.m_Radius )
			m_Bounds.m_Radius = sqrtf(LengthToMeshSq);
	}
	*/
	/*
}








*/
