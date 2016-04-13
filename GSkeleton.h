/*------------------------------------------------

  GSkeleton Header file



-------------------------------------------------*/

#ifndef __GSKELETON__H_
#define __GSKELETON__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GQuaternion.h"
#include "GList.h"
#include "GAsset.h"
#include "GMatrix.h"



//	Macros
//------------------------------------------------
//	limited to 32 bones to make use of bitfields
//	limited to 29 for max shader constants (96 max)
#define MAX_BONES	29		

namespace GSkeletonDataFlags
{
	const u8	DebugColours	= 1<<0;	//	colour for each bone
};

typedef enum
{
	GRotationBlendResult_Merged = 0,	//	rotations were blended
	GRotationBlendResult_Original,		//	rotations we left alone as blend rate was so low (or error)
	GRotationBlendResult_New,			//	rotations we set to the blend rotations passed as rate was so high

} GRotationBlendResult;

//	Types
//------------------------------------------------
class GBone;
class GSkeleton;
class GSkin;

//-------------------------------------------------------------------------
//	matrix list but with a few additonal funcs for rotations
//-------------------------------------------------------------------------
class GMatrixRotations : public GList<GMatrix>
{
public:
	
public:
	GMatrixRotations()		{	};
	~GMatrixRotations()		{	};

	GRotationBlendResult	Blend(GMatrixRotations& BlendRotations, float BlendRate);	//	blend these rotations with the rotations passed in
};


//-------------------------------------------------------------------------
//	bone data
//-------------------------------------------------------------------------
typedef struct 
{
	u16				ChildBones;	//	number of child bones
	float3			Offset;		//	offset of bone from parent
	GAssetRef		BoneRef;	//	ref of bone

} GBoneHeader;


//-------------------------------------------------------------------------
//	Bone in a skeleton
//-------------------------------------------------------------------------
class GBone
{
	friend GSkeleton;
public:
	GList<GBone*>		m_Children;		//	child bones
	GAssetRef			m_BoneRef;		//	bone ref
	GSkeleton*			m_pSkeleton;	//	owner skeleton

	GMatrix				m_Relative;		//	local transformation matrix
	GMatrix				m_Absolute;		//	transformation matrix in heirachy
	u32					m_ParentBones;	//	bitmask of this bones parents (-1 if uninitialised as not ALL bones can be parent)
	u32					m_ChildBones;	//	bitmask of this bones children (similar to ParentBones bitmask)

private:
	GBone*				m_pParent;		//	parent bone
	int					m_BoneCount;	//	number of bones (inc this) under this bone. stored to save calculating it over and over
	float3				m_Offset;		//	bone offset from parent
	int					m_Index;		//	index in skeleton, -1 if not yet set

public:
	GBone();
	~GBone()			{	DeleteChildren();	};

	GBone*				AddNewBone();					//	create a new bone and add to children
	GBone*				AddBone(GBone* pBone);			//	add this bone to children
	inline int			BoneCount()						{	return (m_BoneCount <= 0) ? CountBones() : m_BoneCount;	};	//	number of bones. this + child bone counts
	GBone*				FindBone(GAssetRef BoneRef,int& Index);	//	search through children for this bone
	Bool				FindBone(GBone* pBone,int& Index);	//	search through children for this bone pointer
	void				MoveOffset(float3& Change);			//	move this bone and all subbones' offset
	void				SetOffset(float3& Offset);			//	move this bone and all subbones' offset
	inline float3		Offset()						{	return m_Offset;	};	//	move this bone and all subbones' offset
	GBone*				ParentBone()					{	return m_pParent;	};
	void				DebugBoneTree(int Recursion=0);	//	debug print out the tree from this bone
	inline int			GetIndex()						{	return (m_Index==-1) ? CalcIndex() : m_Index;	};
	inline int			GetParentIndex()				{	return m_pParent ? m_pParent->GetIndex() : -1;	};
	GBone*				GetBoneIndex(int Index);		//	returns a pointer to the bone at this index in this bone. eg. 2 would be the 1st child of the 1st child bone, or the 2nd child bone
	GAssetRef			SetBoneRef(GAssetRef NewRef);	//	sets this bones ref, and checks the skeleton for duplicates

protected:
	Bool				DeleteBone(GBone* pBone);		//	deletes this child bone
	Bool				Load(int& BonesRemaining, GBinaryData& Data);
	void				Save(int& BoneCounter, GBinaryData& Data);
	void				DebugDraw(GBone* pParentBone, int& BoneIndex, const GAssetRef& HighlightBone=GAssetRef_Invalid);
	void				DebugDraw(GBone* pParentBone, float3 ParentBonePos, float3 ParentRotatedPos, int& BoneIndex, GList<GMatrix>& TransformRotations, const GAssetRef& HighlightBone);
	void				Reset();						//	removes children and resets bone
	void				DeleteChildren();				//	delete child bones
	int					CountBones(Bool Recurseup=FALSE);	//	updates our bone counter. if Recurse up is true, parents recalc bone counts too
	void				GetBonePosition(GList<float3>& BonePositions, float3 ParentPos, int& BoneIndex );	//	recursive func to gather bone positions
	void				GetBoneTransform(GList<GMatrix>& BoneFinal, int& BoneIndex, int ParentIndex, GList<GMatrix>& MainRotations, GList<GMatrix>* pModifiedRotations, u32& BonesNeedUpdating, Bool Recurse, const u32& ValidAnimRotations, const u32& ValidModifiedRotations );
	void				InitBoneMatrix();
	int					CalcIndex();					//	runs through skeleton to get index
	void				SetParentBoneMask(u32 CurrentParentBoneMask);	//	recursive function to setup bones' parent bones bitmask
	void				UpdateChildBoneMask();			//	goes up through its parents and adds to bitmask
};

