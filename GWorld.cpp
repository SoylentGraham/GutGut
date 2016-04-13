/*------------------------------------------------

  GWorld.cpp

	World class that handles all the in-game objects and
	rendering of the map


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GWorld.h"
#include "GCamera.h"
#include "GApp.h"
#include "GAssetList.h"
#include "GPhysics.h"


//	globals
//------------------------------------------------
GDeclareCounter(GameObjIntersectionTest);



//	Definitions
//------------------------------------------------


GGameObject::GGameObject()
{
	m_Mesh		= GAssetRef_Invalid;
	m_Texture	= GAssetRef_Invalid;

	m_Flags		= 0x0;
	m_Position	= float3(0,0,0);
	m_Rotation	= GQuaternion();
	m_Colour	= float4(1,1,1,1);
	m_ExtractedMovement	= float3(0,0,0);
	
	m_SubMapOn	= -1;

	m_pPhysics	= NULL;
	m_pShader	= NULL;
}


GGameObject::~GGameObject()
{
}

void GGameObject::SetShader(GShader* pShader)				
{	
	//	warning if shader is already setup
	if ( m_pShader )
	{
		GDebug_Break("Shader already assigned to game object\n");
	}

	//	set new shader
	m_pShader = pShader;

	if ( Shader() )		
		Shader()->m_pOwner = this;	
}

void GGameObject::SetPhysics(GPhysicsObject* pPhysics)	
{	
	//	warning if physics is already setup
	if ( m_pPhysics )
	{
		GDebug_Break("Physics already assigned to game object\n");
	}

	//	set new physics
	m_pPhysics = pPhysics;	
	
	if ( Physics() )	
		Physics()->m_pOwner = this;	
}

GMesh* GGameObject::GetMesh()
{
	return GAssets::g_Meshes.Find( m_Mesh );
}

GTexture* GGameObject::GetTexture()
{
	return GAssets::g_Textures.Find( m_Texture );
}


GDrawResult GGameObject::Draw(u32 DrawFlags)
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

	//	todo: convert any m_Flags to drawinfo flags

	GIncCounter( DrawGameObjects, 1 );

	//	gameobject specific overloaded func to change display, debug stuff etc
	if ( !PreDraw( pMesh, DrawInfo ) )
		return GDrawResult_Cancelled;

	//	physics debug drawing routine
	if ( Physics() )
		if ( !Physics()->PreDraw( pMesh, DrawInfo ) )
			return GDrawResult_Cancelled;

	//	draw the mesh
	GDrawResult DrawResult = pMesh->Draw( DrawInfo );

	//	call gameobject overloaded post draw func
	PostDraw( pMesh, DrawInfo );
	
	return DrawResult;
}


Bool GGameObject::PreDraw(GMesh* pMesh, GDrawInfo& DrawInfo)
{
	return TRUE;
}

void GGameObject::PostDraw(GMesh* pMesh, GDrawInfo& DrawInfo)
{
}

int GGameObject::UpdateSubMapOn(GWorld* pWorld)
{
	//	get the submap for our position
	int NewSubmap = pWorld->SubmapOn( m_Position );

	//	submap hasnt changed
	if ( NewSubmap == m_SubMapOn )
		return m_SubMapOn;
		
	//	remove from old list
	pWorld->RemoveObjectFromSubmapList( this, m_SubMapOn );

	//	add into new list
	pWorld->AddObjectToSubmapList( this, NewSubmap );

	//	update submap on number
	m_SubMapOn = NewSubmap;

	return m_SubMapOn;
}



void GGameObject::GetLightSource(float4& Light)
{
	//	todo:
	//	gather nearby lights in map and get average
	Light.Set(0,0,0,0);
}


GSubMap* GGameObject::GetSubmapOn(GWorld* pWorld)
{
	if ( m_SubMapOn == -1 )
		return NULL;

	if ( !pWorld->m_pMap )
		return NULL;

	return pWorld->m_pMap->m_SubMaps[ m_SubMapOn ];
}


GBounds& GGameObject::GetBounds()
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

/*static*/Bool GGameObject::CheckIntersection(GGameObject* pObjA, GGameObject* pObjB)
{
	GBounds& BoundsA = pObjA->GetBounds();
	GBounds& BoundsB = pObjB->GetBounds();

	GIncCounter(GameObjIntersectionTest,1);

	//	check intersection
	return ( BoundsA.Intersects( pObjA->m_Position, BoundsB, pObjB->m_Position ) );

}







//------------------------------------------------


GWorldRender::GWorldRender()
{
	m_pCamera = NULL;
	m_BasePortal = 0x0;
}


