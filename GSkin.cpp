/*------------------------------------------------

  GSkin.cpp

	Mesh skinning implementation.


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GSkeleton.h"
#include "GDisplay.h"
#include "GApp.h"
#include "GMesh.h"
#include "GAssetList.h"
#include "GFile.h"
#include "GWorld.h"


//	globals
//------------------------------------------------
const u32 GSkin::g_Version	= 0x99990004;

extern const char g_SkinShaderARB[];

#define BONEINDEX_ATTRIB	(GMesh::FREE0_ATTRIB_INDEX)	//	attrib number in shader for bone matrix index
#define ANIMATE_STEP		(1.f)						//	how many frames do we advance per frame (*60 in a second)

//	Definitions
//------------------------------------------------


//-------------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------------
GSkin::GSkin()
{
	m_Skeleton	= GAssetRef_Invalid;
	m_Mesh		= GAssetRef_Invalid;
}

//-------------------------------------------------------------------------
//	Load skin
//-------------------------------------------------------------------------
Bool GSkin::Load(GBinaryData& Data)
{
	/*
	#define LOAD_DATA(ptr,size)	\
	{															\
		if ( DataSize < size )							\
		{														\
			GDebug_Break("Not enough data for skin var\n");	\
			return FALSE;										\
		}														\
		memcpy( ptr, pData, size );						\
		pData += size;									\
		DataSize -= size;								\
		DataRead += size;								\
	}														
	#define LOAD_VAR(var)	LOAD_DATA( &var, GDataSizeOfVar( var ) )
	*/

	//	read header
	GSkinHeader Header;
	//LOAD_VAR( Header );
	if ( !Data.Read( &Header, GDataSizeOf(GSkinHeader), "Skin header" ) )
		return FALSE;

	//	take data from header
	m_Mesh = Header.MeshRef;
	m_Skeleton = Header.SkeletonRef;

	//	load in additional data
	if ( Header.DataFlags & GSkinHeaderDataFlags::BoneVertexLinks )
	{
		u16 BoneCount;
		//LOAD_VAR( BoneCount );
		if ( !Data.Read( &BoneCount, GDataSizeOf(u16), "Skin BoneVertex size" ) )
			return FALSE;

		m_BoneVertexList.Resize( BoneCount );
		for ( int b=0;	b<m_BoneVertexList.Size();	b++ )
		{
			u32 VertexCount = 0;
			//LOAD_VAR( VertexCount );
			if ( !Data.Read( &VertexCount, GDataSizeOf(u32), "Skin BoneVertex bone size" ) )
				return FALSE;

			m_BoneVertexList[b].Vertexes.Resize( VertexCount );
			//LOAD_DATA( m_BoneVertexList[b].Vertexes.Data(), m_BoneVertexList[b].Vertexes.DataSize() );
			if ( !Data.Read( m_BoneVertexList[b].Vertexes.Data(), m_BoneVertexList[b].Vertexes.DataSize(), "Skin BoneVertex bone data" ) )
				return FALSE;
		}
	}

	if ( Header.DataFlags & GSkinHeaderDataFlags::VertexWeights )
	{
		u32 VertexCount = 0;
		//LOAD_VAR( VertexCount );
		if ( !Data.Read( &VertexCount, GDataSizeOf(u32), "Skin vertex weight vertex count" ) )
			return FALSE;

		m_VertexWeights.Resize( VertexCount );
		//LOAD_DATA( m_VertexWeights.Data(), m_VertexWeights.DataSize() );
		if ( !Data.Read( m_VertexWeights.Data(), m_VertexWeights.DataSize(), "Skin vertex weight vertex data" ) )
			return FALSE;
	}

	if ( Header.DataFlags & GSkinHeaderDataFlags::VertexBones )
	{
		u32 VertexCount;
		//LOAD_VAR( VertexCount );
		if ( !Data.Read( &VertexCount, GDataSizeOf(u32), "Skin VertexBone vertex count" ) )
			return FALSE;

		m_VertexBones.Resize( VertexCount );
		//LOAD_DATA( m_VertexBones.Data(), m_VertexBones.DataSize() );
		if ( !Data.Read( m_VertexBones.Data(), m_VertexBones.DataSize(), "Skin VertexBone data" ) )
			return FALSE;
	}

	if ( Header.DataFlags & GSkinHeaderDataFlags::BoneBoneList )
	{
		u16 BoneBoneCount;
		u32 VertexCount;
		//LOAD_VAR( BoneBoneCount );
		//LOAD_VAR( VertexCount );
		if ( !Data.Read( &BoneBoneCount, GDataSizeOf(u16), "Skin BoneBone count" ) )
			return FALSE;

		if ( !Data.Read( &VertexCount, GDataSizeOf(u32), "Skin BoneBone vertex count" ) )
			return FALSE;


		m_BoneBoneList.Resize( BoneBoneCount );
		m_BoneBoneVertexList.Resize( VertexCount );
		//LOAD_DATA( m_BoneBoneList.Data(), m_BoneBoneList.DataSize() );
		//LOAD_DATA( m_BoneBoneVertexList.Data(), m_BoneBoneVertexList.DataSize() );
		if ( !Data.Read( m_BoneBoneList.Data(), m_BoneBoneList.DataSize(), "Skin BoneBone data" ) )
			return FALSE;
		if ( !Data.Read( m_BoneBoneVertexList.Data(), m_BoneBoneVertexList.DataSize(), "Skin BoneBone vertex data" ) )
			return FALSE;
	}



	#undef LOAD_VAR
	#undef LOAD_DATA

	return TRUE;
}

