/*------------------------------------------------

  Base physics



-------------------------------------------------*/

#ifndef __GPHYSICS__H_
#define __GPHYSICS__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GWorld.h"



//	Macros
//------------------------------------------------
const float		g_GravityMetresSec	= 9.81f;	//	gravity in metres per second
const float		g_WorldToMetres		= 0.05f;	//	100.f is 1m
//extern float3	g_GravityVector;

namespace GPhysicsFlags
{
	const u32	DontAddGravity	= 1<<0;	//	wont automaticcly add gravity force
	const u32	StickToFloor	= 1<<1;	//	stick to surfaces
	const u32	DontRotate		= 1<<2;	//	dont imply rotation from physics
	const u32	NewOwnerPos		= 1<<3;	//	don't use OwnerLastPos
};


//	Types
//------------------------------------------------
class GMesh;


//--------------------------------------------------------------------------------------------------------
// instance of a mesh to test physics against
//--------------------------------------------------------------------------------------------------------
typedef struct
{
	GMesh*		pMesh;		//	pointer to mesh object
	float3		Pos;		//	world pos of mesh
	GQuaternion	Rotation;	//	world rotation of mesh

} GMeshTestRef;

//--------------------------------------------------------------------------------------------------------
// data of a single triangle to test intersections with
//--------------------------------------------------------------------------------------------------------
typedef struct
{
	float3	Normal;		//	triangle normal
	GPlane	Plane;		//	triangle plane
	float3&	Vert0;		//	vertexes on triangle (in mesh space)
	float3& Vert1;
	float3& Vert2;

} GCollisionTestTriangle;



//-------------------------------------------------------------------------
//	base physics type
//-------------------------------------------------------------------------
class GPhysicsObject
{
public:
	float3				m_Velocity;
	float3				m_Force;
	float				m_Friction;
	float				m_Mass;
	float				m_Bounce;
	float3				m_LastFloorNormal;
	float				m_LastFloorFriction;
	GGameObject*		m_pOwner;
	u32					m_PhysicsFlags;			//	GPhysicsFlags
	GList<GMeshTestRef>	m_CollisionTestCases;
	float3				m_DeltaMovement;		//	non-physics movement

private:
	float3				m_GravityForce;			//	gravity force applied this frame

public:
	GPhysicsObject();
	~GPhysicsObject();
	
	virtual void	PreUpdate(GWorld* pWorld);	//	before collisions are processed
	virtual void	PostUpdate(GWorld* pWorld);	//	after collisions are handled
	void			ProcessCollision(GWorld& World);
	void			CheckMeshCollision(GMesh* pMesh, float3& MeshPos, float3& From, float3& To);
	virtual void	DoIntersection(float3& From, float3& Dir, float3& MeshPos, GPlane& TrianglePlane, float3& TriangleNormal, float3& TriangleV1, float3& TriangleV2, float3& TriangleV3);
	virtual void	DoCollision(GPhysicsObject* pObject, float3& Dist, float VdotN);
	virtual int		CollisionIterations()	{	return 1;	};
	virtual void	PostIteration()			{	};			//	called after each map collision iteration
	static void		ProcessCollision(GPhysicsObject* pObjA, GPhysicsObject* pObjB);
	virtual float3	GetPosition()			{	return m_pOwner ? m_pOwner->m_Position : float3(0,0,0);	};	//	return base position of physics
	virtual float3	AccumulatedMovement()	{	return m_Velocity + m_Force + m_DeltaMovement;	};
	virtual Bool	PreDraw(GMesh* pMesh, GDrawInfo& DrawInfo)		{	return TRUE;	};	//	drawing routine for physics, called just before gameobject is draw, return FALSE to cancel game object draw

private:
	void			CheckTriangleCollision(float3& MeshPos, GPlane& Plane, float3& v1, float3& v2, float3& v3, float3& From, float3& Dir );
};



//--------------------------------------------------------------------------------------------------------
// sphere collision handler for balls
//--------------------------------------------------------------------------------------------------------
class GPhysicsSphere : public GPhysicsObject
{
public:
	float			m_SphereRadius;
	float3			m_SphereOffset;		//	offset position from parent
	float3			m_LastRollAxis;		//	store last roll axis
	float			m_LastRoll;			//	continue roll

public:
	GPhysicsSphere();
	~GPhysicsSphere();
	
	virtual void	PostUpdate(GWorld* pWorld);	//	after collisions are handled
	virtual void	DoIntersection(float3& From, float3& Dir, float3& MeshPos, GPlane& TrianglePlane, float3& TriangleNormal, float3& TriangleV1, float3& TriangleV2, float3& TriangleV3);
	virtual float3	GetPosition()			{	return m_pOwner ? m_pOwner->m_Position+m_SphereOffset : m_SphereOffset;	};	//	return base position of physics
	virtual Bool	PreDraw(GMesh* pMesh, GDrawInfo& DrawInfo);
};




//	Declarations
//------------------------------------------------


//	Inline Definitions
//-------------------------------------------------





#endif