GWorldRender::~GWorldRender()
{

}

void GWorldRender::Reset()
{
	//	reset all temporary vars
	m_MapObjects.Empty();
	m_GameObjects.Empty();
	m_Portals.Empty();
	m_SubMaps.Empty();
}


void GWorldRender::Build(GWorld& World, GCamera* pCamera, u32 BasePortal)
{
	//	build a new render through this camera
	if ( !pCamera )
		return;

	//	reset world render
	Reset();

	//	set camera pointer
	m_pCamera = pCamera;
	m_pCamera->m_WorldUp = World.m_WorldUp;

	//	start on the map the camera is on, or nearest to
	int CameraOnSubMap = World.SubmapOn( pCamera->m_Position );

	//	camera not actually on a submap, find the nearest one
	if ( CameraOnSubMap == -1 )
		CameraOnSubMap = World.SubmapNearest( pCamera->m_Position );

	//	camera still isnt on a submap, render all our objects, but no maps
	if ( CameraOnSubMap == -1 )
	{
		for ( int o=0;	o<World.m_ObjectList.Size();	o++ )
		{
			//	get the object
			GGameObject* pGameObject = World.m_ObjectList[o];
			if ( !pGameObject )
			{
				GDebug_Break("Gameobject in world list is NULL\n");
				continue;
			}
			
			GBounds& Bounds = pGameObject->GetBounds();

			//	check culling
			if ( Bounds.IsCulled( pGameObject->m_Position, pCamera ) )
			{
				continue;
			}

			//	we will draw this object
			m_GameObjects.Add( (0xffff<<16) | o );
		}

		return;
	}

	//	set base portal
	m_BasePortal = BasePortal;

	//	build frustum culling planes on camera
	m_pCamera->CalcFrustumPlanes();

	//	start rendering from this submap (all portals)
	BuildThroughSubmap( World, CameraOnSubMap, 0xffff, m_pCamera );

	//	build world renders for our fancy portals (mirrors etc)
	for ( int i=0;	i<m_Portals.Size();	i++ )
	{
		int SubMap = ( m_Portals[i]>>16 ) & 0xffff;
		int PortalIndex = m_Portals[i] & 0xffff;

		GMapPortal& Portal = World.m_pMap->m_SubMaps[SubMap]->m_Portals[PortalIndex];

		//	build world render for mirrors
		if ( Portal.m_Type != GPortal_Mirror )
			continue;

		//	dont process the base portal if this is it
		if ( m_BasePortal == m_Portals[i] )
			continue;
	
		//	make up camera from this portal (for portal culling)
		GCamera PortalCamera;
		Portal.MakeCamera(PortalCamera,pCamera);

		//	make world render for this camera
		GWorldRender PortalWorldRender;
		PortalWorldRender.Build( World, &PortalCamera, m_Portals[i] );

		//	render and capture to portal's texture
		PortalWorldRender.Draw( World, 0x0 );
		PortalCamera.CaptureTexture( Portal.m_Texture );
	}		

	//	finished building
}


void GWorldRender::BuildThroughSubmap(GWorld& World, int Submap, u16 PortalBit, GCamera* pCamera)
{
	//	check params
	if ( Submap == -1 )
	{
		GDebug_Break("Trying to build invalid submap\n");
		return;
	}

	//	have we already processed this submap through this portal?
	if ( FindBitsInList( m_SubMaps, (u16)Submap, PortalBit ) )
		return;

	int i;
	GSubMap* pSubMap = World.m_pMap->m_SubMaps[Submap];
	if ( !pSubMap )
	{
		GDebug::Print("Trying to build missing submap %s\n", GAsset::RefToName( World.m_pMap->m_SubMaps[Submap]->m_AssetRef ) );
		return;
	}

	//	build list of mapobjects being drawn
	GList<GPreDrawResult> MapObjectPreDrawResults;
	pSubMap->PreDrawMapObjects( pCamera, m_MapObjects, MapObjectPreDrawResults, Submap );

	//	build list of game objects being drawn
	pSubMap->PreDrawGameObjects( pCamera, World, m_GameObjects, MapObjectPreDrawResults, Submap );

	//	build list of visible portals
	GList<u32> VisiblePortals;
	pSubMap->GetVisiblePortals( pCamera, VisiblePortals, Submap );

	//	add visible portals to our list
	m_Portals.Add( VisiblePortals );

	//	build through submaps that are visible in our list
	for ( i=0;	i<VisiblePortals.Size();	i++ )
	{
		u16 PortalIndex = VisiblePortals[i] & 0xffff;

		//	if its a regular map-map portal, build through it
		GMapPortal& Portal = pSubMap->m_Portals[PortalIndex];
		if ( Portal.m_Type == GPortal_Normal )
		{
			//	get index of submap on the other side of the portal
			int OtherSubmapIndex = World.m_pMap->GetSubmapIndex( Portal.m_OtherSubmap );
			GSubMap* pOtherSubMap = World.m_pMap->m_SubMaps[OtherSubmapIndex];

			//	get the portal's index on the other submap
			int OtherPortalIndex = pOtherSubMap->GetPortalIndex( Portal.m_OtherPortal );

			//	no such portal on the other submap
			if ( OtherPortalIndex == -1 )
				continue;

			//	make up camera from this portal (for portal culling)
			GCamera PortalCamera;
			Portal.MakeCamera(PortalCamera,pCamera);

			BuildThroughSubmap( World, OtherSubmapIndex, 1<<OtherPortalIndex, &PortalCamera );
		}
	}

}


