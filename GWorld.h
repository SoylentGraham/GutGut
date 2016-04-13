/*------------------------------------------------

  GWorld Header file



-------------------------------------------------*/

#ifndef __GWORLD__H_
#define __GWORLD__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GList.h"
#include "GAsset.h"
#include "GAssetList.h"
#include "GMap.h"
#include "GMesh.h"
#include "GObject.h"
#include "GTexture.h"
#include "GSkyBox.h"



//	Macros
//------------------------------------------------
namespace GGameObjectFlags
{
	const u32	ForceColour		= 1<<0;	//	forces all subobjects to use this colour
	const u32	MergeColourMult	= 1<<1;	//	merges colour with subobject's colour
	const u32	MergeColourAdd	= 1<<2;	//	merges colour with subobject's colour
	const u32	MergeColourSub	= 1<<3;	//	merges colour with subobject's colour
};


//	Types
//------------------------------------------------
class GGameObject;
class GWorld;
class GPhysicsObject;
class GShader;
typedef GList<GGameObject*> GGameObjectList;


//-------------------------------------------------------------------------
//	very similar to the map object, but with physics
//	todo: derive both from a base type?
//-------------------------------------------------------------------------
class GGameObject
{
public:
	GAssetRef		m_Mesh;			//	
	GAssetRef		m_Texture;		//	
	
	float3			m_Position;		//	
	GQuaternion		m_Rotation;		//	
	float4			m_Colour;		//	
	u32				m_Flags;		//	GGameObjectFlags
	int				m_SubMapOn;		//	

	float3			m_ExtractedMovement;	//	movement last extracted (zero after use), set by physics etc

private:
	GShader*		m_pShader;		//	
	GPhysicsObject*	m_pPhysics;		//	

public:
	GGameObject();
	~GGameObject();

	GDrawResult		Draw(u32 DrawFlags);							//	draws the object for this game object

	virtual void	Update()								{	};	//	overloaded by game object types
	virtual Bool	PreDraw(GMesh* pMesh, GDrawInfo& DrawInfo);		//	called just before being drawn (before transformations)
	virtual void	PostDraw(GMesh* pMesh, GDrawInfo& DrawInfo);	//	called after drawing the mesh

	int				UpdateSubMapOn(GWorld* pWorld);			//	update our submapon index from our current position. and updates the list in the world. (returns new submapon)
	void			GetLightSource(float4& Light);			//	calculate a light source
	GSubMap*		GetSubmapOn(GWorld* pWorld);			//	returns a pointer to the submap we're on

	void			SetShader(GShader* pShader);
	void			SetPhysics(GPhysicsObject* pPhysics);
	GShader*		Shader()								{	return m_pShader;	};
	GPhysicsObject*	Physics()								{	return m_pPhysics;	};
	const GShader*			Shader() const 					{	return m_pShader;	};
	const GPhysicsObject*	Physics() const 				{	return m_pPhysics;	};

	GBounds&		GetBounds();							//	
	GMesh*			GetMesh();								//	
	GTexture*		GetTexture();							//	

	static Bool		CheckIntersection(GGameObject* pObjA, GGameObject* pObjB);
};




//-------------------------------------------------------------------------
//	map and object container
//-------------------------------------------------------------------------
class GWorld
{
	friend GMap;
	friend GGameObject;
public:
	GMap*					m_pMap;							//	map in this world
	GGameObjectList			m_ObjectList;					//	all the objects in this world
	GGameObjectList*		m_pSubmapObjectList;			//	array[submap.size] of game object pointers for quick access to all the objects in a submap
	float3					m_WorldUp;						//	world up vector (usually 0,1,0)
	GSkyBox*				m_pSkyBox;

public:
	GWorld();
	~GWorld();

	//	world virtual funcs
	virtual void		Update();							//	updates all the objects(+shaders, physics etc) in the world
	virtual void		PreDrawMapObjects(GList<u32>& MapObjects)		{	};
	virtual void		PostDrawMapObjects(GList<u32>& MapObjects)		{	};
	virtual void		PreDrawGameObjects(GList<u32>& GameObjects)		{	};
	virtual void		PostDrawGameObjects(GList<u32>& GameObjects)	{	};
	
	void				SetMap(GMap* pMap);						//	set the map for this world. (creates submap lists etc)
	void				Empty()									{	SetMap(NULL);	};	//	deletes everything out of our world
	Bool				AddObject(GGameObject* pObject);		//	add an object to the world
	void				Draw(GCamera& Camera, u32 DrawFlags);	//	render the world from this camera
	inline int			SubmapOn(float3& Position)				{	return m_pMap ? m_pMap->SubmapOn(Position) : -1;	};	//	get submap index for this position (world space)
	inline int			SubmapNearest(float3& Position)			{	return m_pMap ? m_pMap->SubmapNearest(Position) : -1;	};	//	get submap index for this position (world space)
	inline GMapLight*	GetLight(float3& Pos, int Submap=-1)	{	return m_pMap ? m_pMap->GetLight(Pos, Submap ) : NULL;	};

protected:
	Bool				RemoveObjectFromSubmapList( GGameObject* pObject, int SubMapIndex );
	Bool				AddObjectToSubmapList( GGameObject* pObject, int SubMapIndex );
	float4*				GetNearestLightPos(const float3& Pos);	//	find the nearest light for this pos
	void				GatherPhysicsTestCases(GList<GPhysicsObject*>& PhysicsObjects);
	
private:
	void				BuildWorldRender(GWorldRender& WorldRender,GCamera* pCamera);	//	build a world render object
	
};


class GWorldRender
{
	friend GWorld;
	friend GSubMap;

private:
	//	internal variables
	GCamera*			m_pCamera;			//	camera to render/cull everything with
	GList<u32>			m_MapObjects;		//	--submap index--|---mapobject index---				list of map objects on submaps we're going to render
	GList<u32>			m_GameObjects;		//	--submap index--|---gameobject index---				list of game objects
	GList<u32>			m_Portals;			//	--submap index--|---portal index---					list of all portal type
	GList<u32>			m_SubMaps;			//	--submap index--|---portals rendered through---		list of submaps we've processed and through which portals they were processed (bitfield)
	u32					m_BasePortal;		//	base portal. (submapindex|portalindex) invalid if 0xffffffff

public:
	GWorldRender();
	~GWorldRender();

	void	Reset();								//	resets the world view ready for a new build
	void	Draw(GWorld& World, u32 DrawFlags);		//	renders the world. add optional flags for debugging etc
	void	Build(GWorld& World, GCamera* pCamera, u32 BasePortal=0xffffffff);	//	build from this camera

protected:
	void	BuildThroughSubmap(GWorld& World, int Submap, u16 PortalBit, GCamera* pCamera);
	Bool	FindIndexInList(GList<u32>& List, u16 Submap, u16 Index);
	Bool	FindBitsInList(GList<u32>& List, u16 Submap, u16 Bits);
};



//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------





#endif

