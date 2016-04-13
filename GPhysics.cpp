/*------------------------------------------------

  GPhysics.cpp

	Base physics code


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GPhysics.h"
#include "GApp.h"
#include "GAssetList.h"
#include "GMesh.h"


//	globals
//------------------------------------------------
//float3		g_GravityVector( 0.f, -g_GravityMetresSec*g_WorldToMetres, 0.f );

//#define DEBUG_PHYSICS
#define DEBUG_PHYSICS_COLLISION_POINT

GDeclareCounter(PhysicsCollisionTest);
GDeclareCounter(PhysicsCollisionSuccess);


//	Definitions
//------------------------------------------------



GPhysicsObject::GPhysicsObject()
{
	m_Velocity		= float3(0,0,0);
	m_Force			= float3(0,0,0);
	m_Friction		= 1.0f;
	m_Mass			= 1.0f;
	m_Bounce		= 1.0f;
	m_LastFloorNormal	= float3(0,0,0);
	m_LastFloorFriction	= 0.f;
	m_pOwner		= NULL;
	m_PhysicsFlags	= 0x0;

}

GPhysicsObject::~GPhysicsObject()
{
}

//-------------------------------------------------------------------------
//	before collisions are tested. (add physics specific forces)
//-------------------------------------------------------------------------
void GPhysicsObject::PreUpdate(GWorld* pWorld)
{
	if ( ( m_PhysicsFlags & GPhysicsFlags::DontAddGravity ) == 0x0 )
	{
		//	add gravity
		m_GravityForce = float3(0,g_GravityMetresSec,0);
		if ( pWorld )
			m_GravityForce = pWorld->m_WorldUp * g_GravityMetresSec;
		m_GravityForce *= -g_WorldToMetres;

		m_GravityForce /= 1.f / FIXED_FRAME_RATEF;
		m_GravityForce *= g_App->FrameDelta();
		m_Force += m_GravityForce;// * m_Mass;
	}

	//	reset floor normal
	m_LastFloorNormal = float3(0,0,0);
	m_LastFloorFriction = 1.f;
}

//-------------------------------------------------------------------------
//	after all collisions have been tested.
//	updates movement/rolling etc
//-------------------------------------------------------------------------
void GPhysicsObject::PostUpdate(GWorld* pWorld)
{
	//GDebug::Print("Velocity(%3.3f,%3.3f,%3.3f) Force(%3.3f,%3.3f,%3.3f) \n", m_Velocity.x, m_Velocity.y, m_Velocity.z, m_Force.x, m_Force.y, m_Force.z );

	//	update physics movement
	m_Velocity += m_Force;
	m_Force.Set( 0.f, 0.f, 0.f );
	
	//	move pos
	m_pOwner->m_Position += (m_Velocity * g_App->FrameDelta());

	//	stick object to its floor
	if ( m_PhysicsFlags & GPhysicsFlags::StickToFloor )
	{
//		m_Velocity = float3(0,0,0);
	}

	//	reduce velocity
	m_Velocity *= 1.f - ( m_Friction * g_App->FrameDelta() * (1.f-m_LastFloorFriction) );
	m_Force = float3(0,0,0);

	//	reset delta movement
	m_DeltaMovement = float3(0,0,0);

	GDebug::CheckFloat( m_Velocity.x );
	GDebug::CheckFloat( m_Velocity.y );
	GDebug::CheckFloat( m_Velocity.z );
}


/*static*/void GPhysicsObject::ProcessCollision(GPhysicsObject* pObjA, GPhysicsObject* pObjB)
{
	GIncCounter(PhysicsCollisionTest,1);

	float3 Dist = (pObjB->m_pOwner->m_Position - pObjA->m_pOwner->m_Position); 
	float DistDot2 = Dist.DotProduct(Dist);  

	// balls are too embedded to be of any use    
	if ( DistDot2 < NEAR_ZERO )     
		return;   
	
	Dist /= sqrtf(DistDot2);  
	
	// relative velocity of ball2 to ball1   
	float3 V = pObjB->AccumulatedMovement() - pObjA->AccumulatedMovement();
    float VdotN = V.DotProduct(Dist);        

	// balls are separating, no need to add impulse 
	// else make them bounce against each other.  
	// there is a small threshold value in case they are moving 
    // very very slowly towards each other.    
	if ( VdotN >= -NEAR_ZERO ) 
		return;

	GIncCounter(PhysicsCollisionSuccess,1);

	pObjA->DoCollision( pObjB, Dist, VdotN );
	pObjB->DoCollision( pObjA, Dist, -VdotN );

}


