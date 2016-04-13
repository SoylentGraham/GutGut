/*------------------------------------------------

  GSkeleton.cpp

	Skeleton implementation. Bone control, animation


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GSkeleton.h"
#include "GDisplay.h"
#include "GApp.h"
#include "GMesh.h"
#include "GAssetList.h"
#include "GBinaryData.h"


//	globals
//------------------------------------------------
float3			GSkeleton::g_DebugColour =			float4( 1.0f, 0.5f, 0.0f, 1.f );	//	orange
float3			GSkeleton::g_DebugHighlightColour =	float4( 1.0f, 1.0f, 0.0f, 1.f );	//	yellow
const u32		GSkeleton::g_Version		= 0x11110002;
const u32		GSkeletonAnim::g_Version	= 0x11220003;
const float		g_BoneDebugRad = 0.1f;
#define			FRAME_SNAP	NEAR_ZERO	//	if we're this close to a start or end keyframe, jump to that keyframe rather than interpolating between



//	Definitions
//------------------------------------------------

Bool CompareIsLess(GAnimKeyFrame& a, GAnimKeyFrame& b)
{
	return a.m_FrameNumber < b.m_FrameNumber;
}

inline Bool CompareIsLess(GAnimKeyFrame*& a, GAnimKeyFrame*& b)
{
	return CompareIsLess( *a, *b );
}


//-------------------------------------------------------------------------
//	blend these rotations with the rotations passed in
//-------------------------------------------------------------------------
GRotationBlendResult GMatrixRotations::Blend(GMatrixRotations& BlendRotations, float BlendInterp)
{
	//	need to be the same size
	if ( Size() != BlendRotations.Size() )
	{
		GDebug_Break("Attempting to blend two sets of rotations with differing number of rotations\n");
		return GRotationBlendResult_Original;
	}

	//	interp is very low, just use current rotations
	if ( BlendInterp < NEAR_ZERO )
	{
		return GRotationBlendResult_Original;
	}

	//	interp is very high, just use new rotations
	if ( BlendInterp > 1.f - NEAR_ZERO )
	{
		Copy( BlendRotations );
		return GRotationBlendResult_New;
	}

	//	convert rotations to quaternions, and slerp between them
	for ( int r=0;	r<Size();	r++ )
	{
		GQuaternion QStart;
		GQuaternion QEnd;
		MatrixToQuaternion( ElementAt(r), QStart );
		MatrixToQuaternion( BlendRotations[r], QEnd );
		
		GQuaternion Rot;
		Rot = InterpQ( QStart, QEnd, BlendInterp );

		ElementAt(r).SetRotation( Rot );
	}

	return GRotationBlendResult_Merged;
}


//-------------------------------------------------------------------------
//	constructor
//-------------------------------------------------------------------------
GBone::GBone()
{
	m_pSkeleton	= NULL;
	Reset();
}

//-------------------------------------------------------------------------
//	removes children and resets bone
//-------------------------------------------------------------------------
void GBone::Reset()
{
	DeleteChildren();
	m_Relative.SetIdentity();
	m_Absolute.SetIdentity();

	m_Offset		= float3( 0, 0, 0 );
	m_pParent		= NULL;
	m_BoneRef		= GAssetRef_Invalid;
	m_BoneCount		= -1;
	m_Index			= -1;
	m_ParentBones	= 0xffffffff;
	m_ChildBones	= 0xffffffff;
}

//-------------------------------------------------------------------------
//	destroys child bones
//-------------------------------------------------------------------------
void GBone::DeleteChildren()
{
	//	
	for ( int i=0;	i<m_Children.Size();	i++ )
	{
		GDelete( m_Children[i] );
	}

	m_Children.Empty();
}


//-------------------------------------------------------------------------
//	
//-------------------------------------------------------------------------
void GBone::DebugDraw(GBone* pParentBone, int& BoneIndex, const GAssetRef& HighlightBone)
{
	//	draw link from parent bone to this
	if ( pParentBone) 
	{
		glBegin( GL_LINES );
			
			glVertex3f( 0, 0, 0 );
			glVertex3fv( m_Offset );
/*
			glVertex3f( -g_BoneDebugRad, 0, 0 );
			glVertex3fv( m_Offset );
			
			glVertex3f( g_BoneDebugRad, 0, 0 );
			glVertex3fv( m_Offset );
			
			glVertex3f( 0, 0, -g_BoneDebugRad );
			glVertex3fv( m_Offset );
			
			glVertex3f( 0, 0, g_BoneDebugRad );
			glVertex3fv( m_Offset );
*/
		glEnd();
	}

	float3 Colour;

	//	set to own colour if none highlighted
	if ( HighlightBone == GAssetRef_Invalid )
	{
		Colour = m_pSkeleton->GetBoneColour(BoneIndex);
	}
	else
	{
		if ( HighlightBone == m_BoneRef )
			Colour = GSkeleton::g_DebugHighlightColour;
		else
			Colour = GSkeleton::g_DebugColour;
	}

	glColor3fv( Colour );

	//	apply offset from parent
	g_Display->Translate( m_Offset, NULL );

	//	draw bone "node"
	{
		g_Display->PushScene();
		GDebugSphere Sphere;
		Sphere.Pos		= float3( 0,0,0 );
		Sphere.Colour	= Colour;
		Sphere.Radius	= ( HighlightBone == m_BoneRef ) ? g_BoneDebugRad * 1.1f : g_BoneDebugRad;
		Sphere.Draw();
		g_Display->PopScene();
	}

	//	increase index
	BoneIndex++;

	//	draw children
	for ( int i=0;	i<m_Children.Size();	i++ )
	{
		g_Display->PushScene();
		glColor3fv( Colour );
		m_Children[i]->DebugDraw( this, BoneIndex, HighlightBone );
		g_Display->PopScene();
	}
}


