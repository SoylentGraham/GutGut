/*------------------------------------------------

  GMap Header file



-------------------------------------------------*/

#ifndef __GMAP__H_
#define __GMAP__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GAsset.h"
#include "GQuaternion.h"
#include "GList.h"
#include "GObject.h"
#include "GDisplay.h"


//	Macros
//------------------------------------------------


namespace GMapObjectFlags
{
	const u32	ShowObjectsInside	= 1<<0;	//	if there are objects inside this mapobject, show them instead of automaticcly culling. flag if this object is see-through etc
	const u32	DontCastShadow		= 1<<1;	//	this mapobject doesnt cast shadows
	const u32	CullFrontFaces		= 1<<2;	//	cull reverse order


	inline const char*	GetFlagName(int FlagNo)
	{
		switch ( FlagNo )
		{
			case 0:	return "Show Objects Inside";	break;
			case 1:	return "Dont Cast Shadow";	break;
			case 2:	return "Cull Front Faces";	break;
		}
		return NULL;
	}

};


typedef enum GPortalType
{
	GPortal_Normal=0,	//	regular portal
	GPortal_Mirror,		//	portal is actually a mirror
//	GPortal_Warp,		//	portal renders the view of another submap
};


//	Types
//------------------------------------------------
class GCamera;
class GMap;
class GSubMap;
class GMapObject;
class GMapList;
class GSubMapList;
class GMapObjectList;
class GWorld;
class GWorldRender;
class GMesh;
class GShader;
class GTexture;


//-------------------------------------------------------------------------
//	Header for a map in a datafile.
//	<Header>
//	<submap assets>
//-------------------------------------------------------------------------
typedef struct 
{
	u16		SubMapCount;		//	number of sub map's in this map

} GMapHeader;


//-------------------------------------------------------------------------
//	Header for a submap in a datafile. assetref's follow the header in the data
//-------------------------------------------------------------------------
typedef struct 
{
	u16		MapObjectCount;			//	number of map objects in subobject
	u16		ObjectInsideListSize;	//	number of elements in the ObjectInsideList
	u16		Portals;				//	number of portals

} GSubMapHeader;

//-------------------------------------------------------------------------
//	Header for a mapobject in a datafile.
//-------------------------------------------------------------------------
typedef struct 
{
	float3			Position;
	GQuaternion		Rotation;
	float4			Colour;
	u32				Flags;
	GAssetRef		MeshRef;
	GAssetRef		TextureRef;
	GAssetRef		ShaderRef;	//	todo: work out what shader to use from this

} GMapObjectHeader;


//-------------------------------------------------------------------------
//	data for a portal in a datafile
//-------------------------------------------------------------------------
typedef struct 
{
	GAssetRef		AssetRef;	//	this portal's ref
	GPortalType		Type;		//	type of portal
	float3			Verts[4];	//	portal vertexes
	float3			Normal;		//	portal normal
	GAssetRef		Ref1;		//	misc ref's (in union)
	GAssetRef		Ref2;		//	

} GMapPortalHeader;


//-------------------------------------------------------------------------
//	Portal in a submap
//-------------------------------------------------------------------------
class GMapPortal
{
public:
	GAssetRef		m_PortalRef;				//	this portal's ref
	GPortalType		m_Type;						//	what type of portal
	float3			m_PortalVerts[4];			//	vertexes defining the portal (world pos)
	float3			m_PortalNormal;				//	portal's normal

	union
	{
		struct	//	PortalNormal
		{
			GAssetRef		m_OtherSubmap;		//	the map on the other side of this portal
			GAssetRef		m_OtherPortal;		//	reference to the portal on the other submap
		};

		struct  //	PortalMirror
		{
			GAssetRef		m_Texture;			//	reference to the texture to render to
		};
	};

public:

	void			MakeCamera(GCamera& PortalCamera,GCamera* pParentCamera);	//	create portal camera based on this portal and the camera looking at this portal
	float3			PortalCenter();												//	get the center of the portal
	void			DebugDraw();
	void			GenerateFromMesh(GAssetRef MeshRef);						//	generate the portal's verts from this mesh
	void			ReOrderVerts(int VertNo);									//	reorder the verts on the portal (we need this to make it square)
	void			MoveVerts(float3& Movement);								//	move all verts
};



//-------------------------------------------------------------------------
//	Light in a submap
//-------------------------------------------------------------------------
class GMapLight
{
public:
	float4			m_Pos;			//	position of this light
	float4			m_Colour;		//	colour of the light
	float			m_Strength;		//	strength of the light. 0..1. this affects the darkness of shadows and the ambient lighting

public:
	GMapLight();
	~GMapLight()	{	};

	inline void		Copy(const GMapLight& Light)			{	memcpy( this, &Light, sizeof( GMapLight ) );	};
	inline			operator = (const GMapLight& Light)	{	Copy(Light);	};
};


//-------------------------------------------------------------------------
//	sub object of an object. basiccly contains what data to render and a modelspace offset for that subobject from the object
//-------------------------------------------------------------------------
class GMapObject : public GAsset
{
public:
	const static u32	g_Version;

public:
	GAssetRef		m_Mesh;			//	
	GAssetRef		m_Texture;		//	
	float3			m_Position;		//	
	GQuaternion		m_Rotation;		//	
	float4			m_Colour;		//	
	u32				m_Flags;		//	GMapObjectFlags

