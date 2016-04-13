/*------------------------------------------------

  GTexture.cpp

	Texture handling class


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GTexture.h"
#include "GDebug.h"
#include "GGutFile.h"
#include "GDisplay.h"


//	globals
//------------------------------------------------
const u32 GTexture::g_Version = 0x33330002;



//	Definitions
//------------------------------------------------

GTexture::GTexture()
{
	m_Size			= int2(0,0);
	m_TextureFlags	= 0x0;
	m_GLIndex		= 0;
}



GTexture::~GTexture()
{
	DeleteTextureData();
	Deload();
}


void GTexture::AllocTexture(int2 Size, u32 Flags )
{
	//	delete current data
	DeleteTextureData();

	//	update flags
	m_TextureFlags = Flags;

	//	update size
	m_Size = Size;

	//	size per line
	int LineWidth = ColourSize() * Size.x;

	m_Data.Resize( LineWidth * Size.y );
}



void GTexture::DeleteTextureData()
{
	//	delete colour data
	m_Data.Empty();
}


void GTexture::MakeTestTexture()
{
	int2 Size(8,8);

	Deload();

	//	allocate data
	AllocTexture(Size, GTextureFlags::AlphaChannel );

	//	fill in data with a corner of red, green, blue, yellow
	u32 Red		= RGBA(	255,	0,		0,		255 );
	u32 Green	= RGBA(	0,		255,	0,		255 );
	u32 Blue	= RGBA(	0,		0,		255,	255 );
	u32 Yellow	= RGBA(	255,	255,	0,		255 );
	int RGBASize = 4;

	int i;


	#define COLOUR_LINE(fromx,fromy,len,col)					\
	{															\
		for ( i=0;	i<len;	i++ )								\
		{														\
			int Index = IndexFromPixel( fromx+i, fromy );		\
			u32* pData = (u32*)(&m_Data[ Index ]);				\
			*pData = col;										\
		}														\
	}												

	COLOUR_LINE(0,0,4,Red);		COLOUR_LINE(4,0,4,Green);
	COLOUR_LINE(0,1,4,Red);		COLOUR_LINE(4,1,4,Green);
	COLOUR_LINE(0,2,4,Red);		COLOUR_LINE(4,2,4,Green);
	COLOUR_LINE(0,3,4,Red);		COLOUR_LINE(4,3,4,Green);

	COLOUR_LINE(0,4,4,Blue);	COLOUR_LINE(4,4,4,Yellow);
	COLOUR_LINE(0,5,4,Blue);	COLOUR_LINE(4,5,4,Yellow);
	COLOUR_LINE(0,6,4,Blue);	COLOUR_LINE(4,6,4,Yellow);
	COLOUR_LINE(0,7,4,Blue);	COLOUR_LINE(4,7,4,Yellow);
	

	Upload();
}



Bool GTexture::Upload()
{
	//	no data
	if ( m_Data.Size() == 0 )
	{
		GDebug_Break("No data in texture to upload\n");
		return FALSE;
	}

	//	allocate index
	glGenTextures( 1, &m_GLIndex );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	
	if ( m_GLIndex == 0 )
	{
		GDebug::CheckGLError();
		GDebug::Print("Failed to generate texture\n");
		return FALSE;
	}

	//	set as active texture	
	Select();

	//	set filter type
	enum
	{
		FilterNearest = 0,
		FilterLinear,
		FilterMipMap
	};
	int Filter = FilterNearest;

	if ( m_TextureFlags & GTextureFlags::LinearFilter )
	{
		Filter = FilterLinear;
	}
	else if ( m_TextureFlags & GTextureFlags::MipMapFilter )
	{
		Filter = FilterMipMap;
	}


	//	build texture
	switch ( Filter )
	{
		case FilterNearest:
			glTexParameteri( TextureType(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri( TextureType(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D( TextureType(), 0, glFormat(),	m_Size.x, m_Size.y, 0, glFormat(), GL_UNSIGNED_BYTE, m_Data.Data() );
			break;

		case FilterLinear:
			glTexParameteri(TextureType(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(TextureType(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D( TextureType(), 0, glFormat(),	m_Size.x, m_Size.y, 0, glFormat(), GL_UNSIGNED_BYTE, m_Data.Data() );
			break;

		case FilterMipMap:
		{
			glTexParameteri(TextureType(), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(TextureType(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			u32 SizeX = GNearestPower( m_Size.x );
			u32 SizeY = GNearestPower( m_Size.y );
			int Lod = 0;
			while ( SizeX > 4 && SizeY > 4 )
			{
				glTexImage2D( TextureType(), Lod, glFormat(), SizeX, SizeY, 0, glFormat(), GL_UNSIGNED_BYTE, m_Data.Data() );
				SizeX >>= 1;
				SizeY >>= 1;
				Lod++;
			}
		}
		break;
	}




	GDebug::CheckGLError();

	return TRUE;
}



Bool GTexture::Deload()
{
	if ( m_GLIndex == 0 )
		return FALSE;

	glDeleteTextures( 1, &m_GLIndex );

	GDebug::CheckGLError();

	m_GLIndex = 0;


	return TRUE;
}



Bool GTexture::Load(GBinaryData& Data)
{
	/*
	//	read a block of data, update sizes and data pointer
	#define READ_BLOCK( addr, size )					\
	{													\
		if ( (int)DataSize < (int)(size) )				\
		{												\
			GDebug::Print("Texture is missing data\n");	\
			return FALSE;								\
		}												\
		memcpy( addr, pData, size );					\
		pData += size;									\
		DataSize -= size;								\
		DataRead += size;								\
	}
	*/

	//	read-in the header
	GTextureHeader Header;
	//READ_BLOCK( &Header, GDataSizeOf(GTextureHeader) );
	if ( !Data.Read( &Header, GDataSizeOf(GTextureHeader), "Texture header" ) )
		return FALSE;

	//	from the header, allocate data
	AllocTexture( int2( Header.Width, Header.Height ), Header.TextureFlags );

	//	now read in the blocks of data that follow the header
	//READ_BLOCK( m_Data.Data(), m_Data.Size() * GDataSizeOf(u8) );
	if ( !Data.Read( m_Data.Data(), m_Data.DataSize(), "Texture data" ) )
		return FALSE;

	//#undef READ_BLOCK

	return TRUE;
}



