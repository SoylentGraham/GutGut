/*------------------------------------------------

  GMesh.cpp

	Base mesh object with vertex, normal and primitive (currently just triangle) information



-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GMesh.h"
#include "GDebug.h"
#include "GList.h"
#include "GMatrix.h"
#include "GQuaternion.h"
#include "GApp.h"
#include "GGutFile.h"
#include "GAssetList.h"
#include "GBinaryData.h"


//	globals
//------------------------------------------------
const u32		GMesh::g_Version = 0x22220007;
u32				GMesh::g_ForceFlagsOn = 0x0;
u32				GMesh::g_ForceFlagsOff = 0x0;

#define SHADOW_INFINITY		1000.f	//	max distance the shadow will extend

GDeclareCounter(CulledMeshes);
GDeclareCounter(DrawMeshes);

//	Definitions
//------------------------------------------------



Bool GTriStrip::Load(GBinaryData& Data)
{
	//	grab header
	GTriStripHeader Header;
	if ( !Data.Read( &Header, GDataSizeOf(GTriStripHeader), "TriStrip Header" ) )
		return FALSE;

	//	check theres enough data for indicies
	m_Indicies.Resize( Header.Indicies );
	
	//	copy data
	if ( !Data.Read( m_Indicies.Data(), m_Indicies.DataSize(), "Tristrip indicies" ) )
		return FALSE;

	return TRUE;
}


void GTriStrip::Save(GBinaryData& Data)
{
	//	add header
	GTriStripHeader Header;
	Header.Indicies = m_Indicies.Size();
	Data.Write( &Header, GDataSizeOf(GTriStripHeader) );

	//	add data
	Data.Write( m_Indicies.Data(), m_Indicies.DataSize() );
}




GMesh::GMesh()
{
	//	initialise variables
	m_VBOVertexID	= 0;
	m_VBONormalID	= 0;
	m_VBOTexCoordID	= 0;

	m_ShadowData.m_pOwner = this;
}




GMesh::~GMesh()
{
	Cleanup();
}

void GMesh::Cleanup()
{
	//	deload any opengl data
	Deload();

	//	cleanup geometry
	//	delete data
	m_Verts.Empty();
	m_Normals.Empty();
	
	m_Triangles.Empty();
	m_TriangleNeighbours.Empty();
	m_TriangleColours.Empty();

	m_TriStrips.Empty();
	m_TriStripColours.Empty();
}


void GMesh::AllocVerts(int v)
{
	int OldSize = VertCount();

	//	reallocate vertex data
	m_Verts.Resize(v);

	//	fill new verts with magic number
	for ( int i=OldSize;	i<VertCount();	i++ )
	{
		m_Verts[i].x = 7777.7777f;
		m_Verts[i].y = 8888.8888f;
		m_Verts[i].z = 9999.9999f;
	}

	//	if additional data exists, resize that
	if ( m_Normals.Data() )
		m_Normals.Resize( v );

	if ( m_TextureUV.Data() )
		m_TextureUV.Resize( v );

}


void GMesh::AllocTriangles(int t)
{
	//	reallocate triangles
	m_Triangles.Resize(t);

	//	if additional data exists, resize that
	if ( m_TriangleNeighbours.Data() )
		m_TriangleNeighbours.Resize( t );

	if ( m_TriangleColours.Data() )
		m_TriangleColours.Resize( t );

	if ( m_TriangleCenters.Data() )
		m_TriangleCenters.Resize( t );
}



void GMesh::AllocTriStrips(int t)
{
	//	reallocate triangle strips
	m_TriStrips.Resize(t);

	//	if additional data exists, resize that
	if ( m_TriStripColours.Data() )
		m_TriStripColours.Resize( t );
}


//-------------------------------------------------------------------------
//		//	any verts this far from each other will be merged
//	const float DistanceTolerance = 0.001f;
//
//	//	UV coords need to be this close to allow a merge
//	const float UVTolerance = 0.01f;
//
//-------------------------------------------------------------------------
void GMesh::MergeVerts(float DistanceTolerance, Bool CheckUV, float UVTolerance)
{
	int va,vb,v,t,i;
	float DistanceToleranceSq = DistanceTolerance * DistanceTolerance;
	float UVToleranceSq = UVTolerance * UVTolerance;

	//	list of verts we've removed and kept for quick access later
	GList<int2> VertsRemoved;	//	1st int is the vert removed, 2nd is the vert its replaced with
	GList<int> VertsKept;
	VertsKept.Realloc( VertCount() );
	VertsRemoved.Realloc( VertCount() );


	//	merge any verts
	for ( va=0;	va<VertCount();	va++ )
	{
		if ( va % 200 == 0 )
		{
			GDebug_Print("MergeVerts: Merging vertex %d/%d...\n", va, VertCount() );
		}

		//	skip this vert if we've removed it
		Bool Skip = FALSE;
		for ( i=0;	i<VertsRemoved.Size();	i++ )
		{
			if ( VertsRemoved[i][0] == va )
			{
				Skip = TRUE;
				break;
			}
		}

		if ( Skip )
			continue;


		//	this vert hasnt already been removed, so we keep it
		VertsKept.Add( va );

		//	find a match for va
		for ( vb=va+1;	vb<VertCount();	vb++ )
		{
			float3& verta = m_Verts[va];
			float3& vertb = m_Verts[vb];
			float vertdist = (verta - vertb).LengthSq();
			
			//	merge if vertexes are close
			Bool Merge = (vertdist <= DistanceToleranceSq );

			//	check UV close-ness before merging
			if ( Merge && m_TextureUV.Data() && CheckUV )
			{
				float2 UVDist;
				UVDist.x = fabsf( m_TextureUV[va].x - m_TextureUV[vb].x );
				UVDist.y = fabsf( m_TextureUV[va].y - m_TextureUV[vb].y );
				
				if ( UVDist.LengthSq() > UVToleranceSq )
					Merge = FALSE;
			}


			if ( Merge )
			{
				//	we're merging vb into va, so we're gonna be removing vb
				VertsRemoved.Add( int2( vb, va ) );

				//	average vert pos
				verta += vertb;
				verta *= 0.5f;

				//	merge normals
				if ( m_Normals.Size() )
				{
					float3& normala = m_Normals[ va ];
					float3& normalb = m_Normals[ vb ];
					normala += normalb;
					normala *= 0.5f;
					normala.Normalise();
				}
			}
		}
	}



	//	stats
/*	GDebug_Print("Verts Merged: %d\n", VertsRemoved.Size() );
	for ( i=0;	i<VertsRemoved.Size();	i++ )
	{
		GDebug_Print("%d\n", VertsRemoved[i][0] );
	}
*/

	//	remove any verts that arent referenced by any triangles
	for ( v=0;	v<VertsKept.Size();	v++ )
	{
		if ( v % 20 == 0 )
		{
			GDebug_Print("MergeVerts: checking vertex references #%d...\n", v );
		}
	
		int vindex = VertsKept[v];
		Bool Unused = TRUE;

		//	remove triangle references to vb
		for ( t=0;	t<TriCount();	t++ )
		{
			for ( i=0;	i<3;	i++ )
			{
				if ( m_Triangles[t][i] == vindex )
				{
					Unused = FALSE;
					goto BreakTriCheck;
				}
			}

		}
		BreakTriCheck:;

		//	remove triangle references to vb
		for ( t=0;	t<TriStripCount();	t++ )
		{
			GTriStrip& TriStrip = m_TriStrips[t];
			for ( i=0;	i<TriStrip.m_Indicies.Size();	i++ )
			{
				if ( TriStrip.m_Indicies[i] == vindex )
				{
					Unused = FALSE;
					goto BreakTriStripCheck;
				}
			}

		}
		BreakTriStripCheck:;

		//	not referenced by any triangles, remove
		if ( Unused )
		{
			//	add to removed list. we can't modify vertskept or it'll mess up our main for loop, so we'll remove entries in both lists later
			VertsRemoved.Add( int2(vindex,-1) );
		}
	}



	//	update VertsKept list 
	for ( v=0;	v<VertsRemoved.Size();	v++ )
	{
		//	find the index in the kept list for this removed vert
		int KeptIndex = VertsKept.FindIndex( VertsRemoved[v][0] );

		//	if this removed vert is in the kept list, remove it from the kept list
		if ( KeptIndex != -1 )
		{
			VertsKept.RemoveAt( KeptIndex );
		}
	}

/*	//	make sure we havent gone wrong somewhere
	if ( VertsRemoved.Size() + VertsKept.Size() != m_Verts.Size() )
	{
		GDebug_Break("Temporary vertex array sizes are wrong\n");
		return;
	}
*/
	//	nothing changed
	if ( VertsRemoved.Size() == 0 )
	{
		GDebug_Print("No verts removed, aborting merge\n");
		return;
	}

	//	copy old arrays
	int NewVertCount = VertsKept.Size();
	GList<float3> OldVerts;
	GList<float3> OldNormals;
	GList<float2> OldTextureUV;
	OldVerts.Copy( m_Verts );
	OldNormals.Copy( m_Normals );
	OldTextureUV.Copy( m_TextureUV );

	//	copy verts from old verts
	for ( v=0;	v<VertsKept.Size();	v++ )
	{
		m_Verts[ v ] = OldVerts[ VertsKept[v] ];
		
		if ( m_Normals.Data() )
			m_Normals[ v ] = OldNormals[ VertsKept[v] ];

		if ( m_TextureUV.Data() )
			m_TextureUV[ v ] = OldTextureUV[ VertsKept[v] ];
	}

	//	resize
	GDebug_Print("MergeVerts: updating vertex indexes...\n" );
	AllocVerts( VertsKept.Size() );
		
	//	replace vert indexes
	for ( v=0;	v<VertsRemoved.Size();	v++ )
	{
		int VertRemoved = VertsRemoved[v][0];
		int VertReplaced = VertsRemoved[v][1];

		//	remove triangle references to vb
		for ( t=0;	t<TriCount();	t++ )
		{
			for ( i=0;	i<3;	i++ )
			{
				//	replace index
				if ( m_Triangles[t][i] == VertRemoved )
				{
					m_Triangles[t][i] = VertReplaced;
				}
			}
		}

		//	remove triangle strip references to vb
		for ( t=0;	t<TriStripCount();	t++ )
		{
			GTriStrip& TriStrip = m_TriStrips[t];
			for ( i=0;	i<TriStrip.m_Indicies.Size();	i++ )
			{
				if ( TriStrip.m_Indicies[i] == VertRemoved )
				{
					TriStrip.m_Indicies[i] = VertReplaced;
				}
			}
		}
	}


	//	re-index vert indexes
	for ( t=0;	t<TriCount();	t++ )
	{
		for ( i=0;	i<3;	i++ )
		{
			for ( v=0;	v<VertsKept.Size();	v++ )
			{
				int OldVert = VertsKept[v];
				int NewVert = v;

				//	replace index
				if ( m_Triangles[t][i] == OldVert )
					m_Triangles[t][i] = NewVert;
			}
		}
	}

	for ( t=0;	t<TriStripCount();	t++ )
	{
		GTriStrip& TriStrip = m_TriStrips[t];
		for ( i=0;	i<TriStrip.m_Indicies.Size();	i++ )
		{
			for ( v=0;	v<VertsKept.Size();	v++ )
			{
				int OldVert = VertsKept[v];
				int NewVert = v;

				//	replace index
				if ( TriStrip.m_Indicies[i] == OldVert )
					TriStrip.m_Indicies[i] = NewVert;
			}
		}
	}


	//	debug check
	GDebug_Print("MergeVerts: debug checks...\n" );

	for ( t=0;	t<TriCount();	t++ )
	{
		for ( i=0;	i<3;	i++ )
		{
			if ( m_Triangles[t][i] >= VertCount() )
			{
				GDebug_Break("Invalid vert in triangle\n");
			}
		}
	}

	for ( t=0;	t<TriStripCount();	t++ )
	{
		GTriStrip& TriStrip = m_TriStrips[t];
		for ( i=0;	i<TriStrip.m_Indicies.Size();	i++ )
		{
			if ( TriStrip.m_Indicies[i] >= VertCount() )
			{
				GDebug_Break("Invalid vert in tristrip\n");
			}
		}
	}

	//	check incase something went funny
	CheckFloats();

	//	finished
	GDebug_Print("Removed %d verts\n", VertsRemoved.Size() );

	if ( VertsRemoved.Size() )
		GDebug_Print("Warning: Skins will need re-skinning!\n" );
}



void GMesh::GenerateTriangleNeighbours()
{
	//	update size
	m_TriangleNeighbours.Resize(TriCount());

	//	reset neighbours
	for ( int t=0;	t<TriCount();	t++ )
	{
		m_TriangleNeighbours[t] = int3(-1,-1,-1);
	}


	int p1i, p2i, p1j, p2j;
	int P1i, P2i, P1j, P2j;
	int i,j,ki,kj;

	//	do connections
	for( i=0;	i<TriCount()-1;	i++ )
	{
		for( j=i+1;	j<TriCount();	j++ )
		{
			for( ki=0;	ki<3;	ki++ )
			{
				if( m_TriangleNeighbours[i][ki] == -1 )
				{
					for(	kj=0;	kj<3;	kj++ )
					{
						p1i	= ki;
						p1j	= kj;
						p2i	= (ki+1)%3;
						p2j	= (kj+1)%3;

						p1i	= m_Triangles[i][p1i];
						p2i	= m_Triangles[i][p2i];
						p1j	= m_Triangles[j][p1j];
						p2j	= m_Triangles[j][p2j];

						P1i=((p1i+p2i)-abs(p1i-p2i))/2;
						P2i=((p1i+p2i)+abs(p1i-p2i))/2;
						P1j=((p1j+p2j)-abs(p1j-p2j))/2;
						P2j=((p1j+p2j)+abs(p1j-p2j))/2;

						//	they are neighbours
						if((P1i==P1j) && (P2i==P2j))
						{
							m_TriangleNeighbours[i][ki]	= j;	  
							m_TriangleNeighbours[j][kj]	= i;	  
						}
					}
				}
			}
		}
	}



}



void GMesh::GenerateDebugColours()
{
	//	generates a debug colour for every triangle
	int t;
	
	//	make up green colours for triangles	
	m_TriangleColours.Resize( TriCount() );
	for ( t=0;	t<TriCount();	t++ )
	{
		m_TriangleColours[t] = GetDebugColour(t,GDebugColourBase_Green);
	}

	//	make up yellow colours for tristrips
	m_TriStripColours.Resize( TriStripCount() );
	for ( t=0;	t<TriStripCount();	t++ )
	{
		m_TriStripColours[t] = GetDebugColour(t,GDebugColourBase_Yellow);
	}

}