	GAssetRef		m_ShaderRef;	//	todo: work out what shader to use from this
	GShader*		m_pShader;		//	shader if applicable

public:
	GMapObject();
	~GMapObject();

	//	virtual 
	virtual GAssetType	AssetType()		{	return GAssetMapObject;	};
	virtual u32			Version()		{	return GMapObject::g_Version;	};
	virtual Bool		Load(GBinaryData& Data);
	virtual Bool		Save(GBinaryData& Data);

	GDrawResult			Draw(u32 DrawFlags);

	GMesh*				GetMesh();
	GTexture*			GetTexture();

	GBounds&			GetBounds();						//	

};



/*
	submap of a map. contains a list of map objects present in the map, portals etc for this submap
*/
class GSubMap : public GAsset
{
	friend GWorld;
public:
	const static u32	g_Version;

public:
	GList<GAssetRef>	m_MapObjects;			//	list of mapobjects in this submap
	GList<GMapPortal>	m_Portals;				//	list of portals in this submap
	GList<u32>			m_ObjectInsideList;		//	pair of u16's: object index is inside object index
	GList<GMapLight>	m_Lights;				//	list of lights in this submap

public:
	GSubMap();
	~GSubMap();

	//	virtual 
	virtual GAssetType	AssetType()		{	return GAssetSubMap;	};
	virtual u32			Version()		{	return GSubMap::g_Version;	};
	virtual Bool		Load(GBinaryData& Data);
	virtual Bool		Save(GBinaryData& Data);

	void				Draw(u32 DrawFlags);									//	renders this submap on its own
	void				DrawShadow(u32 DrawFlags);

	void				GenerateBounds(Bool Force=FALSE);						//	calculate bounds and object inside list
	Bool				PositionInside(float3& Position);						//	check if this position is inside any of our mapobjects, if so, its inside this map
	float3				GetSubmapCenter();										//	find the average center of all our objects

	void				PreDrawMapObjects(GCamera* pCamera, GList<u32>& VisibleMapObjects, GList<GPreDrawResult>& Results, int SubMapIndex);
	void				PreDrawGameObjects(GCamera* pCamera, GWorld& World, GList<u32>& VisibleGameObjects, GList<GPreDrawResult>& MapObjectPreDrawResults, int SubMapIndex);
	void				GetVisiblePortals(GCamera* pCamera, GList<u32>& VisiblePortals, int SubMapIndex);

	int					GetMapObjectIndex(GAssetRef MapObjectRef)				{	return m_MapObjects.FindIndex(MapObjectRef);	};
	Bool				ChangeMapObjectRef(GAssetRef MapObjectRef,GAssetRef NewRef);
	void				RemoveMapObject(GAssetRef MapObjectRef);
	void				AddMapObject(GAssetRef MapObjectRef);					//	add a new map object

	int					GetPortalIndex(GAssetRef PortalRef);					//	find the index of the portal with this reference
	GMapPortal*			GetPortal(GAssetRef PortalRef);							//	return the portal with this reference
	Bool				ChangePortalRef(GAssetRef PortalRef,GAssetRef NewRef );
	void				DeletePortal(GAssetRef PortalRef);						//	delete portal
	GAssetRef			GetFreePortalRef();										//	return an unused assetref
	void				AddPortal(GMapPortal& NewPortal);						//	add a new portal into the list

	void				GetLight(GMapLight& Light, float3& Pos);				//	fill in the light struct for the position (generate a light, grab nearest etc)

private:
	void				BuildObjectInsideList();								//	rebuilds the m_ObjectInsideList list by calulcating which object bounding boxes are inside others
	Bool				ObjectInsideObject(int ObjectA, int ObjectB);			//	uses m_ObjectInsideList to tell if object A inside object B ?
};




/*
	map. contains sub maps
*/
class GMap : public GAsset
{
public:
	const static u32	g_Version;

public:
	GList<GSubMap*>		m_SubMaps;

public:
	GMap();
	~GMap();

	//	virtual 
	virtual GAssetType	AssetType()		{	return GAssetMap;	};
	virtual u32			Version()		{	return GMap::g_Version;	};
	virtual Bool		Load(GBinaryData& Data);
	virtual Bool		Save(GBinaryData& Data);

	int					GetSubmapIndex(GAssetRef SubmapRef);
	GSubMap*			GetSubMap(GAssetRef SubmapRef);
	int					SubmapOn(float3& Position);				//	get submap index for this position (world space)
	int					SubmapNearest(float3& Position);		//	get nearest submap index for this position (world space)
	void				DeleteSubMap(GAssetRef SubMapRef);		//	delete submap

	void				GenerateBounds(Bool Force=FALSE);		//	applies a bounds generation for all submaps

	void				Draw(u32 DrawFlags);					//	draws all our submaps
	GMapLight*			GetLight(float3& Pos, int Submap=-1);
};


















//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------





#endif