void GBone::DebugDraw(GBone* pParentBone, float3 ParentBonePos, float3 ParentRotatedPos, int& BoneIndex, GList<GMatrix>& TransformRotations, const GAssetRef& HighlightBone)
{
	float3 BonePos( ParentBonePos + m_Offset );
	float3 RotatedBonePos( BonePos );
	
	GMatrix& BoneAbsolute = m_Absolute;
	BoneAbsolute.InverseTranslateVect( RotatedBonePos );
	BoneAbsolute.InverseRotateVect( RotatedBonePos );
	
	GMatrix& BoneMatrix = TransformRotations[BoneIndex];
	BoneMatrix.TransformVector( RotatedBonePos );
	

	//	draw link from parent bone to this
	if ( pParentBone) 
	{
		glBegin( GL_LINES );
			
			glVertex3fv( ParentRotatedPos );
			glVertex3fv( RotatedBonePos );
/*
			glVertex3f( -g_BoneDebugRad, 0, 0 );
			glVertex3fv( m_Offset );
			
			glVertex3f( g_BoneDebugRad, 0, 0 );
			glVertex3fv( m_Offset );
			
			glVertex3f( 0, 0, -g_BoneDebugRad );
			glVertex3fv( m_Offset );
			
			glVertex3f( 0, 0, g_BoneDebugRad );
			glVertex3fv( m_Offset );
*/
		glEnd();
	}

	float3 Colour;

	//	set to own colour if none highlighted
	if ( HighlightBone == GAssetRef_Invalid )
	{
		Colour = m_pSkeleton->GetBoneColour(BoneIndex);
	}
	else
	{
		if ( HighlightBone == m_BoneRef )
			Colour = GSkeleton::g_DebugHighlightColour;
		else
			Colour = GSkeleton::g_DebugColour;
	}
	glColor3fv( Colour );

	//	draw bone "node"
	{
		g_Display->PushScene();
		GDebugSphere Sphere;
		Sphere.Pos		= RotatedBonePos;
		Sphere.Colour	= Colour;
		Sphere.Radius	= ( HighlightBone == m_BoneRef ) ? g_BoneDebugRad * 1.1f : g_BoneDebugRad;
		Sphere.Draw();
		g_Display->PopScene();
	}

	//	increase index
	BoneIndex++;

	//	draw children
	for ( int i=0;	i<m_Children.Size();	i++ )
	{
		glColor3fv( Colour );
		m_Children[i]->DebugDraw( this, BonePos, RotatedBonePos, BoneIndex, TransformRotations, HighlightBone );
	}
}


//-------------------------------------------------------------------------
//	create a new bone and add to children
//-------------------------------------------------------------------------
GBone* GBone::AddNewBone()
{
	//	create new
	GBone* pBone = new GBone;
	
	//	add
	if ( !AddBone( pBone ) )
	{
		//	failed to add bone, delete it
		GDelete( pBone );
		return NULL;
	}

	return pBone;
}


//-------------------------------------------------------------------------
//	add this bone to children
//-------------------------------------------------------------------------
GBone* GBone::AddBone(GBone* pBone)
{
	//	check args
	if ( !pBone )
	{
		GDebug_Break("NULL bone specified\n");
		return NULL;
	}
	
	//	check if we have all our bones already
	if ( m_pSkeleton->BoneCount() == MAX_BONES )
	{
		GDebug::Print("Failed to add bone to skeleton, already at max number of bones (%d)\n", MAX_BONES );
		return NULL;
	}

	//	check bone doesnt already exist in children
	if ( m_Children.Exists( pBone ) )
	{
		GDebug_Break("Bone is already a child\n");
		return NULL;
	}

	//	set parent skeleton
	pBone->m_pSkeleton = m_pSkeleton;

	//	check ref
	while ( m_pSkeleton->FindBone( pBone->m_BoneRef ) )
	{
		IncrementAssetRef( pBone->m_BoneRef );
	}

	//	set parent and add to child list
	pBone->m_pParent = this;
	m_Children.Add( pBone );

	//	update bone counters
	CountBones(TRUE);

	//	notify skeleton of new bone
	m_pSkeleton->OnBoneAdded(pBone);

	return pBone;
}

//-------------------------------------------------------------------------
//	recalc number of bones
//-------------------------------------------------------------------------
int GBone::CountBones(Bool RecurseUp)
{
	//	start with this
	m_BoneCount = 1;

	//	add child counter
	for ( int i=0;	i<m_Children.Size();	i++ )
	{
		m_BoneCount += m_Children[i]->BoneCount();
	}

	if ( RecurseUp && m_pParent )
	{
		m_pParent->CountBones( TRUE );
	}

	return m_BoneCount;
}


//-------------------------------------------------------------------------
//	recursivly load bones from data
//-------------------------------------------------------------------------
Bool GBone::Load(int& BonesRemaining, GBinaryData& Data)
{
	//	processing too many bones
	if ( BonesRemaining <= 0 )
	{
		GDebug::Print("Trying to load too many bones\n");
		return FALSE;
	}

	//	load header
	GBoneHeader Header;
	if ( !Data.Read( &Header, GDataSizeOf(GBoneHeader), "Bone header" ) )
		return FALSE;

	m_Offset	= Header.Offset;
	m_BoneRef	= Header.BoneRef;

	//	processed this bone
	BonesRemaining --;

	//	load child bone one at a time
	for ( int i=0;	i<Header.ChildBones;	i++ )
	{
		GBone* pBone = AddNewBone();
		if ( !pBone )
			continue;

		if ( ! pBone->Load( BonesRemaining, Data ) )
			return FALSE;
	}

	return TRUE;
}