void GMesh::GenerateNormals(Bool ReverseOrder)
{
	//	generate our own normals from our triangles
	
	//	no geometry to generate the normals from?
	if ( TriCount() == 0 && TriStripCount() == 0 )
	{
		GDebug_Break("No geometry to generate normals from\n");
		return;
	}

	//	normals need verts!
	if ( VertCount() == 0 )
	{
		GDebug_Break("No verts to generate normals for\n");
		return;
	}


	m_Normals.Resize( VertCount() );
	m_Normals.SetAll( float3(0,1,0) );


	GList<Bool>	NormalIsSet;
	NormalIsSet.Resize( VertCount() );
	NormalIsSet.SetAll( (Bool)FALSE );

	//	go through each triangle
	for ( int t=0;	t<TriCount();	t++ )
	{
		float3 Normal;
		int3& Triangle = m_Triangles[t];
		int a = Triangle[0];
		int b = Triangle[1];
		int c = Triangle[2];
		float3& va = m_Verts[a];
		float3& vb = m_Verts[b];
		float3& vc = m_Verts[c];

		if ( a==b || b==c || a==c )
		{
			GDebug_Break("Found invalid triangle (has duplicate index['s] %d,%d,%d)\n", a, b, c);
			continue;
		}

		//	calculate normal
		if ( !ReverseOrder )
			Normal = CalcNormal( va, vb, vc );
		else
			Normal = CalcNormal( vc, vb, va );

		//	set new normal for first vertex
		if ( NormalIsSet[a] )
		{
			float3 NewNormal = m_Normals[a].CrossProduct( Normal );
			if ( NewNormal.LengthSq() != 0.f )
				m_Normals[a] = NewNormal;
		}
		else 
		{
			m_Normals[a] = Normal;
		}

		//	set new normal for second vertex
		if ( NormalIsSet[b] )
		{
			float3 NewNormal = m_Normals[b].CrossProduct( Normal );
			if ( NewNormal.LengthSq() != 0.f )
				m_Normals[b] = NewNormal;
		}
		else 
		{
			m_Normals[b] = Normal;
		}
		
		//	set new normal for third vertex
		if ( NormalIsSet[c] )	
		{
			float3 NewNormal = m_Normals[c].CrossProduct( Normal );
			if ( NewNormal.LengthSq() != 0.f )
				m_Normals[c] = NewNormal;
		}
		else 
		{
			m_Normals[c] = Normal;
		}

		//	check normals
		m_Normals[a].Normalise();
		GDebug::CheckFloat( m_Normals[a] );
		m_Normals[b].Normalise();
		GDebug::CheckFloat( m_Normals[b] );
		m_Normals[c].Normalise();
		GDebug::CheckFloat( m_Normals[c] );

		//	update "is set" data
		NormalIsSet[a] = 
		NormalIsSet[b] = 
		NormalIsSet[c] = TRUE;
	}

	//	go through each tristrip
	for ( int ts=0;	ts<TriStripCount();	ts++ )
	{
		float3 Normal;
		GTriStrip& TriStrip = m_TriStrips[ts];

		GTriangle Triangle;
		Triangle[0] = TriStrip.m_Indicies[0];
		Triangle[1] = TriStrip.m_Indicies[1];
		
		for ( int i=2;	i<TriStrip.m_Indicies.Size();	i++ )
		{
			Triangle[i%3] = TriStrip.m_Indicies[i];

			int& a = Triangle[0];
			int& b = Triangle[1];
			int& c = Triangle[2];
			float3& va = m_Verts[a];
			float3& vb = m_Verts[b];
			float3& vc = m_Verts[c];

			if ( a==b || b==c || a==c )
			{
				GDebug_Break("Found invalid triangle in tristrip (has duplicate index['s] %d,%d,%d)\n", a, b, c);
				continue;
			}

			//	calculate normal
			if ( !ReverseOrder )
				Normal = CalcNormal( va, vb, vc );
			else
				Normal = CalcNormal( vc, vb, va );

			//	set new normal for first vertex
			if ( NormalIsSet[a] )
			{
				float3 NewNormal = m_Normals[a].CrossProduct( Normal );
				if ( NewNormal.LengthSq() != 0.f )
					m_Normals[a] = NewNormal;
			}
			else 
			{
				m_Normals[a] = Normal;
			}

			//	set new normal for second vertex
			if ( NormalIsSet[b] )
			{
				float3 NewNormal = m_Normals[b].CrossProduct( Normal );
				if ( NewNormal.LengthSq() != 0.f )
					m_Normals[b] = NewNormal;
			}
			else 
			{
				m_Normals[b] = Normal;
			}
			
			//	set new normal for third vertex
			if ( NormalIsSet[c] )	
			{
				float3 NewNormal = m_Normals[c].CrossProduct( Normal );
				if ( NewNormal.LengthSq() != 0.f )
					m_Normals[c] = NewNormal;
			}
			else 
			{
				m_Normals[c] = Normal;
			}

			//	check normals
			m_Normals[a].Normalise();
			m_Normals[b].Normalise();
			m_Normals[c].Normalise();

			//	update "is set" data
			NormalIsSet[a] = 
			NormalIsSet[b] = 
			NormalIsSet[c] = TRUE;
		}
	}



	//	check in case anything went wrong
	CheckFloats();
}




float3 GMesh::GetTriangleNormal(GTriangle& Triangle)
{
	float3 n;
	const int& v1 = Triangle[0];
	const int& v2 = Triangle[1];
	const int& v3 = Triangle[2];

	n = m_Normals[ v1 ];
	n += m_Normals[ v2 ];
	n += m_Normals[ v3 ];
	n.Normalise();

	return n;
}


float3 GMesh::GetTriangleCenter(int t)
{
	float3 TriangleCenter;
	float3& v1 = m_Verts[ m_Triangles[t][0] ];
	float3& v2 = m_Verts[ m_Triangles[t][1] ];
	float3& v3 = m_Verts[ m_Triangles[t][2] ];

	TriangleCenter = v1;
	TriangleCenter += v2;
	TriangleCenter += v3;
	TriangleCenter *= 1.f/3.f;

	return TriangleCenter;
}



void GMesh::GenerateTriangleNormals(GList<GTriangle>& Triangles, GList<float3>& TriangleNormals)
{
	//	resize normal list
	TriangleNormals.Resize( Triangles.Size() );

	//	generate normals
	for ( int t=0;	t<Triangles.Size();	t++ )
	{
		TriangleNormals[t] = GetTriangleNormal( Triangles[t] );
	}
}


void GMesh::GenerateTriangleCenters()
{
	//	generate triangle centers from average of the triangles points
	m_TriangleCenters.Resize( TriCount() );

	for ( int t=0;	t<TriCount();	t++ )
	{
		m_TriangleCenters[t] = GetTriangleCenter(t);
	}
}





Bool GMesh::Load(GBinaryData& Data)
{
	#define READ_BLOCK(addr,size,typestr)	{	if ( !Data.Read( addr, size, typestr ) )	return FALSE;	}

	//	read a list from data (resize list then copy)
	#define READ_DATA_LIST( list, flag, arraysize, typestr )		\
	{																\
		if ( Header.MeshDataFlags & GMeshDataFlags::flag )			\
		{															\
			list.Resize( arraysize );								\
			READ_BLOCK( list.Data(), list.DataSize(), typestr );	\
		}															\
	}

	//	where we used to read a list, just skip over the data, if any
	#define SKIP_OVER_DATA_LIST( flag, arraysize, type, typestr )	\
	{																\
		if ( Header.MeshDataFlags & GMeshDataFlags::flag )			\
		{															\
			READ_BLOCK( NULL, sizeof(type) * arraysize, typestr );	\
		}															\
	}	


	//	read-in the header
	GMeshHeader Header;
	if ( !Data.Read( &Header, GDataSizeOf(GMeshHeader), "Mesh header" ) )
		return FALSE;

	//	from the header, allocate data
	AllocVerts( Header.VertexCount );
	AllocTriangles( Header.TriangleCount );
	AllocTriStrips( Header.TriStripCount );

	//	now read in the blocks of data
	READ_BLOCK( m_Verts.Data(),		VertCount() * GDataSizeOf(float3), "Mesh Verts" );

	READ_DATA_LIST( m_Normals,		VertNormals,	VertCount(), "Mesh Vert normals" );
	READ_DATA_LIST( m_TextureUV,	VertTextureUV,	VertCount(), "Mesh Vert UV's" );

	READ_BLOCK( m_Triangles.Data(),	TriCount() * GDataSizeOf(int3), "Mesh Triangles" );

	SKIP_OVER_DATA_LIST( /*m_TriangleNormals, */TriangleNormals,	TriCount(), float3, "Mesh Triangle normals" );	//	no longer used
	READ_DATA_LIST( m_TriangleColours,			TriangleColours,	TriCount(), "Mesh Triangle colours" );
	READ_DATA_LIST( m_TriangleNeighbours,		TriangleNeighbours,	TriCount(), "Mesh Triangle neighbours" );
	READ_DATA_LIST( m_TriangleCenters,			TriangleCenters,	TriCount(), "Mesh Triangle centers" );

	//	load each triangle strip
	for ( int t=0;	t<TriStripCount();	t++ )
	{
		if ( !m_TriStrips[t].Load( Data ) )
			return FALSE;
	}

	READ_DATA_LIST( m_TriStripColours,	TriStripColours,	TriStripCount(), "Mesh TriStrip colours" );

	//	load shadow data
	SKIP_OVER_DATA_LIST( /*m_ShadowQuads,	*/		OLD_ShadowQuads,	Header.UNUSEDShadowQuads, int4, "Mesh Shadow quads" );
	SKIP_OVER_DATA_LIST( /*m_ShadowVerts,		*/	OLD_ShadowVerts,	Header.UNUSEDShadowQuads*4, float3, "Mesh Shadow verts" );
	SKIP_OVER_DATA_LIST( /*m_ShadowVertNormals,*/	OLD_ShadowNormals,	Header.UNUSEDShadowQuads*4, float3, "Mesh Shadow vert normals" );

	//	load each triangle strip plane
	if ( Header.MeshDataFlags & GMeshDataFlags::TriStripPlanes )
	{
		m_TriStripPlanes.Resize( TriStripCount() );
		for ( int t=0;	t<TriStripCount();	t++ )
		{
			READ_DATA_LIST( m_TriStripPlanes[t], TriStripPlanes, m_TriStrips[t].m_Indicies.Size() - 2, "Mesh TriStrip planes"   );
		}
	}

	READ_DATA_LIST( m_TrianglePlanes,		TrianglePlanes,	Header.TriangleCount, "Mesh Triangle planes"   );

	//	read collision objects
	if ( Header.MeshDataFlags & GMeshDataFlags::CollisionObjects )
	{
		//	read general data for collision objects
		u8 ColObjCount = 0;			//	number of objects
		u32 ColObjVersion = 0x0;	//	version saved
		u16 ColObjDataLength = 0;	//	amount of data saved for collision objects

		READ_BLOCK( &ColObjCount,		GDataSizeOf(u8), "Collision objects count" );
		READ_BLOCK( &ColObjVersion,		GDataSizeOf(u32), "Collision objects version" );
		READ_BLOCK( &ColObjDataLength,	GDataSizeOf(u16), "Collision objects data size" );

		if ( ColObjDataLength > 0 )
		{
			GBinaryData ColObjData;
			ColObjData.Data().Resize( ColObjDataLength );
			READ_BLOCK( ColObjData.Data().Data(), ColObjData.Data().DataSize(), "Collison objects data" );
			ColObjData.ResetRead();

			//	if version matches, load
			if ( ColObjVersion != GCollisionObj::g_Version )
			{
				GDebug_Print("Mesh collision object version mismatch: 0x%08x. Expected 0x%08x\n", ColObjVersion, GCollisionObj::g_Version );
			}
			else
			{
				//	load each object from data
				for ( int c=0;	c<ColObjCount;	c++ )
				{
					GCollisionObj NewColObject;
					if ( NewColObject.Load( ColObjData ) )
					{
						m_CollisionObjects.Add( NewColObject );
					}
				}
			}
		}
	}

	//	load shadow data
	if ( Header.MeshDataFlags & GMeshDataFlags::ShadowData )
	{
		m_ShadowData.Load( Data );
	}

	#undef READ_BLOCK
	#undef READ_DATA_LIST
	#undef SKIP_OVER_DATA_LIST

	//	check in case anything went wrong
	CheckFloats();

	return TRUE;
}



Bool GMesh::Save(GBinaryData& SaveData)
{
	//	add the mesh header
	GMeshHeader Header;
	Header.TriangleCount	= TriCount();
	Header.VertexCount		= VertCount();
//	Header.BoundsOffset		= m_Bounds.m_Offset;
//	Header.BoundsRadius		= m_Bounds.m_Radius;
	Header.TriStripCount	= TriStripCount();
	Header.MeshDataFlags	= 0x0;
//	Header.ShadowQuads		= m_ShadowQuads.Size();

	//	work out what data we're saving (have to set flags before saving header)
	if ( m_Normals.Data() )				Header.MeshDataFlags |= GMeshDataFlags::VertNormals;
	if ( m_TextureUV.Data() )			Header.MeshDataFlags |= GMeshDataFlags::VertTextureUV;

	//if ( m_TriangleNormals.Data() )		Header.MeshDataFlags |= GMeshDataFlags::TriangleNormals;
	if ( m_TriangleColours.Data() )		Header.MeshDataFlags |= GMeshDataFlags::TriangleColours;
	if ( m_TriangleNeighbours.Data() )	Header.MeshDataFlags |= GMeshDataFlags::TriangleNeighbours;
	if ( m_TriangleCenters.Data() )		Header.MeshDataFlags |= GMeshDataFlags::TriangleCenters;

	if ( m_TriStripColours.Data() )		Header.MeshDataFlags |= GMeshDataFlags::TriStripColours;

//	if ( m_ShadowQuads.Data() )			Header.MeshDataFlags |= GMeshDataFlags::ShadowQuads;
//	if ( m_ShadowVerts.Data() )			Header.MeshDataFlags |= GMeshDataFlags::ShadowVerts;
//	if ( m_ShadowVertNormals.Data() )	Header.MeshDataFlags |= GMeshDataFlags::ShadowNormals;

	if ( m_TrianglePlanes.Data() )		Header.MeshDataFlags |= GMeshDataFlags::TrianglePlanes;
	if ( m_TriStripPlanes.Data() )		Header.MeshDataFlags |= GMeshDataFlags::TriStripPlanes;

	if ( m_CollisionObjects.Data() )	Header.MeshDataFlags |= GMeshDataFlags::CollisionObjects;

	if ( m_ShadowData.ValidData() )		Header.MeshDataFlags |= GMeshDataFlags::ShadowData;

	SaveData.Write( &Header, GDataSizeOf(GMeshHeader) );

	#define SAVE_DATA_ADD( DataVar, Flag, ExpectedSize )				\
	{																	\
		if ( Header.MeshDataFlags & GMeshDataFlags::Flag )				\
		{																\
			if ( DataVar.Size() != ExpectedSize )						\
			{															\
				GDebug_Break("Saving mesh data; List is wrong size\n");	\
				return FALSE;											\
			}															\
			SaveData.Write( DataVar.Data(), DataVar.DataSize() );	\
		}																\
	}																	\


	//	add vertex data first
	SaveData.Write( m_Verts.Data(),	m_Verts.DataSize() );
	SAVE_DATA_ADD( m_Normals,			VertNormals,	VertCount() );
	SAVE_DATA_ADD( m_TextureUV,			VertTextureUV,	VertCount() );
	
	//	add triangle data
	if ( TriCount() )
	{
		SaveData.Write( m_Triangles.Data(),	TriCount() * GDataSizeOf(int3) );
		//SAVE_DATA_ADD( m_TriangleNormals,		TriangleNormals,	TriCount() );
		SAVE_DATA_ADD( m_TriangleColours,		TriangleColours,	TriCount() );
		SAVE_DATA_ADD( m_TriangleNeighbours,	TriangleNeighbours,	TriCount() );
		SAVE_DATA_ADD( m_TriangleCenters,		TriangleCenters,	TriCount() );
	}

	//	save each triangle strip
	if ( TriStripCount() )
	{
		for ( int t=0;	t<TriStripCount();	t++ )
		{
			m_TriStrips[t].Save( SaveData );
		}
		SAVE_DATA_ADD( m_TriStripColours, TriStripColours,	TriStripCount() );
	}

	//	save shadow data
	/*
	if ( m_ShadowQuads.Data() )
	{
		SAVE_DATA_ADD( m_ShadowQuads,		ShadowQuads,	m_ShadowQuads.Size() );
		SAVE_DATA_ADD( m_ShadowVerts,		ShadowVerts,	m_ShadowQuads.Size()*4 );
		SAVE_DATA_ADD( m_ShadowVertNormals,	ShadowNormals,	m_ShadowQuads.Size()*4 );
	}
	*/

	//	save each triangle strip plane
	if ( m_TriStripPlanes.Size() == TriStripCount() )
	{
		for ( int t=0;	t<TriStripCount();	t++ )
		{
			SAVE_DATA_ADD( m_TriStripPlanes[t], TriStripPlanes,	m_TriStrips[t].m_Indicies.Size() - 2 );
		}
	}

	SAVE_DATA_ADD( m_TrianglePlanes,	TrianglePlanes,	TriCount() );

	//	save collision objects
	if ( Header.MeshDataFlags & GMeshDataFlags::CollisionObjects )
	{
		//	save general data for collision objects
		u8 ColObjCount = m_CollisionObjects.Size();								//	number of objects
		u32 ColObjVersion = GCollisionObj::g_Version;	//	version saved
		u16 ColObjDataLength = 0;						//	amount of data saved for collision objects

		//	get binary data before saving general data
		GBinaryData ColObjData;
		for ( int c=0;	c<m_CollisionObjects.Size();	c++ )
			m_CollisionObjects[c].Save( ColObjData );

		//	update size of data
		ColObjDataLength = ColObjData.Size();

		//	save general data
		SaveData.Write( &ColObjCount,	GDataSizeOf(u8) );
		SaveData.Write( &ColObjVersion,	GDataSizeOf(u32) );
		SaveData.Write( &ColObjDataLength,	GDataSizeOf(u16) );

		//	save binary data
		SaveData.Write( ColObjData );
	}

	//	save shadow data
	if ( Header.MeshDataFlags & GMeshDataFlags::ShadowData )
	{
		m_ShadowData.Save( SaveData );
	}


	#undef SAVE_DATA_ADD
	return TRUE;
}


