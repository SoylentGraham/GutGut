/*------------------------------------------------

  GSkin Header file



-------------------------------------------------*/

#ifndef __GSKIN__H_
#define __GSKIN__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GMatrix.h"
#include "GSkeleton.h"
#include "GAsset.h"
#include "GSkin.h"



//	Macros
//------------------------------------------------
namespace GSkinHeaderDataFlags
{
	const u32	BoneVertexLinks		= 1<<0;	//	list of links from bone->vertex
	const u32	VertexWeights		= 1<<1;	//	list of weights for a vertex
	const u32	VertexBones			= 1<<2;	//	list of bones for a vertex
	const u32	BoneBoneList		= 1<<3;	//	
};

namespace GSkinShaderFlags
{
	const u32	NewAnim				= 1<<0;	//	change to new anim and forces bone update
	const u32	BlendAnim			= 1<<1;	//	blend current anims
	const u32	ForceUpdate			= 1<<2;	//	force it to get new frame
};


//	Types
//------------------------------------------------
class GSkin;
class GSkeleton;
class GMesh;
class GSkinShader;


//-------------------------------------------------------------------------
//	skin data
//-------------------------------------------------------------------------
typedef struct 
{
	GAssetRef		SkeletonRef;
	GAssetRef		MeshRef;
	u32				DataFlags;		//	flags of what data is contained inside the data

} GSkinHeader;

//-------------------------------------------------------------------------
//	struct for each bone which contains a list of verts
//-------------------------------------------------------------------------
typedef struct 
{
	GList<int2>		Vertexes;	//	vertex index, weight index

} GSkinBoneVertexList;


//-------------------------------------------------------------------------
//	bone bone struct entry (todo: convert to a single u32)
//-------------------------------------------------------------------------
typedef struct 
{
	s8		BoneA;
	s8		BoneB;
	u16		NoOfVerts;

} GSkinBoneBone;

//-------------------------------------------------------------------------
//	GSkin joins a skeleton to a mesh, contains vertex weights etc
//	the skin is seperate from the skeleton so the skeleton can be used with different meshes
//	so we dont have to keep details about the mesh in the skeleton
//-------------------------------------------------------------------------
class GSkin : public GAsset
{
public:
	const static u32	g_Version;

public:
	GAssetRef					m_Skeleton;			//	skeleton assigned to this skin
	GAssetRef					m_Mesh;				//	mesh assigned to this skin
	GList<float2>				m_VertexWeights;	//	2 weights per vertex. one for each bone
	GList<s82>					m_VertexBones;		//	2 bones per vertex.
	GList<GSkinBoneVertexList>	m_BoneVertexList;	//	for each bone, a list of vertexes linked with it

	GList<GSkinBoneBone>		m_BoneBoneList;
	GList<u32>					m_BoneBoneVertexList;
	
public:
	GSkin();
	~GSkin()		{	};

	//	asset virtual
	virtual GAssetType	AssetType()						{	return GAssetSkin;	};
	virtual u32			Version()						{	return GSkin::g_Version;	};
	virtual Bool		Load(GBinaryData& Data);
	virtual Bool		Save(GBinaryData& Data);

	//	
	void				Draw(GDrawInfo& DrawInfo, GSkinShader* pShader, GAssetRef HighlightBone=GAssetRef_Invalid);	//	draws mesh and skeleton
	
	void				GenerateBoneVertexWeights();	//	auto generate weight and vertex-bone links based on vertex positions nearest to bones
	void				GenerateBoneBoneLists();		//	generate the bone-bone links from the existing bone-vertex link info

	GSkeleton*			GetSkeleton();
	GMesh*				GetMesh();
};




//-------------------------------------------------------------------------
//	vertex shader for skinning
//-------------------------------------------------------------------------
class GSkinShader : public GShader
{
protected:
	float				m_AnimFrame;				//	current frame in skeleton anim
	GAssetRef			m_Anim;						//	current skeleton anim
	GMatrixRotations	m_AnimRotations;
	u32					m_ValidAnimRotations;		//	bitfield of non-identity matrixes

	GMatrixRotations	m_ModifiedRotations;
	u32					m_ValidModifiedRotations;	//	bitfield of non-identity matrixes

	u32					m_ModifiedBones;			//	bitfield of bones(final matrixes) that need recalculating
	u32					m_VertexModifiedBones;		//	bitfield of bones that need vertex buffer vertexes recalculating
	GList<float3>		m_SoftwareVertexBuffer;		//	local vertex buffer for software mode

