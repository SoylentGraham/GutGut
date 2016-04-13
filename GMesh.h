/*------------------------------------------------

  GMesh Header file



-------------------------------------------------*/

#ifndef __GMESH__H_
#define __GMESH__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GAsset.h"
#include "GMatrix.h"
#include "GList.h"
#include "GTexture.h"
#include "GDisplay.h"
#include "GCollisionObject.h"


//	Macros
//------------------------------------------------

//-------------------------------------------------------------------------
//	flags identifying what data is stored with mesh assets
//-------------------------------------------------------------------------
namespace GMeshDataFlags
{
	const u32	VertNormals			= 1<<0;		//	vertex normals
	const u32	VertTextureUV		= 1<<1;		//	texture UV coords 
	const u32	TriangleNormals		= 1<<2;		//	triangle face normals
	const u32	TriangleColours		= 1<<3;		//	triangle debug colours
	const u32	TriangleNeighbours	= 1<<4;		//	triangle neighbour data
	const u32	TriangleCenters		= 1<<5;		//	triangle face centers
	const u32	TriStripColours		= 1<<6;		//	tristrip debug colours
	const u32	OLD_ShadowQuads		= 1<<7;		//	shadow quads
	const u32	OLD_ShadowVerts		= 1<<8;		//	shadow vert
	const u32	OLD_ShadowNormals	= 1<<9;		//	shadow normals
	const u32	TrianglePlanes		= 1<<10;	//	
	const u32	TriStripPlanes		= 1<<11;	//	
	const u32	VertTextureUV2		= 1<<12;	//	multitexturing coords
	const u32	CollisionObjects	= 1<<13;	//	has new collison objects setup
	const u32	ShadowData			= 1<<14;	//	new shadow data
};


//-------------------------------------------------------------------------
//	types of buffers used with shaders
//-------------------------------------------------------------------------
namespace GMeshBuffer
{
	const u32	Vertex		= 1<<0;
	const u32	Normal		= 1<<1;
	const u32	TextureUV	= 1<<2;
	const u32	TextureUV2	= 1<<3;
	const u32	Colour		= 1<<4;
};



//	Types
//------------------------------------------------
class GQuaternion;
class GMesh;

typedef int3			GTriangle;
typedef int4			GQuad;
typedef GList<GPlane>	GPlaneList;


//-------------------------------------------------------------------------
//	Header for a mesh in a datafile. vertex data etc follows directly after the header
//-------------------------------------------------------------------------
typedef struct 
{
	u32			VertexCount;	//	number of vertexes and vertex normals
	u32			TriangleCount;	//	number of triangles and triangle neighbours
	u32			TriStripCount;	//	number of triangle strips
	u32			UNUSEDShadowQuads;	//	number of quads for shadows
	float3		UNUSEDBoundsOffset;	//	bounds offset from our object/meshes 0,0,0
	float		UNUSEDBoundsRadius;	//	bounds radius
	u32			MeshDataFlags;	//	flags what data exists in this mesh save data

} GMeshHeader;


//-------------------------------------------------------------------------
//	triangle strip definition
//-------------------------------------------------------------------------
class GTriStrip
{
public:
	GList<int>	m_Indicies;			//	list of vertex indicies

public:
	Bool		Load(GBinaryData& Data);	//	return amount of data used
	void		Save(GBinaryData& Data);	//	save data
};

typedef struct 
{
	u16		Indicies;	//	no of indicies

} GTriStripHeader;


//-------------------------------------------------------------------------
//	shadow data for a mesh
//-------------------------------------------------------------------------
class GMeshShadow
{
public:
	GList<int4>		m_Quads;	//	quad along each triangle edge
	GMesh*			m_pOwner;	//	owner mesh

public:
	GMeshShadow();
	~GMeshShadow();

	void			Generate();					//	generate data from parent
	void			Cleanup();					//	delete existing data
	Bool			Load(GBinaryData& Data);
	void			Save(GBinaryData& Data);
	Bool			ValidData();				//	do we want to save the data we have?
};

//-------------------------------------------------------------------------
//	Simple mesh to render contain vertex and geometry data
//-------------------------------------------------------------------------
class GMesh : public GAsset
{
public:
	const static u32	g_Version;
	static u32			g_ForceFlagsOn;
	static u32			g_ForceFlagsOff;

	enum
	{
		VERTEX_ATTRIB_INDEX		= 0,
		WEIGHT_ATTRIB_INDEX		= 1,
		NORMAL_ATTRIB_INDEX		= 2,
		COLOUR0_ATTRIB_INDEX	= 3,
		COLOUR1_ATTRIB_INDEX	= 4,
		FOGCOORD_ATTRIB_INDEX	= 5,
		FREE0_ATTRIB_INDEX		= 6,
		FREE1_ATTRIB_INDEX		= 7,
		TEXCOORD0_ATTRIB_INDEX	= 8,
		TEXCOORD1_ATTRIB_INDEX	= 9,