void GMesh::BindVertexArray(GList<float3>& VertexBuffer, Bool UseAttribMode)
{
	if ( VertexBuffer.Size() != m_Verts.Size() || m_Verts.Size()==0 )
		return;

	//	use attrib call
	if ( UseAttribMode )
	{
		BindAttribArray( VertexBuffer, VERTEX_ATTRIB_INDEX );
		return;
	}

	//	enable texcoord array
	glEnableClientState( GL_VERTEX_ARRAY );

	//	use VBO data
	if ( g_DisplayExt.HardwareSupported( GHardware_VertexBufferObjects ) )
	{
		//	enabled
		if ( g_DisplayExt.HardwareEnabled(GHardware_VertexBufferObjects) && m_VBOVertexID )
		{
			//	upload data
			UploadVertexBufferData( (float*)VertexBuffer.Data(), VertexBuffer.DataSize(), m_VBOVertexID );

			SelectBufferObject( m_VBOVertexID );
			glVertexPointer( 3, GL_FLOAT, 0, NULL );
		}
		else
		{
			//	not enabled or setup, use regular data
			SelectBufferObjectNone();
			glVertexPointer( 3, GL_FLOAT, 0, VertexBuffer.Data() );
		}
	}
	else
	{
		//	if VBO not supproted just set ptr
		glVertexPointer( 3, GL_FLOAT, 0, VertexBuffer.Data() );
	}

	GDebug::CheckGLError();
}


void GMesh::BindNormalArray(GList<float3>& NormalBuffer, Bool UseAttribMode)
{
	if ( NormalBuffer.Size() != m_Verts.Size() || m_Verts.Size()==0 )
		return;

	//	use attrib call
	if ( UseAttribMode )
	{
		BindAttribArray( NormalBuffer, NORMAL_ATTRIB_INDEX );
		return;
	}

	//	enable normal array
	glEnableClientState( GL_NORMAL_ARRAY );

	//	use VBO data
	if ( g_DisplayExt.HardwareEnabled( GHardware_VertexBufferObjects ) && m_VBONormalID )
	{
		//	enabled
		if ( g_DisplayExt.HardwareEnabled(GHardware_VertexBufferObjects) && m_VBONormalID )
		{
			SelectBufferObject( m_VBONormalID );
			glNormalPointer( GL_FLOAT, 0, NULL );
		}
		else
		{
			//	not enabled or setup, use regular data
			SelectBufferObjectNone();
			glNormalPointer( GL_FLOAT, 0, NormalBuffer.Data() );
		}
	}
	else
	{
		//	if VBO not supproted just set ptr
		glNormalPointer( GL_FLOAT, 0, NormalBuffer.Data() );
	}

	GDebug::CheckGLError();
}


void GMesh::BindTextureUVArray(GList<float2>& TextureUVBuffer, Bool UseAttribMode)
{
	if ( TextureUVBuffer.Size() != m_Verts.Size() || m_Verts.Size()==0 )
		return;

	//	use attrib call
	if ( UseAttribMode )
	{
		BindAttribArray( TextureUVBuffer, TEXCOORD0_ATTRIB_INDEX );
		return;
	}

	//	change multitexture client state before enabling/disabling
	if ( g_DisplayExt.HardwareSupported( GHardware_MultiTexturing ) )
		g_DisplayExt.glClientActiveTextureARB()( GL_TEXTURE0_ARB );
	
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	//	use VBO data
	if ( g_DisplayExt.HardwareEnabled( GHardware_VertexBufferObjects ) && m_VBOTexCoordID )
	{
		SelectBufferObject( m_VBOTexCoordID );
		glTexCoordPointer( 2, GL_FLOAT, 0, NULL );
	}
	else
	{
		//	if VBO not supproted just set ptr
		glTexCoordPointer( 2, GL_FLOAT, 0, TextureUVBuffer.Data() );
	}

	GDebug::CheckGLError();
}


void GMesh::BindTextureUV2Array(GList<float2>& TextureUV2Buffer, Bool UseAttribMode)
{
	if ( TextureUV2Buffer.Size() != m_Verts.Size() || m_Verts.Size()==0 )
		return;

	//	use attrib call
	if ( UseAttribMode )
	{
		BindAttribArray( TextureUV2Buffer, TEXCOORD1_ATTRIB_INDEX );
		return;
	}

	//	change multitexture client state before enabling/disabling
	if ( g_DisplayExt.HardwareSupported( GHardware_MultiTexturing ) )
		g_DisplayExt.glClientActiveTextureARB()( GL_TEXTURE1_ARB );
	
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

/*	//	use VBO data
	if ( g_DisplayExt.HardwareEnabled( GHardware_VertexBufferObjects ) && m_VBOTexCoord2ID )
	{
		SelectBufferObject( m_VBOTexCoord2ID );
		glTexCoordPointer( 2, GL_FLOAT, 0, NULL );
	}
	else
*/	{
		//	if VBO not supproted just set ptr
		glTexCoordPointer( 2, GL_FLOAT, 0, TextureUV2Buffer.Data() );
	}

	GDebug::CheckGLError();}


void GMesh::BindColourArray(GList<float3>& ColourBuffer, Bool UseAttribMode)
{
	if ( ColourBuffer.Size() != m_Verts.Size() || m_Verts.Size()==0 )
		return;

	//	use attrib call
	if ( UseAttribMode )
	{
		BindAttribArray( ColourBuffer, COLOUR0_ATTRIB_INDEX );
		return;
	}

	//	todo: VBO for colours

	glEnableClientState( GL_COLOR_ARRAY );
	glColorPointer( 3, GL_FLOAT, 0, ColourBuffer.Data() );

	GDebug::CheckGLError();
}

//-------------------------------------------------------------------------
//	bind data array to an attrib
//-------------------------------------------------------------------------
void GMesh::BindAttribArray(void* pBuffer, int Type, int ElementSize, int AttribIndex)
{
	if ( !pBuffer )
	{
		GDebug_Break("tried to bind NULL buffer to vertex attrib %d\n", AttribIndex );
		return;
	}
	
	//	check valid attrib index
	GDebug::CheckIndex( AttribIndex, 0, MAX_ATTRIB_INDEX );
	
	#ifdef USE_ARB_PROGRAM
		//	need hardware support
		if ( !g_DisplayExt.HardwareEnabled( GHardware_ARBVertexProgram ) )
		{
			GDebug_Break("Hardware extension required for vertex attrib array\n");
			return;
		}

		//	enable attrib
		g_DisplayExt.glEnableVertexAttribArrayARB()( AttribIndex );

		//	bind data
		g_DisplayExt.glVertexAttribPointerARB()( AttribIndex, ElementSize, Type, GL_FALSE, 0, pBuffer );

	#else
		//	need hardware support
		if ( !g_DisplayExt.HardwareEnabled( GHardware_NVVertexProgram ) )
		{
			GDebug_Break("Hardware extension required for vertex attrib array\n");
			return;
		}

		//	enable attrib
		glEnableClientState( GL_VERTEX_ATTRIB_ARRAY0_NV+AttribIndex );

		//	bind data
		g_DisplayExt.glVertexAttribPointerNV()( AttribIndex, ElementSize, Type, 0, pBuffer );
	#endif

	GDebug::CheckGLError();
}

//-------------------------------------------------------------------------
//	draw all the geometry in the mesh. Drawinfo provides all the drawing/debug 
//	flags, transform and lighting info
//-------------------------------------------------------------------------
GDrawResult GMesh::Draw(GDrawInfo& DrawInfo)
{
	//	modify flags
	DrawInfo.Flags |= GMesh::g_ForceFlagsOn;
	DrawInfo.Flags &= ~GMesh::g_ForceFlagsOff;

	//	check mesh before drawing
	if ( !VertCount() || ( !TriCount() && !TriStripCount() ) )
		return GDrawResult_Nothing;

	//	alpha is set at zero, its not visible!
	if ( DrawInfo.RGBA[3] < NEAR_ZERO )
	{
		return GDrawResult_NotVisible;
	}

	//	check culling
	/*
	todo: new collision object culling
	if ( ! ( DrawInfo.Flags & GDrawInfoFlags::DontCullTest ) )
	{
		if ( m_Bounds.IsCulled( DrawInfo.WorldPos ) )
		{
			GIncCounter(CulledMeshes,1);
			return GDrawResult_Culled;
		}
	}
	*/

	GIncCounter(DrawMeshes,1);

	//	save scene settings
	g_Display->PushScene();
	g_Display->Translate( DrawInfo.Translation, DrawInfo.pRotation, DrawInfo.DoTranslation(), DrawInfo.DoRotation() );

	
	//	bits of what additional data we're processing
	u32 DrawData = 0x0;
	#define NORMALS			(1<<0)	
	#define TEXTURE			(1<<1)
	#define TEXTURE2		(1<<2)	//	multitexturing
	#define COLOUR			(1<<3)	//	vertex colours
	#define ALPHA			(1<<4)
	#define TEXALPHA		(1<<5)
	#define TEX2ALPHA		(1<<6)
	#define NOMAINRENDER	(1<<7)

	GList<float3> TEMP_COLOUR_BUFFER;

	GList<float3>* pVertexBuffer		= &m_Verts;
	GList<float3>* pNormalBuffer		= &m_Normals;
	GList<float2>* pTextureUVBuffer		= &m_TextureUV;
	GList<float2>* pTextureUV2Buffer	= &m_TextureUV;	//	temp
	GList<float3>* pColourBuffer		= &TEMP_COLOUR_BUFFER;

	//	setup shader
	Bool BindAttribs = FALSE;
	if ( DrawInfo.pShader )
	{
		GShaderMode Mode = DrawInfo.pShader->SetDrawMode();
		
		if ( Mode == GShaderMode_Hardware )
			BindAttribs = TRUE;
		
		if ( Mode == GShaderMode_None )
			DrawInfo.pShader = NULL;
	}

	//	setup texturing
	GTexture* pTexture = NULL;

	if ( !(DrawInfo.Flags & GDrawInfoFlags::DisableTextures) )
		pTexture = GAssets::g_Textures.Find( DrawInfo.TextureRef );

	if ( pTexture )
	{
		DrawData |= TEXTURE;
		pTexture->Select();
		glEnable( pTexture->TextureType() );
	
		if ( pTexture->AlphaChannel() )
			DrawData |= TEXALPHA;
	}

	GTexture* pTexture2 = NULL;
	
	if ( !(DrawInfo.Flags & GDrawInfoFlags::DisableTextures == 0x0) )
		pTexture2 = GAssets::g_Textures.Find( DrawInfo.TextureRef2 );

	if ( pTexture2 )
	{
		DrawData |= TEXTURE2;
		pTexture2->Select2();
		glEnable( pTexture2->TextureType() );

		if ( pTexture2->AlphaChannel() )
			DrawData |= TEX2ALPHA;
	}

	//	disable texturing if no textures
	if ( ( DrawData & (TEXTURE|TEXTURE2) ) == 0x0 )
	{
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_TEXTURE_1D );
	}


	//	do we want to do our basic render
	if ( DrawInfo.Flags & GDrawInfoFlags::DebugColours )
		DrawData |= NOMAINRENDER;


	//	setup colour
	glColor4fv( DrawInfo.RGBA );

	//	setup alpha
	if ( DrawInfo.RGBA[3] < 1.f || (DrawData & (ALPHA|TEXALPHA|TEX2ALPHA) ) )
	{
		DrawData |= ALPHA;
		glEnable( GL_BLEND );

		if ( DrawData & TEXALPHA )
		{
			//	use texture1 alpha blending mode
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else if ( DrawData & TEX2ALPHA )
		{
			//	use texture2 alpha blending mode
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			//	RGBA alpha, use basic blending
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	}



	//	setup lighting
	if ( DrawInfo.Flags & GDrawInfoFlags::HardwareLighting )
	{
		//	grab light positions or something
		/* todo: position lights
		*/
		glEnable( GL_LIGHTING );
	}


	//	set face culling
	if ( ( DrawInfo.Flags & GDrawInfoFlags::DontCullBackfaces )==0x0 )
	{
		glEnable( GL_CULL_FACE );

		//	cull backfaces unless flagged
		if ( DrawInfo.Flags & GDrawInfoFlags::CullFrontFaces )
		{
			//glCullFace( GL_BACK );
			glCullFace( GL_FRONT );
		}
		else
		{
			//glCullFace( GL_FRONT );
			glCullFace( GL_BACK );
		}
	}

	//	setup other flags
	if ( DrawInfo.Flags & GDrawInfoFlags::Wireframe )
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	}

	//	setup vertex shader
	if ( DrawInfo.pShader )
	{
		//	has a shader, setup a vertex buffer
		DrawInfo.pShader->PreDraw( this, DrawInfo,	pVertexBuffer, 
													pNormalBuffer,
													pTextureUVBuffer,
													pTextureUV2Buffer,
													pColourBuffer );
	}

	//	setup pixel shader
	if ( DrawInfo.pPixelShader )
	{
		DrawInfo.pPixelShader->PreDraw( this, DrawInfo,	pVertexBuffer, 
													pNormalBuffer,
													pTextureUVBuffer,
													pTextureUV2Buffer,
													pColourBuffer );
	}


	//	bind textures
	if ( DrawData & TEXTURE )
	{
		BindTextureUVArray( *pTextureUVBuffer, BindAttribs );
	}
	else
	{
		if ( g_DisplayExt.HardwareSupported( GHardware_MultiTexturing ) )
			g_DisplayExt.glClientActiveTextureARB()( GL_TEXTURE0_ARB );
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	}

	if ( DrawData & TEXTURE2 )
	{
		BindTextureUV2Array( *pTextureUV2Buffer, BindAttribs );
	}
	else
	{
		if ( g_DisplayExt.HardwareSupported( GHardware_MultiTexturing ) )
		{
			g_DisplayExt.glClientActiveTextureARB()( GL_TEXTURE1_ARB );
			glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		}
	}

	//	setup normals
	BindNormalArray( *pNormalBuffer, BindAttribs );

	//	set vertex pointer
	BindVertexArray( *pVertexBuffer, BindAttribs );

	//	setup colours
	if ( DrawData & COLOUR )
		BindColourArray( *pColourBuffer, BindAttribs );
	else
		glDisableClientState( GL_COLOR_ARRAY );


	//	do primitive render
	if ( ( DrawData & NOMAINRENDER ) == 0x0 )
	{
		//	need to do two-pass(back then front) transparent render
		if ( DrawData & (ALPHA|TEXALPHA|TEX2ALPHA) )
		{
			glEnable( GL_CULL_FACE );
	
			//	draw back faces first
			if ( DrawInfo.Flags & GDrawInfoFlags::CullFrontFaces )
				glCullFace( GL_FRONT );
			else
				glCullFace( GL_BACK );
			DrawPrimitives();

			//	now draw (alpha'd) faces on top of back faces
			if ( DrawInfo.Flags & GDrawInfoFlags::CullFrontFaces )
				glCullFace( GL_BACK );
			else
				glCullFace( GL_FRONT );
			DrawPrimitives();
		}
		else
		{
			//	no transparencies, just render
			DrawPrimitives();
		}
	}


	//--------------------------
	//	draw debug stuff
	
	//	draw bounding box
	if ( DrawInfo.Flags & GDrawInfoFlags::DebugCollisionObjects )
	{
		for ( int c=0;	c<m_CollisionObjects.Size();	c++ )
			m_CollisionObjects[c].DebugDraw( DrawInfo.WorldPos );
	}

	//	draw normals pointing out of the verts
	if ( DrawInfo.Flags & GDrawInfoFlags::DebugNormals )
	{
		DrawDebugNormals( *pVertexBuffer );
	}

	//	draw normals pointing out of the faces
	if ( DrawInfo.Flags & GDrawInfoFlags::DebugFaceNormals )
	{
		DrawDebugTrianglePlanes( *pVertexBuffer );
	}	

	//	draw triangles individually in their debug colours
	if ( DrawInfo.Flags & GDrawInfoFlags::DebugColours )
	{
		if ( DrawData & ALPHA )
		{
			glEnable( GL_CULL_FACE );
			glCullFace( GL_FRONT );
			DrawDebugColours( *pVertexBuffer, DrawInfo.RGBA.w, DrawInfo.pShader );
		
			glCullFace( GL_BACK );
			DrawDebugColours( *pVertexBuffer, DrawInfo.RGBA.w, DrawInfo.pShader );
		}
		else
		{
			DrawDebugColours( *pVertexBuffer, 1.f, DrawInfo.pShader );
		}
	}

	//	draw vertexes
	if ( DrawInfo.Flags & GDrawInfoFlags::DebugVertexes )
	{
		GList<float3>* pDebugColours = DrawInfo.pVertexColours ? DrawInfo.pVertexColours : pColourBuffer;
		DrawDebugVertexes( *pVertexBuffer, *pDebugColours, DrawInfo.pShader );
	}


	//	draw second pass with white outlines only
	if ( DrawInfo.Flags & GDrawInfoFlags::DebugOutline )
	{
		glDisable( GL_DEPTH_TEST );
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_LIGHTING );
		glDisable( GL_CULL_FACE );
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glColor4f( 1,1,1,1 );
		DrawPrimitives();
	}

	//	turn off textures
	if ( pTexture )
		pTexture->SelectNone();

	if ( pTexture2 )
		pTexture2->SelectNone2();

	//	disable attribs
	if ( BindAttribs )
	{
		//	disable shader attribs
		#ifdef USE_ARB_PROGRAM
			for ( int ai=0;	ai<MAX_ATTRIB_INDEX;	ai++ )
				g_DisplayExt.glDisableVertexAttribArrayARB()( ai );
		#else
			for ( int ai=0;	ai<MAX_ATTRIB_INDEX;	ai++ )
				glDisableClientState( GL_VERTEX_ATTRIB_ARRAY0_NV+ai );
		#endif
	}
	else
	{
		//	disable regular attribs
		glDisableClientState( GL_COLOR_ARRAY );
		glDisableClientState( GL_NORMAL_ARRAY );
		glDisableClientState( GL_VERTEX_ARRAY );

		if ( g_DisplayExt.HardwareEnabled( GHardware_MultiTexturing ) )
		{
			g_DisplayExt.glClientActiveTextureARB()( GL_TEXTURE0_ARB );
			glDisableClientState( GL_TEXTURE_COORD_ARRAY );
			
			g_DisplayExt.glClientActiveTextureARB()( GL_TEXTURE1_ARB );
			glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		}
		else
		{
			glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		}
	}

	//	finish with shader
	if ( DrawInfo.pShader )
	{
		DrawInfo.pShader->PostDraw( this, DrawInfo );
	}

	//	finish with pixel shader
	if ( DrawInfo.pPixelShader )
	{
		DrawInfo.pPixelShader->PostDraw( this, DrawInfo );
	}

	//	restore scene
	g_Display->PopScene();

	//	drawn okay
	return GDrawResult_Drawn;
}