Bool GWorldRender::FindIndexInList(GList<u32>& List, u16 Submap, u16 Index)		
{	
	u32 Match = Index;
	Match |= ((u32)Submap) << 16;
	return List.Exists( Match );
}

Bool GWorldRender::FindBitsInList(GList<u32>& List, u16 Submap, u16 Bits)
{
	//	need to AND with bits
	for ( int i=0;	i<List.Size();	i++ )
	{
		u16 ListSubmap = ( List[i] >> 16 ) & 0xffff;
		u16 ListBits = List[i] & 0xffff;

		if ( ListSubmap == Submap )
			if ( ListBits & Bits )
				return TRUE;
	}

	return FALSE;
}


void GWorldRender::Draw(GWorld& World, u32 DrawFlags)
{
	//	add "do not test" flags to save cpu time as we've already checked culling etc etc
	DrawFlags |= GDrawInfoFlags::DontCullTest | GDrawInfoFlags::DontAutoInsideCull;

	//	setup camera
	m_pCamera->SetupView( FALSE, World.m_pSkyBox );

	//	
	int i;

	//	list of objects to render shadows for
	GList<GMapObject*>	ShadowMapObjects;
	GList<GMapLight*>	ShadowMapObjects_Lights;
	GList<GGameObject*>	ShadowGameObjects;
	GList<GMapLight*>	ShadowGameObjects_Lights;

	//	
	World.PreDrawMapObjects( m_MapObjects );

	//	render map objects
	for ( i=0;	i<m_MapObjects.Size();	i++ )
	{
		u32 Submap = (m_MapObjects[i]>>16) & 0xffff;
		u32 MapObjectIndex = m_MapObjects[i] & 0xffff;

		GMapObject* pMapObject = GAssets::g_MapObjects.Find( World.m_pMap->m_SubMaps[Submap]->m_MapObjects[MapObjectIndex] );
		pMapObject->Draw( DrawFlags );

		//	do we draw a shadow for this mapobject
		if ( ( pMapObject->m_Flags & GMapObjectFlags::DontCastShadow ) == 0x0 )
			ShadowMapObjects.Add( pMapObject );

		ShadowMapObjects_Lights.Add( World.m_pMap->GetLight( pMapObject->m_Position, Submap) );
	}

	//
	World.PostDrawMapObjects( m_MapObjects );

	//	
	World.PreDrawGameObjects( m_GameObjects );

	//	render game objects
	for ( i=0;	i<m_GameObjects.Size();	i++ )
	{
		u32 Submap = (m_GameObjects[i]>>16) & 0xffff;
		u32 GameObjectIndex = m_GameObjects[i] & 0xffff;

		GGameObject* pGameObject = NULL;

		if ( Submap == 0xffff )
			pGameObject = World.m_ObjectList[GameObjectIndex];
		else
			pGameObject = World.m_pSubmapObjectList[Submap][GameObjectIndex];
		
		pGameObject->Draw( DrawFlags );

		//	do we draw a shadow for this GameObject
		//if ( ( pGameObject->m_Flags & GGameObjectFlags::DontCastShadow ) == 0x0 )
			ShadowGameObjects.Add( pGameObject );
		
		ShadowGameObjects_Lights.Add( World.GetLight( pGameObject->m_Position, (int)((s16)Submap)) );
	}

	//	
	World.PostDrawGameObjects( m_GameObjects );

	//	render portals
	if ( DrawFlags & GDrawInfoFlags::DebugPortals )
	{
		for ( i=0;	i<m_Portals.Size();	i++ )
		{
			u32 Submap = (m_Portals[i]>>16) & 0xffff;
			u32 PortalIndex = m_Portals[i] & 0xffff;

			GMapPortal* pPortal = &World.m_pMap->m_SubMaps[Submap]->m_Portals[PortalIndex];

			//	draw lines indicating where the portal is
			pPortal->DebugDraw();
		}
	}

	/*
	//	render accumulated shadows
	if ( ( DrawFlags & GDrawInfoFlags::DisableMapObjShadows ) == 0x0 )
	{
		for ( i=0;	i<ShadowMapObjects.Size();	i++ )
		{
			ShadowMapObjects[i]->DrawShadow( DrawFlags, ShadowMapObjects_Lights[i] );
		}
	}

	if ( ( DrawFlags & GDrawInfoFlags::DisableGameObjShadows ) == 0x0 )
	{
		for ( i=0;	i<ShadowGameObjects.Size();	i++ )
		{
			//ShadowMapObjects[i]->DrawShadow( DrawFlags, ShadowGameObjects_Lights[i] );
			ShadowGameObjects[i]->DrawShadow();
		}
	}
	*/
	
	//	draw any remainding debug items
	g_Display->DrawDebugItems();
}