//-------------------------------------------------------------------------
//	save skin
//-------------------------------------------------------------------------
Bool GSkin::Save(GBinaryData& SaveData)
{
	//	write a header
	GSkinHeader Header;
	Header.MeshRef = m_Mesh;
	Header.SkeletonRef = m_Skeleton;
	Header.DataFlags = 0x0;

	if ( m_BoneVertexList.Size() )	Header.DataFlags |= GSkinHeaderDataFlags::BoneVertexLinks;
	if ( m_VertexWeights.Size() )	Header.DataFlags |= GSkinHeaderDataFlags::VertexWeights;
	if ( m_VertexBones.Size() )		Header.DataFlags |= GSkinHeaderDataFlags::VertexBones;
	if ( m_BoneBoneList.Size() )	Header.DataFlags |= GSkinHeaderDataFlags::BoneBoneList;
 
	SaveData.Write( &Header, GDataSizeOf(GSkinHeader) );

	//	save additional data
	if ( Header.DataFlags & GSkinHeaderDataFlags::BoneVertexLinks )
	{
		u16 BoneCount = m_BoneVertexList.Size();
		SaveData.Write( &BoneCount, GDataSizeOf(u16) );

		for ( int b=0;	b<m_BoneVertexList.Size();	b++ )
		{
			u32 VertexCount = m_BoneVertexList[b].Vertexes.Size();
			SaveData.Write( &VertexCount, GDataSizeOf(u32) );
			
			//	this is okay to be empty!
			if ( m_BoneVertexList[b].Vertexes.Size() > 0 )
				SaveData.Write( m_BoneVertexList[b].Vertexes.Data(), m_BoneVertexList[b].Vertexes.DataSize() );
		}
	}

	if ( Header.DataFlags & GSkinHeaderDataFlags::VertexWeights )
	{
		u32 VertexCount = m_VertexWeights.Size();
		SaveData.Write( &VertexCount, GDataSizeOf(u32) );
		SaveData.Write( m_VertexWeights.Data(), m_VertexWeights.DataSize() );
	}

	if ( Header.DataFlags & GSkinHeaderDataFlags::VertexBones )
	{
		u32 VertexCount = m_VertexBones.Size();
		SaveData.Write( &VertexCount, GDataSizeOf(u32) );
		SaveData.Write( m_VertexBones.Data(), m_VertexBones.DataSize() );
	}

	if ( Header.DataFlags & GSkinHeaderDataFlags::BoneBoneList )
	{
		u16 BoneBoneCount = m_BoneBoneList.Size();
		u32 VertexCount = m_BoneBoneVertexList.Size();
		SaveData.Write( &BoneBoneCount, GDataSizeOf(u16) );
		SaveData.Write( &VertexCount, GDataSizeOf(u32) );

		m_BoneBoneList.Resize( BoneBoneCount );
		m_BoneBoneVertexList.Resize( VertexCount );

		SaveData.Write( m_BoneBoneList.Data(), m_BoneBoneList.DataSize() );
		SaveData.Write( m_BoneBoneVertexList.Data(), m_BoneBoneVertexList.DataSize() );
	}
 

	return TRUE;
}

//-------------------------------------------------------------------------
//	draws the skeleton and the mesh together
//-------------------------------------------------------------------------
void GSkin::Draw(GDrawInfo& DrawInfo, GSkinShader* pShader, GAssetRef HighlightBone)
{
	GSkeleton* pSkeleton = GAssets::g_Skeletons.Find( m_Skeleton );
	GMesh* pMesh = GAssets::g_Meshes.Find( m_Mesh );
	
	//	copy drawinfo to be modified
	GDrawInfo TempDrawInfo( DrawInfo );
	TempDrawInfo.pShader = pShader;

	//	generate colours for vert display
	GList<float3> SkinVertColours;
	if ( pMesh && pSkeleton && (DrawInfo.Flags & GDrawInfoFlags::DebugVertexes) )
	{
		SkinVertColours.Resize( pMesh->VertCount() );
		SkinVertColours.SetAll( float3(1,1,1) );
		for ( int v=0;	v<m_VertexBones.Size();	v++ )
		{
			int BoneIndex = m_VertexBones[v][0];
			float3 Tmp = pSkeleton->GetBoneColour( BoneIndex );
			SkinVertColours[v] = Tmp;
		}

		TempDrawInfo.pVertexColours = &SkinVertColours;
	}

	//	draw mesh
	if ( pMesh )
	{
		pMesh->Draw( TempDrawInfo );
	}

	//	draw debug skeleton
	if ( pSkeleton && ( TempDrawInfo.Flags & GDrawInfoFlags::DebugSkeleton ) )
	{
		//	create matrixes from skinstate
		GList<GMatrix>* pBoneRotations = pShader ? &pShader->m_BoneFinal : NULL;
		if ( pBoneRotations )
			if ( pBoneRotations->Size() == 0 )
				pBoneRotations = NULL;
		
		g_Display->PushScene();

		g_Display->Translate( TempDrawInfo.Translation, DrawInfo.pRotation );

		//	draw skeleton on top of mesh
		pSkeleton->DebugDraw( pBoneRotations, HighlightBone );

		g_Display->PopScene();
	}
	
}





void GSkin::GenerateBoneVertexWeights()
{
	int b;

	//	clear out current data
	m_VertexWeights.Empty();
	m_BoneVertexList.Empty();
	m_VertexBones.Empty();


	GMesh* pMesh = GetMesh();
	GSkeleton* pSkeleton = GetSkeleton();

	if ( !pMesh || !pSkeleton )
	{
		if ( !pMesh )		GDebug::Print("Failed to get mesh to generate skin's bone-vertex weights\n");
		if ( !pSkeleton )	GDebug::Print("Failed to get skeleton to generate skin's bone-vertex weights\n");
		return;
	}

	//	gather positions for each bone
	GList<float3> BonePositions;
	pSkeleton->GetBonePositions( BonePositions );

	int BoneCount = BonePositions.Size();
	int VertCount = pMesh->m_Verts.Size();

	//	alloc data
	m_VertexWeights.Resize( VertCount );
	m_VertexWeights.SetAll( float2( 0.f, 0.f ) );
	
	//	alloc and init
	m_VertexBones.Resize( VertCount );
	m_VertexBones.SetAll( s82( -1, -1 ) );

	//	initialise data
	m_BoneVertexList.Resize( BoneCount );
	for ( b=0;	b<BoneCount;	b++ )
		m_BoneVertexList[b].Vertexes.Realloc(0);

	//	go through each vertex and find the nearest bone
	for ( int v=0;	v<VertCount;	v++ )
	{
		float3& VertPos = pMesh->m_Verts[v];

		//	find nearest bone from list
		int NearBone = 0;
		float NearLengthSq = ( VertPos - BonePositions[0] ).LengthSq();

		for ( int b=1;	b<BoneCount;	b++ )
		{
			float BoneLengthSq = ( VertPos - BonePositions[b] ).LengthSq();
			if ( BoneLengthSq < NearLengthSq )
			{
				NearBone = b;
				NearLengthSq = BoneLengthSq;
			}
		}

		//	setup weight
		m_VertexWeights[v][0] = 1.f;
		m_VertexWeights[v][1] = 0.f;

		//	setup bones
		m_VertexBones[v][0] = NearBone;
		m_VertexBones[v][1] = -1;

		//	nearest bone is NearBone (use weight #0)
		m_BoneVertexList[NearBone].Vertexes.Add( int2( v, 0 ) );
	}

	
	//	calc bone-bone list
	GenerateBoneBoneLists();


}

