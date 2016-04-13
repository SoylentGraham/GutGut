/*------------------------------------------------

  GDisplay Header file



-------------------------------------------------*/

#ifndef __GDISPLAY__H_
#define __GDISPLAY__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GWindow.h"
#include "GMenu.h"
#include "GQuaternion.h"
#include "GAsset.h"
#include "GDisplayExt.h"

//	Macros
//------------------------------------------------
#define g_Display		(GDisplay::g_pDisplay)
#define g_DisplayExt	(g_Display->Ext())


//	32 bit rgba
#define RGBA(r,g,b,a)		( (r) | (g<<8) | (b<<16) | (a<<24) )
#define RGB24(r,g,b)		RGBA(r,g,b,0)
#define RGBA_Red(rgba)		( (rgba) & 0xff )
#define RGBA_Green(rgba)	( (rgba>>8) & 0xff )
#define RGBA_Blue(rgba)		( (rgba>>16) & 0xff )
#define RGBA_Alpha(rgba)	( (rgba>>24) & 0xff )

//	32 bit rgb element -> 0..1 float
#define RGBA_Redf(rgba)		( (float)RGBA_Red(rgba) / 255.f )
#define RGBA_Greenf(rgba)	( (float)RGBA_Green(rgba) / 255.f )
#define RGBA_Bluef(rgba)	( (float)RGBA_Blue(rgba) / 255.f )
#define RGBA_Alphaf(rgba)	( (float)RGBA_Alpha(rgba) / 255.f )

//	float3 -> 32bit rgb
#define RGB24_float3(f3)	RGB24( (u8)((f3).x * 255.f), (u8)((f3).y * 255.f), (u8)((f3).z * 255.f) )

//	float4 -> 32bit rgba
#define RGBA_float4(f4)		RGBA( (u8)((f4).x * 255.f), (u8)((f4).x * 255.f), (u8)((f4).z * 255.f), (u8)((f4).w * 255.f) )

//	 32/24bit rgba -> float4
#define float4_RGBA(rgba)	float4( RGBA_Red(rgba), RGBA_Green(rgba), RGBA_Blue(rgba), RGBA_Alpha(rgba) )
#define float4_RGB24(rgb,a)	float4( RGBA_Red(rgb), RGBA_Green(rgb), RGBA_Blue(rgb), a )

//	16 bit rgb
#define FIVEBITS(n)			( n & (0x1f) )	//	31, 00011111
#define RGB16(r,g,b)		( FIVEBITS(r) | (FIVEBITS(g)<<5) | (FIVEBITS(g)<<10) )
#define RGB16_Red(rgb)		( FIVEBITS(rgb) )
#define RGB16_Green(rgb)	( FIVEBITS((rgb)>>5) )
#define RGB16_Blue(rgb)		( FIVEBITS((rgb)>>10) )

//	5bit rgb element -> 8bit rgb element
#define RGB16_Red8(rgb)		((u8)( (float)RGB16_Red(rgb) * (255.f/31.f) ))
#define RGB16_Green8(rgb)	((u8)( (float)RGB16_Green(rgb) * (255.f/31.f) ))
#define RGB16_Blue8(rgb)	((u8)( (float)RGB16_Blue(rgb) * (255.f/31.f) ))

//	5bit rgb element -> 0..1 float
#define RGB16_Redf(rgb)		( (float)RGB16_Red(rgb) / 31.f )
#define RGB16_Greenf(rgb)	( (float)RGB16_Green(rgb) / 31.f )
#define RGB16_Bluef(rgb)	( (float)RGB16_Blue(rgb) / 31.f )

//	frame rate
#define FIXED_FRAME_RATE	(60)
#define FIXED_FRAME_RATEF	(60.f)



//	flags for drawing meshes
namespace GDrawInfoFlags
{
	const u32	Wireframe				= 1<<0;		//	render in wireframe
	const u32	HardwareLighting		= 1<<1;		//	default lighting
	const u32	DontCullBackfaces		= 1<<2;		//	dont cull backfaces (which is default)
	const u32	CullFrontFaces			= 1<<3;		//	cull front faces by default instead of backfaces
	const u32	DontTranslate			= 1<<5;		//	wont translate
	const u32	DontRotate				= 1<<6;		//	wont rotate/calc rotation matrix
	const u32	DontCullTest			= 1<<7;		//	dont do a cull test
	const u32	DontAutoInsideCull		= 1<<8;		//	used for objects: dont do auto cull objects inside others
	const u32	ForceColour				= 1<<9;		//	forces sub colours to match the passed in colour
	const u32	MergeColourMult			= 1<<10;	//	merges colour with sub colours (multiplies colour)
	const u32	MergeColourAdd			= 1<<11;	//	merges colour with sub colours (Adds colour)
	const u32	MergeColourSub			= 1<<12;	//	merges colour with sub colours (Subtracts colour)

