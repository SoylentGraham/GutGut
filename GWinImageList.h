/*------------------------------------------------

  GWinImageList Header file



-------------------------------------------------*/

#ifndef __GWINIMAGELIST__H_
#define __GWINIMAGELIST__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GList.h"
#include "GAsset.h"
#include "GWin32.h"



//	Macros
//------------------------------------------------


//	Types
//------------------------------------------------
class GTexture;


typedef struct 
{
	GAssetRef	Ref;		//	useful identity number
	GTexture*	pTexture;	//	pointer to texture this image is based on
	HICON		IconHandle;	//	handle to icon

} GWinImageListImage;


class GWinImageList
{
private:
	HIMAGELIST					m_Handle;		//	handle for image list
	int2						m_ImageSize;	//	dimensions of the images
	int							m_ImageMax;		//	max no of images
	GList<GWinImageListImage>	m_Images;		//	images this list contains
	Bool						m_Alpha;		//	images have an alpha channel(transparent)

public:
	GWinImageList();
	~GWinImageList();

	void				Init(int2 ImageSize,int MaxImages, Bool Alpha);	//	creates imagelist
	void				Destroy();										//	destroys imagelist

	int					AddImage(GAssetRef Ref,GTexture* pTexture);		//	add an image to the image list based on a ref and a texture to get the graphic from
	int					AddImage(GWinImageListImage& Image);			//	add an image to the image list
	int					AddIcon(GAssetRef Ref, GString& Filename, int IconIndex=0);		//	add an image based on an icon file
	int					AddIcon(GAssetRef Ref, int Resource, int IconIndex=0);		//	add an image based on an icon resource
	int					GetImageIndex(GAssetRef Ref);					//	get the index of an image from a ref
	Bool				RemoveImage(GAssetRef Ref, Bool DeleteTexture);	//	remove a particular image
	Bool				RemoveImageIndex(int Index, Bool DeleteTexture);	//	remove a particular image
	void				RemoveAllImages(Bool DeleteTextures);			//	remove all images
	inline HIMAGELIST	Handle()										{	return m_Handle;	};

};





//	Declarations
//------------------------------------------------



//	Inline Definitions
//-------------------------------------------------



#endif	//	__GWINIMAGELIST__H_