//-------------------------------------------------------------------------
//	calculate the bone-bone lists from the current bone/vertex links
//-------------------------------------------------------------------------
void GSkin::GenerateBoneBoneLists()
{
	GMesh* pMesh = GetMesh();
	GSkeleton* pSkeleton = GetSkeleton();

	if ( !pMesh || !pSkeleton )
	{
		if ( !pMesh )		GDebug::Print("Failed to get mesh to generate skin's bone-bone links\n");
		if ( !pSkeleton )	GDebug::Print("Failed to get skeleton to generate skin's bone-bone links\n");
		return;
	}
	
	int BoneCount = pSkeleton->BoneCount();
	int VertCount = pMesh->VertCount();

	//	clear out current data
	m_BoneBoneList.Empty();
	m_BoneBoneVertexList.Empty();

	for ( int BoneA=0;	BoneA<BoneCount;	BoneA++ )
	{
		for ( int BoneB=-1;	BoneB<BoneCount;	BoneB++ )
		{
			int ABVertCount = 0;

			for ( int v=0;	v<VertCount;	v++ )
			{
				if ( (m_VertexBones[v][0] == BoneA) && (m_VertexBones[v][1] == BoneB) )
				{
					//	add entry
					ABVertCount++;
					m_BoneBoneVertexList.Add( v );
				}
			}

			//	create bone bone entry if applicable
			if ( ABVertCount > 0 )
			{
				GSkinBoneBone BoneBone;
				BoneBone.BoneA = BoneA;
				BoneBone.BoneB = BoneB;
				BoneBone.NoOfVerts = ABVertCount;
				m_BoneBoneList.Add( BoneBone );
			}
		}
	}
}

GSkeleton* GSkin::GetSkeleton()					
{	
	return GAssets::g_Skeletons.Find( m_Skeleton );	
}

GMesh* GSkin::GetMesh()						
{	
	return GAssets::g_Meshes.Find( m_Mesh );	
}







GSkinShader::GSkinShader()
{
	m_AnimFrame					= 0.f;
	m_Anim						= GAssetRef_Invalid;
	m_Skin						= GAssetRef_Invalid;
	m_LastBoneCounter			= 0;
	m_ModifiedBones				= 0x0;
	m_ValidAnimRotations		= 0x0;
	m_ValidModifiedRotations	= 0x0;
	m_VertexModifiedBones		= 0x0;
	m_NewAnim					= GAssetRef_Invalid;
	m_NewAnimFrame				= 0.f;
	m_Flags						= 0x0;
	m_AnimSpeed					= 1.f;

	m_BlendAnim					= GAssetRef_Invalid;
	m_BlendAnimFrame			= 0.f;
	m_BlendAnimNewFrame			= 0.f;
	m_BlendAmount				= 0.f;
	m_BlendRate					= 0.f;

}

GSkinShader::~GSkinShader()
{
}


//-------------------------------------------------------------------------
//	set the attribs for this vertex
//-------------------------------------------------------------------------
Bool GSkinShader::HardwarePreDrawVertex(float3& Vertex, float3& Normal, int VertexIndex )
{
	GSkin* pSkin = GetSkin();
	
	if ( !pSkin )
		return FALSE;
	
	s82& BoneIndex = pSkin->m_VertexBones[VertexIndex];

	//	set vertex attrib (bone matrix indexes)
	g_DisplayExt.glVertexAttrib2sARB()( BONEINDEX_ATTRIB, BoneIndex[0], BoneIndex[1] );

	//	set vertex to the inverse buffer vertex
	Vertex = m_InverseVertexBuffer[ VertexIndex ];

	return TRUE;
}