	const u32	DebugPhysicsCollisions	= 1<<17;	//	draw debug triangles, points etc to do with actual intersections
	const u32	DebugPhysicsShapes		= 1<<18;	//	draw boxes/spheres etc used in collision tests
	const u32	DisableTextures			= 1<<19;	//	meshes will be drawn without textures
	const u32	DebugNormals			= 1<<20;	//	draws all the normals facing out
	const u32	DebugFaceNormals		= 1<<21;	//	draws all the normals facing out for the faces (from plane info)
	const u32	DebugColours			= 1<<22;	//	renders the model without lighting and using the meshes debug colours
	const u32	DebugShadows			= 1<<23;	//	draws lines instead of actual shadows
	const u32	DebugOutline			= 1<<24;	//	draws a second pass in wireframe mode
	const u32	DebugCollisionObjects	= 1<<25;	//	draw collision objects on meshes
	const u32	DebugPortals			= 1<<27;	//	draws portals for debugging
	const u32	DisableMapObjShadows	= 1<<28;	//	wont draw shadows for mapobjects
	const u32	DisableGameObjShadows	= 1<<29;	//	wont draw shadows for game objects
	const u32	DebugSkeleton			= 1<<30;	//	draws skeleton
	const u32	DebugVertexes			= 1<<31;	//	draws a point at each vertex with a colour specified by an array passed in the drawinfo struct
};



//	result of a mesh draw
typedef enum
{
	GDrawResult_Drawn,		//	mesh was drawn successfully
	GDrawResult_Error,		//	error drawing mesh
	GDrawResult_Nothing,	//	no data to draw
	GDrawResult_Culled,		//	mesh wasnt drawn as its out of view. frustrum culling etc
	GDrawResult_NotVisible,	//	mesh wasnt drawn as its completley transparent (could possibly be used by other things)
	GDrawResult_Cancelled,	//	

} GDrawResult;


typedef enum
{
	GPreDrawResult_Unknown=0,		//	cull hasnt been pre-calculated
	GPreDrawResult_Draw,			//	has been checked to be not culled
	GPreDrawResult_NoObject,		//	object is missing
	GPreDrawResult_Culled,			//	has been culled
	GPreDrawResult_CulledInside,	//	has been culled because an object it was inside was culled
	GPreDrawResult_IsInside,		//	object is hidden by an object it is inside

} GPreDrawResult;


//	Types
//------------------------------------------------
class GApp;
class GCamera;
class GMapLight;
class GShader;
class GPixelShader;



/*
	Generic window for opengl app's
*/
class GAppWindow : public GOpenglWindow
{
public:
	
public:
	GAppWindow();
	~GAppWindow();
	
	
};






//-------------------------------------------------------------------------
//	Plane class
//-------------------------------------------------------------------------
class GPlane : public float4
{
public:
	
public:
	GPlane()										{	Set(0,0,0,0);	};
	GPlane(float3& v0,float3& v1,float3& v2)		{	CalcEquation( v0, v1, v2 );	};
	GPlane(GPlane& plane)							{	Set( plane );	};

	Bool			Intersection( float& IntLength, float3& Pos, float3& Dir );
	void			CalcEquation( float3& v0, float3& v1, float3& v2 );

	void			InvertNormal()					{	x=-x;	y=-y;	z=-z;	w=-w;	};
	float3			Normal()						{	return float3(x,y,z);	};
};



/*
	rendering info
*/
class GDrawInfo
{
public:
	static GDrawInfo	g_Default;	//	default to copy
public:
	GAssetRef		TextureRef;		//	texture reference
	GAssetRef		TextureRef2;	//	2nd (multitexture) texture
	GAssetRef		Flags;			//	GDrawInfoFlags
	float4			RGBA;			//	rgba colour. if alpha isnt 1 or 0, we render with 2 passes for correct zsorting

	float3			Translation;	//	translation translation from current world pos
	float3			WorldPos;		//	world position, includes Translation
	GQuaternion*	pRotation;		//	local rotation
	GMapLight*		pLight;			//	local light for the object
	GShader*		pShader;		//	shader
	GPixelShader*	pPixelShader;	//	pixel shader
	GList<float3>*	pVertexColours;	//	alternative colour set for verts when using colours and debugging (DebugVertexes flag)

public:
	GDrawInfo()										{	SetDefault();	};
	GDrawInfo(GDrawInfo* pDrawInfo)					{	Copy( pDrawInfo );	};
	GDrawInfo(GDrawInfo& DrawInfo)					{	Copy( &DrawInfo );	};
	GDrawInfo(Bool MakeDefault)						{	InitDefault();	};
	inline void		Copy(GDrawInfo* pDrawInfo)		{	memcpy( this, pDrawInfo, sizeof( GDrawInfo ) );	};
	inline void		operator=(GDrawInfo* pDrawInfo)	{	Copy( pDrawInfo );	};
	inline void		operator=(GDrawInfo& DrawInfo)	{	Copy( &DrawInfo );	};

