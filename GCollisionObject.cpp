/*------------------------------------------------

  GCollisionObject.cpp

	Collision object used for culling and physics

-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GCollisionObject.h"
#include "GDisplay.h"
#include "GBinaryData.h"


//	globals
//------------------------------------------------
float4	GCollisionObj::g_DebugColour(0,1,1,1);
u32		GCollisionObj::g_Version = 0x55660001;


//	Definitions
//------------------------------------------------

GCollisionObj::GCollisionObj()
{
	m_ColType	= GCollisionObjType_None;
	m_Ref		= GAssetRef_Invalid;
	m_Offset	= float3(0,0,0);

}
	
//--------------------------------------------------------------------------------------------------------
//	checks a point being inside this (point is relative to owner)
//--------------------------------------------------------------------------------------------------------
Bool GCollisionObj::Intersection(float3& Point)
{
	if ( m_ColType == GCollisionObjType_Sphere )	
		return IntersectionSpherePoint( Point );

	if ( m_ColType == GCollisionObjType_Box )	
		return IntersectionBoxPoint( Point );

	return FALSE;
}

//--------------------------------------------------------------------------------------------------------
//	checks intersection with another collision object (ColPos is relative to owner)
//--------------------------------------------------------------------------------------------------------
Bool GCollisionObj::Intersection(GCollisionObj& ColObj,float3& ColPos)
{
	//	sphere-sphere
	if ( m_ColType == GCollisionObjType_Sphere && ColObj.m_ColType == GCollisionObjType_Sphere )	
		return IntersectionSphereSphere( ColObj, ColPos );

	//	sphere-box
	if ( m_ColType == GCollisionObjType_Sphere && ColObj.m_ColType == GCollisionObjType_Box )	
		return IntersectionSphereBox( ColObj, ColPos );

	//	box-sphere
	if ( m_ColType == GCollisionObjType_Box && ColObj.m_ColType == GCollisionObjType_Sphere )	
		return IntersectionBoxSphere( ColObj, ColPos );

	//	box-box
	if ( m_ColType == GCollisionObjType_Box && ColObj.m_ColType == GCollisionObjType_Box )	
		return IntersectionBoxBox( ColObj, ColPos );


	return FALSE;
}

//--------------------------------------------------------------------------------------------------------
//	make into a sphere collision object
//--------------------------------------------------------------------------------------------------------
void GCollisionObj::SetSphereParams(float Radius, float3& Center)
{
	//	set as sphere
	m_ColType = GCollisionObjType_Sphere;

	m_Offset = Center;
	m_SphereRadius = Radius;
}

//--------------------------------------------------------------------------------------------------------
//	make into a box collision object
//--------------------------------------------------------------------------------------------------------
void GCollisionObj::SetBoxParams(float3& Min, float3& Max, float3& Center)
{
	//	set as box
	m_ColType = GCollisionObjType_Box;

	m_Offset = Center;
	m_BoxMin = Min;
	m_BoxMax = Max;
}

//--------------------------------------------------------------------------------------------------------
//	draws sphere/box etc
//--------------------------------------------------------------------------------------------------------
void GCollisionObj::DebugDraw(float3& ParentWorldPos,float4& Colour)
{
	//	draw debug sphere
	if ( m_ColType == GCollisionObjType_Sphere )
	{
		g_Display->DebugSphere( ParentWorldPos + m_Offset, Colour, m_SphereRadius );
	}

	//	draw debug box
	if ( m_ColType == GCollisionObjType_Box )
	{
		float3 Min = m_BoxMin + ParentWorldPos + m_Offset;
		float3 Max = m_BoxMax + ParentWorldPos + m_Offset;

		float3 Corners[8];

		Corners[0] = float3( Min.x, Min.y, Min.z );	//	   1__z__2
		Corners[1] = float3( Max.x, Min.y, Min.z );	//	 x/|    /|
		Corners[2] = float3( Max.x, Min.y, Max.z );	//	0/_|__3/ |
		Corners[3] = float3( Min.x, Min.y, Max.z );	//	 | |   | |
		Corners[4] = float3( Min.x, Max.y, Min.z );	//	y| |5__|_|6
		Corners[5] = float3( Max.x, Max.y, Min.z );	//	 | /   | /
		Corners[6] = float3( Max.x, Max.y, Max.z );	//	 |/____|/
		Corners[7] = float3( Min.x, Max.y, Max.z );	//	4      7

		#define DRAW_CORNER_LINE(f,t)		g_Display->DebugLine( Corners[f], Colour, Corners[t] )
		//	top
		DRAW_CORNER_LINE(0,1);
		DRAW_CORNER_LINE(1,2);
		DRAW_CORNER_LINE(2,3);
		DRAW_CORNER_LINE(3,0);
		
		//	bottom
		DRAW_CORNER_LINE(4,5);
		DRAW_CORNER_LINE(5,6);
		DRAW_CORNER_LINE(6,7);
		DRAW_CORNER_LINE(7,4);

		//	sides
		DRAW_CORNER_LINE(0,4);
		DRAW_CORNER_LINE(1,5);
		DRAW_CORNER_LINE(2,6);
		DRAW_CORNER_LINE(3,7);		

		#undef DRAW_CORNER_LINE
	}
}

//--------------------------------------------------------------------------------------------------------
//	check sphere overlaps this sphere (ColPos is other parent relative to this parent)
//--------------------------------------------------------------------------------------------------------
Bool GCollisionObj::IntersectionSphereSphere(GCollisionObj& SphereObj,float3 ColPos)
{
	//	get center of other sphere
	ColPos += SphereObj.m_Offset;

	//	get dist from center to center
	float DistSq = (m_Offset - ColPos).LengthSq();

	//	negate distance by both radius' (squared)
	DistSq -= (m_SphereRadius + SphereObj.m_SphereRadius) * (m_SphereRadius + SphereObj.m_SphereRadius);

	//	if dist is < 0, then the length of the radius' is more than the distance, so they must overlap
	if ( DistSq <= 0.f )
		return TRUE;

	return FALSE;
}


//--------------------------------------------------------------------------------------------------------
//	does this box intersect this sphere
//--------------------------------------------------------------------------------------------------------
Bool GCollisionObj::IntersectionSphereBox(GCollisionObj& BoxObj,float3 ColPos)
{
	GDebug_Print("Sphere-box test not implemented\n");
	return FALSE;
}

//--------------------------------------------------------------------------------------------------------
//	checks for intersection with a box collision object (ColPos is relative to owner)
//--------------------------------------------------------------------------------------------------------
Bool GCollisionObj::IntersectionBoxSphere(GCollisionObj& SphereObj,float3 ColPos)
{
	GDebug_Print("box-sphere test not implemented\n");

	//	todo:
	//	get dir from sphere center to box center
	//	check to see if spherecenter + dir*rad is inside box

	return FALSE;
}

//--------------------------------------------------------------------------------------------------------
//	checks for intersection with a box collision object (ColPos is relative to owner)
//--------------------------------------------------------------------------------------------------------
Bool GCollisionObj::IntersectionBoxBox(GCollisionObj& BoxObj,float3 ColPos)
{
	GDebug_Print("box-sphere test not implemented\n");
	
	//	todo:
	//	translate box to local coords
	//	test if any corners are inside this box

	return FALSE;
}

//--------------------------------------------------------------------------------------------------------
//	checks point-intersection with sphere (point is relative to owner)
//--------------------------------------------------------------------------------------------------------
Bool GCollisionObj::IntersectionSpherePoint(float3 Point)
{
	//	translate so point is local to sphere
	Point += m_Offset;

	//	get dist to point
	float DistSq = Point.LengthSq();

	if ( DistSq > (m_SphereRadius*m_SphereRadius) )
		return FALSE;

	return TRUE;
}

//--------------------------------------------------------------------------------------------------------
//	checks point-intersection with box (point is relative to owner)
//--------------------------------------------------------------------------------------------------------
Bool GCollisionObj::IntersectionBoxPoint(float3 Point)
{
	//	translate so point is local to box
	Point += m_Offset;

	//	check is inside box
	if ( Point.x < m_BoxMin.x || Point.x > m_BoxMax.x )	return FALSE;
	if ( Point.y < m_BoxMin.y || Point.y > m_BoxMax.y )	return FALSE;
	if ( Point.z < m_BoxMin.z || Point.z > m_BoxMax.z )	return FALSE;

	return TRUE;
}


//-------------------------------------------------------------------------
//	load GCollisionObj from data
//-------------------------------------------------------------------------
Bool GCollisionObj::Load(GBinaryData& Data)
{
	//	read type
	u8 CollisionType = 0;
	if ( !Data.Read( &CollisionType, GDataSizeOf(u8), "Collision Object type" ) )
		return FALSE;
	m_ColType = (GCollisionObjType)CollisionType;

	//	read assetref
	if ( !Data.Read( &m_Ref, GDataSizeOf(GAssetRef), "Collision object ref" ) )
		return FALSE;

	//	read offset
	if ( !Data.Read( &m_Offset, GDataSizeOf(float3), "Collision object offset" ) )
		return FALSE;

	//	read params
	if ( !Data.Read( &m_Padding[0], GDataSizeOf(float)*9, "Collision object data" ) )
		return FALSE;
	
	return TRUE;
}

//-------------------------------------------------------------------------
//	save GCollisionObj data
//-------------------------------------------------------------------------
void GCollisionObj::Save(GBinaryData& Data)
{
	//	write type
	u8 CollisionType = (u8)m_ColType;
	Data.Write( &CollisionType, GDataSizeOf(u8) );
	
	//	write assetref
	Data.Write( &m_Ref, GDataSizeOf(GAssetRef) );
	
	//	write offset
	Data.Write( &m_Offset, GDataSizeOf(float3) );

	//	write params
	Data.Write( &m_Padding[0], GDataSizeOf(float)*9 );
}