//------------------------------------------------



GWorld::GWorld()
{
	m_pMap				= NULL;
	m_pSubmapObjectList	= NULL;
	m_WorldUp			= float3(0,1,0);
	m_pSkyBox			= NULL;
}


GWorld::~GWorld()
{
}


void GWorld::SetMap(GMap* pMap)
{
	//	changing map
	m_pMap = pMap;

	//	delete objects etc
	if ( m_pMap == NULL )
	{
		GDeleteArray( m_pSubmapObjectList );
	}

	//	create new submap object lists
	if ( m_pMap )
	{
		m_pSubmapObjectList = new GGameObjectList[ m_pMap->m_SubMaps.Size() ];
	}		

}



Bool GWorld::RemoveObjectFromSubmapList( GGameObject* pObject, int SubMapIndex )
{
	//	invalid submap index
	if ( SubMapIndex >= m_pMap->m_SubMaps.Size() )
	{
		GDebug_Break("Invalid submap index\n");
		return FALSE;
	}

	//	check submap index
	if ( SubMapIndex < 0 )
		return FALSE;

	//	remove from list
	if ( m_pSubmapObjectList )
	{
		int Index = m_pSubmapObjectList[SubMapIndex].FindIndex( pObject );
		m_pSubmapObjectList[SubMapIndex].RemoveAt( Index );
	}		

	return TRUE;
}




Bool GWorld::AddObjectToSubmapList( GGameObject* pObject, int SubMapIndex )
{
	//	invalid submap index
	if ( SubMapIndex >= m_pMap->m_SubMaps.Size() )
	{
		GDebug_Break("Invalid submap index\n");
		return FALSE;
	}

	//	check submap index
	if ( SubMapIndex < 0 )
		return FALSE;

	//	add to list
	if ( m_pSubmapObjectList )
	{
		m_pSubmapObjectList[SubMapIndex].Add( pObject );
	}		

	return TRUE;
}




