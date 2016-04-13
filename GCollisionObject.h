/*------------------------------------------------

  Collision object used for culling and physics



-------------------------------------------------*/

#ifndef __GCOLLISIONOBJECT__H_
#define __GCOLLISIONOBJECT__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GAsset.h"


//	Macros
//------------------------------------------------
typedef enum GCollisionObjType
{
	GCollisionObjType_None =0,
	GCollisionObjType_Sphere,
	GCollisionObjType_Box,
};

//	Types
//------------------------------------------------

//--------------------------------------------------------------------------------------------------------
// base collision object type
//--------------------------------------------------------------------------------------------------------
class GCollisionObj
{
public:
	static float4		g_DebugColour;
	static u32			g_Version;

public:
	GCollisionObjType	m_ColType;	//	type of collision obj
	GAssetRef			m_Ref;		//	reference for collision obj
	float3				m_Offset;	//	local offset from parent mesh/physics object etc

	//	type-specific params
	union
	{
		struct // total extra-data size
		{
			float m_Padding[9];
		};
		struct // sphere
		{
			float m_SphereRadius;
			float m_SpherePadding[8];
		};
		struct // box
		{
			float3 m_BoxMin;
			float3 m_BoxMax;
			float m_BoxPadding[3];
		};
	};

public:
	GCollisionObj();
	
	GCollisionObjType	CollisionType()								{	return m_ColType;	};
	float3				CollisionOffset()							{	return m_Offset;	};
	
	Bool				Intersection(float3& Point);							//	checks a point being inside this (point is relative to owner)
	Bool				Intersection(GCollisionObj& ColObj,float3& ColPos);		//	checks intersection with another collision object (ColPos is relative to owner)

	void				SetSphereParams(float Radius, float3& Center);			//	make into a sphere collision object
	void				SetBoxParams(float3& Min, float3& Max, float3& Center);	//	make into a box collision object

	void				DebugDraw(float3& ParentWorldPos,float4& Colour=g_DebugColour);	//	draws sphere/box etc

	Bool				Load(GBinaryData& Data);								//	load GCollisionObj from data
	void				Save(GBinaryData& Data);								//	save GCollisionObj data

protected:
	Bool				IntersectionSphereSphere(GCollisionObj& SphereObj,float3 ColPos);	
	Bool				IntersectionSphereBox(GCollisionObj& BoxObj,float3 ColPos);	
	Bool				IntersectionBoxSphere(GCollisionObj& SphereObj,float3 ColPos);	
	Bool				IntersectionBoxBox(GCollisionObj& BoxObj,float3 ColPos);	

	Bool				IntersectionSpherePoint(float3 Point);					//	checks point-intersection with sphere (point is relative to owner)
	Bool				IntersectionBoxPoint(float3 Point);						//	checks point-intersection with box (point is relative to owner)
};



//	Declarations
//------------------------------------------------


//	Inline Definitions
//-------------------------------------------------





#endif