Bool GTexture::Save(GBinaryData& SaveData)
{
	//	add the texture header
	GTextureHeader Header;
	Header.Height	= m_Size.x;
	Header.Width	= m_Size.y;
	Header.TextureFlags	= m_TextureFlags;

	SaveData.Write( &Header, GDataSizeOf(GTextureHeader) );

	//	add in the actual texture data
	SaveData.Write( m_Data.Data(), m_Data.DataSize() );

	return TRUE;
}



void GTexture::CheckPixelOrder(GPixelOrder& Order)
{
	switch ( Order )
	{
		//	add alpha channel
		case GPixelOrder_RGB:
			if ( AlphaChannel() )
				Order = GPixelOrder_RGBA;
			break;

		case GPixelOrder_BGR:
			if ( AlphaChannel() )
				Order = GPixelOrder_BGRA;
			break;

		//	remove alpha channel
		case GPixelOrder_RGBA:
			if ( !AlphaChannel() )
				Order = GPixelOrder_RGB;
			break;

		case GPixelOrder_BGRA:
			if ( !AlphaChannel() )
				Order = GPixelOrder_BGR;
			break;
	};
}



inline void Convert_RGB_BGR(Pixel3* pPixel)
{
	Pixel3 rgb( *pPixel );
	Pixel3 bgr( rgb[2], rgb[1], rgb[0] );
	*pPixel = bgr;
}

inline void Convert_RGBA_BGRA(Pixel4* pPixel)
{
	Pixel4 rgba( *pPixel );
	Pixel4 bgra( rgba[2], rgba[1], rgba[0] , rgba[3] );
	*pPixel = bgra;
}

inline void Convert_BGR_RGB(Pixel3* pPixel)
{
	Pixel3 bgr( *pPixel );
	Pixel3 rgb( bgr[2], bgr[1], bgr[0] );
	*pPixel = rgb;
}

inline void Convert_BGRA_RGBA(Pixel4* pPixel)
{
	Pixel4 bgra( *pPixel );
	Pixel4 rgba( bgra[2], bgra[1], bgra[0] , bgra[3] );
	*pPixel = rgba;
}