void GPhysicsObject::DoCollision(GPhysicsObject* pObject, float3& Dist, float VdotN)
{
	// calculate the amount of impulse
	float BounceFactor = m_Bounce + pObject->m_Bounce;
	float htotal = (-BounceFactor * VdotN) / (m_Mass + pObject->m_Mass); 

	//	hit strength relative to mass of other object
	float h1 = htotal * pObject->m_Mass;    
	
	//	apply power to velocity rather than force
	m_Velocity -= Dist * h1;
}


void GPhysicsObject::ProcessCollision(GWorld& World)
{
	GMap* pMap = World.m_pMap;
	if ( !pMap )
		return;

	//	get the submap we're on
	int SubMapIndex = pMap->SubmapOn( m_pOwner->m_Position );
	GSubMap* pSubMap = (SubMapIndex==-1) ? NULL : pMap->m_SubMaps[SubMapIndex];

	//	not on submap
	if ( !pSubMap )
		return;

	//	get current movement
	float3 Movement = AccumulatedMovement() * g_App->FrameDelta();

	//	check each mapobject
	for ( int mo=0;	mo<pSubMap->m_MapObjects.Size();	mo++)
	{
		GMapObject* pMapObject = GAssets::g_MapObjects.Find( pSubMap->m_MapObjects[mo] );
		if ( !pMapObject )
			continue;

		//	todo: check bounds intersection

		GMesh* pMesh = pMapObject->GetMesh();
		if ( !pMesh )
			continue;

		//	todo: incorporate rotation
		float3 MeshPos = pMapObject->m_Position;

		//	todo: check bounds

		//	raycast to mesh, meshpos is relative to mesh
		float3 RayFrom = (MeshPos*-1.f) + (m_pOwner->m_Position);
		CheckMeshCollision( pMesh, MeshPos, RayFrom, Movement );
	}
}


void GPhysicsObject::CheckMeshCollision(GMesh* pMesh, float3& MeshPos, float3& From, float3& Dir)
{
	/*
	//	paramter check
	if ( Dir.LengthSq() < NEAR_ZERO )
	{
//		GDebug::Break("Raycast length too small\n");
		return;
	}
	*/

	//	todo: make better this!

	//	check each triangle
	GList<GTriangle>& TriangleList = pMesh->m_Triangles;
	int t,i;

	//	debug check pre-calculcated planes
	if ( pMesh->m_TrianglePlanes.Size() < TriangleList.Size() )
	{
		GDebug_Print("Mesh is missing triangle planes\n");
		return;
	}

	for ( t=0;	t<TriangleList.Size();	t++ )
	{
		float3& v1 = pMesh->m_Verts[TriangleList[t][0]];
		float3& v2 = pMesh->m_Verts[TriangleList[t][1]];
		float3& v3 = pMesh->m_Verts[TriangleList[t][2]];

		GPlane& Plane = pMesh->m_TrianglePlanes[t];

		CheckTriangleCollision( MeshPos, Plane, v1, v2, v3, From, Dir );
	}

	//	debug check pre-calculcated planes
	if ( pMesh->m_TriStripPlanes.Size() < pMesh->m_TriStrips.Size() )
	{
		GDebug_Print("Mesh is missing tristrip planes\n");
		return;
	}
	
	//	check tristrips
	for ( t=0;	t<pMesh->m_TriStrips.Size();	t++ )
	{
		GTriStrip& TriStrip = pMesh->m_TriStrips[t];
		GTriangle Triangle;

		GPlaneList& PlaneList = pMesh->m_TriStripPlanes[t];

		//	add first 2 bits of triangle
		Triangle[0] = TriStrip.m_Indicies[0];
		Triangle[1] = TriStrip.m_Indicies[1];
	
		if ( PlaneList.Size() < TriStrip.m_Indicies.Size()-2 )
		{
			GDebug_Print("Mesh is missing tristrip planes\n");
			continue;
		}

		//	replace the next triangle element in sequence along the tristrip
		for ( i=2;	i<TriStrip.m_Indicies.Size();	i++ )
		{
			int im = i%3;
			Triangle[ im ] = TriStrip.m_Indicies[i];

			float3 v1 = pMesh->m_Verts[Triangle[0]];
			float3 v2 = pMesh->m_Verts[Triangle[1]];
			float3 v3 = pMesh->m_Verts[Triangle[2]];
			
			GPlane& Plane = PlaneList[i-2];

			CheckTriangleCollision( MeshPos, Plane, v1, v2, v3, From, Dir );
		}
	}


}