	inline void		SetDefault()					{	Copy( &GDrawInfo::g_Default );	};
	__forceinline	DoTranslation()					{	return ( ( Flags & GDrawInfoFlags::DontTranslate ) == 0x0 );	};
	__forceinline	DoRotation()					{	return pRotation && ( ( Flags & GDrawInfoFlags::DontRotate ) == 0x0 );	};

private:
	void			InitDefault();
};


/*
	Debug items
*/
class GDebugPoint
{
public:
	float3		Pos;
	float4		Colour;
	void		Draw();
};

class GDebugSphere : public GDebugPoint
{
public:
	float		Radius;
	void		Draw();
};


class GDebugPosition : public GDebugPoint
{
public:
	GQuaternion	Rot;
	void		Draw();

	inline operator	=(const GDebugPosition& p)	{	Rot = p.Rot;	Pos = p.Pos;	Colour = p.Colour;	};
};

class GDebugLine : public GDebugPoint
{
public:
	float3		PosTo;
	void		Draw();
};



/*
	Basic bounds class for culling, intersections etc
*/
class GBounds
{
public:
	float3	m_Offset;
	float	m_Radius;	//	+x, +y, +z

public:
	GBounds(float Radius=0.f,float3 Offset=float3(0,0,0) )	{	m_Offset = Offset;	m_Radius = Radius;	};
	~GBounds()	{	};

	void	DebugDraw(float3& ThisPos,float4 Colour=float4(1,1,1,1) );
	Bool	IsValid()					{	return (m_Radius>0.f);	};			//	has this been setup?
	Bool	IsCulled(float3& ThisPos,GCamera* pCamera=NULL);					//	tests to see if these bounds are culled with the active camera's frustum
	Bool	Intersects(float3& ThisPos, GBounds& Bounds, float3& BoundsPos );	//	do these bounds intersect this?
	Bool	Inside(float3& ThisPos, float3& Position);							//	is this position inside these bounds?
	Bool	Inside(float3& ThisPos, GBounds& Bounds, float3& BoundsPos );		//	are these bounds completely inside this?
};



/*
	Display wrapper class. all render specific stuff should be added here to keep it away from the camera or app
*/
class GDisplay
{
public:
	static float4		g_DebugColour;				//	main debug colour
	static Bool			g_OpenglInitialised;		//	if opengl has been initialised, stops us calling some opengl calls
	static GDisplay*	g_pDisplay;					//	global display object
	
public:
	GOpenglWindow*		m_pWindow;					//	reference to opengl window
	GDisplayExt			m_Extensions;				//	extension handler
	
public:
	GDisplay();
	~GDisplay();

	Bool				Init();						//	initialises display and checks hardware support
	void				Draw();
	void				Shutdown();

	//	window
	GOpenglWindow*		Window()					{	return m_pWindow;	};
	inline HWND			Hwnd()						{	return Window() ? Window()->Hwnd() : NULL;	};
	int2				ScreenSize();
	int2				Size()						{	return Window() ? Window()->m_ClientSize : int2(-1,-1);	}
	int2				Pos()						{	return Window() ? Window()->m_ClientPos : int2(-1,-1);	}

	//	opengl
	void				Translate(float3& Transform, GQuaternion* pRotation, Bool DoTranslation=TRUE, Bool DoRotation=TRUE);
	void				Translate(GMatrix& Matrix);
	void				Translate(GQuaternion& Quaternion);
	void				PushScene();
	void				PopScene();
	float2				ObjectToScreen(float3& WorldPos);
	GDisplayExt&		Ext()						{	return m_Extensions;	};
	
	//	debug stuff
	void				DrawDebugItems();												//	draws all the debug items in the cache and empties it
	void				ClearDebugItems();												//	empties debug item cache
	static void			DebugPoint(float3& Pos, float4& Colour);						//	adds a new debug point to the cache
	static void			DebugPosition(float3& Pos, float4& Colour, GQuaternion& Rot);	//	adds a new debug position to the cache
	static void			DebugSphere(float3& Pos, float4& Colour, float Radius);			//	adds a new debug sphere to the cache
	static void			DebugLine(float3& Pos, float4& Colour, float3& PosTo);			//	adds a new debug line sto the cache

};






//	Declarations
//------------------------------------------------
float3	CalcNormal(float3& va, float3& vb, float3& vc );	//	calculate a normal for these vertexes/positions
Bool	PointInsideTriangle(float3& Point, float3& v0, float3& v1, float3& v2, GPlane& Plane);
float3	ClosestPointOnTriangle(float3& a, float3& b, float3& c, float3& p);
float	TriangleSurfaceArea(float3& va, float3& vb, float3& vc);



//	Inline Definitions
//-------------------------------------------------





#endif

