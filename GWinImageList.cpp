/*------------------------------------------------

  GWinImageList.cpp

	Win32 Image list handler
	http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/commctls/imagelist/imagelist.asp

-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GWinImageList.h"
#include "GDebug.h"
#include "GTexture.h"
#include "GFile.h"

//	globals
//------------------------------------------------



//	Definitions
//------------------------------------------------

GWinImageList::GWinImageList()
{
	//	init vars
	m_Handle	= NULL;
	m_ImageSize	= int2(0,0);
	m_ImageMax	= 0;
	m_Alpha		= FALSE;
}



GWinImageList::~GWinImageList()
{
	//	cleanup
	Destroy();
}




void GWinImageList::Init(int2 ImageSize,int MaxImages, Bool Alpha)
{
	//	already initalised?
	if ( m_Handle != NULL )
	{
		GDebug_Break("Image list Already initialised\n");
		return;
	}

	//	check params
	if ( ImageSize.x <= 0 || ImageSize.y <= 0 )
	{
		GDebug_Break("Invalid image size specified\n");
		return;
	}

	//	check params
	if ( MaxImages <= 0 )
	{
		GDebug_Break("Invalid max images specified\n");
		return;
	}

	//	set vars
	m_ImageSize	= ImageSize;
	m_ImageMax	= MaxImages;
	m_Alpha		= Alpha;
	u32 Flags = 0x0;

	//	currently only 24bit images supported
	Flags |= ILC_COLOR24;
	
	if ( m_Alpha )
	{
		Flags |= ILC_MASK;
	}
	
	//	create win32 image list
	m_Handle = ImageList_Create( m_ImageSize.x, m_ImageSize.y, Flags, 0, m_ImageMax );

	//	failed to create image list
	if ( m_Handle == INVALID_HANDLE_VALUE )
	{
		GDebug::CheckWin32Error();
		return;
	}


}




void GWinImageList::Destroy()
{
	//	remove images
	RemoveAllImages(TRUE);

	//	destroy control
	if ( !ImageList_Destroy( m_Handle ) )
	{
		GDebug::CheckWin32Error();
	}
	
	m_Handle = NULL;
}


int GWinImageList::AddImage(GAssetRef Ref,GTexture* pTexture)
{
	GWinImageListImage Image;
	Image.Ref		= Ref;
	Image.pTexture	= pTexture;
	
	return AddImage( Image );
}



int GWinImageList::AddImage(GWinImageListImage& Image)
{
	//	add image
	if ( m_Images.Size() >= m_ImageMax )
	{
		GDebug::Print("Tried to add too many images to image list\n");
		return -1;
	}

	//	add to list
	int Index = m_Images.Add( Image );

	//	add to control
	GDebug_Break("Todo\n");
	
	return Index;
}


int GWinImageList::AddIcon(GAssetRef Ref, GString& Filename, int IconIndex)
{
	//	add image
	if ( m_Images.Size() >= m_ImageMax )
	{
		GDebug::Print("Tried to add too many images to image list\n");
		return -1;
	}

	//
	GWinImageListImage Image;
	Image.Ref			= Ref;
	Image.pTexture		= NULL;
	Image.IconHandle	= NULL;

	GString FullPath;

	//	amend filename
	if ( !ExtractFullFilePath( Filename, FullPath ) )
	{
		Filename.Insert( 0, GApp::g_AppPath );
	}

	//	check file exists
	GFile FileTest;
	if ( !FileTest.Load(Filename) )
	{
		GDebug::Print("Failed to load file \"%s\"\n", Filename );
		return -1;
	}

	//	get the small icon handle from the filename
	if ( m_ImageSize.x <= 16 )
		ExtractIconEx( Filename, IconIndex, NULL, &Image.IconHandle, 1 );
	else
		ExtractIconEx( Filename, IconIndex, &Image.IconHandle, NULL, 1 );		

	//	failed to get icon
	if ( !Image.IconHandle )
	{
		GDebug::CheckWin32Error();
		return -1;
	}

	//	add icon
	if ( ImageList_AddIcon( m_Handle, Image.IconHandle ) == -1 )
	{
		//	failed to add
		GDebug::CheckWin32Error();
		return -1;
	}

	//	addded okay
	int Index = m_Images.Add( Image );

	return Index;
}


int GWinImageList::AddIcon(GAssetRef Ref, int Resource, int IconIndex)
{
	//	add image
	if ( m_Images.Size() >= m_ImageMax )
	{
		GDebug::Print("Tried to add too many images to image list\n");
		return -1;
	}

	//
	GWinImageListImage Image;
	Image.Ref			= Ref;
	Image.pTexture		= NULL;
	Image.IconHandle	= NULL;

	//	get icon from resource
	Image.IconHandle = LoadIcon( GApp::g_HInstance, MAKEINTRESOURCE(Resource) );

	//	failed to get icon
	if ( !Image.IconHandle )
	{
		GDebug::CheckWin32Error();
		return -1;
	}

	//	add icon
	if ( ImageList_AddIcon( m_Handle, Image.IconHandle ) == -1 )
	{
		//	failed to add
		GDebug::CheckWin32Error();
		return -1;
	}

	//	addded okay
	int Index = m_Images.Add( Image );

	return Index;
}



int GWinImageList::GetImageIndex(GAssetRef Ref)
{
	//	match ref
	for ( int i=0;	i<m_Images.Size();	i++ )
	{
		if ( m_Images[i].Ref == Ref )
			return i;
	}

	return -1;
}


Bool GWinImageList::RemoveImage(GAssetRef Ref, Bool DeleteTexture)
{
	//	get index
	int Index = GetImageIndex( Ref );
	
	//	remove
	return RemoveImageIndex( Index, DeleteTexture );
}


Bool GWinImageList::RemoveImageIndex(int Index, Bool DeleteTexture)
{
	//	check index
	GDebug_CheckIndex( Index, 0, m_Images.Size() );

	//	remove from control
	if ( !ImageList_Remove( m_Handle, Index ) )
	{
		GDebug::CheckWin32Error();
	}

	//	delete texture
	if ( m_Images[Index].pTexture )
	{
		GDelete( m_Images[Index].pTexture );
	}

	//	remove from list
	m_Images.RemoveAt( Index );

	return TRUE;
}



void GWinImageList::RemoveAllImages(Bool DeleteTextures)
{
	//	remove all images
	while( m_Images.Size() )
	{
		RemoveImageIndex( 0, DeleteTextures );
	}
}