//-------------------------------------------------------------------------
//	draw little lines sticking out the verts representing normals
//-------------------------------------------------------------------------
void GMesh::DrawDebugNormals(GList<float3>& Verts)
{
	glPushClientAttrib( GL_CLIENT_VERTEX_ARRAY_BIT );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );

	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );
//	glDisable( GL_DEPTH_TEST );

	glColor3fv( GDisplay::g_DebugColour );

	glBegin(GL_LINES);
	for ( int i=0;	i<VertCount();	i++ )
	{
		float3 n = m_Normals[i];
		float3 v = Verts[i];
		
		glVertex3fv( v );
		v += n * 0.2f;
		glVertex3fv( v );
	}
	glEnd();

	glPopAttrib();
	glPopClientAttrib();

	GIncCounter( DebugPolys, VertCount() );
}




//-------------------------------------------------------------------------
//	draw little lines sticking out the middle of the triangles representing normals
//-------------------------------------------------------------------------
void GMesh::DrawDebugTrianglePlanes(GList<float3>& Verts)
{
	const float NormalLength = 2.0f;

	glPushClientAttrib( GL_CLIENT_VERTEX_ARRAY_BIT );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );

	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );
//	glDisable( GL_DEPTH_TEST );

	glColor3fv( GDisplay::g_DebugColour );

	glBegin(GL_LINES);

	//	draw triangles
	if ( m_TrianglePlanes.Size() == TriCount() )
	{
		for ( int t=0;	t<TriCount();	t++ )
		{
			float3& v1 = m_Verts[ m_Triangles[t][0] ];
			float3& v2 = m_Verts[ m_Triangles[t][1] ];
			float3& v3 = m_Verts[ m_Triangles[t][2] ];
			float3 n = m_TrianglePlanes[t].Normal();
			n.Normalise();
			
			float3 v = v1 + v2 + v3;
			v *= 1.f/3.f;
			
			glVertex3fv( v );
			v += n * NormalLength;
			glVertex3fv( v );
		}
	}

	//	draw triangle strips
	if ( m_TriStripPlanes.Size() == TriStripCount() )
	{
		for ( int ts=0;	ts<TriStripCount();	ts++ )
		{
			GTriStrip& TriStrip = m_TriStrips[ts];
			GPlaneList& TriStripPlanes = m_TriStripPlanes[ts];
			
			float3 v[3];
			v[0] = m_Verts[ TriStrip.m_Indicies[0] ];
			v[1] = m_Verts[ TriStrip.m_Indicies[1] ];
		
			for ( int i=2;	i<TriStrip.m_Indicies.Size();	i++ )
			{
				v[i%3] = m_Verts[ TriStrip.m_Indicies[i] ];

				float3 Center = v[0] + v[1] + v[2];
				Center  *= 1.f/3.f;
				float3 n = TriStripPlanes[i-2].Normal();
				n.Normalise();

				glVertex3fv( Center );
				Center += n * NormalLength;
				glVertex3fv( Center );
			}
		}
	}
	
	glEnd();

	glPopAttrib();
	glPopClientAttrib();

	GIncCounter( DrawPolys, TriCount() );
}




//-------------------------------------------------------------------------
//	render our geometry manually(not using lists) so we can specify the debug 
//	colours for each polygon
//-------------------------------------------------------------------------
void GMesh::DrawDebugColours(GList<float3>& Verts,float Alpha,GShader* pShader)
{
	//	shader not needed if not in hardware mode
	if ( pShader )
		if ( pShader->DrawMode() != GShaderMode_Hardware )
			pShader = NULL;
		
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );

	int t;

	//	manually draw triangles
	float3 Colour = GetDebugColour(0);
	glBegin( GL_TRIANGLES );
		for ( t=0;	t<TriCount();	t++ )
		{
			if ( m_TriangleColours.Size() > t )
				Colour = m_TriangleColours[t];

			int3& tri = m_Triangles[t];

			//	copy (not references) so shader can modify it
			float3 v1 = Verts[ tri[0] ];
			float3 v2 = Verts[ tri[1] ];
			float3 v3 = Verts[ tri[2] ];
			float3 n1 = m_Normals[ tri[0] ];
			float3 n2 = m_Normals[ tri[1] ];
			float3 n3 = m_Normals[ tri[2] ];

			//	provide colour with alpha
			glColor4f( Colour.x, Colour.y, Colour.z, Alpha );

			//	make sure we still pass in normals for back/front face culling
			if ( pShader )		pShader->HardwarePreDrawVertex( v1, n1, tri[0] );
			glVertex3fv( v1 );
			glNormal3fv( n1 );

			if ( pShader )		pShader->HardwarePreDrawVertex( v2, n2, tri[1] );
			glVertex3fv( v2 );
			glNormal3fv( n2 );

			if ( pShader )		pShader->HardwarePreDrawVertex( v3, n3, tri[2] );
			glVertex3fv( v3 );
			glNormal3fv( n3 );
		}
	glEnd();
	GIncCounter( DrawPolys, TriCount() );

	
	//	manually draw tri strips
	for ( int ts=0;	ts<TriStripCount();	ts++ )
	{
		GTriStrip& TriStrip = m_TriStrips[ts];
	
		//	provide colour with alpha
		if ( m_TriStripColours.Size() > ts )
			Colour = m_TriStripColours[ts];

		glColor4f( Colour.x, Colour.y, Colour.z, Alpha );

		
		glBegin( GL_TRIANGLE_STRIP );
			for ( t=0;	t<TriStrip.m_Indicies.Size();	t++ )
			{
				int Index = TriStrip.m_Indicies[t];
				//	copy (not references) so shader can modify it
				float3 v = Verts[ Index ];
				float3 n = m_Normals[ Index ];

				//	still need to pass in normals for back/front face culling
				if ( pShader )		pShader->HardwarePreDrawVertex( v, n, Index );
				glVertex3fv( v );
				glNormal3fv( n );
			}
		glEnd();
	
		GIncCounter( DrawPolys, TriStrip.m_Indicies.Size()-2 );
	}



	glPopAttrib();

}

//-------------------------------------------------------------------------
//	draw a point at each vertex with the specified colour
//-------------------------------------------------------------------------
void GMesh::DrawDebugVertexes(GList<float3>& Verts, GList<float3>& VertColours,GShader* pShader)
{
	if ( Verts.Size() != VertColours.Size() || Verts.Size() == 0 )
		return;
	
	//	shader not needed if not in hardware mode
	if ( pShader )
		if ( pShader->DrawMode() != GShaderMode_Hardware )
			pShader = NULL;
		
	//	draw each vert
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_DEPTH_TEST );
	glPointSize(3.f);

	glBegin( GL_POINTS );
	glColor3fv( GDisplay::g_DebugColour );
	for ( int v=0;	v<Verts.Size();	v++ )
	{
		//	set colour
		glColor3fv( VertColours[v] );

		float3 Vertex = Verts[v];
		float3 Normal;	//	unused

		if ( pShader )	
			pShader->HardwarePreDrawVertex( Vertex, Normal, v );

		glVertex3fv( Vertex );
	}
	glEnd();

	glPopAttrib();
}

//-------------------------------------------------------------------------
//	send all the main polygon data to the graphics card
//-------------------------------------------------------------------------
void GMesh::DrawPrimitives()
{
	Bool DrawRange = g_DisplayExt.HardwareEnabled( GHardware_DrawRangeElements );
	
	//	draw triangles
	if ( TriCount() > 0 )
	{
		if ( DrawRange )
		{
			g_DisplayExt.glDrawRangeElementsARB()( GL_TRIANGLES, 0, TriCount()*3, TriCount()*3, GL_UNSIGNED_INT, m_Triangles.Data() );
		}
		else
		{
			glDrawElements( GL_TRIANGLES, TriCount()*3, GL_UNSIGNED_INT, m_Triangles.Data() );
		}
	}

	GDebug::CheckGLError();
	GIncCounter( DrawPolys, TriCount() );

	//	draw tri strips
	for ( int ts=0;	ts<TriStripCount();	ts++ )
	{
		GTriStrip& TriangleStrip = m_TriStrips[ts];
		
		if ( DrawRange )
		{
			g_DisplayExt.glDrawRangeElementsARB()( GL_TRIANGLE_STRIP, 0, TriangleStrip.m_Indicies.Size(), TriangleStrip.m_Indicies.Size(), GL_UNSIGNED_INT, TriangleStrip.m_Indicies.Data() );
		}
		else
		{
			glDrawElements( GL_TRIANGLE_STRIP, TriangleStrip.m_Indicies.Size(), GL_UNSIGNED_INT, TriangleStrip.m_Indicies.Data() );
		}

		GDebug::CheckGLError();
		GIncCounter( DrawPolys, TriangleStrip.m_Indicies.Size() - 2 );
	}

}








inline void CalcVert( GMesh* pMesh, GDrawInfo& DrawInfo, int& VertIndex, GList<Bool>& VertCalcd, GList<float3>& VertCached )
{
	//	if it's not already been calculated, calc it
	//	only needed for modelspace rotation

	if ( ! VertCalcd[ VertIndex ] )
	{
		VertCached[ VertIndex ] = pMesh->m_Verts[ VertIndex ];

		//	apply rotation matrix
		if ( DrawInfo.DoRotation() )
		{
			if ( DrawInfo.pRotation->IsValid() )
			{
				GMatrix RotMatrix;
				QuaternionToMatrix( *DrawInfo.pRotation, RotMatrix );
				VertCached[ VertIndex ] = RotMatrix * VertCached[ VertIndex ];
			}
		}

		//	translate (world space)
		if ( DrawInfo.DoTranslation() )
		{
			VertCached[ VertIndex ] += DrawInfo.Translation;
		}

		VertCalcd[ VertIndex ] = TRUE;
	}
	

}

	

inline void ShadowCalcPlane( GMesh* pMesh, GDrawInfo& DrawInfo, int3& Triangle, GPlane& Plane, GList<Bool>& VertCalcd, GList<float3>& VertCached )
{
	int& vi1 = Triangle[0];
	int& vi2 = Triangle[1];
	int& vi3 = Triangle[2];

	//	vert not already calculated, calc it
	CalcVert( pMesh, DrawInfo, vi1, VertCalcd, VertCached );
	CalcVert( pMesh, DrawInfo, vi2, VertCalcd, VertCached );
	CalcVert( pMesh, DrawInfo, vi3, VertCalcd, VertCached );

	float3& v1 = VertCached[ vi1 ];
	float3& v2 = VertCached[ vi2 ];
	float3& v3 = VertCached[ vi3 ];

	Plane.CalcEquation( v1, v2, v3 );
}