void GWorld::Update()
{
	int i;
	GPhysicsObject* pPhysics;

	//	do phsyics' pre-update
	for ( i=0;	i<m_ObjectList.Size();	i++ )
	{
		pPhysics = m_ObjectList[i]->Physics();
		if ( pPhysics )
		{
			pPhysics->PreUpdate(this);
			pPhysics->m_CollisionTestCases.Empty();
		}
	}


	//	do objects update
	for ( i=0;	i<m_ObjectList.Size();	i++ )
	{
		m_ObjectList[i]->Update();

		//	update shader
		if ( m_ObjectList[i]->Shader() )
		{
			m_ObjectList[i]->Shader()->Update();
		}
	}

	//	do objects collisions
	if ( m_pMap )
	{
		GList<GPhysicsObject*> PhysicsObjects;

		//	gather test cases for multiple iterations
		GatherPhysicsTestCases( PhysicsObjects );

		//	todo: currently just map objects
		//	check each physics collision test

		GList<int> RemovePhysicsObjectIndexes;
		int Iteration = 0;

		while ( PhysicsObjects.Size() > 0 )
		{
			for ( int po=0;	po<PhysicsObjects.Size();	po++ )
			{
				GPhysicsObject* pPhysics = PhysicsObjects[po];

				//	check each map collision test
				for ( int tc=0;	tc<pPhysics->m_CollisionTestCases.Size();	tc++ )
				{
					GMeshTestRef& TestRef = pPhysics->m_CollisionTestCases[tc];

					float3 Movement = ( pPhysics->m_Velocity + pPhysics->m_Force ) * g_App->FrameDelta();
										
					//if ( Movement.LengthSq() < NEAR_ZERO )
					//	continue;

					float3 Position = pPhysics->GetPosition();

					//	todo: check bounds

					//	raycast to mesh, meshpos is relative to mesh
					pPhysics->CheckMeshCollision( TestRef.pMesh, TestRef.Pos, Position, Movement );
				}


				//	check inter-object collision
				//	todo: reduce to objects in this submap
				for ( int obj=0;	obj<PhysicsObjects.Size();	obj++ )
				{
					if ( obj == po )
						continue;

					if ( GGameObject::CheckIntersection( pPhysics->m_pOwner, PhysicsObjects[obj]->m_pOwner ) )
					{
						GPhysicsObject::ProcessCollision( pPhysics, PhysicsObjects[obj] );
					}
				}
		

				//	call overloaded post iteration func				
				pPhysics->PostIteration();

				//	no more iterations for this object? remove from list
				if ( Iteration+1 >= pPhysics->CollisionIterations() )
					RemovePhysicsObjectIndexes.Add( po );
			}

			//	remove entries from list
			for ( int i=RemovePhysicsObjectIndexes.Size()-1;	i>=0;	i-- )
			{
				PhysicsObjects.RemoveAt( RemovePhysicsObjectIndexes[i] );
			}
			RemovePhysicsObjectIndexes.Empty();

			//	next iteration
			Iteration++;
		}
	}

	//	do physics post update
	for ( i=0;	i<m_ObjectList.Size();	i++ )
	{
		pPhysics = m_ObjectList[i]->Physics();
		if ( pPhysics )
			pPhysics->PostUpdate(this);
		
		m_ObjectList[i]->UpdateSubMapOn( this );
	}

}

//-------------------------------------------------------------------------
//	gather the map's mesh test cases for the physics objects. this is so
//	we dont have to recalc which meshes we want to test for multiple iterations
//-------------------------------------------------------------------------
void GWorld::GatherPhysicsTestCases(GList<GPhysicsObject*>& PhysicsObjects)
{
	for ( int sm=0;	sm<m_pMap->m_SubMaps.Size();	sm++ )
	{
		GSubMap* pSubMap = m_pMap->m_SubMaps[sm];
		GGameObjectList& SubMapObjList = m_pSubmapObjectList[sm];
		int SubMapObjCount = SubMapObjList.Size();

		if ( !SubMapObjCount )
			continue;
		
		//	check each mapobject
		for ( int mo=0;	mo<pSubMap->m_MapObjects.Size();	mo++)
		{
			GMapObject* pMapObject = GAssets::g_MapObjects.Find( pSubMap->m_MapObjects[mo] );
			if ( !pMapObject )
				continue;

			//	todo: check bounds intersection

			//	todo: try gettings collision mesh first

			GMesh* pMapObjectMesh = pMapObject->GetMesh();
			if ( !pMapObjectMesh )
				continue;

			float3 MeshPos = pMapObject->m_Position;

			GMeshTestRef TestRef;
			TestRef.pMesh = pMapObjectMesh;
			TestRef.Pos = MeshPos;

			for ( int go=0;	go<SubMapObjCount;	go++ )
			{
				GGameObject* pGameObject = SubMapObjList.ElementAt(go);
				GPhysicsObject* pPhysics = pGameObject->Physics();
				if ( !pPhysics )
					continue;
				
				//	todo: check some stuff here

				
				//	todo: this will be slow! sort pointer addresses?

				//	add to list if its not already in there 
				if ( !PhysicsObjects.Exists( pPhysics ) )
				{
					PhysicsObjects.Add( pPhysics );
					PhysicsObjects.Sort();
				}

				pPhysics->m_CollisionTestCases.Add( TestRef );
			}
		}
	}

}


Bool GWorld::AddObject(GGameObject* pObject)
{
	//	check the object hasnt already been added
	if ( m_ObjectList.Exists( pObject ) )
	{
		GDebug_Break("Object already exists in world\n");
		return FALSE;
	}

	//	add to world and update submap reference
	m_ObjectList.Add( pObject );
	pObject->UpdateSubMapOn(this);

	return TRUE;
}


void GWorld::Draw(GCamera& Camera, u32 DrawFlags)
{
	//	create a world render to put everything into
	GWorldRender WorldRender;

	//	setup world render
	WorldRender.Build( *this, &Camera );

	//	draw world render
	WorldRender.Draw( *this, DrawFlags );
}




