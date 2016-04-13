/*------------------------------------------------

  GGutFile Header file



-------------------------------------------------*/

#ifndef __GGUTFILE__H_
#define __GGUTFILE__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GList.h"
#include "GAsset.h"
#include "GString.h"
#include "GBinaryData.h"


//	Macros
//------------------------------------------------





//	Types
//------------------------------------------------
class GMeshList;
class GTextureList;
class GMapList;
class GMapObjectList;


//--------------------------------------------------------------------------------------------------------
//	standard header for gut files
//--------------------------------------------------------------------------------------------------------
typedef struct
{
	u32		Version;		//	file version - matches GGutFile::g_Version
	u16		AssetCount;		//	number of assets

	inline void	Reset();	//	reset header to a default state

} GGutFileHeader;




//--------------------------------------------------------------------------------------------------------
//	file class which contains assets
//--------------------------------------------------------------------------------------------------------
class GGutFile
{
public:
	const static u32	g_Version;		//	current file version
	const static char*	g_FileExt;		//	default file extension
	const static char*	g_FileFilter;	//	file filter for dialogs


public:
	GList<GAsset*>		m_Assets;	//	assets present in the file
	GGutFileHeader		m_Header;	//	file header
	

public:
	GGutFile();
	~GGutFile();

	void	AddAssets(GMeshList* pList);		//	add all the assets from this asset list
	void	AddAssets(GTextureList* pList);		//	add all the assets from this asset list
	void	AddAssets(GMapList* pList);			//	add all the assets from this asset list
	void	AddAssets(GMapObjectList* pList);	//	add all the assets from this asset list
	void	RemoveAssets(Bool Delete);			//	remove all the meshes from the file. specify whether or not to delete them
	GAsset*	FindAsset(GAssetType Type, GAssetRef Ref);	//	find an asset matching the ref and type

	int		LoadAssets();						//	load assets into global asset lists
	Bool	Load(const GString& Filename);		//	load all the data out of the specified filename and into this class
	Bool	Save(const GString& Filename);		//	save all the data out of this and into a file
	
private:
	Bool	ImportData(GBinaryData& Data);		//	turn current data(private data) into assets etc
	Bool	ExportData(GBinaryData& Data);		//	turn current assets(public data) into data to be saved
};




//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------
inline void GGutFileHeader::Reset()
{
	Version		= GGutFile::g_Version;
	AssetCount	= 0;
};



#endif