void GMesh::DrawShadow(GDrawInfo& DrawInfo)
{
	//	check stuff we need
	if ( !DrawInfo.pLight )
	{
		GDebug_Break("Shadow rendering requires a light\n");
		return;
	}

	if ( m_TriangleNeighbours.Size() != TriCount() )
	{
		GDebug_Print("Mesh is missing triangle neighbours for shadows\n");
		return;
	}

	GIncCounter( DrawShadows, 1 );

	float4	ShadowColour( 0.f, 0.f, 0.f, 0.4f );

	//	planes information
	GList<Bool> PlaneVisible;
	PlaneVisible.Resize( TriCount() );

	//	rotate verts with matrix only once!
	GList<Bool> VertCalcd;
	GList<float3> VertCached;
	VertCalcd.Resize( VertCount() );
	VertCached.Resize( VertCount() );
	VertCalcd.SetAll( (Bool)FALSE );

	//	determine faces facing the light
	for ( int t=0;	t<TriCount();	t++ )
	{
		//	calc plane for this triangle
		GPlane TrianglePlane;
		ShadowCalcPlane( this, DrawInfo, m_Triangles[t], TrianglePlane, VertCalcd, VertCached );

		float Side	=	TrianglePlane[0] * DrawInfo.pLight->m_Pos.x +
						TrianglePlane[1] * DrawInfo.pLight->m_Pos.y +
						TrianglePlane[2] * DrawInfo.pLight->m_Pos.z +
						TrianglePlane[3] * 1.f;
						//pGeoPlanes[f].d * 0.f;
		PlaneVisible[t] = ( Side < 0.f );
	}


	//	setup rendering
	glPushAttrib( GL_ALL_ATTRIB_BITS );

	if ( DrawInfo.Flags & GDrawInfoFlags::DebugShadows )
	{
		glColor3fv( GDisplay::g_DebugColour );

	}
	else
	{

		glEnable( GL_CULL_FACE );
		glCullFace( GL_FRONT );

 		glDisable(GL_LIGHTING);
		glDepthMask(GL_FALSE);	//	dont write to the depth buffer
		glDepthFunc(GL_LEQUAL);
		glDepthFunc(GL_LESS);

		glEnable(GL_STENCIL_TEST);
		glColorMask(0, 0, 0, 0);	//	dont write to the colour buffer
		glStencilFunc(GL_ALWAYS, 1, 0xfffffff);
	}


	//	keep a recorded of which triangles/edges to draw once we've done it once
	//	first 16 bits are triangle, next bits are edge
	GList<u32> TriangleProcessList;
	TriangleProcessList.Realloc( TriCount() * 3 );


	// First Pass. Increase Stencil Value In The Shadow
	glFrontFace( GL_CCW );
	glStencilOp( GL_KEEP, GL_KEEP, GL_INCR );

	DoShadowRender( DrawInfo, PlaneVisible, VertCalcd, VertCached, TriangleProcessList );


	//	remove polys from the shadow volume (if neither of these flags are set)
	if ( ! ( DrawInfo.Flags & (GDrawInfoFlags::DebugShadows) ) )
	{
		// Second Pass. Decrease Stencil Value In The Shadow
		glFrontFace( GL_CW );
		glStencilOp( GL_KEEP, GL_KEEP, GL_DECR );

		//	render all the same values/polygons again
		//	use a list saved from last time
		for ( int i=0;	i<TriangleProcessList.Size();	i++ )
		{
			u32 Tri = TriangleProcessList[ i ] & 0x0000ffff;
			u32 Edge = (TriangleProcessList[ i ]>>16) & 0x0000ffff;

			DoShadowCastFromEdge( DrawInfo, Tri, Edge, VertCalcd, VertCached );
		}
	}

	if ( ! ( DrawInfo.Flags & GDrawInfoFlags::DebugShadows ) )
	{
		glFrontFace( GL_CCW );
		glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );	// Enable Rendering To Colour Buffer For All Components

		// Draw A Shadowing Rectangle Covering The Entire Screen
		glColor4fv( ShadowColour);
		if ( ShadowColour[3] < 1.f )
		{
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}

		//	keep, and draw into any bits of the stencil buffer that have been written (not Zero)
		glStencilFunc( GL_NOTEQUAL, 0, 0xffffffff );
		glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
		


		//	draw a plain polygon over the camera
		glPushMatrix();
		glLoadIdentity();
		glOrtho( -1, 1, -1, 1, GCamera::g_pActiveCamera->m_NearZ, -GCamera::g_pActiveCamera->m_FarZ );

		#define QUADW	1.f
		#define QUADZ	-GCamera::g_pActiveCamera->m_NearZ

		glBegin( GL_TRIANGLE_STRIP );
			glVertex3f( -QUADW, -QUADW, QUADZ );
			glVertex3f( -QUADW,  QUADW, QUADZ );
			glVertex3f(  QUADW, -QUADW, QUADZ );
			glVertex3f(  QUADW,  QUADW, QUADZ );
		glEnd();

		glPopMatrix();
	}


	glPopAttrib();

}

/*
void GMesh::DrawShadow(GDrawInfo& DrawInfo)
{
	//	check stuff we need
	if ( !DrawInfo.pLight )
	{
		GDebug_Break("Shadow rendering requires a light\n");
		return;
	}

	if ( m_TriangleNeighbours.Size() != TriCount() )
	{
		GDebug_Print("Mesh is missing triangle neighbours for shadows\n");
		return;
	}

	GIncCounter( DrawShadows, 1 );

	float4	ShadowColour( 0.f, 0.f, 0.f, 0.4f );

	//	planes information
	GList<Bool> PlaneVisible;
	PlaneVisible.Resize( TriCount() );

	//	rotate verts with matrix only once!
	GList<Bool> VertCalcd;
	GList<float3> VertCached;
	VertCalcd.Resize( VertCount() );
	VertCached.Resize( VertCount() );
	VertCalcd.SetAll( (Bool)FALSE );

	//	determine faces facing the light
	for ( int t=0;	t<TriCount();	t++ )
	{
		//	calc plane for this triangle
		GPlane TrianglePlane;
		ShadowCalcPlane( this, DrawInfo, m_Triangles[t], TrianglePlane, VertCalcd, VertCached );

		float Side	=	TrianglePlane[0] * DrawInfo.pLight->m_Pos.x +
						TrianglePlane[1] * DrawInfo.pLight->m_Pos.y +
						TrianglePlane[2] * DrawInfo.pLight->m_Pos.z +
						TrianglePlane[3] * 1.f;
						//pGeoPlanes[f].d * 0.f;
		PlaneVisible[t] = ( Side < 0.f );
	}


	//	setup rendering
	glPushAttrib( GL_ALL_ATTRIB_BITS );

	if ( DrawInfo.Flags & GDrawInfoFlags::DebugShadows )
	{
		glColor3fv( GDisplay::g_DebugColour );

	}
	else
	{

		glEnable( GL_CULL_FACE );
		glCullFace( GL_FRONT );

 		glDisable(GL_LIGHTING);
		glDepthMask(GL_FALSE);	//	dont write to the depth buffer
		glDepthFunc(GL_LEQUAL);
		glDepthFunc(GL_LESS);

		glEnable(GL_STENCIL_TEST);
		glColorMask(0, 0, 0, 0);	//	dont write to the colour buffer
		glStencilFunc(GL_ALWAYS, 1, 0xfffffff);
	}


	//	keep a recorded of which triangles/edges to draw once we've done it once
	//	first 16 bits are triangle, next bits are edge
	GList<u32> TriangleProcessList;
	TriangleProcessList.Realloc( TriCount() * 3 );


	// First Pass. Increase Stencil Value In The Shadow
	glFrontFace( GL_CCW );
	glStencilOp( GL_KEEP, GL_KEEP, GL_INCR );

	DoShadowRender( DrawInfo, PlaneVisible, VertCalcd, VertCached, TriangleProcessList );


	//	remove polys from the shadow volume (if neither of these flags are set)
	if ( ! ( DrawInfo.Flags & (GDrawInfoFlags::DebugShadows) ) )
	{
		// Second Pass. Decrease Stencil Value In The Shadow
		glFrontFace( GL_CW );
		glStencilOp( GL_KEEP, GL_KEEP, GL_DECR );

		//	render all the same values/polygons again
		//	use a list saved from last time
		for ( int i=0;	i<TriangleProcessList.Size();	i++ )
		{
			u32 Tri = TriangleProcessList[ i ] & 0x0000ffff;
			u32 Edge = (TriangleProcessList[ i ]>>16) & 0x0000ffff;

			DoShadowCastFromEdge( DrawInfo, Tri, Edge, VertCalcd, VertCached );
		}
	}

	if ( ! ( DrawInfo.Flags & GDrawInfoFlags::DebugShadows ) )
	{
		glFrontFace( GL_CCW );
		glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );	// Enable Rendering To Colour Buffer For All Components

		// Draw A Shadowing Rectangle Covering The Entire Screen
		glColor4fv( ShadowColour);
		if ( ShadowColour[3] < 1.f )
		{
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}

		//	keep, and draw into any bits of the stencil buffer that have been written (not Zero)
		glStencilFunc( GL_NOTEQUAL, 0, 0xffffffff );
		glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
		


		//	draw a plain polygon over the camera
		glPushMatrix();
		glLoadIdentity();
		glOrtho( -1, 1, -1, 1, GCamera::g_pActiveCamera->m_NearZ, -GCamera::g_pActiveCamera->m_FarZ );

		#define QUADW	1.f
		#define QUADZ	-GCamera::g_pActiveCamera->m_NearZ

		glBegin( GL_TRIANGLE_STRIP );
			glVertex3f( -QUADW, -QUADW, QUADZ );
			glVertex3f( -QUADW,  QUADW, QUADZ );
			glVertex3f(  QUADW, -QUADW, QUADZ );
			glVertex3f(  QUADW,  QUADW, QUADZ );
		glEnd();

		glPopMatrix();
	}


	glPopAttrib();

}
*/



void GMesh::DoShadowCastFromEdge( GDrawInfo& DrawInfo, u32& Triangle, u32& TriangleEdge, GList<Bool>& VertsCalcd, GList<float3>& VertsCached )
{
	// Get The Points On The Edge
	int& vi1 = m_Triangles[Triangle][ (int)TriangleEdge ];
	int& vi2 = m_Triangles[Triangle][ (int)(TriangleEdge+1)%3 ];

	CalcVert( this, DrawInfo, vi1, VertsCalcd, VertsCached );
	CalcVert( this, DrawInfo, vi2, VertsCalcd, VertsCached );

	float3& v1 = VertsCached[vi1];
	float3& v2 = VertsCached[vi2];

	// Calculate The Two Vertices In Distance
	float3 v3 = v1;
	float3 v4 = v2;

	v3 += ( v1 - (DrawInfo.pLight->m_Pos) ) * SHADOW_INFINITY;
	v4 += ( v2 - (DrawInfo.pLight->m_Pos) ) * SHADOW_INFINITY;

	//	debugging? draw lines in place of the shadow volume
	if ( DrawInfo.Flags & GDrawInfoFlags::DebugShadows )
	{
		glBegin( GL_LINES );
		GIncCounter( DebugPolys, 2 );
	}
	else
	{
		glBegin( GL_TRIANGLE_STRIP );
		GIncCounter( ShadowPolys, 2 );
	}

	//	draw verts
		glVertex3fv( v1 );
		glVertex3fv( v3 );
		glVertex3fv( v2 );
		glVertex3fv( v4 );
	glEnd();


}




void GMesh::DoShadowRender( GDrawInfo& DrawInfo, GList<Bool>& PlaneVisibleTable, GList<Bool>& VertsCalcd, GList<float3>& VertsCached, GList<u32>& CacheTriangleCastList )
{
	for ( u32 Tri=0;	Tri<(u32)TriCount();	Tri++ )
	{
		if ( !PlaneVisibleTable[Tri] )
			continue;

		// Go Through Each Edge
		u32 Edge;
		int Neighbour;
		
		Edge = 0;
		Neighbour = m_TriangleNeighbours[Tri][0];
		if ( (Neighbour==-1) || (!PlaneVisibleTable[Neighbour]) )
		{
			CacheTriangleCastList.Add( Tri | (0<<16) );
			DoShadowCastFromEdge( DrawInfo, Tri, Edge, VertsCalcd, VertsCached );				
		}

		Edge = 1;
		Neighbour = m_TriangleNeighbours[Tri][1];
		if ( (Neighbour==-1) || (!PlaneVisibleTable[Neighbour]) )
		{
			CacheTriangleCastList.Add( Tri | (1<<16) );
			DoShadowCastFromEdge( DrawInfo, Tri, Edge, VertsCalcd, VertsCached );				
		}				

		Edge = 2;
		Neighbour = m_TriangleNeighbours[Tri][2];
		if ( (Neighbour==-1) || (!PlaneVisibleTable[Neighbour]) )
		{
			CacheTriangleCastList.Add( Tri | (2<<16) );
			DoShadowCastFromEdge( DrawInfo, Tri, Edge, VertsCalcd, VertsCached );				
		}				
	}


}


//-------------------------------------------------------------------------
//	generate a sphere that covers all the verts in the mesh
//-------------------------------------------------------------------------
void GMesh::GenerateBounds(Bool Force)
{
	GDebug_Print("Todo: generate new collision object bounds\n");
	/*
	if ( m_Bounds.m_Radius > 0.f && !Force )
	{
		return;
	}

	if ( VertCount() == 0 )
	{
		GDebug_Break("Need verts to make bounding box\n");
		m_Bounds.m_Offset = float3(0,0,0);
		m_Bounds.m_Radius = 0.f;
		return;
	}

	//	find middle of verts
	float3 Min, Max;
	GetVertexMinMax( Min, Max );

	//	the offset is the middle of the minimum and maximum. if its an equal min and max, the middle will be 0,0,0
	m_Bounds.m_Offset.x = (Max.x + Min.x) / 2.f;
	m_Bounds.m_Offset.y = (Max.y + Min.y) / 2.f;
	m_Bounds.m_Offset.z = (Max.z + Min.z) / 2.f;

	//	set the radius to the biggest offset from our middle. (doesnt matter whether we use min or max, middle is in the middle of both)
	m_Bounds.m_Radius = Max.x - m_Bounds.m_Offset.x;

	if ( Max.y - m_Bounds.m_Offset.y > m_Bounds.m_Radius )
		m_Bounds.m_Radius = Max.y - m_Bounds.m_Offset.y;
	
	if ( Max.z - m_Bounds.m_Offset.z > m_Bounds.m_Radius )
		m_Bounds.m_Radius = Max.z - m_Bounds.m_Offset.z;
	*/
}



void GMesh::GetVertexMinMax(float3& Min, float3& Max)
{
	if ( m_Verts.Size() < 1 )
	{
		GDebug_Break("No verts to calculate min max\n");
		return;
	}

	//	find middle of verts
	Min = m_Verts[0];
	Max = m_Verts[0];
	for ( int i=1;	i<VertCount();	i++ )
	{
		float3& v = m_Verts[i];
		
		if ( v.x < Min.x )	Min.x = v.x;
		if ( v.y < Min.y )	Min.y = v.y;
		if ( v.z < Min.z )	Min.z = v.z;

		if ( v.x > Max.x )	Max.x = v.x;
		if ( v.y > Max.y )	Max.y = v.y;
		if ( v.z > Max.z )	Max.z = v.z;
	}
}