		MAX_ATTRIB_INDEX		= 16, //	cant use this
	};

public:
	GList<float3>			m_Verts;				//	model space vertex positions
	GList<float3>			m_Normals;				//	for each vertex, the vertex's normal
	GList<float2>			m_TextureUV;			//	for each vertex, the vertex's UV texture coords
	GList<float2>			m_TextureUV2;			//	multitexturing UV coords

	GList<GTriangle>		m_Triangles;			//	triangle defintion of 3 vertex indexes
	GList<int3>				m_TriangleNeighbours;	//	for each triangle, this value specifies which other triangle is at which edge
	GList<float3>			m_TriangleColours;		//	debug colour per triangle
	GList<float3>			m_TriangleCenters;		//	center of each triangle (average between points)
	GList<GPlane>			m_TrianglePlanes;		//	plane for each triangle

	GList<GTriStrip>		m_TriStrips;			//	tristrip definitions
	GList<float3>			m_TriStripColours;		//	debug colour for each triangle strip
	GList<GPlaneList>		m_TriStripPlanes;		//	plane[list] for each triangle strip

	GMeshShadow				m_ShadowData;			//	shadow data

	GList<GCollisionObj>	m_CollisionObjects;		//	collision objects/markers
	
private:
	u32						m_VBOVertexID;			//	glID for VBO for vertexes
	u32						m_VBONormalID;			//	glID for VBO for normals
	u32						m_VBOTexCoordID;		//	glID for VBO for tex coords

public:
	GMesh();
	~GMesh();

	//	virtual
	virtual GAssetType	AssetType()		{	return GAssetMesh;	};
	virtual u32			Version()		{	return GMesh::g_Version;	};
	virtual Bool		Load(GBinaryData& Data);
	virtual Bool		Save(GBinaryData& Data);

	inline int			VertCount()			{	return m_Verts.Size();	};
	inline int			TriCount()			{	return m_Triangles.Size();	};
	inline int			TriStripCount()		{	return m_TriStrips.Size();	};
	void				ShowInfo();						//	popup info about the mesh

	virtual Bool		Upload();
	virtual Bool		Deload();

	//	mesh creation
	void				AllocVerts(int VertCount);
	void				AllocTriangles(int t);
	void				AllocTriStrips(int t);
	void				Cleanup();
						
	void				MergeVerts(float DistanceTolerance=0.001f, Bool CheckUV=TRUE, float UVTolerance=0.01f);	//	reduces re-used verts
	void				MergeMesh(GMesh* pMesh);		//	add in all the geometry from this mesh

	void				CopyVert(int From, int To);		//	copy details in vert From into vert To
	void				CopyTriStrips(GList<GTriStrip>& TriStripList);
	
	void				RemoveTriangle(int Triangle);	//	remove all the data for this triangle (index)
	void				RemoveInvalidTriangles();		//	finds and removes invalid triangles
	inline void			DeleteShadowData()				{	m_ShadowData.Cleanup();	};	//	deletes all the data for shadows

	void				MoveVerts(float3& Change);		//	move all the verts around
	void				RotateVerts(GQuaternion& Rotation)	{	MultiplyVerts( float3(0,0,0), Rotation );	};
	void				CenterVerts(float3 Center=float3(0,0,0));
	void				SnapToFloor(float FloorY=0.f);
	void				ScaleUV(float2 UVScale);
	void				Scale(float3 Mult);
	inline void			Scale(float Mult)				{	Scale( float3( Mult, Mult, Mult ) );	};
	void				MultiplyVerts(float3& Translation, GQuaternion& Rotation);

	void				CheckFloats();					//	check all verts for invalid floats

	void				TriStrip();						//	tristrip our triangles where possible
	void				SplitTriStrips();				//	break tristrips down to triangles
	void				InvertVertexNormals();			//	invert all normals
	void				InvertPlaneNormals();			//	invert all normals
						
	void				GenerateTriangleNeighbours();	//	works out which triangles are connected to which other triangles
	void				GenerateDebugColours();			//	generates a (debug)colour for every triangle
	void				GenerateTriangleCenters();		//	generates a colour for every triangle
	void				GenerateNormals(Bool ReverseOrder=FALSE);		//	generates our own vertex normals from triangles
	void				GenerateBounds(Bool Force=FALSE);	//	recalculate bounding parameters
	void				GenerateShadowData()			{	m_ShadowData.Generate();	};
	void				GenerateTrianglesFromTriStrips(GList<GTriangle>& TriangleList);	//	create triangles for each tristrip and add to this list
	void				GenerateTriangleNormals(GList<GTriangle>& Triangles, GList<float3>& TriangleNormals);	//	generates triangle normals for the given triangles
	void				GenerateSphere(int SectionsX, int SectionsY);				//	generate sphere
	void				GenerateCube();					//	generate sphere
	void				GeneratePlanes(Bool ReverseOrder=FALSE);				//	recalculates planes for all triangles
	void				GenerateTextureUV();			//	generates basic UV textures for primitives
	void				GenerateTetrahedron(float Scale=1.f);	//	generates a tetrahedron shape
	
