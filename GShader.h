/*------------------------------------------------

  GShader Header file



-------------------------------------------------*/

#ifndef __GSHADER__H_
#define __GSHADER__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GMatrix.h"
#include "GList.h"
#include "GAsset.h"
#include "GDisplay.h"



//	Macros
//------------------------------------------------
#define USE_ARB_PROGRAM		//	else NV, used for general usage

typedef enum
{
	GShaderMode_None=0,
	GShaderMode_Hardware,
	GShaderMode_Software,

} GShaderMode;


//	Types
//------------------------------------------------
class GMesh;
class GDrawInfo;
class GGameObject;

//-------------------------------------------------------------------------
//	shader class. This is used on all items to manipulate the object. either by 
//	animation or shading etc
//-------------------------------------------------------------------------
class GShader
{
public:
	u32					m_VertexProgramID;			//	hardware program index
	GShaderMode			m_CurrentMode;				//	what mode is active (use when rendering)
	GGameObject*		m_pOwner;					//	game object this shader is a member of
	u32					m_FirstConstant;			//	first constant that is availible for use

	enum
	{
		MAX_CONSTANTS=96,	//	0..95
	};

public:
	GShader();
	~GShader();

	//	vertex shader hardware stuff
	Bool				Upload();					//	load to hardware
	Bool				Deload();					//	unload from hardware
	inline Bool			IsLoaded() const			{	return (m_VertexProgramID != 0);	};	//	has this been uploaded?
	static Bool			InitHardware(int HardwareType);	//	sets up hardware for shader support
	static void			ShutdownHardware();			//	cleanup hardware setup

	#ifdef USE_ARB_PROGRAM
		static inline Bool	HardwareSupported()		{	return g_DisplayExt.HardwareEnabled( GHardware_ARBVertexProgram );	};
	#else
		static inline Bool	HardwareSupported()		{	return g_DisplayExt.HardwareEnabled( GHardware_NVVertexProgram );	};
	#endif

	//	vertex shader funcs
	virtual GShaderMode	SetDrawMode();				//	set and return what will be the active mode, ie. hardware if supported, or software, or none
	inline GShaderMode	DrawMode()					{	return m_CurrentMode;	};

	Bool				PreDraw(GMesh* pMesh,GDrawInfo& DrawInfo, GList<float3>*& pVertexBuffer, GList<float3>*& pNormalBuffer, GList<float2>*& pTextureUVBuffer, GList<float2>*& pTextureUV2Buffer, GList<float3>*& pColourBuffer);	//	called before drawing primitives
	void				PostDraw(GMesh* pMesh,GDrawInfo& DrawInfo);		//	called after drawing all primitives

	//	game specific shader functions
	virtual Bool		GetVertexProgram(GString& String)=0;	//	fill the string with the program code
	virtual void		Update()								{	};					//	

	virtual Bool		HardwareVersion()						{	return FALSE;	};	//	does this have a hardware shader
	virtual Bool		HardwarePreDraw(GMesh* pMesh,GDrawInfo& DrawInfo, GList<float3>*& pVertexBuffer, GList<float3>*& NormalBuffer, GList<float2>*& pTextureUVBuffer, GList<float2>*& pTextureUV2Buffer, GList<float3>*& pColourBuffer)=0;	//	do pre-render stuff
	virtual Bool		HardwarePreDrawVertex(float3& Vertex, float3& Normal, int VertexIndex )=0;	//	called before manually(not using lists) drawing a single vertex in hardware mode

	virtual Bool		SoftwareVersion()						{	return FALSE;	};	//	does this have a software implementation
	virtual Bool		SoftwarePreDraw(GMesh* pMesh,GDrawInfo& DrawInfo, GList<float3>*& pVertexBuffer, GList<float3>*& NormalBuffer, GList<float2>*& pTextureUVBuffer, GList<float2>*& pTextureUV2Buffer, GList<float3>*& pColourBuffer)=0;	//	do software implementation


protected:
	//	useful shader functions
	inline void			LoadConstant(int ConstantIndex, float x, float y=0, float z=0, float w=0 )	{	LoadConstant( ConstantIndex, float4( x, y, z, w ) );	};
	inline void			LoadConstant(int ConstantIndex, float2& xy, float z=0, float w=0 )			{	LoadConstant( ConstantIndex, float4( xy.x, xy.y, z, w ) );	};
	inline void			LoadConstant(int ConstantIndex, float3& xyz, float w=0 )					{	LoadConstant( ConstantIndex, float4( xyz.x, xyz.y, xyz.z, w ) );	};
	void				LoadConstant(int ConstantIndex, float4& xyzw );

};







//-------------------------------------------------------------------------
//	pixel shader class. Works just like the vertex shader but with no software mode
//-------------------------------------------------------------------------
class GPixelShader
{
public:
	u32					m_FragmentProgramID;		//	hardware program index
	GGameObject*		m_pOwner;					//	game object this shader is a member of

public:
	GPixelShader();
	~GPixelShader();

	//	pixel shader hardware stuff
	Bool				Upload();								//	load to hardware
	Bool				Deload();								//	unload from hardware
	inline Bool			IsLoaded() const						{	return (m_FragmentProgramID != 0);	};	//	has this been uploaded?

	static inline Bool	HardwareSupported()						{	return g_DisplayExt.HardwareEnabled( GHardware_ARBFragmentProgram );	};

	//	pixel shader funcs
	Bool				PreDraw(GMesh* pMesh,GDrawInfo& DrawInfo, GList<float3>*& pVertexBuffer, GList<float3>*& pNormalBuffer, GList<float2>*& pTextureUVBuffer, GList<float2>*& pTextureUV2Buffer, GList<float3>*& pColourBuffer);	//	called before drawing primitives
	void				PostDraw(GMesh* pMesh,GDrawInfo& DrawInfo);						//	called after drawing all primitives

	//	game specific shader functions
	virtual Bool		GetFragmentProgram(GString& String)=0;	//	fill the string with the program code
	virtual void		Update()								{	};	//	shader specific update

	virtual Bool		HardwarePreDraw(GMesh* pMesh,GDrawInfo& DrawInfo, GList<float3>*& pVertexBuffer, GList<float3>*& NormalBuffer, GList<float2>*& pTextureUVBuffer, GList<float2>*& pTextureUV2Buffer, GList<float3>*& pColourBuffer)=0;	//	do pre-render stuff
};

//	Declarations
//------------------------------------------------


//	Inline Definitions
//-------------------------------------------------



#endif