void GMesh::GetVertexCenter(float3& Center)
{
	if ( VertCount() < 1 )
	{
		GDebug_Break("No verts to calculate min max\n");
		return;
	}

	if ( VertCount() == 1 )
	{
		Center = m_Verts[0];
		return;
	}

	//	find middle of verts
	Center = m_Verts[0];
	for ( int i=1;	i<VertCount();	i++ )
	{
		float3& v = m_Verts[i];
		
		Center += v;
	}

	Center /= (float)VertCount();
}

	
float3 GMesh::GetFurthestVert()
{
	if ( m_Verts.Size() == 0 )
		return float3(0,0,0);

	float3 Furthest = m_Verts[0];
	float FurthestLenSq = Furthest.LengthSq();

	for ( int v=1;	v<m_Verts.Size();	v++ )
	{
		float LenSq = m_Verts[v].LengthSq();
		if ( LenSq > FurthestLenSq )
			Furthest = m_Verts[v];
	}

	return Furthest;
}


void GMesh::CopyVert(int From, int To)
{
	//	copy vertex data
	m_Verts[To]		= m_Verts[From];
	
	if ( m_Normals.Size() )
		m_Normals[To]	= m_Normals[From];

	if ( m_TextureUV.Size() )
		m_TextureUV[To]	= m_TextureUV[From];
}


void GMesh::ShowInfo()
{
	GWin32::Popupf( Popup_OK,
					"Mesh Info",
					"Vertexes: %d\n"
					"Triangles: %d\n"
					"TriStrips: %d\n",
					m_Verts.Size(),
					m_Triangles.Size(),
					m_TriStrips.Size()
					);
}



void GMesh::SplitTriStrips()
{
	//	split triangle strips down to triangles
	GenerateTrianglesFromTriStrips( m_Triangles );
	m_TriStrips.Empty();
	m_TriStripColours.Empty();
	GenerateTriangleNeighbours();
	GenerateDebugColours();
	GenerateTriangleCenters();
}






void GMesh::TriStrip()
{
	//	no triangles to turn into strips
	if ( !TriCount() )
	{
		GDebug_Print("No triangles to turn into strips\n");
		return;
	}

	//	merge verts then tri strip
	GDebug_Print("Tristripping mesh: Merging verts...\n");
	MergeVerts();
	
	u16	 NumOfStrips		= 0;
	u16  NumOfStripIndicies	= 0;
	GList<u16>	StripIndicies;
	GList<u16>	StripLengths;
	//StripIndicies.Resize( VertCount()*2 );
	//StripLengths.Resize( VertCount()*2 );
	GList<int> StrippedTriangles;

	GDebug_Print("Tristripping mesh: Detecting strips...\n");
	DetectStrips( NumOfStrips, StripLengths, NumOfStripIndicies, StripIndicies, StrippedTriangles );

	if ( NumOfStrips == 0 )
	{
		GDebug_Print("Couldnt make any tri-strips out of mesh\n");
		return;
	}

	//	make our new tri-strips
	GDebug_Print("Tristripping mesh: Creating %d tristrips...\n", NumOfStrips);
	AllocTriStrips( NumOfStrips );

	int currentInd = 0;
	for ( int ts=0;		ts<NumOfStrips;		ts++ )
	{
		m_TriStrips[ts].m_Indicies.Resize( (u16)StripLengths[ ts ] );
		
		for ( int i=0;	i<m_TriStrips[ts].m_Indicies.Size();	i++ )
		{
			m_TriStrips[ts].m_Indicies[i] = StripIndicies[ currentInd ];
			currentInd++;
		}

		//memcpy( m_TriStrips[ ts ].m_Indicies.Data(), &StripIndicies[ currentInd ], sizeof(u16)*m_TriStrips[ ts ].m_Indicies.Size() );
		//currentInd += (u16)StripLengths[ ts ];
	}

	//	remove triangles
	GDebug_Print("Tristripping mesh: Removing %d old triangles...\n", StrippedTriangles.Size());
	for ( int t=0;	t<StrippedTriangles.Size();	t++ )
	{
		//	t is also how many we've removed, so indexes drop by 1 as theyre removed
		int Triangle = StrippedTriangles[t] - t;

		RemoveTriangle( Triangle );
	}

	//	resize triangle list now that we've removed the ones we needed to
	AllocTriangles( TriCount() - StrippedTriangles.Size() );

	//	recreate colours
	GenerateDebugColours();

	//	result
	GDebug_Print("Created %d triangle strips\n", TriStripCount() );
}










class aHit{
public:
	u16 hits;
	u16 t[3];
	u8 mask[3];
	GTriangle p[3];
	BOOL Stripped;

public:
	aHit()	
	{
		hits=0;	
		t[0]=t[1]=t[2] = 0;
		mask[0]=mask[1]=mask[2]=0;
		p[0] = GTriangle(0,0,0);
		p[1] = GTriangle(0,0,0);
		p[2] = GTriangle(0,0,0);
		Stripped=FALSE;
	};
	~aHit()	{};

};
typedef aHit* pHit;


void GMesh::DetectStrips(u16& nr_strips, GList<u16>& strip_length, u16& nr_indices, GList<u16>& stripindex, GList<int>& StrippedTriangles)
{
	u16 lastp0,lastp1,lastp2;
	u16 n,nr,trynr;
	u16 p0,p1,p2;
	u16 i,k,next,p;
	u8 mask;
	GTriangle* temp, * tempEnd;
	aHit* tempHitList;
	// First part makes a hitlist per triangle, how many sides(hits) a triangle has shared,
	// the indexes of the hits, up to 3 masks that define wich points the triangles each share
	// and the points of each shared triangle.
	//HitList=(struct Hit *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,length*sizeof(struct Hit));
	GList<aHit> HitList;
	HitList.Resize( TriCount() );
	tempHitList=&HitList[0];

	for(	n=0;	n<TriCount();	n++,tempHitList++	)
	{
		if ( n % 10 == 0 )
		{
			GDebug_Print("DetectStrips: Calculating Triangle #%d...\n", n );
		}

		p0 = m_Triangles[n].x;
		p1 = m_Triangles[n].y;
		p2 = m_Triangles[n].z;
		temp = &m_Triangles[0];
		tempEnd = &m_Triangles[TriCount()-1];
		tempEnd++;
	
		while(	temp<tempEnd	)
		{
			u16 p;
			// The following piece of code accounts for 99% of the time when converting large models
			nr=2;                               // count-down, we only want the ones with two points the same
			mask=0;
			if ((p=temp->x)==p0){mask|=0x11;nr--;}
			else {if (p==p1){mask|=0x12;nr--;}
					else {if (p==p2){mask|=0x14;nr--;}}
					}
			if ((p=temp->y)==p0){mask|=0x21;nr--;}
			else {if (p==p1){mask|=0x22;nr--;}
					else {if (p==p2){mask|=0x24;nr--;}}
					}
			if ((p=temp->z)==p0){mask|=0x41;nr--;}
			else {if (p==p1){mask|=0x42;nr--;}
					else {if (p==p2){mask|=0x44;nr--;}}
					}

			// That's it
			if(!nr)																	//this doesn't happen very often
			{
				u16 nrs=tempHitList->hits;
				//Rotate and save points so that p0 and p1 point to points that are the same.
				// to check later, if we can make a strip of this
				switch (mask>>4){
					case 5:
					tempHitList->p[nrs].y=temp->x;
					tempHitList->p[nrs].x=temp->z;
					tempHitList->p[nrs].z=temp->y;
					break;

					case 6:
					tempHitList->p[nrs].x=temp->y;
					tempHitList->p[nrs].y=temp->z;
					tempHitList->p[nrs].z=temp->x;
					break;

					case 3:
					tempHitList->p[nrs].x=temp->x;
					tempHitList->p[nrs].y=temp->y;
					tempHitList->p[nrs].z=temp->z;
					break;
					}
					tempHitList->t[nrs]=temp-&m_Triangles[0];         // (temp-ilist) is actually the triangle-number 
					tempHitList->mask[nrs]=mask;            // mask is for later
					
					if(++tempHitList->hits==3)temp=tempEnd;		// break while-loop if we have 3 hits
				}
				temp++;
			}
	}
	
	// Next:
	// Start with trying to make a strip of all triangles with 1 hit as a starting point,
	// then 2 , finally 3.
	// That's all.
	for(	trynr=1;	trynr<=3;	trynr++)
	{
		for(	p=0;	p<TriCount();	p++)
		{
			if ( n % 10 == 0 )
			{
				GDebug_Print("DetectStrips: Stripping Triangle #%d...\n", p );
			}

			if(HitList[p].hits==trynr&&!HitList[p].Stripped)
			{
				n=p;                              		// n is first triangle of possible strip
				i=0;                                  // i is triangle-counter of this possible strip
				k=10;                                 // found a matching triangle

				while(k>=10)                          // while found a triangle
				{
					for (k=0;k<trynr;k++)    					// try all possible triangles
					{
						next=HitList[n].t[k];						// possible triangle

						if(!HitList[next].Stripped)				// not included yet ?
						{
							if(!i)							// if testing with first triangle, it must be rotated so that
							{									// points that are the same as next triangle are p1 and p2
								switch (HitList[n].mask[k]&0x0f)
								{
									case 6:
									lastp0=m_Triangles[p].x;         // lastp0-p2 is first triangle of strip
									lastp1=m_Triangles[p].y;         // is a local triangle that defines the last
									lastp2=m_Triangles[p].z;					// triangle added to the strip
									break;

									case 3:
									lastp1=m_Triangles[p].x;
									lastp2=m_Triangles[p].y;
									lastp0=m_Triangles[p].z;
									break;

									case 5:
									lastp0=m_Triangles[p].y;
									lastp1=m_Triangles[p].z;
									lastp2=m_Triangles[p].x;
									break;
								}

								stripindex.Add( lastp0 );				 //save
								stripindex.Add( lastp1 );
								stripindex.Add( lastp2 );
							}
				
							if(i&1)                                  // odd or even, makes a difference, see OpenGL
							{
								if (HitList[n].p[k].x==lastp0&&HitList[n].p[k].y==lastp2)    //new one fits ?
								{
									lastp0=HitList[n].p[k].x;          // update last triangle used
									lastp1=HitList[n].p[k].y;
									lastp2=HitList[n].p[k].z;
									HitList[next].Stripped=TRUE;				 //this one is done
									stripindex.Add( lastp2 );  //save, p2 defines this triangle
									nr_indices++;
									n=next;                //use this one as next source
									k=10;									// break for-loop with k, found one
									i++;
								}
							}
						
							else
							{
								if (	HitList[n].p[k].x==lastp2	&&	HitList[n].p[k].y==lastp1	)    //new one fits ?
								{
									lastp0=HitList[n].p[k].x;          // update last triangle used
									lastp1=HitList[n].p[k].y;
									lastp2=HitList[n].p[k].z;
									HitList[next].Stripped=TRUE;				 //this one is done
								
									if(!i++)
									{
										(nr_indices)+=3;								 //first triangle had 3 indices and keep it
										HitList[n].Stripped=TRUE;          //and is done also
									}
									
									stripindex.Add( lastp2 );  // save, p2 defines this triangle
									nr_indices++;
									n=next;								//use this one as next source
									k=10;									// break for-loop with k, found one
				
								}
							}	
						}
					}
				}
			
				i++;														//actual number of triangles in strip is one more
				if(i>1)	
				{
					strip_length.Add( i+2 );		 //actual strip-length (in indices) is traingles +2
					nr_strips++;
				}
	
			}
		}
	}
	

	//	make a list of the triangles that have been stripped
	StrippedTriangles.Empty();

	for( p=0;	p<TriCount();	p++ )
	{
		if ( HitList[p].Stripped )
		{
			StrippedTriangles.Add( p );
		}
	}
}



//-------------------------------------------------------------------------
//	reverse vertex normals
//-------------------------------------------------------------------------
void GMesh::InvertVertexNormals()
{
	int i;

	//	invert vertex normals
	for ( i=0;	i<m_Normals.Size();	i++ )
	{
		m_Normals[i].x = -m_Normals[i].x;
		m_Normals[i].y = -m_Normals[i].y;
		m_Normals[i].z = -m_Normals[i].z;
	}

}

//-------------------------------------------------------------------------
//	reverse plane normals
//-------------------------------------------------------------------------
void GMesh::InvertPlaneNormals()
{
	int i;

	//	invert plane normals
	for ( i=0;	i<m_TrianglePlanes.Size();	i++ )
	{
		m_TrianglePlanes[i].InvertNormal();
	}

	for ( i=0;	i<m_TriStripPlanes.Size();	i++ )
	{
		for ( int ii=0;	ii<m_TriStripPlanes[i].Size();	ii++ )
		{
			m_TriStripPlanes[i].ElementAt(ii).InvertNormal();
		}
	}

}



void GMesh::RemoveTriangle(int Triangle)
{
	//	remove this triangle
	m_Triangles.RemoveAt( Triangle );

	//	remove additional information

	if ( m_TriangleNeighbours.Size() )
		m_TriangleNeighbours.RemoveAt( Triangle );

	if ( m_TriangleColours.Size() )
		m_TriangleColours.RemoveAt( Triangle );
		
	if ( m_TriangleCenters.Size() )
		m_TriangleCenters.RemoveAt( Triangle );

}


void GMesh::CheckFloats()
{
	int i;
	
	//	check all verts for invalid floats
	for ( i=0;	i<m_Verts.Size();	i++ )
	{
		GDebug::CheckFloat( m_Verts[i].x );
		GDebug::CheckFloat( m_Verts[i].y );
		GDebug::CheckFloat( m_Verts[i].z );
	}

	//	check normals
	for ( i=0;	i<m_Normals.Size();	i++ )
	{
		GDebug::CheckFloat( m_Normals[i] );
	}

	//	check UV's
	for ( i=0;	i<m_TextureUV.Size();	i++ )
	{
		GDebug::CheckFloat( m_TextureUV[i] );
	}

	//	check triangle colours
	for ( i=0;	i<m_TriangleColours.Size();	i++ )
	{
		GDebug::CheckFloat( m_TriangleColours[i] );
	}
	
	//	check triangle planes
	for ( i=0;	i<m_TrianglePlanes.Size();	i++ )
	{
		GDebug::CheckFloat( m_TrianglePlanes[i] );
	}

	//	check triangle centers
	for ( i=0;	i<m_TriangleCenters.Size();	i++ )
	{
		GDebug::CheckFloat( m_TriangleCenters[i] );
	}

	//	check tristrip colours
	for ( i=0;	i<m_TriStripColours.Size();	i++ )
	{
		GDebug::CheckFloat( m_TriStripColours[i] );
	}


}


void GMesh::GenerateTrianglesFromTriStrips(GList<GTriangle>& TriangleList)
{
	int t,i;

	//	find edges in triangle strips
	for ( t=0;	t<m_TriStrips.Size();	t++ )
	{
		GTriStrip& TriStrip = m_TriStrips[t];
		GTriangle Triangle;

		//	add first 2 bits of triangle
		Triangle[0] = TriStrip.m_Indicies[0];
		Triangle[1] = TriStrip.m_Indicies[1];
		
		//	replace the next triangle element in sequence along the tristrip
		for ( i=2;	i<TriStrip.m_Indicies.Size();	i++ )
		{
			Triangle[ i % 3 ] = TriStrip.m_Indicies[i];
			TriangleList.Add( Triangle );
		}
	}

}

