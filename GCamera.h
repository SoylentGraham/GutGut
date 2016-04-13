/*------------------------------------------------

  GCamera Header file



-------------------------------------------------*/

#ifndef __GCAMERA__H_
#define __GCAMERA__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GMatrix.h"
#include "GAsset.h"
#include "GSkyBox.h"
#include "GQuaternion.h"



//	Macros
//------------------------------------------------
namespace GCameraFlags
{
	const u32	Clear				= 1<<1;		//	clears the viewport
	const u32	ClearZ				= 1<<2;		//	clears the depth buffer
	const u32	ClearStencil		= 1<<3;		//	clears the stencil buffer
	const u32	NoPerspective		= 1<<4;		//	disables perspective when setting up the projection

	const u32	FailCullTests		= 1<<20;	//	debug: fail all cull tests
};




//	Types
//------------------------------------------------
class GCamera
{
public:
	static GCamera*	g_pActiveCamera;	//	camera currently being used

public:
	GMatrix		m_CameraMatrix;
	u32			m_CameraFlags;	//	GCameraFlags
	float3		m_LookAt;
	float3		m_Position;
	float3		m_WorldUp;
	float		m_FOV;			//	horizontal fov in degrees
	float2		m_OrthoMin;		//	orthogrpahic projection settings (left/up)
	float2		m_OrthoMax;		//	orthogrpahic projection settings (right/down)
	float2		m_OrthoNearFar;	//	near/far for ortho mode
	float		m_NearZ;
	float		m_FarZ;
	int4		m_Viewport;		//	where to render into (x,y,w,h)
	float4		m_ClearColour;	//	colour when clearing the screen
	
	float3		m_OrthoOffset;	//	scroll-like offset for ortho mode
	GQuaternion	m_OrthoRotation;

private:
	//float4		m_FrustumPlanes[6][4];
	float4		m_FrustumPlanes[6];
	int			m_FrustumPlaneCount;	//	0..6
	Bool		m_OrthoMode;			//	currently in ortho(2d) mode

public:
	GCamera();
	~GCamera();

	inline float	FOVHorz() const										{	return m_FOV;	};
	inline float	FOVVert() const										{	return FOVHorz() / ViewportAspectRatio();	};
	float2			GetWorldVisibility();								//	returns what portion of the world is viewable from 0..1 on x and y axis' by using the FOV values
	float2			GetEnvMapScroll();									//	returns scroll values for enviroment mapping based on the camera's viewing direction


	void			UpdateMatrix();
	void			SetupView(Bool SetupMatriciesOnly=FALSE,GSkyBox* pSkyBox=NULL);	//	setup the view according to the cmaera settings
	void			SetupFrustum(float left, float right,float bottom, float top,float nearval, float farval);
	void			SetupPerspective();
	
	Bool			BeginOrtho();										//	sets us up into ortho(2d) mode
	void			EndOrtho();

	void			CaptureTexture(GAssetRef TextureRef);

	inline float3	GetFowardVector() const								{	return (m_LookAt - m_Position);	};
	
	Bool			PointInViewport(int2& Pos);
	inline float	ViewportAspectRatio() const;

	void			CalcFrustumPlanes();								//	calculate frustum planes from currently camera display
	void			CalcFrustumPlanes(int Planes, float3* pVerts, float3& ExternalCameraPosition );	//	calculate frustum planes based on vertex positions and existing camera position. used in portals
	Bool			CullTest(float3& Pos, float Radius);				//	general cull test checks frustum and can check other stuff
	Bool			SphereInsideFrustum(float3& Pos, float Radius);		//	frustum plane cull test
};



//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------
inline float GCamera::ViewportAspectRatio() const
{
	float w = (float)(m_Viewport[2] - m_Viewport.x);
	float h = (float)(m_Viewport[3] - m_Viewport.y);
	return w / h;
//	return h / w;
}



#endif