//-------------------------------------------------------------------------
//	recursivly save bones
//-------------------------------------------------------------------------
void GBone::Save(int& BoneCounter, GBinaryData& Data)
{
	//	save header
	GBoneHeader Header;
	Header.ChildBones	= m_Children.Size();
	Header.BoneRef		= m_BoneRef;
	Header.Offset		= m_Offset;

	Data.Write( &Header, GDataSizeOf(GBoneHeader) );

//	GDebug::Print("Saved bone %s %x\n", GAsset::RefToName( m_BoneRef ), m_BoneRef );

	//	processed this bone
	BoneCounter++;

	//	save child bones one at a time
	for ( int i=0;	i<m_Children.Size();	i++ )
	{
		m_Children[i]->Save( BoneCounter, Data );
	}
}


//-------------------------------------------------------------------------
//	recursivly finds a bone with this ref
//-------------------------------------------------------------------------
GBone* GBone::FindBone(GAssetRef BoneRef, int& Index)
{
	//	this is the bone we're looking for
	if ( m_BoneRef == BoneRef )
		return this;

	Index++;

	//	check children
	for ( int i=0;	i<m_Children.Size();	i++ )
	{
		GBone* pBone = m_Children[i]->FindBone(BoneRef,Index);
		if ( pBone )
			return pBone;
	}

	return NULL;
}

//-------------------------------------------------------------------------
//	recursivly finds a bone based on its pointer. Index is incremented as we
//	search for the bone
//-------------------------------------------------------------------------
Bool GBone::FindBone(GBone* pBone,int& Index)
{
	//	found our bone!
	if ( this == pBone )
		return TRUE;

	//	not this bone, increment index
	Index++;

	//	search children
	for ( int i=0;	i<m_Children.Size();	i++ )
	{
		if ( m_Children[i]->FindBone( pBone, Index ) )
			return TRUE;
	}

	return FALSE;
}

//-------------------------------------------------------------------------
//	deletes the specified child bone
//-------------------------------------------------------------------------
Bool GBone::DeleteBone(GBone* pBone)
{
	int BoneIndex = m_Children.FindIndex( pBone );
	
	if ( BoneIndex == -1 )
	{
		GDebug_Break("Tried to delete bone from bone that isnt a child bone\n");
		return FALSE;
	}

	//	delete
	GDelete( m_Children[BoneIndex] );

	//	remove from list
	m_Children.RemoveAt( BoneIndex );

	return TRUE;
}

//-------------------------------------------------------------------------
//	move this bone and all subbones' offset
//-------------------------------------------------------------------------
void GBone::MoveOffset(float3& Change)
{
	//	change offset
	m_Offset += Change;

	//	reinitialise matrixes
	InitBoneMatrix();
}


void GBone::SetOffset(float3& Offset)
{
	//	set offset
	m_Offset = Offset;

	//	reinitialise matrixes
	InitBoneMatrix();
}

	
GAssetRef GBone::SetBoneRef(GAssetRef NewRef)
{
	//	check for duplicate ref
	while ( m_pSkeleton->FindBone( NewRef ) )
	{
		IncrementAssetRef( NewRef );
	}

	//	set and return new ref
	m_BoneRef = NewRef;
	return NewRef;
}



//-------------------------------------------------------------------------
//	debug print out the tree from this bone
//-------------------------------------------------------------------------
void GBone::DebugBoneTree(int Recursion)
{
	int i;

	//	print this item
	for ( i=0;	i<Recursion;	i++ )
	{
		GDebug::Print("-");
	}
	GDebug::Print("* %s\n", GAsset::RefToName(m_BoneRef) );

	for ( i=0;	i<m_Children.Size();	i++ )
	{
		m_Children[i]->DebugBoneTree( Recursion + 1 );
	}
}

//-------------------------------------------------------------------------
//	returns a pointer to the bone at this index in this bone. eg. 2 would be the 1st child of the 1st child bone, or the 2nd child bone
//-------------------------------------------------------------------------
GBone* GBone::GetBoneIndex(int Index)
{
	//	index 0 is this bone
	if ( Index == 0 )
		return this;

	//	if the index is out of range of the number of children (all bone count - this)
	if ( Index > BoneCount() -1 )
	{
		GDebug_Break("Invalid bone index request %d\n", Index );
		return NULL;
	}

	//	index 1 will always be the first child index
	if ( Index == 1 )
		return m_Children[0];

	//	work out which child this bone comes under
	int RunningIndex = 1;
	for ( int i=0;	i<m_Children.Size();	i++ )
	{
		int ChildBoneCount = m_Children[i]->BoneCount();
		
		//	index is in this child bone
		if ( Index < RunningIndex + ChildBoneCount )
		{
			return m_Children[i]->GetBoneIndex( Index - RunningIndex );
		}
		
		//	increase running total before checking next child
		RunningIndex += ChildBoneCount;
	}

	//	not found...?
	GDebug_Break("Bone index %d not found in %d bones\n", BoneCount() - 1 );
	return NULL;
}

//-------------------------------------------------------------------------
//	recursive func to gather bone positions
//-------------------------------------------------------------------------
void GBone::GetBonePosition(GList<float3>& BonePositions, float3 ParentPos, int& BoneIndex )
{
	//	update pos
	ParentPos += m_Offset;
	BonePositions[BoneIndex] = ParentPos;

	//	increase index
	BoneIndex++;

	//	recurse to children
	for ( int c=0;	c<m_Children.Size();	c++ )
	{
		m_Children[c]->GetBonePosition( BonePositions, ParentPos, BoneIndex );
	}
}


void GBone::InitBoneMatrix()
{
	GMatrix& Relative = m_Relative;
	GMatrix& Absolute = m_Absolute;
	Relative.SetIdentity();
	Absolute.SetIdentity();

	//	calc local/relative
	Relative.SetTranslate( m_Offset );

	//	calc absolute
	if ( m_pParent )
	{
		Absolute = m_pParent->m_Absolute;
		Absolute *= Relative;
	}
	else
	{
		Absolute = Relative;
	}

	//	recurse through children
	for ( int c=0;	c<m_Children.Size();	c++ )
	{
		m_Children[c]->InitBoneMatrix();
	}
}