//-------------------------------------------------------------------------
//	set the vertex buffer to our inverse vertex buffer, load vertex attribs 
//	and load bone matrixes into vertex program constants
//-------------------------------------------------------------------------
Bool GSkinShader::HardwarePreDraw(GMesh* pMesh,GDrawInfo& DrawInfo, GList<float3>*& pVertexBuffer, GList<float3>*& pNormalBuffer, GList<float2>*& pTextureUVBuffer, GList<float2>*& pTextureUV2Buffer, GList<float3>*& pColourBuffer)
{
	GSkin* pSkin = GetSkin();
	GSkeletonAnim* pAnim = GetAnim();

	//	update inverse vertex buffer and bones
	CalcInverseVertexBuffer( *pVertexBuffer );
	UpdateFinalBones();

	//	assign inverse buffer as vertex buffer
	pVertexBuffer = &m_InverseVertexBuffer;

	//	set vertex attribs (bone matrix indexes)
	pMesh->BindAttribArray( pSkin->m_VertexBones.Data(), GL_BYTE, 2, BONEINDEX_ATTRIB );

	//	load matrixes into constant registers
	int ConstantIndex = m_FirstConstant; //	0..3 has projection matrix

	for ( int m=0;	m<m_BoneFinal.Size();	m++ )
	{
		GMatrix& Mat = m_BoneFinal[m];

		/*
		//	load columns
		LoadConstant( ConstantIndex+0, Mat.m_Column[0] );
		LoadConstant( ConstantIndex+1, Mat.m_Column[1] );
		LoadConstant( ConstantIndex+2, Mat.m_Column[2] );
		LoadConstant( ConstantIndex+3, Mat.m_Column[3] );
		ConstantIndex += 4;
		*/

		//	load columns as rows
		LoadConstant( ConstantIndex+0, Mat.m_Column[0].x, Mat.m_Column[1].x, Mat.m_Column[2].x, Mat.m_Column[3].x );
		LoadConstant( ConstantIndex+1, Mat.m_Column[0].y, Mat.m_Column[1].y, Mat.m_Column[2].y, Mat.m_Column[3].y );
		LoadConstant( ConstantIndex+2, Mat.m_Column[0].z, Mat.m_Column[1].z, Mat.m_Column[2].z, Mat.m_Column[3].z );
		ConstantIndex += 3;

		/*//	no longer passing row 4
		LoadConstant( ConstantIndex+3, Mat.m_Column[0].w, Mat.m_Column[1].w, Mat.m_Column[2].w, Mat.m_Column[3].w );
		ConstantIndex += 4;
		*/

	}

	return TRUE;
}


//-------------------------------------------------------------------------
//	get the vertex program code
//-------------------------------------------------------------------------
Bool GSkinShader::GetVertexProgram(GString& String)
{
	#ifdef USE_ARB_PROGRAM

		//	copy program
		String = g_SkinShaderARB;
		return TRUE;

	#else

		//	load NV program from file
		GString ProgramFilename("GutSkinningNV.vp");

		GFile File;
		if ( !File.Load( ProgramFilename ) )
			return FALSE;
		
		//	no data in file
		if ( File.m_Data.Size() == 0 )
			return FALSE;

		String = GString( (char*)File.m_Data.Data(), File.m_Data.Size() );
		return TRUE;
		
	#endif
}


void GSkinShader::CalcInverseVertexBuffer( GList<float3>& VertexBuffer )
{
	GSkin* pSkin = GetSkin();
	if ( !pSkin )	
		return;

	GSkeleton* pSkeleton = GetSkeleton(pSkin->m_Skeleton);
	
	//	need new inverse vertex buffer
	if ( m_InverseVertexBuffer.Size() != VertexBuffer.Size() )
	{
		//	force update of whole vertex buffer
		m_VertexModifiedBones = (1<<m_LastBoneCounter)-1;

		//	copy new vertex buffer into inverse buffer
		m_InverseVertexBuffer.Copy( VertexBuffer );
	
		//	inverse vertexes and setup constant data
		for ( int b=0;	b<pSkeleton->BoneCount();	b++ )
		{
			//	get bone
			GBone* pBone = pSkeleton->GetBoneIndex(b);

			//	modify each vert attatched to this bone
			for ( int i=0;	i<pSkin->m_BoneVertexList[b].Vertexes.Size();	i++ )
			{
				int& VertIndex = pSkin->m_BoneVertexList[b].Vertexes[i][0];
				int& WeightIndex = pSkin->m_BoneVertexList[b].Vertexes[i][1];
				float Weight = pSkin->m_VertexWeights[VertIndex][WeightIndex];

				//	modify vertex
				float3& VertPos = m_InverseVertexBuffer[ VertIndex ];
				GMatrix& BoneRelative = pBone->m_Relative;
				GMatrix& BoneAbsolute = pBone->m_Absolute;
				BoneAbsolute.InverseTranslateVect( VertPos );
				BoneAbsolute.InverseRotateVect( VertPos );
			}
		}
	}
}