//-------------------------------------------------------------------------
//	skeleton structure
//-------------------------------------------------------------------------
typedef struct 
{
	u8				TotalBones;	//	number of bones, cant be more than MAX_BONES
	u8				DataFlags;	//	flags of what data follows

} GSkeletonHeader;

//-------------------------------------------------------------------------
//	GSkeleton class controls heirachy of bones, stores and animates animations etc
//-------------------------------------------------------------------------
class GSkeleton : public GAsset
{
	friend GBone;
public:
	const static u32	g_Version;
	static float3		g_DebugColour;
	static float3		g_DebugHighlightColour;

public:
	GList<float3>		m_BoneColours;	//	debug colours for bones

private:
	GBone				m_RootBone;
	GList<GBone*>		m_BonePtrList;	//	quick access list to bones via indexes

public:
	GSkeleton();
	~GSkeleton()		{	};

	//	asset virtual
	virtual GAssetType	AssetType()						{	return GAssetSkeleton;	};
	virtual u32			Version()						{	return GSkeleton::g_Version;	};
	virtual Bool		Load(GBinaryData& Data);
	virtual Bool		Save(GBinaryData& Data);

	//
	void				DebugDraw(GList<GMatrix>* pTransformRotations, GAssetRef HighlightBone=GAssetRef_Invalid);	//	draws bones in the skeleton
	float3&				GetBoneColour(int BoneIndex=-1)	{	return (BoneIndex==-1||BoneIndex>=m_BoneColours.Size()) ? GSkeleton::g_DebugColour : m_BoneColours[BoneIndex];	};

	void				GenerateDebugColours(Bool GenerateForAll);	//	makes new debug colours for our bones
	void				GetBonePositions(GList<float3>& BonePositions);
	void				GetBoneTransform(GList<GMatrix>& BoneFinalRotations, GList<GMatrix>& MainRotations, GList<GMatrix>* pModifiedRotations, u32 BonesNeedUpdating, const u32& ValidAnimRotations, const u32& ValidModifiedRotations );

	inline int			BoneCount()						{	return m_RootBone.BoneCount();	};
	GBone*				FindBone(GAssetRef BoneRef)		{	int Index=0;	return m_RootBone.FindBone( BoneRef, Index );	};	//	search through children for this bone
	GBone*				RootBone()						{	return &m_RootBone;	};
	Bool				DeleteBone(GAssetRef BoneRef);	//	delete a bone from the skeleton
	Bool				ChangeBoneRef(GAssetRef BoneRef, GAssetRef NewRef);	//	change the reference for a bone

	inline Bool			ValidBoneIndex(int BoneIndex)	{	return ( BoneIndex >= 0 && BoneIndex < m_RootBone.BoneCount() );	};
//	GBone*				GetBoneIndex(int BoneIndex)		{	return m_RootBone.GetBoneIndex( BoneIndex );	};	//	get the bone at this index
	inline GBone*		GetBoneIndex(int BoneIndex);	//	get the bone at this index
	int					GetBoneIndex(GAssetRef BoneRef);
	int					GetBoneIndex(GBone* pBone);		//	find the index for this bone

protected:
	void				OnBoneAdded(GBone* pBone);		//	callback when a bone is added
	void				InitBoneMatrixes();				//	
	void				RebuildBonePtrList();
	inline void			SetupBoneParentMasks()			{	m_RootBone.SetParentBoneMask(0x0);	};	//	setup each bone's parent bone mask

};


