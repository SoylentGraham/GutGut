/*------------------------------------------------

  GAsset Header file



-------------------------------------------------*/

#ifndef __GASSET__H_
#define __GASSET__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GList.h"
#include "GString.h"





//	Macros
//------------------------------------------------
#define GAssetRef_Invalid	((GAssetRef)0x0)	//	"     "
#define	AssetRefCharCount		5	//	n chars in a ref



//	Types
//------------------------------------------------
class GBinaryData;


enum GAssetType
{
	GAssetUnknown,
	
	GAssetMesh,
	GAssetTexture,
	GAssetObject,
	GAssetMap,
	GAssetSubMap,
	GAssetMapObject,
	GAssetSkeleton,
	GAssetSkin,
	GAssetSkeletonAnim,
	GAssetFont,

	GAssetMax,
};
extern const char* g_AssetTypeNames[ GAssetMax ];



/*
	individual reference to an asset
*/
typedef u32		GAssetRef;


/*
	Asset header appears in files for all assets (before their specific headers
*/
class GAssetHeader
{
public:
	u32			BlockSize;		//	entire size in bytes of this asset (does not include this GGutFileAssetBlockHeader)
	GAssetType	AssetType;		//	type of asset
	u32			AssetVersion;	//	version type of this asset (varies per asset type)
	GAssetRef	AssetRef;		//	unique reference for each asset

};










/*
	base asset type
*/
class GAsset
{
public:
	GAssetRef	m_AssetRef;				//	unique reference per asset


public:
	GAsset();
	~GAsset();


	//	asset info
	inline GAssetRef	AssetRef()										{	return m_AssetRef;	};
	inline void			SetAssetRef(u32 Ref)							{	m_AssetRef = Ref;	};
	inline void			SetAssetRef(char* pString)						{	m_AssetRef = NameToRef(pString);	};
	inline char*		GetAssetName()									{	return RefToName( m_AssetRef );	};
	const char*			AssetTypeName()									{	return g_AssetTypeNames[AssetType()];	};
	virtual GAssetType	AssetType()										{	return GAssetUnknown;	};
	virtual u32			Version()										{	return 0xffffffff;	};

	//	file io
	Bool				LoadAsset(GAssetHeader* pAssetHeader,GBinaryData& Data);		//	loads in generic asset stuff, do not overload
	virtual Bool		Load(GBinaryData& Data);										//	load directly from data. DataSize is how big pData is, and DataRead is the bytes read. returns successfull load
	virtual Bool		Save(GBinaryData& Data);														//	add all the data the asset needs to save into the array passed in

	virtual Bool		Upload()										{	return TRUE;	};			//	load to hardware
	virtual Bool		Deload()										{	return TRUE;	};			//	unload from hardware

public:
	//	static
	static char*		RefToName(GAssetRef Ref);		//	generate a name from an assetref
	static GAssetRef	NameToRef(const char* String);	//	generate a ref from a name
	static GAssetRef	NameToRef(GString& String)		{	return NameToRef( String.Export() );	};


};




//	Declarations
//------------------------------------------------
inline Bool	ValidAssetType(GAssetType Type);
GAssetRef	IncrementAssetRef(GAssetRef& AssetRef);		//	change this assetref, from say Mesh to Mesh1. If all characters are used up, it will change MeshA to MeshB


//	Inline Definitions
//-------------------------------------------------
inline Bool	ValidAssetType(GAssetType Type)
{
	return ( Type > GAssetUnknown && Type < GAssetMax );
};




#endif