//-------------------------------------------------------------------------
//	update internal vertex buffer if required and set vertex buffer reference to it
//-------------------------------------------------------------------------
Bool GSkinShader::SoftwarePreDraw(GMesh* pMesh,GDrawInfo& DrawInfo, GList<float3>*& pVertexBuffer, GList<float3>*& pNormalBuffer, GList<float2>*& pTextureUVBuffer, GList<float2>*& pTextureUV2Buffer, GList<float3>*& pColourBuffer)
{
	GSkin* pSkin = GetSkin();
	GSkeletonAnim* pAnim = GetAnim();

	if ( !pSkin || !pAnim )
		return FALSE;

	GSkeleton* pSkeleton = GetSkeleton(pSkin->m_Skeleton);

	//	calc a new inverse buffer if we need it
	CalcInverseVertexBuffer( *pVertexBuffer );

	if ( m_SoftwareVertexBuffer.Size() != m_InverseVertexBuffer.Size() )
		m_SoftwareVertexBuffer.Resize( m_InverseVertexBuffer.Size() );
	
	//	assign local buffer
	pVertexBuffer = &m_SoftwareVertexBuffer;

	//	update any bones that have been modified before modifying vertex buffer
	UpdateFinalBones();
	
	//	recalc all verts
	m_VertexModifiedBones = 0xffffffff;

	//	use bone-bone entries if availible
	if ( pSkin->m_BoneBoneList.Size() > 0 )
	{
		int BBVertIndex = 0;

		for ( int BBIndex=0;	BBIndex<pSkin->m_BoneBoneList.Size();	BBIndex++ )
		{
			const s8& BoneA = pSkin->m_BoneBoneList[BBIndex].BoneA;
			const s8& BoneB = pSkin->m_BoneBoneList[BBIndex].BoneB;
			s82 VertBone;
			VertBone[0] = BoneA;
			VertBone[1] = BoneB;

			u32 BoneChanged[2] = { FALSE, FALSE };
			if ( BoneA != -1 )	BoneChanged[0] = m_VertexModifiedBones & (1<<((u32)BoneA));
			if ( BoneB != -1 )	BoneChanged[1] = m_VertexModifiedBones & (1<<((u32)BoneB));

			if ( BoneChanged[0] || BoneChanged[1] )
			{
				//	get bones
				GBone* pBone[2] = { NULL, NULL };
				pBone[0] = BoneA == -1 ? NULL : pSkeleton->GetBoneIndex( BoneA );
				pBone[1] = BoneB == -1 ? NULL : pSkeleton->GetBoneIndex( BoneB );

				//	neither bone exists
				if ( !pBone[0] && !pBone[1] )
				{
					continue;
				}

				for ( int v=0;	v<pSkin->m_BoneBoneList[BBIndex].NoOfVerts;	v++)
				{
					int VertIndex = pSkin->m_BoneBoneVertexList[ BBVertIndex + v ];
					float2 VertWeight = pSkin->m_VertexWeights[VertIndex];
			
					//	get the inverse for the vertex
					float3& FinalVertex = m_SoftwareVertexBuffer[ VertIndex ];
					float3& InverseVertex = m_InverseVertexBuffer[ VertIndex ];

					//	-1 means use both bones, else, use a particular index
					int UseBoneIndex = -1;
					if ( pBone[0] && !pBone[1] )		UseBoneIndex = 0;
					else if ( !pBone[0] && pBone[1] )	UseBoneIndex = 1;

					//	set to singular bone
					if ( UseBoneIndex != -1 )
					{
						//	transform vertex
						GMatrix& BoneFinal = m_BoneFinal[ VertBone[UseBoneIndex] ];
						FinalVertex = InverseVertex;
						BoneFinal.TransformVector( FinalVertex );
					}
					else
					{
						//	blend between bones
						GMatrix& BoneFinalA = m_BoneFinal[ BoneA ];
						GMatrix& BoneFinalB = m_BoneFinal[ BoneB ];
						
						float3 VertexA = InverseVertex;
						float3 VertexB = InverseVertex;
						BoneFinalA.TransformVector( VertexA );
						BoneFinalB.TransformVector( VertexB );

						FinalVertex = VertexA;
						FinalVertex += VertexB * VertWeight[1];
					}
				}
			}

			BBVertIndex += pSkin->m_BoneBoneList[BBIndex].NoOfVerts;
		}

	}
	else
	{
		for ( int v=0;	v<m_SoftwareVertexBuffer.Size();	v++ )
		{
			const s82& VertBone = pSkin->m_VertexBones[v];
			float2 VertWeight = pSkin->m_VertexWeights[v];

			//	get bones
			GBone* pBone[2] = { NULL, NULL };
			pBone[0] = VertBone[0] == -1 ? NULL : pSkeleton->GetBoneIndex( VertBone[0] );
			pBone[1] = VertBone[1] == -1 ? NULL : pSkeleton->GetBoneIndex( VertBone[1] );

			//	neither bone exists
			if ( !pBone[0] && !pBone[1] )
			{
				GDebug::Print("Warning: no bones for vertex %d\n",v);
				continue;
			}

			u32 BoneChanged[2] = { FALSE, FALSE };

			if ( pBone[0] )	BoneChanged[0] = m_VertexModifiedBones & (1<<((u32)VertBone[0]));
			if ( pBone[1] )	BoneChanged[1] = m_VertexModifiedBones & (1<<((u32)VertBone[1]));

			//	neither bone has changed, vertex doesnt need updated
			if ( (!BoneChanged[0]) && (!BoneChanged[1]) )
			{
			//	GDebug::Print("bones %d and %d not updated\n", VertBone[0], VertBone[1] );
				continue;
			}

			//	get the inverse for the vertex
			float3& FinalVertex = m_SoftwareVertexBuffer[ v ];
			float3& InverseVertex = m_InverseVertexBuffer[ v ];

			//	-1 means use both bones, else, use a particular index
			int UseBoneIndex = -1;
			if ( pBone[0] && !pBone[1] )		UseBoneIndex = 0;
			else if ( !pBone[0] && pBone[1] )	UseBoneIndex = 1;
		

			//	set to singular bone
			if ( UseBoneIndex != -1 )
			{
				//	transform vertex
				GMatrix& BoneFinal = m_BoneFinal[ VertBone[UseBoneIndex] ];
				FinalVertex = InverseVertex;
				BoneFinal.TransformVector( FinalVertex );
			}
			else
			{
				//	blend between bones
				GMatrix& BoneFinalA = m_BoneFinal[ VertBone[0] ];
				GMatrix& BoneFinalB = m_BoneFinal[ VertBone[1] ];
				
				float3 VertexA = InverseVertex;
				float3 VertexB = InverseVertex;
				BoneFinalA.TransformVector( VertexA );
				BoneFinalB.TransformVector( VertexB );

				FinalVertex = VertexA;
				FinalVertex += VertexB * VertWeight[1];
			}

		}
	}

	//	vertex-bone related data has been updated
	m_VertexModifiedBones = 0x0;



	return TRUE;
}