//-------------------------------------------------------------------------
//	skeleton anim structure
//-------------------------------------------------------------------------
typedef struct 
{
	u16			KeyFrames;
	u16			BoneCount;
	GAssetRef	SkinRef;

} GSkeletonAnimHeader;


typedef struct 
{
	float		FrameNumber;
	u32			ValidRotMask;
	float3		RootOffset;

} GSkeletonAnimKeyframeHeader;


//-------------------------------------------------------------------------
//	keyframe of anim
//-------------------------------------------------------------------------
class GAnimKeyFrame : public GMatrixRotations
{
public:
	GAnimKeyFrame();

	float				m_FrameNumber;					//	which frame does this keyframe represent(whole numbers only)
	u32					m_ValidRotMask;					//	bitmask of which rotations are applied(non identity)
	float3				m_RootOffset;					//	root bone's offset from first frame. this is absolute, not relative to previous frames

//	inline const u32	ValidRotMask()					{	return m_ValidRotMask;	};
	inline const u32	ValidRotMask()					{	return 0xffffffff;	};
	void				UpdateValidRotMask();			//	update bitmask of which rotations need to be applied
	void				SetAllIdentity();				//	reset all rotations
	void				UpdateBoneCount(int Bonecount);	//	update num of matrixes

	virtual void		Copy(GAnimKeyFrame& Keyframe);	//	copies frame info AND matrixes

	inline Bool			operator==(GAnimKeyFrame& k)	{	return floorf(m_FrameNumber) == floorf( k.m_FrameNumber );	};
};

class GAnimKeyFrameList	: public GList<GAnimKeyFrame*>
{
public:
	void		DeleteAll();
	Bool		DeleteAt(int Index);
};

//-------------------------------------------------------------------------
//	GSkeletonAnim contains animation data
//-------------------------------------------------------------------------
class GSkeletonAnim : public GAsset
{
public:
	const static u32		g_Version;

public:
	GAssetRef				m_SkinRef;		//	main skin(and inside, skeleton) associated with for this anim
	GAnimKeyFrame			m_FirstFrame;	//	base keyframe
	GAnimKeyFrameList		m_Keyframes;	//	keyframes
	int						m_BoneCount;	//	number of matrixes matches this number of bones

public:
	GSkeletonAnim();
	~GSkeletonAnim();

	//	asset virtual
	virtual GAssetType		AssetType()						{	return GAssetSkeletonAnim;	};
	virtual u32				Version()						{	return GSkeletonAnim::g_Version;	};
	virtual Bool			Load(GBinaryData& Data);
	virtual Bool			Save(GBinaryData& Data);
	
	void					SetBoneCount(int BoneCount);	//	resizes keyframe data to this bone count
	GSkin*					GetSkin();
	
	GAnimKeyFrame*			AddKeyframe(float Frame);
	Bool					RemoveKeyframe(float Frame);
	GAnimKeyFrame*			GetKeyframe(float Frame);
	int						GetKeyframeIndexFromFrame(float Frame);

	Bool					RemoveKeyframeIndex(int Index);
	GAnimKeyFrame*			GetKeyframeIndex(int KeyframeIndex);

	void					GetRotations(float Frame, GMatrixRotations& Rotations, u32& ValidRotMask, float PreviousFrame, float3& ExtractedMovement );	//	extract rotations from anim on this frame (calculates interpolated frames too)

	inline int				KeyframeCount()					{	return m_Keyframes.Size();	};			//	doesnt include base keyframe..
	inline float			LastKeyframe()					{	return (m_Keyframes.Size() == 0) ? 0.f : m_Keyframes[ m_Keyframes.LastIndex() ]->m_FrameNumber;	};

	Bool					Copy(GSkeletonAnim* pAnim,GAssetRef NewRef=GAssetRef_Invalid);		//	make this anim a copy of pAnim
};




//	Declarations
//------------------------------------------------
Bool CompareIsLess(GAnimKeyFrame& a, GAnimKeyFrame& b);
Bool CompareIsLess(GAnimKeyFrame*& a, GAnimKeyFrame*& b);





//	Inline Definitions
//-------------------------------------------------
inline int GBone::CalcIndex()						
{	
	m_Index = m_pSkeleton->GetBoneIndex(this);
	return m_Index;
}

inline GBone* GSkeleton::GetBoneIndex(int BoneIndex)		
{	
	if ( m_BonePtrList.Size() == 0 )
		RebuildBonePtrList();

	return m_BonePtrList[BoneIndex];
}





#endif