void GBone::GetBoneTransform(GList<GMatrix>& BoneFinal, int& BoneIndex, int ParentIndex, GList<GMatrix>& MainRotations, GList<GMatrix>* pModifiedRotations, u32& BonesNeedUpdating, Bool Recurse, const u32& ValidAnimRotations, const u32& ValidModifiedRotations )
{
	//	no more bones need updating
	if ( !BonesNeedUpdating )
		return;

	//	does this bone need updating?
	if ( BonesNeedUpdating & (1<<BoneIndex) )
	{
		GMatrix& Final = BoneFinal[BoneIndex];

		//	anim, intially same as absolute
		//Final = m_Absolute;

		u32 AnimMatrixValid = ValidAnimRotations & (1<<BoneIndex);
		u32 ModifiedMatrixValid = ValidModifiedRotations & (1<<BoneIndex);
		if ( ! pModifiedRotations )
			ModifiedMatrixValid = 0x0;

		//	only apply if both matrixes are valid
		if ( AnimMatrixValid || ModifiedMatrixValid )
		{
			//	animated final transformation. transform is already interpolated
			GMatrix Transform;

			if ( AnimMatrixValid )
				Transform = MainRotations[BoneIndex];
			else
				Transform.SetIdentity();

			if ( ModifiedMatrixValid )
				Transform *= pModifiedRotations->ElementAt(BoneIndex);

			//	calc final this-frame matrix
			GMatrix RelativeFinal( m_Relative );
			RelativeFinal *= Transform;	//	postmultiply

			if ( ParentIndex == -1 )
			{
				Final = RelativeFinal;
			}
			else
			{
				Final = BoneFinal[ParentIndex];
				Final *= RelativeFinal;
			}
		}
		else
		{
			Final = m_Relative;
		}

		//	this bone doesnt need updating
		BonesNeedUpdating &= ~(1<<BoneIndex);
	}

	//	increase index
	int ThisBoneIndex = BoneIndex;
	BoneIndex++;

	//	recurse through children
	if ( Recurse )
	{
		for ( int c=0;	c<m_Children.Size();	c++ )
		{
			//	no more bones need updating
			if ( !BonesNeedUpdating )
				return;
			
			//	recurse
			m_Children[c]->GetBoneTransform( BoneFinal, BoneIndex, ThisBoneIndex, MainRotations, pModifiedRotations, BonesNeedUpdating, Recurse, ValidAnimRotations, ValidModifiedRotations );
		}
	}

}


//-------------------------------------------------------------------------
//	recursive function to setup bones' parent bones bitmask
//-------------------------------------------------------------------------
void GBone::SetParentBoneMask(u32 CurrentParentBoneMask)
{
	//	set this bone's parent bone mask to the current mask being passed down
	m_ParentBones = CurrentParentBoneMask;

	//	add our index to the mask and set children's parent bone mask
	CurrentParentBoneMask |= 1<<GetIndex();

	for ( int c=0;	c<m_Children.Size();	c++ )
		m_Children[c]->SetParentBoneMask( CurrentParentBoneMask );
}


//-------------------------------------------------------------------------
//	update all the parents of this bone to include this bone index as a child of it
//-------------------------------------------------------------------------
void GBone::UpdateChildBoneMask()
{
	int ThisIndexBit = 1<<GetIndex();

	//	go through all the parents of this bone
	GBone* pParentBone = ParentBone();
	while ( pParentBone )
	{
		//	add this as a child of this parent
		pParentBone->m_ChildBones |= ThisIndexBit;

		//	next parent up
		pParentBone = pParentBone->ParentBone();
	}
}

//-------------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------------
GSkeleton::GSkeleton()
{
	//	set root bone's skeleton owner
	m_RootBone.m_pSkeleton = this;
}


//-------------------------------------------------------------------------
//	Load skeleton
//-------------------------------------------------------------------------
Bool GSkeleton::Load(GBinaryData& Data)
{
	//	read skeleton header
	GSkeletonHeader Header;
	if ( !Data.Read( &Header, GDataSizeOf(GSkeletonHeader), "Skeleton header" ) )
		return FALSE;

	int BoneCounter = Header.TotalBones;

	if ( BoneCounter > MAX_BONES )
	{
		GDebug_Print("Number of bones specified in skeleton(%d) is greater than max (%d)\n", BoneCounter, MAX_BONES );
		return FALSE;
	}

	//	load in all bones (recursive)
	if ( !m_RootBone.Load( BoneCounter, Data ) )
		return FALSE;

	//	load in additional data
	if ( Header.DataFlags & GSkeletonDataFlags::DebugColours )
	{
		m_BoneColours.Resize( Header.TotalBones );

		/*
		//	check remaining amount of data
		if ( DataSize-DataRead < ColourDataLength )
		{
			GDebug_Break("Not enough data for skeleton debug colours\n");
			m_BoneColours.Resize(0);
			return FALSE;
		}

		memcpy( m_BoneColours.Data(), pData+DataRead, ColourDataLength );
		DataRead += ColourDataLength;		
		*/
		if ( !Data.Read( m_BoneColours.Data(), m_BoneColours.DataSize(), "Skeleton bone colours" ) )
			return FALSE;
	}

	//	initialise matrixes
	InitBoneMatrixes();
	
	return TRUE;
}