void GTexture::ConvertPixelOrder(GPixelOrder CurrentOrder,GPixelOrder NewOrder)
{
	//	add/remove alpha channel from pixel orders
	CheckPixelOrder(CurrentOrder);
	CheckPixelOrder(NewOrder);

	//	dont want to change anything
	if ( CurrentOrder == NewOrder )
	{
		GDebug::Print("Cant convert texture pixel order to the same thing\n");
		return;
	}

	//	convert pixels
	for ( int i=0;	i<m_Data.Size();	i+=ColourSize() )
	{
		u8* pData = &m_Data[i];

		if ( CurrentOrder == GPixelOrder_RGB	&& NewOrder == GPixelOrder_BGR )	Convert_RGB_BGR(	(Pixel3*)pData );
		if ( CurrentOrder == GPixelOrder_RGBA	&& NewOrder == GPixelOrder_BGRA )	Convert_RGBA_BGRA(	(Pixel4*)pData );
		if ( CurrentOrder == GPixelOrder_BGR	&& NewOrder == GPixelOrder_RGB )	Convert_BGR_RGB(	(Pixel3*)pData );
		if ( CurrentOrder == GPixelOrder_BGRA	&& NewOrder == GPixelOrder_RGBA )	Convert_BGRA_RGBA(	(Pixel4*)pData );

	}

}


//-------------------------------------------------------------------------
//	bind texture and enable texturing
//-------------------------------------------------------------------------
void GTexture::Select()
{
	if ( m_GLIndex == 0 )
	{
		GDebug_Break("Tried to select texture that hasnt been uploaded\n");
		return;
	}

	//	bind texture
	if ( g_DisplayExt.HardwareSupported( GHardware_MultiTexturing ) )
		g_DisplayExt.glActiveTextureARB()( GL_TEXTURE0_ARB );
	
	glBindTexture( TextureType(), m_GLIndex );
	GDebug::CheckGLError();

	//	enable clamping
	if ( m_TextureFlags & GTextureFlags::Clamp )
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		GDebug::CheckGLError();
	}

	//	enable or disable sphere mapping generation on X
	if ( m_TextureFlags & GTextureFlags::EnvMapX )
	{
	//	glEnable(GL_TEXTURE_GEN_S);
	}
	else
	{
		glDisable(GL_TEXTURE_GEN_S);
	}

	//	enable or disable sphere mapping generation on Y
	if ( m_TextureFlags & GTextureFlags::EnvMapY )
	{
	//	glEnable(GL_TEXTURE_GEN_T);
	}
	else
	{
		glDisable(GL_TEXTURE_GEN_T);
	}

}


//-------------------------------------------------------------------------
//	bind texture to 2nd texture 
//-------------------------------------------------------------------------
void GTexture::Select2()
{
	if ( m_GLIndex == 0 )
	{
		GDebug_Break("Tried to select texture that hasnt been uploaded\n");
		return;
	}

	//	dont do anything if multitexturing isnt supported
	if ( !g_DisplayExt.HardwareSupported( GHardware_MultiTexturing ) )
	{
		return;
	}
	
	//	bind texture
	g_DisplayExt.glActiveTextureARB()( GL_TEXTURE1_ARB );

	glBindTexture( TextureType(), m_GLIndex );
	GDebug::CheckGLError();

	if ( m_TextureFlags & GTextureFlags::Clamp )
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		GDebug::CheckGLError();
	}

	//glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );	//	AND
	//glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD );		//	ADD
	GDebug::CheckGLError();
}

//-------------------------------------------------------------------------
//	bind to no texture and disable texturing
//-------------------------------------------------------------------------
/*static*/void GTexture::SelectNone()
{
	//	bind to no texture
	if ( g_DisplayExt.HardwareSupported( GHardware_MultiTexturing ) )
		g_DisplayExt.glActiveTextureARB()( GL_TEXTURE0_ARB );

	glBindTexture( GL_TEXTURE_2D, 0 );

	GDebug::CheckGLError();
}


//-------------------------------------------------------------------------
//	bind to no 2nd texture and disable texturing
//-------------------------------------------------------------------------
/*static*/void GTexture::SelectNone2()
{
	if ( !g_DisplayExt.HardwareSupported( GHardware_MultiTexturing ) )
		return;

	//	bind to no texture
	g_DisplayExt.glActiveTextureARB()( GL_TEXTURE1_ARB );
	glBindTexture( GL_TEXTURE_2D, 0 );

	GDebug::CheckGLError();
}