/*
void GMesh::GenerateShadowData()
{
	int t,et;	//	triangle, edgetriangle
	int xt, ext;	//	triangle, edgetriangle
	
	//	remove existing data
	m_ShadowVerts.Empty();
	m_ShadowVertNormals.Empty();
	m_ShadowQuads.Empty();

	//	generate comprehensive list of triangles
	GList<GTriangle> Triangles;

	//	add triangles
	Triangles.Add( m_Triangles );

	//	add triangles from tristrips
	GenerateTrianglesFromTriStrips( Triangles );

	//	generate normal for each triangle
	GList<float3> TriangleNormals;
	GenerateTriangleNormals( Triangles, TriangleNormals );

	//	resize list now, and cut out unused items later
	GList<int3> TriangleNeighbours;
	TriangleNeighbours.Resize( Triangles.Size() );
	int3 tmp( -1, -1, -1 );
	TriangleNeighbours.SetAll( tmp );

	//	keep a recorded of duplicate edges (edges with the same data, but different orders)
	GList<int2> DuplicateEdgeData;	//	(triangle|edge)

	//	find all the triangle joins(edges) and add 4 verts, a pair for each triangle
	//	with the relative triangle normal on that vert
	for ( t=0; t<Triangles.Size()-1;	t++ )
	{
		//	check against other triangles
		for( xt=t+1;	xt<Triangles.Size();	xt++ )
		{
			//	for each edge of triangle t
			for( et=0;	et<3;	et++ )
			{
				//	not already set
				if( TriangleNeighbours[t][et] == -1 )
				{
					for( ext=0;	ext<3;	ext++ )
					{
						int tP1	= et;			//	triangle point 1
						int tP2	= (et+1)%3;		//	triangle point 2
						int xP1	= ext;			//	triangle point 1
						int xP2	= (ext+1)%3;	//	triangle point 2
						
						tP1	= Triangles[t][tP1];
						tP2	= Triangles[t][tP2];
						xP1	= Triangles[xt][xP1];
						xP2	= Triangles[xt][xP2];

						int tQ1 = ((tP1+tP2)-abs(tP1-tP2))/2;
						int tQ2 = ((tP1+tP2)+abs(tP1-tP2))/2;
						int xQ1 = ((xP1+xP2)-abs(xP1-xP2))/2;
						int xQ2 = ((xP1+xP2)+abs(xP1-xP2))/2;

						//	they are neighbours
						if( ( tQ1 == xQ1 ) && ( tQ2 == xQ2 ) )
						{
							//	set neighbour data
							TriangleNeighbours[t][et]		= xt;
							TriangleNeighbours[xt][ext]	= t;

							//	this is the same data, we only need one instance, so add a "is duplicate" entry
							DuplicateEdgeData.Add( int2( xt, ext ) );
						}
					}
				}
			}
		}
	}

	//	now generate a "shadow-quad" for each triangle edge
	for ( t=0;	t<Triangles.Size();	t++ )
	{
		//	loop through each edge
		for ( et=0;	et<3;	et++ )
		{
			//	if this is duplicate data, we ignore it
			if ( DuplicateEdgeData.FindIndex( int2( t, et ) ) != -1 )
				continue;

			//	edge is triangle indicie indexes
			int2 Edge( et, (et+1)%3 );

			//	grab verts (this will be the same for both triangles
			float3 EdgeVert[2];
			EdgeVert[0] = m_Verts[ Triangles[t][Edge[0]] ];
			EdgeVert[1] = m_Verts[ Triangles[t][Edge[1]] ];

			//	get triangle index of the other side of the edge
			xt = TriangleNeighbours[t][et];

			//	get normals for each side of the edge
			float3 EdgeNormal[2];
			EdgeNormal[0] = TriangleNormals[t];	//	this triangles normal
			if ( xt == -1 )
			{
				//	no neighbour on this edge, use normal of 0,0,0 which is never visible (will always extrude)
				EdgeNormal[1] = float3(-1,-1,-1);
			}
			else
			{
				EdgeNormal[1] = TriangleNormals[xt];
			//	continue;
			}

			//	dont add if normals are the same
			if ( EdgeNormal[0] == EdgeNormal[1] )
			{
				//continue;
			}
			else
			{
				GDebug_Print("%2.2f %2.2f %2.f +++++++++ %2.2f %2.2f %2.f\n", EdgeNormal[0].x, EdgeNormal[0].y, EdgeNormal[0].z, EdgeNormal[1].x, EdgeNormal[1].y, EdgeNormal[1].z );
			}


			//	add vertex data for quad
			//	we have to add a new vertex for every quads edge as the vertex needs different normal values everytime 
			//	as we can't re-use the data unless we can specify different normals per vertex index
			GQuad ShadowQuad;

			
			//	//	order of quad:
			//		1
			//	   /\
			//	  /  \
			//	0/    \2
			//	 \\    /
			// 	  \\  /
			//	   \\/				\\ == edge
			//	    3				   
			

			//	edge of triangle t
			ShadowQuad.m_Indicies[0] = m_ShadowVerts.Add( EdgeVert[0] );
			ShadowQuad.m_Indicies[3] = m_ShadowVerts.Add( EdgeVert[1] );
			m_ShadowVertNormals.Add( EdgeNormal[0] );
			m_ShadowVertNormals.Add( EdgeNormal[0] );

			//	edge of triangle xt
			ShadowQuad.m_Indicies[1] = m_ShadowVerts.Add( EdgeVert[0] );
			ShadowQuad.m_Indicies[2] = m_ShadowVerts.Add( EdgeVert[1] );
			m_ShadowVertNormals.Add( EdgeNormal[1] );
			m_ShadowVertNormals.Add( EdgeNormal[1] );

			//	add the quad
			m_ShadowQuads.Add( ShadowQuad );
		}
	}

}
*/


//-------------------------------------------------------------------------
//	add in all the geometry from this mesh
//-------------------------------------------------------------------------
void GMesh::MergeMesh(GMesh* pMesh)
{
	int i;

	//	get first vert index so when we add geometry indicies we have the right indexes
	int FirstVert = VertCount();

	//	add verts
	m_Verts.Add( pMesh->m_Verts );
	m_Normals.Add( pMesh->m_Normals );
	m_TextureUV.Add( pMesh->m_TextureUV );
	m_TextureUV2.Add( pMesh->m_TextureUV2 );

	//	add triangles
	for ( i=0;	i<pMesh->m_Triangles.Size();	i++ )
	{
		GTriangle Triangle = pMesh->m_Triangles[i];
		Triangle[0] += FirstVert;
		Triangle[1] += FirstVert;
		Triangle[2] += FirstVert;
		m_Triangles.Add( Triangle );
	}

	//	add triangle data
	#define MERGE_LIST( dest, src )			\
	{										\
		if ( dest.Size() && src.Size() )	\
			dest += src;					\
	}

	MERGE_LIST( m_TriangleCenters, pMesh->m_TriangleCenters );
	MERGE_LIST( m_TriangleColours, pMesh->m_TriangleColours );
	MERGE_LIST( m_TrianglePlanes, pMesh->m_TrianglePlanes );
	//MERGE_LIST( m_TriangleNeighbours, pMesh->m_TriangleNeighbours );	//	data will be invalid

	#undef MERGE_LIST

	//	add triangle strips
	for ( i=0;	i<pMesh->m_TriStrips.Size();	i++ )
	{
		GTriStrip TriStrip;
		for ( int n=0;	n<pMesh->m_TriStrips[i].m_Indicies.Size();	n++ )
		{
			TriStrip.m_Indicies.Add( pMesh->m_TriStrips[i].m_Indicies[n] + FirstVert );
		}
		/*
		 *	MEMORY PROBLEM HERE?
		 */
		m_TriStrips.Add( TriStrip );
	}


	//	add tristrip data
	m_TriStripColours += pMesh->m_TriStripColours;

}


//-------------------------------------------------------------------------
//	modify verts by a translation and rotation
//-------------------------------------------------------------------------
void GMesh::MultiplyVerts(float3& Translation, GQuaternion& Rotation)
{
	int i;

	//	get the matrixes we need
	GMatrix RotMatrix;
	QuaternionToMatrix( Rotation, RotMatrix );

	//	normals need to be rotated by not translated
	for ( i=0;	i<m_Normals.Size();	i++ )
	{
		m_Normals[i] = RotMatrix * m_Normals[i];
	}

	//	verts need to be rotated and translated
	for ( i=0;	i<m_Verts.Size();	i++ )
	{
		m_Verts[i] = RotMatrix * m_Verts[i];
		m_Verts[i] += Translation;
	}


}


//-------------------------------------------------------------------------
//	Scale all verts
//-------------------------------------------------------------------------
void GMesh::Scale(float3 Mult)
{
	//	verts need to be rotated and translated
	for ( int i=0;	i<m_Verts.Size();	i++ )
	{
		m_Verts[i] *= Mult;
	}

}


//-------------------------------------------------------------------------
//	Move all verts
//-------------------------------------------------------------------------
void GMesh::MoveVerts(float3& Change)
{
	for ( int i=0;	i<m_Verts.Size();	i++ )
	{
		m_Verts[i] += Change;
	}
}

//-------------------------------------------------------------------------
//	center all verts around a point
//-------------------------------------------------------------------------
void GMesh::CenterVerts(float3 Center)
{
	//	find middle of verts
	float3 CurrentCenter;
	GetVertexCenter( CurrentCenter );

	GDebug_Print("Current vert center %2.2f, %2.2f, %2.2f\n", CurrentCenter.x, CurrentCenter.y, CurrentCenter.z );

	//	move to center
	MoveVerts( Center - CurrentCenter );

}


void GMesh::SnapToFloor(float FloorY)
{
}


void GMesh::CreateVertexBufferObjects()
{
	//	create vertex buffer objects that dont exist
	if ( m_Verts.Size() && m_VBOVertexID==0 )		
	{
		g_DisplayExt.glGenBuffersARB()( 1, &m_VBOVertexID );
		GDebug::CheckGLError();
	}

	if ( m_Normals.Size() && m_VBONormalID==0 )		
	{
		g_DisplayExt.glGenBuffersARB()( 1, &m_VBONormalID );
		GDebug::CheckGLError();
	}

	if ( m_TextureUV.Size() && m_VBOTexCoordID==0 )	
	{
		g_DisplayExt.glGenBuffersARB()( 1, &m_VBOTexCoordID );
		GDebug::CheckGLError();
	}
	
}


void GMesh::UploadVertexBufferObjects()
{
	if ( m_VBOVertexID && m_Verts.Size() > 0 )
	{
		UploadVertexBufferData( (float*)m_Verts.Data(), m_Verts.DataSize(), m_VBOVertexID );
	}

	if ( m_VBONormalID && m_Normals.Size() > 0 )
	{
		UploadVertexBufferData( (float*)m_Normals.Data(), m_Normals.DataSize(), m_VBONormalID );
	}

	if ( m_VBOTexCoordID && m_TextureUV.Size() > 0 )
	{
		UploadVertexBufferData( (float*)m_TextureUV.Data(), m_TextureUV.DataSize(), m_VBOTexCoordID );
	}
}


void GMesh::UploadVertexBufferData(float* pData, int DataSize, u32& VBOIndex)
{
	//	upload data
	if ( VBOIndex!=0 && DataSize!=0 )
	{
		//	temporary data
		u16 UploadType = GL_STREAM_DRAW_ARB;

		//	static data is set if its our local data
		if ( pData == (float*)m_Verts.Data() || pData == (float*)m_TextureUV.Data() || pData == (float*)m_Normals.Data() )
			UploadType = GL_STATIC_DRAW_ARB;

		SelectBufferObject( VBOIndex );
		g_DisplayExt.glBufferDataARB()( GL_ARRAY_BUFFER_ARB, DataSize, pData, UploadType );
		GDebug::CheckGLError();
	}
}


void GMesh::DeloadVertexBufferObjects()
{
	if ( m_VBOVertexID )
	{
		SelectBufferObject( m_VBOVertexID );
		g_DisplayExt.glBufferDataARB()( GL_ARRAY_BUFFER_ARB, 0, NULL, GL_STATIC_DRAW_ARB );
		GDebug::CheckGLError();
	}

	if ( m_VBONormalID )
	{
		SelectBufferObject( m_VBONormalID );
		g_DisplayExt.glBufferDataARB()( GL_ARRAY_BUFFER_ARB, 0, NULL, GL_STATIC_DRAW_ARB );
		GDebug::CheckGLError();
	}

	if ( m_VBOTexCoordID )
	{
		SelectBufferObject( m_VBOTexCoordID );
		g_DisplayExt.glBufferDataARB()( GL_ARRAY_BUFFER_ARB, 0, NULL, GL_STATIC_DRAW_ARB );
		GDebug::CheckGLError();
	}
}


void GMesh::DestroyVertexBufferObjects()
{
	//	make sure data is deloaded
	DeloadVertexBufferObjects();

	//	destroy objects
	if ( m_VBOVertexID )
	{
		g_DisplayExt.glDeleteBuffersARB()( 1, &m_VBOVertexID );
		m_VBOVertexID = 0;
		GDebug::CheckGLError();
	}

	if ( m_VBONormalID )
	{
		g_DisplayExt.glDeleteBuffersARB()( 1, &m_VBONormalID );
		m_VBONormalID = 0;
		GDebug::CheckGLError();
	}

	if ( m_VBOTexCoordID )
	{
		g_DisplayExt.glDeleteBuffersARB()( 1, &m_VBOTexCoordID );
		m_VBOTexCoordID = 0;
		GDebug::CheckGLError();
	}

}

void GMesh::SelectBufferObject(u32& BufferObjectID)
{
	//	check for invalid ID
	if ( BufferObjectID == 0 )
	{
		//GDebug_Break("Invalid vertex buffer object ID\n");
		SelectBufferObjectNone();
	}

	//	bind index
	g_DisplayExt.glBindBufferARB()( GL_ARRAY_BUFFER_ARB, BufferObjectID );
	GDebug::CheckGLError();
}


void GMesh::SelectBufferObjectNone()
{
	//	bind 0 entry
	g_DisplayExt.glBindBufferARB()( GL_ARRAY_BUFFER_ARB, 0 );
	GDebug::CheckGLError();
}


Bool GMesh::Upload()
{
	/*
	if ( g_Display->HardwareSupportedFlags() & GHardwareFlags::VertexBufferObjects )
	{
		CreateVertexBufferObjects();
		UploadVertexBufferObjects();
		GDebug::CheckGLError();
	}
	*/
	return TRUE;
}


Bool GMesh::Deload()
{
	if ( g_DisplayExt.HardwareSupportedFlags() & GHardwareFlags::VertexBufferObjects )
	{
		DestroyVertexBufferObjects();
		GDebug::CheckGLError();
	}

	return TRUE;
}