//-------------------------------------------------------------------------
//	save skeleton
//-------------------------------------------------------------------------
Bool GSkeleton::Save(GBinaryData& Data)
{
	//	will have to build the bone data first to get the bone counter
	int BoneCounter = 0;
	GBinaryData BoneData;
	m_RootBone.Save( BoneCounter, BoneData );

	//	save Header
	GSkeletonHeader Header;
	Header.TotalBones = BoneCounter;
	Header.DataFlags = 0x0;

	//	are we going to add debug colour info?
	if ( m_BoneColours.Size() == BoneCounter )
		Header.DataFlags |= GSkeletonDataFlags::DebugColours;

	Data.Write( &Header, GDataSizeOf(GSkeletonHeader) );

	//	save bone data
	Data.Write( BoneData );

	//	save debug colours
	if ( Header.DataFlags & GSkeletonDataFlags::DebugColours )
	{
		Data.Write( m_BoneColours.Data(), m_BoneColours.DataSize() );
	}

	return TRUE;
}




//-------------------------------------------------------------------------
//	draws the bones in the skeleton
//-------------------------------------------------------------------------
void GSkeleton::DebugDraw(GList<GMatrix>* pTransformRotations, GAssetRef HighlightBone)
{
	g_Display->PushScene();

	//	turn off depth testing
	glDisable( GL_DEPTH_TEST );

	//	draw bones from the root
	int BoneIndex = 0;

	if ( pTransformRotations )
	{
		m_RootBone.DebugDraw( NULL, float3(0,0,0), float3(0,0,0), BoneIndex, *pTransformRotations,  HighlightBone );
	}
	else
	{
		m_RootBone.DebugDraw( NULL, BoneIndex, HighlightBone );
	}

	g_Display->PopScene();
}


//-------------------------------------------------------------------------
//	deletes this bone from this skeleton
//-------------------------------------------------------------------------
Bool GSkeleton::DeleteBone(GAssetRef BoneRef)
{
	GBone* pBone = FindBone( BoneRef );
	if ( !pBone )
	{
		GDebug_Break("Failed to find bone \"%s\" to delete\n", GAsset::RefToName( BoneRef ) );
		return FALSE;
	}

	//	cant delete root bone
	if ( pBone == RootBone() )
	{
		GDebug_Print("Cannot delete root bone\n");
		return FALSE;
	}

	//	bone is missing parent
	if ( !pBone->m_pParent )
	{
		GDebug_Break("Bone is missing parent whilst trying to delete it\n");
		return FALSE;
	}

	return pBone->m_pParent->DeleteBone( pBone );
}


//-------------------------------------------------------------------------
//	changes the ref on a bone
//-------------------------------------------------------------------------
Bool GSkeleton::ChangeBoneRef(GAssetRef BoneRef, GAssetRef NewRef)
{
	//	find original
	GBone* pBone = FindBone( BoneRef );
	if ( !pBone )
		return FALSE;

	//	make sure it doesnt already exist
	if ( FindBone( NewRef ) )
		return FALSE;

	//	change
	pBone->m_BoneRef = NewRef;

	return TRUE;
}

//-------------------------------------------------------------------------
//	find the index in this skeleton for this bone
//-------------------------------------------------------------------------
int GSkeleton::GetBoneIndex(GBone* pBone)
{
	int Index = 0;
	if ( !m_RootBone.FindBone( pBone, Index ) )
		return -1;

	return Index;
}


int GSkeleton::GetBoneIndex(GAssetRef BoneRef)
{
	int Index = 0;
	if ( !m_RootBone.FindBone( BoneRef, Index ) )
		return -1;

	return Index;
}



//-------------------------------------------------------------------------
//	generate new debug colours for all bones. if !GenerateForAll, only missing 
//	colours are made up
//-------------------------------------------------------------------------
void GSkeleton::GenerateDebugColours(Bool GenerateForAll)
{
	int CurrentColourCount = m_BoneColours.Size();
	m_BoneColours.Resize( BoneCount() );

	int FirstBone = GenerateForAll ? 0 : CurrentColourCount;
	
	for ( int b=FirstBone;	b<m_BoneColours.Size();	b++ )
	{
		m_BoneColours[b] = GetDebugColour(b,GDebugColourBase_Any);

		//	need to make sure all bone colours are quite bright
		m_BoneColours[b] *= float3(1.6f,1.6f,1.6f);
	}
}


//-------------------------------------------------------------------------
//	call back when a new bone is added to the skeleton
//-------------------------------------------------------------------------
void GSkeleton::OnBoneAdded(GBone* pBone)
{
	int BoneIndex = pBone->GetIndex();

	//	add new colour if we have a colour for all the others
	if ( m_BoneColours.Size() > 0 )
	{
		GenerateDebugColours(FALSE);
	}

	//	generate new bone ptr list
	RebuildBonePtrList();
}

//-------------------------------------------------------------------------
//	enumurate bone positions into list
//-------------------------------------------------------------------------
void GSkeleton::GetBonePositions( GList<float3>& BonePositions )
{
	//	resize list
	BonePositions.Resize( BoneCount() );

	//	recurse through bones
	int BoneIndex = 0;
	m_RootBone.GetBonePosition( BonePositions, float3(0,0,0), BoneIndex );
}

//-------------------------------------------------------------------------
//	
//-------------------------------------------------------------------------
void GSkeleton::GetBoneTransform(GList<GMatrix>& BoneFinal, GList<GMatrix>& MainRotations, GList<GMatrix>* pModifiedRotations, u32 BonesNeedUpdating, const u32& ValidAnimRotations, const u32& ValidModifiedRotations )
{
	//#define RECURSE_THROUGH_BONES	//	if defined, we process the bones recursivly, otherwise through the list

	int BoneSize = BoneCount();
	int BoneIndex = 0;
	BoneFinal.Resize( BoneSize );

	#ifdef RECURSE_THROUGH_BONES
	{
		//	recurse through bones
		m_RootBone.GetBoneTransform( BoneFinal, BoneIndex, -1, MainRotations, pModifiedRotations, BonesNeedUpdating, TRUE );
	}
	#else
	{
		//	run through bone list
		for ( int b=0;	b<BoneSize;	b++ )
		{
			if ( BonesNeedUpdating & (1<<b) )
			{
				GBone* pBone = GetBoneIndex(b);
				BoneIndex = b;
				pBone->GetBoneTransform( BoneFinal, BoneIndex, pBone->GetParentIndex(), MainRotations, pModifiedRotations, BonesNeedUpdating, FALSE, ValidAnimRotations, ValidModifiedRotations );

				//	this bone doesnt need updating anymore
				BonesNeedUpdating &= ~(1<<b);

				if ( BonesNeedUpdating == 0x0 )
					break;
			}
		}

	}
	#endif
}


