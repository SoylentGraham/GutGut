/*------------------------------------------------

  GTexture Header file



-------------------------------------------------*/

#ifndef __GTEXTURE__H_
#define __GTEXTURE__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GAsset.h"


//	Macros
//------------------------------------------------
namespace GTextureFlags
{
	const u32	AlphaChannel	= 1<<0;		//	texture has an alpha channel
	const u32	LinearFilter	= 1<<1;		//	if linear and not mipmapped, nearest filter
	const u32	MipMapFilter	= 1<<2;		//	alternative to linear filter
	const u32	OneDimension	= 1<<3;		//	1D texture
	const u32	Clamp			= 1<<4;		//	clamp texture
	const u32	EnvMapX			= 1<<5;		//	enviroment map clamp x
	const u32	EnvMapY			= 1<<6;		//	enviroment map clamp y


	inline const char*	GetFlagName(int FlagNo)
	{
		switch ( FlagNo )
		{
			case 0:	return "Alpha channel";	break;
			case 1:	return "Linear filter";	break;
			case 2:	return "Mipmap filter";	break;
			case 3:	return "One Dimension";	break;
			case 4: return "Clamp texture";	break;
			case 5: return "Enviroment map x";	break;
			case 6: return "Enviroment map y";	break;
		}
		return NULL;
	}
};




//	Types
//------------------------------------------------

typedef Type3<u8>	Pixel3;
typedef Type4<u8>	Pixel4;


/*
	pixel orders
*/
typedef enum GPixelOrder
{
	GPixelOrder_RGB=0,
	GPixelOrder_RGBA,
	GPixelOrder_BGR,
	GPixelOrder_BGRA,
};


/*
	Header for a texture in a datafile. texture data follow directly after header
*/
typedef struct 
{
	u32			TextureFlags;		//	number of vertexes and vertex normals
	u16			Width;				//	number of triangles and triangle neighbours
	u16			Height;

} GTextureHeader;



/*
	Texture handling class
*/
class GTexture : public GAsset
{
public:
	const static u32	g_Version;

public:
	int2		m_Size;
	u32			m_TextureFlags;	//	GTextureFlags
	GList<u8>	m_Data;
	u32			m_GLIndex;		//	opengl index after upload

public:
	GTexture();
	~GTexture();
	
	//	virtual
	virtual GAssetType	AssetType()					{	return GAssetTexture;	};
	virtual u32			Version()					{	return GTexture::g_Version;	};
	virtual Bool		Load(GBinaryData& Data);
	virtual Bool		Save(GBinaryData& Data);

	//	inline
	inline Bool		AlphaChannel()					{	return ( m_TextureFlags & GTextureFlags::AlphaChannel ) ? TRUE : FALSE;	};
	inline int		ColourSize()					{	return 3 + ( AlphaChannel() ? 1 : 0 );	};
	inline GLenum	glFormat()						{	return AlphaChannel() ? GL_RGBA : GL_RGB;	};
	inline int		IndexFromPixel(int px,int py)	{	GDebug_CheckIndex(px,0,m_Size.x);	GDebug::CheckIndex(py,0,m_Size.y);	return ( (ColourSize() * m_Size.x) * py ) + ( ColourSize() * px );	};

	//	texture generation
	void			AllocTexture(int2 Size, u32 Flags );	//	allocate memory for a new texture
	void			DeleteTextureData();					//	delete colour data but dont delete from opengl
	void			MakeTestTexture();						//	make test texture
	void			ConvertPixelOrder(GPixelOrder CurrentOrder,GPixelOrder NewOrder);	//	convert all the data from (eg.) RGB colours to BGR colours
	void			CheckPixelOrder(GPixelOrder& Order);		//	changes to/from an alpha channel version of the pixel order

	//	opengl funcs
	virtual Bool	Upload();						//	create opengl texture
	virtual Bool	Deload();						//	unload opengl texture
	void			Reload()						{	Deload();	Upload();	};
	
	void			Select();						//	bind texture and enable texturing
	void			Select2();						//	bind texture2 and enable texturing
	static void		SelectNone();					//	bind to no texture
	static void		SelectNone2();					//	bind to no texture
	inline u16		TextureType()					{	return ( m_TextureFlags & GTextureFlags::OneDimension ) ? GL_TEXTURE_1D : GL_TEXTURE_2D;	};
};




//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------



#endif

