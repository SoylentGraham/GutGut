/*------------------------------------------------

  GSkyBox Header file



-------------------------------------------------*/

#ifndef __GSKYBOX__H_
#define __GSKYBOX__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GAsset.h"


//	Macros
//------------------------------------------------

namespace GSkyBoxFlags
{
	const u32	ClearBackground		= 1<<0;		//	clear background even if skybox is drawn
};



//	Types
//------------------------------------------------
class GCamera;

//-------------------------------------------------------------------------
//	base class for drawing a background on a camera. base class just draws it 
//	like a plain background texture
//-------------------------------------------------------------------------
class GSkyBox
{
public:
	GAssetRef			m_Texture;
	u32					m_SkyBoxFlags;

public:
	GSkyBox();
	~GSkyBox();

	virtual void		Draw(GCamera& Camera);
};



//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------



#endif