void GSkeleton::InitBoneMatrixes()
{
	m_RootBone.InitBoneMatrix();
}


//-------------------------------------------------------------------------
//	recreates the list of bone pointers in the skeleton, bone indexes are updated here
//	so we need to update info on bones that use bone indexes
//-------------------------------------------------------------------------
void GSkeleton::RebuildBonePtrList()
{
	int i;

	m_BonePtrList.Empty();
	m_BonePtrList.Resize( BoneCount() );

	for ( i=0;	i<BoneCount();	i++ )
	{
		//	update pointer
		m_BonePtrList[i] = RootBone()->GetBoneIndex(i);

		//	update bone's index
		m_BonePtrList[i]->m_Index = i;
	}

	//	reset all bone's parent masks
	SetupBoneParentMasks();

	//	need to reset all the child bone masks before updating them
	for ( i=0;	i<BoneCount();	i++ )
		m_BonePtrList[i]->m_ChildBones = 0x0;

	//	update the child masks
	for ( i=0;	i<BoneCount();	i++ )
		m_BonePtrList[i]->UpdateChildBoneMask();
}



GSkeletonAnim::GSkeletonAnim()
{
	m_SkinRef	= GAssetRef_Invalid;
	m_BoneCount		= 0;

	//	first frame frame number is always zero
	m_FirstFrame.m_FrameNumber = 0.f;
}

GSkeletonAnim::~GSkeletonAnim()
{
	//	cleanup
	m_Keyframes.DeleteAll();
}

Bool GSkeletonAnim::Load(GBinaryData& Data)
{
	int i,b;
	//#define CHECK_SIZE(size,err)	{	if ( DataSize < size )	{	GDebug_Break(err);	return FALSE;	}	}
	//#define UPDATE_READ(size)		{	pData += size;	DataSize -= size;	DataRead += size;	}
	//#define READ(dest,size,err)		{	CHECK_SIZE(size,err);	memcpy( dest, pData, size );	UPDATE_READ( size );	}

	//	get header
	GSkeletonAnimHeader Header;
	//READ( &Header, GDataSizeOf(GSkeletonAnimHeader), "Not enough data for header" );
	if ( ! Data.Read( &Header, GDataSizeOf(GSkeletonAnimHeader), "Skeleton anim header" ) )
		return FALSE;
	m_SkinRef = Header.SkinRef;

	//	alloc data for bones
	SetBoneCount( Header.BoneCount );

	//	read base keyframe
	for ( b=0;	b<Header.BoneCount;	b++ )
	{
		//READ( &m_FirstFrame[b], GDataSizeOf(GMatrix), "Failed to read matrix in first frame" );
		if ( !Data.Read( &m_FirstFrame[b], GDataSizeOf(GMatrix), "Skeleton anim root keyframe rotation" ) )
			return FALSE;
	}

	//	delete any current keyframes
	m_Keyframes.DeleteAll();

	//	alloc one by one
	for ( i=0;	i<Header.KeyFrames;	i++ )
	{
		GSkeletonAnimKeyframeHeader KeyframeHeader;
		//READ( &KeyframeHeader, GDataSizeOf(GSkeletonAnimKeyframeHeader), "Failed to read keyframe header in anim" );
		if ( !Data.Read ( &KeyframeHeader, GDataSizeOf(GSkeletonAnimKeyframeHeader), "Skeleton anim keyframe header" ) )
			return FALSE;
		
		GAnimKeyFrame* pKeyframe = AddKeyframe( floorf(KeyframeHeader.FrameNumber) );
		if ( !pKeyframe )
		{
			GDebug_Break("Failed to add keyframe\n");
		}

		//	copy header data to frame
		pKeyframe->m_RootOffset = KeyframeHeader.RootOffset;
		pKeyframe->m_ValidRotMask = KeyframeHeader.ValidRotMask;

		//	copy in matrixes
		for ( b=0;	b<m_BoneCount;	b++ )
		{
			//READ( &pKeyframe->ElementAt(b), GDataSizeOf(GMatrix),  "Failed to read quaternion in keyframe" );
			if ( !Data.Read( &pKeyframe->ElementAt(b), GDataSizeOf(GMatrix), "Skeleton anim keyframe matrix" ) )
				return FALSE;
		}
	}

//	#undef CHECK_SIZE
//	#undef UPDATE_READ
//	#undef READ

	return TRUE;
}

Bool GSkeletonAnim::Save(GBinaryData& SaveData)
{
	int i,b;
	
	//	add header
	GSkeletonAnimHeader Header;
	Header.BoneCount = m_BoneCount;
	Header.KeyFrames = m_Keyframes.Size();
	Header.SkinRef = m_SkinRef;
	SaveData.Write( &Header, GDataSizeOf(GSkeletonAnimHeader) );

	//	save base keyframe
	for ( b=0;	b<m_BoneCount;	b++ )
	{
		SaveData.Write( &m_FirstFrame[b], GDataSizeOf(GMatrix) );
	}

	//	save other keyframes
	for ( i=0;	i<m_Keyframes.Size();	i++ )
	{
		//	save header for keyframe
		GSkeletonAnimKeyframeHeader KeyframeHeader;
		KeyframeHeader.FrameNumber = m_Keyframes[i]->m_FrameNumber;
		KeyframeHeader.RootOffset = m_Keyframes[i]->m_RootOffset;
		KeyframeHeader.ValidRotMask = m_Keyframes[i]->m_ValidRotMask;

		SaveData.Write( &KeyframeHeader, GDataSizeOf(GSkeletonAnimKeyframeHeader) );

		//	save each matrix in keyframe
		for ( b=0;	b<m_BoneCount;	b++ )
		{
			SaveData.Write( &m_Keyframes[i]->ElementAt(b), GDataSizeOf(GMatrix) );
		}
	}

	return TRUE;
}