//-------------------------------------------------------------------------
//	update anim matrixes to the new frame and/or anim
//-------------------------------------------------------------------------
Bool GSkinShader::UpdateToNewFrame()
{
	//	if its a new anim, or we're blending, force the update
	//if ( m_Flags & (GSkinShaderFlags::NewAnim|GSkinShaderFlags::BlendAnim) )
	if ( m_Flags & (GSkinShaderFlags::NewAnim) )
		m_Flags |= GSkinShaderFlags::ForceUpdate;

	GSkin* pSkin = GetSkin();
	GSkeletonAnim* pAnim = NULL;

	//	if flagged for new anim, grab the new one
	if ( m_Flags & GSkinShaderFlags::NewAnim )
	{
		pAnim = GetNewAnim();
		if ( !pAnim )
		{
			GDebug_Print("Failed to get new anim %s\n", GAsset::RefToName( m_NewAnim ) );
			return FALSE;
		}
	}
	else
	{
		//	current anim
		pAnim = GetAnim();
	}

	if ( !pAnim || !pSkin )
		return FALSE;

	GSkeleton* pSkeleton = GetSkeleton( pSkin->m_Skeleton );
	if ( !pSkeleton )
		return FALSE;

	//	check limits
	Bool IsLastFrame = FALSE;
	float LastFrame = pAnim->LastKeyframe();
	if ( m_NewAnimFrame > LastFrame )
	{
		IsLastFrame = TRUE;
		m_NewAnimFrame = LastFrame;
	}

	//	bones have changed, force update
	if ( m_LastBoneCounter != pSkeleton->BoneCount() )
		m_Flags |= GSkinShaderFlags::ForceUpdate;

	//	not got any anim rotations, force update
	if ( !m_AnimRotations.Size() )
		m_Flags |= GSkinShaderFlags::ForceUpdate;

	//	get change in frame
	float FrameChange = m_NewAnimFrame - m_AnimFrame;
	float BlendFrameChange = m_BlendAnimNewFrame - m_BlendAnimFrame;
	
	//	doesnt need updating
	if ( m_Flags & GSkinShaderFlags::ForceUpdate == 0x0 )
	{
		//	anim frame change is very low, dont bother updating
		if ( fabsf( FrameChange ) < NEAR_ZERO )
		{
			//	if we're blending, check the frame change on the blend anim too
			if ( m_Flags & GSkinShaderFlags::BlendAnim )
			{
				if ( fabsf( BlendFrameChange ) < NEAR_ZERO )
					return FALSE;
			}
			else
			{
				return FALSE;
			}
		}
	}

	//	get matrixes from anim
	float3 ExtractedMovement(0,0,0);
	pAnim->GetRotations( m_NewAnimFrame, m_AnimRotations, m_ValidAnimRotations, m_AnimFrame, ExtractedMovement );

	//	blending, blend sets of rotations
	if ( m_Flags & GSkinShaderFlags::BlendAnim )
	{
		GSkeletonAnim* pBlendAnim = GetBlendAnim();
		if ( pBlendAnim )
		{
			GMatrixRotations BlendAnimRotations;
			u32 ValidBlendAnimRotations = 0x0;
			float3 BlendAnimExtractedMotion( 0,0,0 );

			//	get blend anim's rotations
			pBlendAnim->GetRotations( m_BlendAnimNewFrame, BlendAnimRotations, ValidBlendAnimRotations, m_BlendAnimFrame, BlendAnimExtractedMotion );

			//	blend rotation sets
			m_AnimRotations.Blend( BlendAnimRotations, m_BlendAmount );

			//	add non identity blend anim info
			m_ValidAnimRotations |= ValidBlendAnimRotations;
		}
	}

	//	we've got a new list of bones
	m_LastBoneCounter = m_AnimRotations.Size();

	//	assume all bones have been modified and need to be updated again
	m_ModifiedBones = (1<<m_LastBoneCounter)-1;

	//	update modified rotations size
	int ModifiedSize = m_ModifiedRotations.Size();
	m_ModifiedRotations.Resize( m_AnimRotations.Size() );
	for ( int i=ModifiedSize;	i<m_ModifiedRotations.Size();	i++ )
	{
		m_ModifiedRotations[i].SetIdentity();

		//	this is now identity
		m_ValidModifiedRotations &= ~(1<<i);
	}

	//	changed to new frame
	m_AnimFrame = m_NewAnimFrame;

	if ( m_Flags & GSkinShaderFlags::BlendAnim )
		m_BlendAnimFrame = m_BlendAnimNewFrame;
	
	//	okay, we've changed over to the new anim
	if ( m_Flags & GSkinShaderFlags::NewAnim )
	{
		m_Anim = m_NewAnim;
		m_Flags &= ~GSkinShaderFlags::NewAnim;
	}

	//	has updated
	m_Flags &= ~GSkinShaderFlags::ForceUpdate;

	//	move object
	if ( m_pOwner )
	{
		//	temp
		ExtractedMovement.y *= -1.f;

		//	rotate movement to match player
		m_pOwner->m_Rotation.RotateVector( ExtractedMovement );
		m_pOwner->m_ExtractedMovement = ExtractedMovement;
	}

	return TRUE;
}

void GSkinShader::UpdateFinalBones()
{
	//	no bones need recalculating
	if ( !m_ModifiedBones )
		return;

	GSkeleton* pSkeleton = GetSkeleton();
	if ( !pSkeleton )
		return;

	//	test update all bones
	//m_ModifiedBones = 0xffffffff;	

/*	//	debug count how many bones that need modifying
	int ModifiedBoneCount = 0;
	for ( int i=0;	i<32;	i++ )
		if ( m_ModifiedBones & (1<<i) )
			ModifiedBoneCount++;
	GDebug::Print("Need to update %d / %d bones\n", ModifiedBoneCount, m_LastBoneCounter );
*/
	
	//	get final bone transforms for current anim/modified rotations
	pSkeleton->GetBoneTransform( m_BoneFinal, m_AnimRotations, &m_ModifiedRotations, m_ModifiedBones, m_ValidAnimRotations, m_ValidModifiedRotations );

	//	vertexes need updating
	m_VertexModifiedBones |= m_ModifiedBones;

	//	bones dont need updating any more
	m_ModifiedBones = 0x0;
}


void GSkinShader::SetModifiedMatrix(int BoneIndex, GMatrix& NewMatrix)
{
	Bool IsIdentity = NewMatrix.IsIdentity() ? TRUE : FALSE;
	Bool WasIdentity = ( m_ValidModifiedRotations & (1<<BoneIndex) ) ? FALSE : TRUE;

	//	has it changed?
	if ( IsIdentity && WasIdentity )
		return;

	//	set is-not-identity flag
	if ( IsIdentity )
		m_ValidModifiedRotations &= ~(1<<BoneIndex);
	else
		m_ValidModifiedRotations |= (1<<BoneIndex);

	//	has been modified
	m_ModifiedBones |= (1<<BoneIndex);
	
	//	set child bones of this bone as modified too
	GSkeleton* pSkeleton = GetSkeleton();
	GBone* pBone = pSkeleton ? pSkeleton->GetBoneIndex(BoneIndex) : NULL;
	if ( pBone )
		m_ModifiedBones |= pBone->m_ChildBones;

	//	copy new matrix
	m_ModifiedRotations[BoneIndex] = NewMatrix;
}