void GPhysicsObject::CheckTriangleCollision(float3& MeshPos, GPlane& Plane, float3& v1, float3& v2, float3& v3, float3& From, float3& Dir )
{
	//	get the "triangle radius" (todo: store this permanantly)
/*	float TriRadSq = 0.f;
	float v12Len = (v1 - v2).LengthSq();
	float v13Len = (v1 - v3).LengthSq();
	float v23Len = (v2 - v3).LengthSq();

	if ( v12Len > v13Len )
		TriRadSq = ( v12Len > v23Len ? v12Len : v23Len ) * 0.5f;
	else
		TriRadSq = ( v13Len > v23Len ? v13Len : v23Len ) * 0.5f;

	float3 TriCenter = ( v1 + v2 + v3 ) * 0.33333f;
	float DistToCenterSq = (From-TriCenter).LengthSq();

	//	distance from the triangle center to the test pos is greater than the "radius" of the triangle
	//	means too far away to collide
	if ( DistToCenterSq > TriRadSq )
		return;
*/
	//	get normal
	float3 TriNormal( Plane.Normal() );
	TriNormal.Normalise();
/*
	//	is raycast facing triangle?
	if ( Dir.DotProduct( TriNormal ) < 0.f )
	{
		//	do we collide both sides?
		if ( TRUE )
		{
		//	TriNormal.Invert();
		}
		else
		{
			return;
		}
	}
*/
	DoIntersection( From, Dir, MeshPos, Plane, TriNormal, v1, v2, v3 );
}



void GPhysicsObject::DoIntersection(float3& From, float3& Dir, float3& MeshPos, GPlane& TrianglePlane, float3& TriangleNormal, float3& TriangleV1, float3& TriangleV2, float3& TriangleV3)
{
}








GPhysicsSphere::GPhysicsSphere()
{
	m_SphereRadius	= 1.f;
	m_LastRollAxis	= float3(0,1,0);
	m_LastRoll		= 0.f;
	m_SphereOffset	= float3(0,0,0);
}

GPhysicsSphere::~GPhysicsSphere()
{
}

void GPhysicsSphere::PostUpdate(GWorld* pWorld)
{
	//	do base update
	GPhysicsObject::PostUpdate(pWorld);

	//	roll sphere
	float3 Movement( m_Velocity * g_App->FrameDelta() );
	float MovementLenSq = Movement.LengthSq();

	Bool DoRotation = (MovementLenSq > NEAR_ZERO);

	if ( m_PhysicsFlags	& GPhysicsFlags::DontRotate )
		DoRotation = FALSE;

	//	if there is any movement, apply
	if ( DoRotation )
	{
		float3 Axis = m_LastRollAxis;
		float Roll = 0.f;

		float3 FloorMovement( Movement );

		//	get floor drag/friction/collision from floor collision
		float3 FloorDrag( 1.f-m_LastFloorNormal.x, 1.f-m_LastFloorNormal.y, 1.f-m_LastFloorNormal.z );
		FloorMovement *= FloorDrag;

		float FloorMovementLenSq = FloorMovement.LengthSq();

		//	if we're on a surface, we must roll with our movement
		if ( m_LastFloorNormal.LengthSq() != 0.f && FloorMovementLenSq != 0.f )
		{
			//	roll in the direction of the movement
			Roll += sqrtf( FloorMovementLenSq ) * 2.f;	//	div radius of sphere?
			Axis = FloorMovement.Normal();
			Axis = Axis.CrossProduct( float3(0,1,0) );
		}
		else
		{
			Roll = m_LastRoll;
		}

		#ifdef DEBUG_PHYSICS
			GDisplay::DebugLine( m_pOwner->m_Position, float4(1,1,1,1), m_pOwner->m_Position + Axis );
		#endif

		GQuaternion RollQuat( Axis, Roll );
		RollQuat.Normalise();

		m_LastRoll = Roll;
		m_LastRollAxis = Axis;
		
		//	get new rotation
		if ( RollQuat.IsValid() )
		{
			if ( m_pOwner->m_Rotation.IsValid() )
				m_pOwner->m_Rotation *= RollQuat;
			else
				m_pOwner->m_Rotation = RollQuat;

			m_pOwner->m_Rotation.Normalise();
		}
	
	}
	
}