void GSkeletonAnim::SetBoneCount(int BoneCount)
{
	//	dont need to change
	if ( BoneCount == m_BoneCount )
		return;

	//	check amount
	if ( BoneCount >= MAX_BONES )
	{
		GDebug_Break("Tried to set bone count(%d) on skeleton anim to more than allowed (%d)\n", BoneCount, MAX_BONES );
	}

	int OldBoneCount = BoneCount;

	//	update bone count
	m_BoneCount = BoneCount;

	//	resize lists
	m_FirstFrame.UpdateBoneCount( m_BoneCount );

	for ( int k=0;	k<m_Keyframes.Size();	k++ )
	{
		m_Keyframes[k]->UpdateBoneCount( m_BoneCount );
	}
	
}

GAnimKeyFrame* GSkeletonAnim::AddKeyframe(float Frame)
{
	Frame = floorf(Frame);

	if ( Frame < NEAR_ZERO )
	{
		GDebug::Print("Can't add a keyframe at frame zero\n");
		return NULL;
	}

	//	find existing keyframe
	GAnimKeyFrame* pKeyframe = GetKeyframe(Frame);
	if ( pKeyframe )
	{
		GDebug_Print("Frame %3.3f already exists\n", Frame );
		return pKeyframe;
	}

	//	add keyframe in
	pKeyframe = new GAnimKeyFrame;
	pKeyframe->m_FrameNumber = Frame;

	//	initialise size and set all rotations to identity
	pKeyframe->UpdateBoneCount( m_BoneCount );
	m_Keyframes.Add( pKeyframe );

	//	resort keyframe list
	m_Keyframes.Sort();

	return pKeyframe;
}

//-------------------------------------------------------------------------
//	gets the keyframe index for a frame. if result is 0 it is the root keyframe, 
//	-1 is error
//	otherwise its an index+1 in the list
//-------------------------------------------------------------------------
int GSkeletonAnim::GetKeyframeIndexFromFrame(float Frame)
{
	//	invalid number
	if ( Frame < 0.f )
		return -1;

	//	root keyframe is at 0
	if ( Frame == 0.f )
		return 0;

	//	find index
	for ( int k=0;	k<m_Keyframes.Size();	k++ )
		if ( m_Keyframes[k]->m_FrameNumber == Frame )
			return k+1;

	//	not found
	return -1;
}

Bool GSkeletonAnim::RemoveKeyframe(float Frame)
{
	int Index = GetKeyframeIndexFromFrame( Frame );
	if ( Index == -1 )
	{
		GDebug_Break("Failed to find keyframe at %3.3f to remove\n", Frame );
		return FALSE;
	}
	RemoveKeyframeIndex( Index );
	return TRUE;
}


Bool GSkeletonAnim::RemoveKeyframeIndex(int Index)
{
	//	if trying to remove base keyframe(0), copy frame 1 and delete frame 1
	if ( Index == 0 )
	{
		if ( KeyframeCount() == 0 )
		{
			GDebug_Print("Cannot remove base keyframe if there are no other frames\n");
			return FALSE;
		}

		//	copy frame 1
		m_FirstFrame.Copy( *m_Keyframes[0] );

		//	need to reset keyframe to 0 for root keyframe
		m_FirstFrame.m_FrameNumber = 0.f;

		//	remove frame 1 (which is now the base frame)
		m_Keyframes.RemoveAt( 0 );

		return TRUE;
	}

	return m_Keyframes.DeleteAt( Index-1 );
}


GAnimKeyFrame* GSkeletonAnim::GetKeyframeIndex(int KeyframeIndex)
{
	if ( KeyframeIndex < 0 )
	{
		GDebug_Break("Invalid request for keyframe %d\n", KeyframeIndex );
		return NULL;
	}

	if ( KeyframeIndex == 0 )
	{
		return &m_FirstFrame;
	}

	return m_Keyframes[KeyframeIndex-1];
}

//-------------------------------------------------------------------------
//	returns an existing keyframe at this frame no if it exists
//-------------------------------------------------------------------------
GAnimKeyFrame* GSkeletonAnim::GetKeyframe(float Frame)
{
	//	very low frame return first frame (always frame#0)
	if ( Frame <= FRAME_SNAP )
		return &m_FirstFrame;

	//	find matching keyframe (or very close)
	for ( int k=0;	k<m_Keyframes.Size();	k++ )
	{
		float kframe = m_Keyframes[k]->m_FrameNumber;

		if ( fabsf( kframe - Frame ) <= FRAME_SNAP )
		{
			return m_Keyframes[k];
		}
	}

	return NULL;
}