	GAssetRef			m_Skin;						//	skin (mesh/skeleton)

public:
	GList<float3>		m_InverseVertexBuffer;		//	cached list of inversed vertexes
	GList<GMatrix>		m_BoneFinal;				//	bone transformation for our combined rotations

	int					m_LastBoneCounter;
	u32					m_Flags;					//	GSkinShaderFlags
	GAssetRef			m_NewAnim;					//	go onto this anim
	float				m_NewAnimFrame;				//	go to this frame on the new anim (or on next frame)
	
	GAssetRef			m_BlendAnim;				//	blend into this anim
	float				m_BlendAnimFrame;			//	current frame of the blending anim
	float				m_BlendAnimNewFrame;		//	go to this frame on next frame update for blend
	float				m_BlendAmount;				//	interp between current anim and blend anim, when this reaches 1 switch to the blend anim only
	float				m_BlendRate;				//	increase blend amount by this amount every (whole) frame

	float				m_AnimSpeed;				//	to modify animation speed. default to 1.f


public:
	GSkinShader();
	~GSkinShader();

	//	shader virtual
	virtual Bool		HardwareVersion()			{	return TRUE;	};
	virtual Bool		HardwarePreDraw(GMesh* pMesh,GDrawInfo& DrawInfo, GList<float3>*& pVertexBuffer, GList<float3>*& pNormalBuffer, GList<float2>*& pTextureUVBuffer, GList<float2>*& pTextureUV2Buffer, GList<float3>*& pColourBuffer);
	virtual Bool		HardwarePreDrawVertex(float3& Vertex, float3& Normal, int VertexIndex );
	virtual Bool		GetVertexProgram(GString& String);

	virtual Bool		SoftwareVersion()			{	return TRUE;	};
	virtual Bool		SoftwarePreDraw(GMesh* pMesh,GDrawInfo& DrawInfo, GList<float3>*& pVertexBuffer, GList<float3>*& pNormalBuffer, GList<float2>*& pTextureUVBuffer, GList<float2>*& pTextureUV2Buffer, GList<float3>*& pColourBuffer);

	virtual void		PostDraw(GMesh* pMesh,GDrawInfo& DrawInfo);

	virtual void		Update();							//	continue animation

	//	skinning
	GSkeletonAnim*		GetAnim();
	GSkeletonAnim*		GetNewAnim();
	GSkeletonAnim*		GetBlendAnim();
	GSkin*				GetSkin();
	GSkeleton*			GetSkeleton(GAssetRef SkeletonRef=GAssetRef_Invalid);
	GMesh*				GetMesh(GAssetRef MeshRef=GAssetRef_Invalid);

	inline GAssetRef	CurrentAnim()													{	return ( m_Flags & GSkinShaderFlags::NewAnim ) ? m_NewAnim : m_Anim;	};
	inline GAssetRef	CurrentBlendAnim()												{	return ( m_Flags & GSkinShaderFlags::BlendAnim ) ? m_BlendAnim : GAssetRef_Invalid;	};
	void				SetNewAnim(GAssetRef Anim, float NewFrame);
	void				SetNewFrame(float NewFrame);
	void				BlendToAnim(GAssetRef BlendAnim, float BlendAnimFromFrame, float BlendOverFrames,Bool ForceNewBlend=FALSE);	//	will change into a blend, if its already blending to this anim it wont reset blend settings

	Bool				SetSkin(GAssetRef SkinRef);								//	set new skin ref

	void				SetModifiedMatrixPosFromParent(int BoneIndex, float3& PosFromParent);
	void				SetModifiedMatrix(int BoneIndex, GMatrix& NewMatrix);
	inline void			GetModifiedRotation(int BoneIndex, GMatrix& Matrix)		{	Matrix = m_ModifiedRotations[BoneIndex];	};
	inline Bool			HasModifiedRotations()									{	return ( m_ModifiedRotations.Size() != 0 );	};
	
	inline Bool			HasAnimRotations()										{	return ( m_AnimRotations.Size() != 0 );	};
	inline int			RotationCount()											{	return m_AnimRotations.Size();	};
	inline void			SetAllBonesChanged()									{	m_ModifiedBones = 0xffffffff;	};
	inline void			SetAnimBonesChanged()									{	m_AnimFrame = -1;	};

private:
	Bool				UpdateToNewFrame();										//	updates our anim matrixes. returns if changed
	void				CalcInverseVertexBuffer( GList<float3>& VertexBuffer );
	void				UpdateFinalBones();										//	recalculated final bone matrixes as required
};


//	Declarations
//------------------------------------------------



//	Inline Definitions
//-------------------------------------------------




#endif