void GMesh::GenerateSphere(int SectionsX, int SectionsY)
{
	//	clean out current data
	Cleanup();

	int2 Segments( SectionsX, SectionsY );
	float Radius = 1.f;

	AllocVerts( Segments.x * Segments.y );
	m_TextureUV.Resize( Segments.x * Segments.y );
	m_Normals.Resize( Segments.x * Segments.y );

	float2 Mult;
	Mult.x = 1.f / (float)( Segments.x-1 );
	Mult.y = 1.f / (float)( Segments.y-1 );
	
	for ( int y=0;	y<Segments.y-1;	y++ )
	{
		for ( int x=0;	x<Segments.x-1;	x++ )
		{
			float fx = (float)x * Mult.x;
			float fy = (float)y * Mult.y;
			float fx1 = (float)(x+1) * Mult.x;
			float fy1 = (float)(y+1) * Mult.y;
			
			int vi[4];
			vi[0] = (x+0) + ( (y+0)*Segments.x );
			vi[1] = (x+1) + ( (y+0)*Segments.x );
			vi[2] = (x+1) + ( (y+1)*Segments.x );
			vi[3] = (x+0) + ( (y+1)*Segments.x );

			float3& v1 = m_Verts[vi[0]];
			float3& v2 = m_Verts[vi[1]];
			float3& v3 = m_Verts[vi[2]];
			float3& v4 = m_Verts[vi[3]];
						
			v1.x = (float)sinf( fx * ( PI * 2.f )) * (float)sinf( fy * PI );
			v1.y = (float)cosf( fy * ( PI * 1.f ));
			v1.z = (float)cosf( fx * ( PI * 2.f )) * (float)sinf( fy * PI );

			v2.x = (float)sinf( fx1 * ( PI * 2.f )) * (float)sinf( fy * PI );
			v2.y = (float)cosf( fy * ( PI * 1.f ));
			v2.z = (float)cosf( fx1 * ( PI * 2.f )) * (float)sinf( fy * PI );

			v3.x = (float)sinf( fx1 * ( PI * 2.f )) * (float)sinf( fy1 * PI );
			v3.y = (float)cosf( fy1 * ( PI * 1.f ));
			v3.z = (float)cosf( fx1 * ( PI * 2.f )) * (float)sinf( fy1 * PI );

			v4.x = (float)sinf( fx * ( PI * 2.f )) * (float)sinf( fy1 * PI );
			v4.y = (float)cosf( fy1 * ( PI * 1.f ));
			v4.z = (float)cosf( fx * ( PI * 2.f )) * (float)sinf( fy1 * PI );

			m_Normals[vi[0]] = v1;
			m_Normals[vi[1]] = v2;
			m_Normals[vi[2]] = v3;
			m_Normals[vi[3]] = v4;

			m_TextureUV[vi[0]] = float2( fx, 1.f-fy );
			m_TextureUV[vi[1]] = float2( fx1, 1.f-fy );
			m_TextureUV[vi[2]] = float2( fx1, 1.f-fy1 );
			m_TextureUV[vi[3]] = float2( fx, 1.f-fy1 );

			v1 *= Radius;
			v2 *= Radius;
			v3 *= Radius;
			v4 *= Radius;

			GTriangle Tri1;
			Tri1[0] = vi[0];
			Tri1[1] = vi[1];
			Tri1[2] = vi[3];

			GTriangle Tri2;
			Tri2[0] = vi[1];
			Tri2[1] = vi[2];
			Tri2[2] = vi[3];

			m_Triangles.Add( Tri1 );
			m_Triangles.Add( Tri2 );

		}
	}
		
}



void GMesh::GenerateCube()
{

}




//-------------------------------------------------------------------------
//	raycast to the mesh
//	from and to is the raycast line relative to the mesh(0,0,0). 
//	Radius is a radius for the raycast line. used for sphere testing
//-------------------------------------------------------------------------
Bool GMesh::Raycast(float3 From, float3 To)
{
	//	paramter check
	if ( (From-To).LengthSq() < NEAR_ZERO )
	{
		GDebug::Break("Raycast length too small\n");
		return FALSE;
	}
	
	//	get raycast direction
	float3 RayDir( To-From );
	float3 RayDirNormal( RayDir.Normal() );
	float RayLen = RayDir.Length();

	//	todo: make better this!

	//	check each triangle
	GList<GTriangle> TriangleList;
	GenerateTrianglesFromTriStrips(TriangleList);
	TriangleList += m_Triangles;

	GPlane Plane;

	for ( int t=0;	t<TriangleList.Size();	t++ )
	{
		float3& v1 = m_Verts[TriangleList[t][0]];
		float3& v2 = m_Verts[TriangleList[t][1]];
		float3& v3 = m_Verts[TriangleList[t][2]];
		
		//	get the "triangle radius" (todo: store this permanantly)
		float TriRadSq = 0.f;
		float v12Len = (v1 - v2).LengthSq();
		float v13Len = (v1 - v3).LengthSq();
		float v23Len = (v2 - v3).LengthSq();

		if ( v12Len > v13Len )
			TriRadSq = ( v12Len > v23Len ? v12Len : v23Len ) * 0.5f;
		else
			TriRadSq = ( v13Len > v23Len ? v13Len : v23Len ) * 0.5f;

		float3 TriCenter = ( v1 + v2 + v3 ) * 0.33333f;
		float DistToPos = (From-TriCenter).LengthSq();

		//	distance from the triangle center to the test pos is greater than the "radius" of the triangle
		//	means too far away to collide
		if ( DistToPos > TriRadSq )
			continue;

		//	calc the plane and do first check if our test pos traverses through it
		Plane.CalcEquation( v1, v2, v3 );
		float3 TriNormal( Plane.Normal() );
		TriNormal.Invert();
		TriNormal.Normalise();

		//if ( RaycastToTriangle( Collision, Plane, v1, v2, v3, TestPos, Direction ) )
			//	return TRUE;
	}

	return FALSE;
}



void GMesh::GeneratePlanes(Bool ReverseOrder)
{
	//	delete current data
	m_TrianglePlanes.Empty();	
	m_TriStripPlanes.Empty();

	int t,i;

	m_TrianglePlanes.Resize( m_Triangles.Size() );
	for ( t=0;	t<m_Triangles.Size();	t++ )
	{
		float3& v1 = m_Verts[m_Triangles[t][0]];
		float3& v2 = m_Verts[m_Triangles[t][1]];
		float3& v3 = m_Verts[m_Triangles[t][2]];
		GPlane& Plane = m_TrianglePlanes[t];

		//	TEMP
		Bool rev = ReverseOrder;
		//if ( t & 1 )
		//	rev = !rev;

		if ( !rev )
			Plane.CalcEquation( v1, v2, v3 );
		else
			Plane.CalcEquation( v3, v2, v1 );
	}

	//	check tristrips
	m_TriStripPlanes.Resize( m_TriStrips.Size() );
	for ( t=0;	t<m_TriStrips.Size();	t++ )
	{
		GTriStrip& TriStrip = m_TriStrips[t];
		GPlaneList& TriStripPlaneList = m_TriStripPlanes[t];
		TriStripPlaneList.Resize( TriStrip.m_Indicies.Size()-2 );

		GTriangle Triangle;

		//	add first 2 bits of triangle
		Triangle[0] = TriStrip.m_Indicies[0];
		Triangle[1] = TriStrip.m_Indicies[1];

		//	replace the next triangle element in sequence along the tristrip
		for ( i=2;	i<TriStrip.m_Indicies.Size();	i++ )
		{
			int im = i%3;
			Triangle[ im ] = TriStrip.m_Indicies[i];

			float3& v1 = m_Verts[Triangle[0]];
			float3& v2 = m_Verts[Triangle[1]];
			float3& v3 = m_Verts[Triangle[2]];
	
			GPlane& Plane = TriStripPlaneList[i-2];

			//	reverse order to calc plane every other new triangle 
			if ( !ReverseOrder )
			{
				if ( i & 1 )
					Plane.CalcEquation( v3, v2, v1 );
				else
					Plane.CalcEquation( v1, v2, v3 );
			}
			else
			{
				if ( i & 1 )
					Plane.CalcEquation( v1, v2, v3 );
				else
					Plane.CalcEquation( v3, v2, v1 );
			}


			GDebug::CheckFloat( Plane );
		}
	}

}


//-------------------------------------------------------------------------
//	finds and removes invalid triangles
//-------------------------------------------------------------------------
void GMesh::RemoveInvalidTriangles()
{
	if ( TriCount() )
	{
		int OriginalTriCount = TriCount();
		int InvalidTriangles = 0;

		//	work backwards so we dont have to recalc any indexes
		for ( int t=TriCount()-1;	t>=0;	t-- )
		{
			int& a = m_Triangles[t][0];
			int& b = m_Triangles[t][1];
			int& c = m_Triangles[t][2];

			//	check for triangle that uses the same vert more than once
			if ( a==b || b==c || a==c )
			{
				InvalidTriangles++;
				RemoveTriangle( t );
				continue;
			}

			//	
			float SurfaceArea = TriangleSurfaceArea( m_Verts[a], m_Verts[b], m_Verts[c] );
			GDebug_Print("%d: %3.3f\n", t, SurfaceArea );
			if ( SurfaceArea < 0.1f/*NEAR_ZERO*/ )
			{
				InvalidTriangles++;
				RemoveTriangle(t);
				continue;
			}

		}

		GDebug_Print("Removed %d/%d invalid triangles\n", InvalidTriangles, OriginalTriCount );
	}

	//	do tristrips
	if ( TriStripCount() )
	{
	//	for ( int ts=0;	ts
	}
}
		
//-------------------------------------------------------------------------
//	copies a list of tristrips (we can't currently do a direct memcpy as that will
//	copy the list's pointer, rather than the list's contents
//-------------------------------------------------------------------------
void GMesh::CopyTriStrips(GList<GTriStrip>& TriStripList)
{
	m_TriStrips.Resize( TriStripList.Size() );

	for ( int ts=0;	ts<TriStripList.Size();	ts++ )
	{
		m_TriStrips[ts].m_Indicies.Copy( TriStripList[ts].m_Indicies );
	}
}

//-------------------------------------------------------------------------
//	generates basic UV textures for primitives
//-------------------------------------------------------------------------
void GMesh::GenerateTextureUV()
{
	/*
	//	basic UV for triangles
	float2 TriUV[3] = { float2(0,0), float2(1,0), float2(1,1) };
	
	m_TextureUV.Resize( VertCount() );

	//	generate UV's for triangle strips to try and cover the whole texture
	*/

	m_TextureUV.Resize( VertCount() );

	for ( int i=0;	i<m_TextureUV.Size();	i++ )
	{
		m_TextureUV[i].x = fabsf( sinf( 0.2f * (float)i ) );
		m_TextureUV[i].y = fabsf( cosf( 0.3f * (float)i ) );
	}
}


//-------------------------------------------------------------------------
//	scale up/down UVs to tile/untile
//-------------------------------------------------------------------------
void GMesh::ScaleUV(float2 UVScale)
{
	for ( int i=0;	i<m_TextureUV.Size();	i++ )
	{
		m_TextureUV[i].x *= UVScale.x;
		m_TextureUV[i].y *= UVScale.y;
	}
}


//-------------------------------------------------------------------------
//	find a collision object with the specified ref
//-------------------------------------------------------------------------
GCollisionObj* GMesh::GetCollisionObject(u32 AssetRef)
{
	int Index = GetCollisionObjectIndex(AssetRef);
	if ( Index == -1 )
		return NULL;
	
	return &m_CollisionObjects[Index];
}

//-------------------------------------------------------------------------
//	get the index of a collision object with the specified ref
//-------------------------------------------------------------------------
int GMesh::GetCollisionObjectIndex(u32 AssetRef)
{
	for ( int i=0;	i<m_CollisionObjects.Size();	i++ )
	{
		if ( m_CollisionObjects[i].m_Ref == AssetRef )
			return i;
	}
	return -1;
}

//-------------------------------------------------------------------------
//	delete a single collision object based on its ref
//-------------------------------------------------------------------------
Bool GMesh::DestroyCollisionObject(u32 AssetRef)
{
	int Index = GetCollisionObjectIndex(AssetRef);
	if ( Index == -1 )
		return FALSE;
	
	m_CollisionObjects.RemoveAt(Index);
	return TRUE;
}

//-------------------------------------------------------------------------
//	remove all collision objects
//-------------------------------------------------------------------------
void GMesh::DestroyAllCollisionObjects()
{
	m_CollisionObjects.Empty();
}



void GMesh::GenerateTetrahedron(float Scale)
{
	//	clean out current data
	Cleanup();

	//	tetrahedron is made of 4 verts
	AllocVerts( 4 );
	m_TextureUV.Resize( 4 );
	m_Normals.Resize( 4 );

	float3& Up		= m_Verts[0];
	float3& Foward	= m_Verts[1];
	float3& Left	= m_Verts[2];
	float3& Right	= m_Verts[3];

	float AngleStep = 360.f/3.f;

	Foward.y = 0.f;
	Foward.x = cosf( DegToRad( AngleStep * 0 ) ) * Scale;
	Foward.z = sinf( DegToRad( AngleStep * 0 ) ) * Scale;
	
	Left.y = 0.f;
	Left.x = cosf( DegToRad( AngleStep * 1 ) ) * Scale;
	Left.z = sinf( DegToRad( AngleStep * 1 ) ) * Scale;
	
	Right.y = 0.f;
	Right.x = cosf( DegToRad( AngleStep * 2 ) ) * Scale;
	Right.z = sinf( DegToRad( AngleStep * 2 ) ) * Scale;

	Up.y = sqrtf( (Scale*Scale) + (Scale*Scale) );
	Up.x = 0.f;
	Up.z = 0.f;

	float Length01 = ( m_Verts[0] - m_Verts[1] ).Length();
	float Length02 = ( m_Verts[0] - m_Verts[2] ).Length();
	float Length03 = ( m_Verts[0] - m_Verts[3] ).Length();
	float Length12 = ( m_Verts[1] - m_Verts[2] ).Length();
	float Length13 = ( m_Verts[1] - m_Verts[3] ).Length();
	float Length23 = ( m_Verts[2] - m_Verts[3] ).Length();


	//	setup textures
	m_TextureUV[0] = float2( 0.0f, 0.0f );	//	top
	m_TextureUV[1] = float2( 0.0f, 1.0f );	//	foward
	m_TextureUV[2] = float2( 0.5f, 1.0f );	//	left
	m_TextureUV[3] = float2( 1.0f, 1.0f );	//	right

	//	setup normals
	float3 Center;
	GetVertexCenter( Center );

	m_Normals[0] = ( m_Verts[0] - Center ).Normal();
	m_Normals[1] = ( m_Verts[1] - Center ).Normal();
	m_Normals[2] = ( m_Verts[2] - Center ).Normal();
	m_Normals[3] = ( m_Verts[3] - Center ).Normal();

	//	create triangles
	GTriangle Triangle;

	Triangle = int3( 0, 1, 2 );
	m_Triangles.Add( Triangle );

	Triangle = int3( 0, 2, 3 );
	m_Triangles.Add( Triangle );

	Triangle = int3( 0, 3, 1 );
	m_Triangles.Add( Triangle );

	Triangle = int3( 1, 2, 3 );
	m_Triangles.Add( Triangle );


	//	generate colours
	GenerateDebugColours();
}


//////////////////////////////////////////////////////////////////////////


GMeshShadow::GMeshShadow()
{
	m_pOwner = NULL;
}

GMeshShadow::~GMeshShadow()
{
	Cleanup();
}

//-------------------------------------------------------------------------
//	cleanup current data
//-------------------------------------------------------------------------
void GMeshShadow::Cleanup()
{
	m_Quads.Empty();
}

//-------------------------------------------------------------------------
//	
//-------------------------------------------------------------------------
Bool GMeshShadow::Load(GBinaryData& Data)
{
	//	cleanup current data
	Cleanup();

	//	get number of quads
	u32 QuadCount;
	if ( !Data.Read( &QuadCount, sizeof(QuadCount), "MeshShadow quad count" ) )
		return FALSE;

	m_Quads.Resize( QuadCount );
	if ( !Data.Read( m_Quads.Data(), m_Quads.DataSize(), "MeshShadow quads" ) )
		return FALSE;

	return TRUE;
}

//-------------------------------------------------------------------------
//	
//-------------------------------------------------------------------------
void GMeshShadow::Save(GBinaryData& Data)
{
	//	get number of quads
	u32 QuadCount = m_Quads.Size();
	Data.Write( &QuadCount, sizeof(QuadCount) );
	Data.Write( m_Quads.Data(), m_Quads.DataSize() );
}

//-------------------------------------------------------------------------
//	do we want to save our data?
//-------------------------------------------------------------------------
Bool GMeshShadow::ValidData()
{
	if ( m_Quads.Size() > 0 )
		return TRUE;

	return FALSE;
}

//-------------------------------------------------------------------------
//	generate required shadow data from parent
//-------------------------------------------------------------------------
void GMeshShadow::Generate()
{
	Cleanup();

	if ( !m_pOwner )
	{
		GDebug_Break("Cannot generate shadow data without owner\n");
		return;
	}

	//
}