void GPhysicsSphere::DoIntersection(float3& From, float3& Dir, float3& MeshPos, GPlane& TrianglePlane, float3& TriangleNormal, float3& TriangleV1, float3& TriangleV2, float3& TriangleV3)
{
//	#define DRAW_FAILED
	#define DRAW_SUCCEED
	float4 DebugTriangleColourFailed(1,1,0,1);
	float4 DebugTriangleColourSucceed(1,0,0,1);

	float3 PosLocalToTriangle = From - MeshPos;

	//	get intersection
	float3 IntersectionFrom = PosLocalToTriangle;
	float3 IntersectionDir = TriangleNormal * -m_SphereRadius;
	float IntersectLength;

	//	draw dir
	#ifdef DEBUG_PHYSICS
		//if ( DrawInfo.Flags & GDrawInfoFlags::DebugPhysicsCollisions )
			GDisplay::DebugLine( IntersectionFrom, float4(1,1,1,1), (IntersectionFrom+IntersectionDir) );
	#endif

	//	check intersection point
	if (! TrianglePlane.Intersection( IntersectLength, PosLocalToTriangle, IntersectionDir ) )
	{
		#ifdef DEBUG_PHYSICS_COLLISION_POINT
			//if ( DrawInfo.Flags & GDrawInfoFlags::DebugPhysicsCollisions )
			#ifdef DRAW_FAILED
				//	draw failed triangle
				GDisplay::DebugLine( TriangleV1 + MeshPos, DebugTriangleColourFailed, TriangleV2 + MeshPos );
				GDisplay::DebugLine( TriangleV2 + MeshPos, DebugTriangleColourFailed, TriangleV3 + MeshPos );
				GDisplay::DebugLine( TriangleV3 + MeshPos, DebugTriangleColourFailed, TriangleV1 + MeshPos );
			#endif
		#endif
		return;
	}

	//	intersection too far away
	if ( IntersectLength<0.f || IntersectLength>1.f )
	{
		#ifdef DEBUG_PHYSICS_COLLISION_POINT
			//if ( DrawInfo.Flags & GDrawInfoFlags::DebugPhysicsCollisions )
			#ifdef DRAW_FAILED
				//	draw failed triangle
				GDisplay::DebugLine( TriangleV1 + MeshPos, DebugTriangleColourFailed, TriangleV2 + MeshPos );
				GDisplay::DebugLine( TriangleV2 + MeshPos, DebugTriangleColourFailed, TriangleV3 + MeshPos );
				GDisplay::DebugLine( TriangleV3 + MeshPos, DebugTriangleColourFailed, TriangleV1 + MeshPos );
			#endif
		#endif
		return;
	}

	//	get the intersection point
	float3 IntersectionPoint = PosLocalToTriangle;
	if ( IntersectLength == 1.f )
		IntersectionPoint += IntersectionDir;
	else if ( IntersectLength > 0.f )
		IntersectionPoint += IntersectionDir * IntersectLength;

	//	check point is actually on the triangle and not just the plane
	if ( !PointInsideTriangle( IntersectionPoint, TriangleV1, TriangleV2, TriangleV3, TrianglePlane ) )
	{
		#ifdef DEBUG_PHYSICS_COLLISION_POINT
			//if ( DrawInfo.Flags & GDrawInfoFlags::DebugPhysicsCollisions )
			#ifdef DRAW_FAILED
				//	draw failed triangle
				GDisplay::DebugLine( TriangleV1 + MeshPos, DebugTriangleColourFailed, TriangleV2 + MeshPos );
				GDisplay::DebugLine( TriangleV2 + MeshPos, DebugTriangleColourFailed, TriangleV3 + MeshPos );
				GDisplay::DebugLine( TriangleV3 + MeshPos, DebugTriangleColourFailed, TriangleV1 + MeshPos );
			#endif
		#endif
		return;
	}

	#ifdef DEBUG_PHYSICS_COLLISION_POINT
		//if ( DrawInfo.Flags & GDrawInfoFlags::DebugPhysicsCollisions )
		#ifdef DRAW_SUCCEED
			//	draw succeeded triangle
			GDisplay::DebugLine( TriangleV1 + MeshPos, DebugTriangleColourSucceed, TriangleV2 + MeshPos );
			GDisplay::DebugLine( TriangleV2 + MeshPos, DebugTriangleColourSucceed, TriangleV3 + MeshPos );
			GDisplay::DebugLine( TriangleV3 + MeshPos, DebugTriangleColourSucceed, TriangleV1 + MeshPos );
		#endif
	#endif



	//	react
	float3 Dist( IntersectionPoint - From );	//	dir to intersection

	float DistDot2 = Dist.DotProduct(Dist);  

	// balls are too embedded to be of any use    
    if ( DistDot2 < NEAR_ZERO )     
		return;
	
	float n = sqrtf(DistDot2);  
	Dist /= n;  

	float3 FloorReflectedDir = Dir;
	FloorReflectedDir.Reflect( TriangleNormal );
	float3 FloorReflectedVelocity = m_Velocity;
	FloorReflectedVelocity.Reflect( TriangleNormal );

	//	need a physics object to collide with
	GPhysicsObject WallPhysicsObject;
	WallPhysicsObject.m_Bounce = 0.5f;
	WallPhysicsObject.m_Mass = 1.f;
	WallPhysicsObject.m_Friction = 0.4f;
	WallPhysicsObject.m_Velocity = float3( FloorReflectedVelocity*(1.f-WallPhysicsObject.m_Friction) );
	WallPhysicsObject.m_Force = float3( FloorReflectedDir*(1.f-WallPhysicsObject.m_Friction) );//(0,0,0);



	// relative velocity of ball2 to ball1   
	float3 V = WallPhysicsObject.AccumulatedMovement() - AccumulatedMovement();
    float VdotN = V.DotProduct(Dist);        

	if (VdotN < -NEAR_ZERO)
	{
		DoCollision( &WallPhysicsObject, Dist, VdotN );
	}      

	float     d  = Dist.Length() - n;

	// relative amount of displacement to make the ball touch         
	float ratio1 = (WallPhysicsObject.m_Mass) / (m_Mass + WallPhysicsObject.m_Mass);   
	float ratio2 = 1.0f - ratio1;    

	// move the balls to theire ideal positon.  
	m_pOwner->m_Position -= Dist * d * ratio1;  

	
	if ( m_LastFloorNormal.LengthSq() == 0.f )
	{
		m_LastFloorNormal = TriangleNormal;
	}
	else
	{
		m_LastFloorNormal += TriangleNormal;
		m_LastFloorNormal.Normalise();
	}

}

//--------------------------------------------------------------------------------------------------------
// debug draw collision sphere
//--------------------------------------------------------------------------------------------------------
Bool GPhysicsSphere::PreDraw(GMesh* pMesh, GDrawInfo& DrawInfo)
{
	if ( DrawInfo.Flags & GDrawInfoFlags::DebugPhysicsShapes )
	{
		float3 Pos = DrawInfo.WorldPos + m_SphereOffset;
		g_Display->DebugSphere( Pos, float4(1,1,1,1), m_SphereRadius );
	}

	return TRUE;
}