void GSkeletonAnim::GetRotations( float Frame, GMatrixRotations& Rotations, u32& ValidRotMask, float PreviousFrame, float3& ExtractedMovement )
{
	//GDebug::Print("Getting rotations for frame %2.2f\n", Frame );
	
	//	find before and after keyframe indexes to base matrixes on
	int StartKeyframe = 0;
	int EndKeyframe = 0;

	for ( int i=0;	i<m_Keyframes.Size();	i++ )
	{
		//	find the last keyframe BEFORE this frame
		if ( m_Keyframes[i]->m_FrameNumber <= Frame )
			StartKeyframe = i+1;

		//	find the first keyframe AFTER our frame
		if ( m_Keyframes[i]->m_FrameNumber >= Frame )
		{
			EndKeyframe = i+1;
			
			//	must have found start *and* end
			break;
		}
	}

	//	grab keyframe
	GAnimKeyFrame* pStartKeyframe = GetKeyframeIndex(StartKeyframe);
	if ( !pStartKeyframe )
	{
		GDebug_Break("Invalid keyframe index %d\n",StartKeyframe);
		return;
	}

	//	get rotations
	GMatrixRotations& StartRotations = *pStartKeyframe;

	//	no rotations specified!
	if ( StartRotations.Size() == 0 )
	{
		//	copy originals
		pStartKeyframe->SetAllIdentity();
	}

	//	start and end keyframes are the same, just copy matrixes if theyre set
	if ( StartKeyframe == EndKeyframe )
	{
		Rotations.Copy( StartRotations );
		ValidRotMask = pStartKeyframe->ValidRotMask();
		return;
	}

	//	grab keyframe data
	GAnimKeyFrame* pEndKeyframe = GetKeyframeIndex(EndKeyframe);
	GMatrixRotations& EndRotations = *pEndKeyframe;

	//	get interpolation amount between keyframes
	float FrameLength = pEndKeyframe->m_FrameNumber - pStartKeyframe->m_FrameNumber;
	if ( FrameLength <= 0.f )
	{
		GDebug_Break("Invalid frame length between keyframes %d and %d for frame %3.3f\n", StartKeyframe, EndKeyframe, Frame );
		return;
	}
	
	//	get interploation
	float KeyframeIndexInterpolation = ( Frame - pStartKeyframe->m_FrameNumber) / FrameLength;

	//	blend from start keyframes to end keyframe
	Rotations.Copy( StartRotations );
	GRotationBlendResult BlendResult = Rotations.Blend( EndRotations, KeyframeIndexInterpolation );

	//	update other info depending on how rotations were merged
	switch ( BlendResult )
	{
		//	rotations were blended
		case GRotationBlendResult_Merged:
		{
			ValidRotMask = pStartKeyframe->ValidRotMask() | pEndKeyframe->ValidRotMask();
			//	interp extracted movement
			ExtractedMovement = Interp( pStartKeyframe->m_RootOffset, pEndKeyframe->m_RootOffset, KeyframeIndexInterpolation );
		}
		break;

		//	rotations were set to the start rotations
		case GRotationBlendResult_Original:
		{
			ValidRotMask = pStartKeyframe->ValidRotMask();
			ExtractedMovement = pStartKeyframe->m_RootOffset;
		}
		break;
	
		//	rotations were set to the end rotations
		case GRotationBlendResult_New:
		{
			ValidRotMask = pEndKeyframe->ValidRotMask();
			ExtractedMovement = pEndKeyframe->m_RootOffset;
		}
		break;
	}
}


GSkin* GSkeletonAnim::GetSkin()						
{	
	return GAssets::g_Skins.Find( m_SkinRef );	
}

//-------------------------------------------------------------------------
//	make this anim a copy of the specified anim
//-------------------------------------------------------------------------
Bool GSkeletonAnim::Copy(GSkeletonAnim* pAnim,GAssetRef NewRef)
{
	//	check params
	if ( !pAnim )
	{
		GDebug_Break("Missing source anim\n");
		return FALSE;
	}

	//	if no ref specified, generate it from the pAnim's ref
	if ( NewRef == GAssetRef_Invalid )
	{
		NewRef = pAnim->AssetRef();
		IncrementAssetRef( NewRef );
	}

	//	copy data, by saving and reloading the data! so simple!
	GBinaryData SavedAnim;
	if ( ! pAnim->Save( SavedAnim ) )
		return FALSE;

	int DataRead=0;
	if ( ! Load( SavedAnim ) )
		return FALSE;

	SetAssetRef( NewRef );

	return TRUE;
}




GAnimKeyFrame::GAnimKeyFrame()
{
	m_ValidRotMask	= 0x0;
	m_FrameNumber	= 0.f;
	m_RootOffset	= float3(0,0,0);
}

void GAnimKeyFrame::UpdateValidRotMask()
{
	m_ValidRotMask = 0x0;

	for ( int i=0;	i<Size();	i++ )
	{
		//if ( ElementAt(i).IsValid() )
		if ( !ElementAt(i).IsIdentity() )
		{
			m_ValidRotMask |= 1<<i;
		}
	}
}

void GAnimKeyFrame::UpdateBoneCount(int BoneCount)
{
	int OldCount = Size();
	Resize( BoneCount );

	//	reset mask length (if now less rotations)
	m_ValidRotMask &= (1<<BoneCount)-1;

	//	reset new rotations
	for ( int i=OldCount;	i<BoneCount;	i++ )
	{
		ElementAt(i).SetIdentity();
		m_ValidRotMask &= ~(1<<i);
	}
}

void GAnimKeyFrame::SetAllIdentity()
{
	//	reset matrixes
	for ( int i=0;	i<Size();	i++ )
	{
		ElementAt(i).SetIdentity();
	}

	//	update identity mask
	m_ValidRotMask = 0x0;
}


//-------------------------------------------------------------------------
//	copy frame info and matrixes from the param
//-------------------------------------------------------------------------
void GAnimKeyFrame::Copy(GAnimKeyFrame& Keyframe)
{
	m_FrameNumber = Keyframe.m_FrameNumber;
	m_ValidRotMask = Keyframe.m_ValidRotMask;

	//	do inherited list Copy to copy matrixes
	GList<GMatrix>::Copy( Keyframe );
}


void GAnimKeyFrameList::DeleteAll()
{
	//	delete each entry
	while ( Size() )
	{
		DeleteAt( 0 );
	}
}

Bool GAnimKeyFrameList::DeleteAt(int Index)
{
	GDebug_CheckIndex( Index, 0, Size() );

	GAnimKeyFrame* pKeyframe = ElementAt(Index);
	GDelete( pKeyframe );
	return RemoveAt(Index);
}