void GSkinShader::SetModifiedMatrixPosFromParent(int BoneIndex, float3& PosFromParent)
{
	GSkeleton* pSkeleton = GetSkeleton();
	if ( !pSkeleton )
		return;

	//	check isnt root bone
	if ( BoneIndex == 0 )
	{
		GDebug_Break("Cannot set bone offset from parent if we're the root bone\n");
		return;
	}

	//	grab parent
	GBone* pBone = pSkeleton->GetBoneIndex(BoneIndex);
	int ParentIndex = pBone->GetParentIndex();

	//	get direction quaternion
	GQuaternion Rot;
	Rot.LookAt( PosFromParent );
	GMatrix LookAtMat;
	LookAtMat.SetRotation( Rot );
	
	SetModifiedMatrix(ParentIndex, LookAtMat);
}


//-------------------------------------------------------------------------
//	get skeleton anim
//-------------------------------------------------------------------------	
GSkeletonAnim* GSkinShader::GetAnim()
{
	return GAssets::g_SkeletonAnims.Find( m_Anim );
}

GSkeletonAnim* GSkinShader::GetNewAnim()
{
	return GAssets::g_SkeletonAnims.Find( m_NewAnim );
}

GSkeletonAnim* GSkinShader::GetBlendAnim()
{
	return GAssets::g_SkeletonAnims.Find( m_BlendAnim );
}


//-------------------------------------------------------------------------
//	get skin asset
//-------------------------------------------------------------------------
GSkin* GSkinShader::GetSkin()
{
	return GAssets::g_Skins.Find( m_Skin );
}

//-------------------------------------------------------------------------
//	get skeleton asset
//-------------------------------------------------------------------------
GSkeleton* GSkinShader::GetSkeleton(GAssetRef SkeletonRef)
{
	//	get skeleton ref from skin
	if ( SkeletonRef == GAssetRef_Invalid )
	{
		GSkin* pSkin = GetSkin();
		if ( !pSkin )
			return NULL;
		
		if ( pSkin->m_Skeleton == GAssetRef_Invalid )
			return NULL;

		return GetSkeleton( pSkin->m_Skeleton );
	}

	//	find skeleton
	return GAssets::g_Skeletons.Find( SkeletonRef );
}

//-------------------------------------------------------------------------
//	get mesh asset
//-------------------------------------------------------------------------
GMesh* GSkinShader::GetMesh(GAssetRef MeshRef)
{
	//	get mesh ref from skin
	if ( MeshRef == GAssetRef_Invalid )
	{
		GSkin* pSkin = GetSkin();
		if ( !pSkin )
			return NULL;
		
		if ( pSkin->m_Mesh == GAssetRef_Invalid )
			return NULL;

		return GetMesh( pSkin->m_Mesh );
	}

	//	find skeleton
	return GAssets::g_Meshes.Find( MeshRef );
}

//-------------------------------------------------------------------------
//	continue animation per frame
//-------------------------------------------------------------------------
void GSkinShader::Update()
{
	//	jump straight into new anim if current is invalid
	if ( m_Anim == GAssetRef_Invalid )
	{
		m_Anim = m_NewAnim;
		m_AnimFrame = m_NewAnimFrame;
		m_Flags &= ~GSkinShaderFlags::NewAnim;
		m_Flags |= GSkinShaderFlags::ForceUpdate;
	}

	GSkeletonAnim* pAnim = GetAnim();
	if ( !pAnim )
		return;

	//	calc new frame number if necccesary
	if ( ( m_Flags & (GSkinShaderFlags::NewAnim|GSkinShaderFlags::ForceUpdate) ) == 0x0 )
	{
		float StepRate = m_AnimSpeed * g_App->FrameDelta60();
		float Step = ANIMATE_STEP * StepRate;

		//	set new frame no
		m_NewAnimFrame = m_AnimFrame + Step;

		//	loop around and keep remainder
		if ( pAnim->KeyframeCount() > 0 )
		{
			while ( m_NewAnimFrame > pAnim->LastKeyframe() )
			{
				m_NewAnimFrame -= pAnim->LastKeyframe();
			}
		}
		else
		{
			if ( m_NewAnimFrame > pAnim->LastKeyframe() )
				m_NewAnimFrame = pAnim->LastKeyframe();
		}

		//	update blend info
		if ( m_Flags & GSkinShaderFlags::BlendAnim )
		{
			m_BlendAmount += m_BlendRate * StepRate;
			m_BlendAnimNewFrame = m_BlendAnimFrame + Step;
			GSkeletonAnim* pBlendAnim = GetBlendAnim();

			//	loop blend anim
			if ( pBlendAnim )
			{
				if ( pBlendAnim->KeyframeCount() > 0 )
				{
					while ( m_BlendAnimNewFrame > pAnim->LastKeyframe() )
					{
						m_BlendAnimNewFrame -= pAnim->LastKeyframe();
					}
				}
				else
				{
					if ( m_BlendAnimNewFrame > pAnim->LastKeyframe() )
						m_BlendAnimNewFrame = pAnim->LastKeyframe();
				}
			}

			//	blend has finished, switch over to the blend anim
			if ( m_BlendAmount >= 1.f )
			{
				SetNewAnim( m_BlendAnim, m_BlendAnimFrame );
				m_NewAnimFrame = m_BlendAnimNewFrame;
			}
			//	blend anim has gone backwards so go back to current anim
			else if ( m_BlendAmount < 0.f )
			{
				SetNewAnim( m_Anim, m_AnimFrame );
			}				
		}

	}

	//	update animation bones
	UpdateToNewFrame();
}

	
void GSkinShader::SetNewAnim(GAssetRef Anim, float NewFrame)					
{	
	m_NewAnim = Anim;	
	m_NewAnimFrame = NewFrame;	
	m_Flags |= GSkinShaderFlags::NewAnim;	
	m_Flags &= ~GSkinShaderFlags::BlendAnim;	
}

