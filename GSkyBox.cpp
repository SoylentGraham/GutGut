/*------------------------------------------------

  GSkyBox.cpp

  class for drawing a background onto a camera
	

-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GSkyBox.h"
#include "GTexture.h"
#include "GAssetList.h"
#include "GCamera.h"


//	globals
//------------------------------------------------



//	Definitions
//------------------------------------------------

GSkyBox::GSkyBox()
{
	m_Texture = GAssetRef_Invalid;
	m_SkyBoxFlags = 0x0;
	
	
	m_SkyBoxFlags |= GSkyBoxFlags::ClearBackground;
}

GSkyBox::~GSkyBox()
{
}

//-------------------------------------------------------------------------
//	draw texture 
//-------------------------------------------------------------------------
void GSkyBox::Draw(GCamera& Camera)
{
	//	get texture
	GTexture* pTexture = GAssets::g_Textures.Find(m_Texture);
	if ( !pTexture )
		return;

	float2& Min = Camera.m_OrthoMin;
	float2& Max = Camera.m_OrthoMax;

	glPushAttrib( GL_ALL_ATTRIB_BITS );
	
	glEnable(GL_TEXTURE_2D);
	pTexture->Select();

	float2 WorldVisibility = Camera.GetWorldVisibility();
	float2 WorldEnvMapScroll = Camera.GetEnvMapScroll();

	//GDebug_Print("%3.3f\n",skyscroll);
	
	//	only use "top half" of what would be an enviroment map
	WorldEnvMapScroll.y -= 0.5f;
	WorldVisibility.y *= 2.f;
	//if ( WorldEnvMapScroll.y < 0.f )
	//	WorldEnvMapScroll.y = 0.f;

	float skyxmin = WorldEnvMapScroll.x - ( WorldVisibility.x * 0.5f );
	float skyxmax = WorldEnvMapScroll.x + ( WorldVisibility.x * 0.5f );
	float skyymin = WorldEnvMapScroll.y - ( WorldVisibility.y * 0.5f );
	float skyymax = WorldEnvMapScroll.y + ( WorldVisibility.y * 0.5f );
	//float skyymin = 0.f;
	//float skyymax = 1.f;

	glColor3f( 1, 1, 1 );
	glBegin(GL_QUADS);

		glTexCoord2f( skyxmin, skyymin );
		glVertex3f( Min.x, Min.y, Camera.m_NearZ );
		
		glTexCoord2f( skyxmax, skyymin );
		glVertex3f( Max.x, Min.y, Camera.m_NearZ );

		glTexCoord2f( skyxmax, skyymax );
		glVertex3f( Max.x, Max.y, Camera.m_NearZ );

		glTexCoord2f( skyxmin, skyymax );
		glVertex3f( Min.x, Max.y, Camera.m_NearZ );

	glEnd();

	glPopAttrib();
}
	

