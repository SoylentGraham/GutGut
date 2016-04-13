/*------------------------------------------------

  GObject Header file



-------------------------------------------------*/
/*
#ifndef __GOBJECT__H_
#define __GOBJECT__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GAsset.h"
#include "GQuaternion.h"
#include "GList.h"
#include "GTexture.h"
#include "GMesh.h"



//	Macros
//------------------------------------------------

namespace GSubObjectFlags
{
	const u32	UseColour	= 1<<0;		//	use colour specified in the sub object. overrides parent object's colour when rendered
	const u32	DontRender	= 1<<1;		//	dont render this subobject. but still used for collision etc
};


namespace GObjectFlags
{
};




//	Types
//------------------------------------------------


/*
	Header for an object in a datafile.
*//*
typedef struct 
{
	u16			SubObjectCount;	//	number of sub objects
	float3		BoundsOffset;	//	offset for the bounds
	float		BoundsRadius;	//	radius for the bounds

} GObjectHeader;





/*
	sub object of an object. basiccly contains what data to render and a modelspace offset for that subobject from the object
*//*
class GSubObject
{
public:
	GAssetRef	m_Mesh;				//	main mesh
	GAssetRef	m_Texture;			//	texture for mesh
	GAssetRef	m_ShadowMesh;		//	reduced poly mesh for shadow casting
	GAssetRef	m_CollisionMesh;	//	reduced poly mesh for raycasts and collision checking

	u32			m_Flags;			//	GSubObjectFlags
	float3		m_Position;			//	
	GQuaternion	m_Rotation;			//	
	float4		m_Colour;			//	sub object's colour

public:
	GSubObject();
	~GSubObject();

	void			Set(GAssetRef Mesh, GAssetRef Texture, GAssetRef ShadowMesh=GAssetRef_Invalid, GAssetRef CollisionMesh=GAssetRef_Invalid);
	
	inline void		UseColour(Bool Use=TRUE)	{	if ( Use )	m_Flags |= GSubObjectFlags::UseColour;	else	m_Flags &= ~GSubObjectFlags::UseColour;	};
	inline void		SetColour(float4& Colour)	{	UseColour(TRUE);	m_Colour = Colour;	};

	Bool			Load(u8* pData,int DataSize,int& DataRead);	//	load directly from data. DataSize is how big pData is, and DataRead is the bytes read. returns successfull load
	void			Save(GList<u8>& SaveData);					//	add subobject data to the savedata list

};




/*
	in-game object that contains numerous meshes (in subobjects)
*//*
class GObject : public GAsset
{
public:
	const static u32	g_Version;

public:
	GList<GSubObject*>	m_SubObjects;
	GBounds				m_Bounds;

public:
	GObject();
	~GObject();

	//	virtual
	virtual GAssetType	AssetType()					{	return GAssetObject;	};
	virtual u32			Version()					{	return GObject::g_Version;	};
	virtual Bool		Load(u8* pData,int DataSize,int& DataRead);	//	load directly from data. DataSize is how big pData is, and DataRead is the bytes read. returns successfull load
	virtual Bool		Save(GList<u8>& SaveData);												//	add all the data the asset needs to save into the array passed in

	//	object specific
	GSubObject*			AddSubObject(GSubObject* pSubObject )		{	m_SubObjects.Add( pSubObject );	return pSubObject;	};					//	add an already created sub object
	GSubObject*			AddSubObject(GAssetRef Mesh, GAssetRef Texture, GAssetRef ShadowMesh=GAssetRef_Invalid, GAssetRef CollisionMesh=GAssetRef_Invalid);	//	create new sub object from parameters
	void				DeleteSubObjects();

	void				ChangeMeshRef(GAssetRef MeshRef, GAssetRef NewRef );		//	change any references to this mesh to the new one
	void				ChangeTextureRef(GAssetRef TextureRef, GAssetRef NewRef );	//	change any references to this texture to the new one

	GDrawResult			Draw(GDrawInfo& DrawInfo);
	GDrawResult			DrawShadow(GDrawInfo& DrawInfo);
	void				GenerateBounds(Bool Force=FALSE);						//	calculate bounds based on meshes
};






//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------





#endif

*/