void GSkinShader::SetNewFrame(float NewFrame)										
{	
	m_NewAnimFrame = NewFrame;	
	m_Flags |= GSkinShaderFlags::ForceUpdate;	
}

void GSkinShader::BlendToAnim(GAssetRef BlendAnim, float BlendAnimFromFrame, float BlendOverFrames,Bool ForceNewBlend)	
{
	//	already blending to this anim?
	if ( !ForceNewBlend )
		if ( m_Flags & GSkinShaderFlags::BlendAnim )
			if ( m_BlendAnim == BlendAnim )
				return;

	//	already blending, but in reverse to what we desire? (blend back to current anim)
	if ( m_Flags & GSkinShaderFlags::BlendAnim && BlendAnim == m_Anim )
	{
		m_BlendRate = ( 1.f / BlendOverFrames ) * -1.f;
		return;
	}

	//	already using this anim
	if ( CurrentAnim() == BlendAnim )
		return;

	//	setup blending
	m_Flags |= GSkinShaderFlags::BlendAnim;
	m_Flags &= ~GSkinShaderFlags::NewAnim;
	
	m_BlendAnim = BlendAnim;	
	m_BlendAnimFrame = BlendAnimFromFrame;	
	
	//	start at 0 blend
	m_BlendAmount = 0.f;
	m_BlendRate = 1.f / BlendOverFrames;
}

//-------------------------------------------------------------------------
//	draw debug skeleton after geometry has been draw
//-------------------------------------------------------------------------	
void GSkinShader::PostDraw(GMesh* pMesh,GDrawInfo& DrawInfo)
{
	//	do usual post draw cleanup
	GShader::PostDraw( pMesh, DrawInfo );
	
	//todo: make use of matrixes currently loaded into shader?
	//	draw debug skeleton (with shader off)
	if ( DrawInfo.Flags & GDrawInfoFlags::DebugSkeleton )
	{
		GSkeleton* pSkeleton = GetSkeleton();
		GAssetRef HighlightedBone = GAssetRef_Invalid;

		if ( pSkeleton )
		{
			pSkeleton->DebugDraw( m_BoneFinal.Size() ? &m_BoneFinal : NULL, HighlightedBone );
		}
	}

}


//-------------------------------------------------------------------------
//	setup new skin ref
//-------------------------------------------------------------------------
Bool GSkinShader::SetSkin(GAssetRef SkinRef)
{
	m_Skin = SkinRef;
	GSkin* pSkin = GetSkin();
	if ( !pSkin )
		return FALSE;

	//	allocate modified rotations size
	GSkeleton* pSkeleton = GetSkeleton();

	int BoneCount = pSkeleton ? pSkeleton->BoneCount() : 0;
	
	m_ModifiedRotations.Resize( BoneCount );

	return TRUE;	
}




const char g_SkinShaderARB[] = 
{
	/*
	#	R0	bone matrix row 0
	#	R1	bone matrix row 1
	#	R2	bone matrix row 2
	#	R3	bone matrix row 3
	#	R4	input vertex pos
	#	R5	matrix transformed vertex
	#	R6	scratch
	#	R7	output vertex pos
	#	C0..3	projection matrix
	#	V6	attrib6, matrix index
	*/

	//	header
	"!!ARBvp1.0"

	//	variables
	"ATTRIB	vPOS			= vertex.position;"
	"ATTRIB	vCOL0			= vertex.color;"
	"ATTRIB	vTEX0			= vertex.texcoord[0];"
	"ATTRIB	vTEX1			= vertex.texcoord[1];"
	"ATTRIB	vBI				= vertex.attrib[6];"	//	bone index
	"PARAM	mPRJ[4]			= { state.matrix.mvp };"	//	projection matrix
	"OUTPUT	oPOS			= result.position;"
	"OUTPUT	oCOL0			= result.color;"
	"OUTPUT	oTEX0			= result.texcoord[0];"
	"OUTPUT	oTEX1			= result.texcoord[1];"

	//	warning some drivers dont check the param size! so make sure we dont run out
	"PARAM	BoneMat[87]	= { program.local[8..94] };"	//	29 POSSIBLE matrixes(*3 columns)
	"ADDRESS	A0;"
	"TEMP	R0;"
	"TEMP	R1;"
	"TEMP	R2;"
	"TEMP	R3;"
	"TEMP	R4;"
	"TEMP	R5;"
	"TEMP	R6;"
	"TEMP	R7;"

	// copy vertex into register4
	"MOV R4, vPOS;"

	// matrix index in attrib6
	// get matrix index 0(x) (1 matrix takes up 4 constants, so multiply index by 4[gr - now 3])
	"MUL R6.x, vBI.x, 3;"
	"ARL A0.x, R6.x;"		// get address

	// row-major matrix input
	"MOV R0, BoneMat[ A0.x + 0  ];"	//	row[0].col0 = col[0].row0
	"MOV R1, BoneMat[ A0.x + 1  ];"	//	row[0].col0 = col[0].row0
	"MOV R2, BoneMat[ A0.x + 2  ];"	//	row[0].col0 = col[0].row0

	// transform by bone matrix index (+ base index 9)
	"DP4 R5.x, R0, R4;"	//	row0
	"DP4 R5.y, R1, R4;"	//	row1
	"DP4 R5.z, R2, R4;"	//	row2
	"SGE R5.w, R0.x, R0.x;"	//	set w to 1 if R0.x>=R0.x

	// multiply by weight
	//MUL R0.xyz, R7, v[WGHT].x;
	"MOV R7, R5;"

	// put vertex(r0) into output multiplying with modelview(c[0..3])
	"DP4	oPOS.x, mPRJ[0], R7;"
	"DP4 oPOS.y, mPRJ[1], R7;"
	"DP4 oPOS.z, mPRJ[2], R7;"
	"DP4 oPOS.w, mPRJ[3], R7;"

	// copy input colour
	"MOV oCOL0, vCOL0;"

	// copy input texcoord
	"MOV oTEX0, vTEX0;"
	"MOV oTEX1, vTEX1;"

	//	footer
	"END"
};