	float3				GetTriangleNormal(GTriangle& Triangle);	//	returns a normal for a triangle based on vertex normals
	float3				GetTriangleNormal(int t)		{	return GetTriangleNormal( m_Triangles[t] );	};
	float3				GetTriangleCenter(int t);
	void				GetVertexMinMax(float3& Min, float3& Max);
	void				GetVertexCenter(float3& Center);
	float3				GetFurthestVert();

	void				GetVertexBuffer(GList<float3>& Buffer)	{	Buffer.Copy( m_Verts );	};	//	copies the vertexes into a buffer

	//	drawing			
	GDrawResult			Draw(GDrawInfo& DrawInfo);		//	draw the mesh with certain parameters (texture references, shader, matrix pos, debug flags etc)
	void				DrawShadow(GDrawInfo& DrawInfo);	//	render shadow from this mesh. light pos is temporary until we have some kind of global enviroment
	void				DrawPrimitives();					//	draw triangles and tri-strips
	void				DrawDebugNormals(GList<float3>& Verts);
	void				DrawDebugTrianglePlanes(GList<float3>& Verts);	//	draws normals on triangles
	void				DrawDebugColours(GList<float3>& Verts, float Alpha,GShader* pShader);
	void				DrawDebugVertexes(GList<float3>& Verts, GList<float3>& VertColours, GShader* pShader);

	//	raycasts
	Bool				Raycast(float3 From,float3 To);	//	check this line going through geometry (from/to relative to mesh's 0,0,0)

	inline void			BindAttribArray(GList<float>& Buffer, int AttribIndex)				{	BindAttribArray( Buffer.Data(), GL_FLOAT, 1, AttribIndex );	};
	inline void			BindAttribArray(GList<float2>& Buffer, int AttribIndex)				{	BindAttribArray( Buffer.Data(), GL_FLOAT, 2, AttribIndex );	};
	inline void			BindAttribArray(GList<float3>& Buffer, int AttribIndex)				{	BindAttribArray( Buffer.Data(), GL_FLOAT, 3, AttribIndex );	};
	inline void			BindAttribArray(GList<float4>& Buffer, int AttribIndex)				{	BindAttribArray( Buffer.Data(), GL_FLOAT, 4, AttribIndex );	};
	void				BindAttribArray(void* pBuffer, int Type, int ElementSize, int AttribIndex);

	//	collision objects
	GCollisionObj*		GetCollisionObject(u32 AssetRef);
	int					GetCollisionObjectIndex(u32 AssetRef);
	Bool				DestroyCollisionObject(u32 AssetRef);
	void				DestroyAllCollisionObjects();

private:				
	void				DetectStrips(u16& nr_strips, GList<u16>& strip_length, u16& nr_indices, GList<u16>& stripindex, GList<int>& StrippedTriangles);
	void				DoShadowRender( GDrawInfo& DrawInfo, GList<Bool>& PlaneVisibleTable, GList<Bool>& VertsCalcd, GList<float3>& VertsCached, GList<u32>& CacheTriangleCastList );
	void				DoShadowCastFromEdge( GDrawInfo& DrawInfo, u32& Triangle, u32& TriangleEdge, GList<Bool>& VertsCalcd, GList<float3>& VertsCached );

	void				SelectBufferObject(u32& BufferObjectID);
	void				SelectBufferObjectNone();
	void				CreateVertexBufferObjects();	//	creates objects that dont exist
	void				DestroyVertexBufferObjects();	//	deletes VBO's

	void				UploadVertexBufferObjects();	//	uploads data
	void				UploadVertexBufferData(float* pData, int DataSize, u32& VBOIndex);	//	uploads new data
	void				DeloadVertexBufferObjects();	//	deloads data

	void				BindVertexArray(GList<float3>& VertexBuffer, Bool UseAttribMode);
	void				BindNormalArray(GList<float3>& NormalBuffer, Bool UseAttribMode);
	void				BindTextureUVArray(GList<float2>& TextureUVBuffer, Bool UseAttribMode);
	void				BindTextureUV2Array(GList<float2>& TextureUV2Buffer, Bool UseAttribMode);
	void				BindColourArray(GList<float3>& ColourBuffer, Bool UseAttribMode);
	
};





//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------




#endif

