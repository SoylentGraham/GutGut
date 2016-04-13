/*------------------------------------------------

  GCamera.cpp

	Camera class. used for general camera handling and setting up views


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GCamera.h"
#include "GDebug.h"
#include "GDisplay.h"
#include "GApp.h"
#include "GAssetList.h"


//	globals
//------------------------------------------------
GCamera* GCamera::g_pActiveCamera = NULL;


//	Definitions
//------------------------------------------------


GCamera::GCamera()
{
	m_Viewport.x = 0;
	m_Viewport.y = 0;
	m_Viewport[2] = 256;	//	w
	m_Viewport[3] = 256;	//	h

	m_OrthoMin.x = -1.f;
	m_OrthoMax.x = 1.f;
	m_OrthoMin.y = -1.f;
	m_OrthoMax.y = 1.f;
	m_OrthoNearFar.x = -0.1f;	//	near
	m_OrthoNearFar.y = 1000.f;	//	far

	m_NearZ = 0.1f;
	m_FarZ = 1000.f;

	m_FOV = 45.f;

	m_Position	= float3(0,0,0);
	m_LookAt	= float3(0,0,0);
	m_WorldUp	= g_WorldUp;

	m_ClearColour = float4(1,1,1,1);

	m_OrthoMode			= FALSE;
	m_OrthoOffset		= float3(0,0,0);
	m_OrthoRotation		= GQuaternion( float3(0,1,0), 0.f );

}



GCamera::~GCamera()
{
}




void GCamera::UpdateMatrix()
{
	float3& eye = m_Position;
	float3& center = m_LookAt;

	if ( eye == center )
	{
		GDebug_Break("Camera lookat and position are the same");
	}

	float3 up = m_WorldUp;

//	gluLookAt(	eye.x, eye.y, eye.z,
//				center.x, center.y, center.z,
//				up.x, up.y, up.z );


	float x[3], y[3], z[3];
	float mag;

	// Make rotation matrix 

	// Z vector
	z[0] = eye.x - center.x;
	z[1] = eye.y - center.y;
	z[2] = eye.z - center.z;
	mag = sqrtf(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
	if (mag) {			// mpichler, 19950515 
	  z[0] /= mag;
	  z[1] /= mag;
	  z[2] /= mag;
	}

	// Y vector
	y[0] = up.x;
	y[1] = up.y;
	y[2] = up.z;

	// X vector = Y cross Z
	x[0] = y[1] * z[2] - y[2] * z[1];
	x[1] = -y[0] * z[2] + y[2] * z[0];
	x[2] = y[0] * z[1] - y[1] * z[0];

	// Recompute Y = Z cross X
	y[0] = z[1] * x[2] - z[2] * x[1];
	y[1] = -z[0] * x[2] + z[2] * x[0];
	y[2] = z[0] * x[1] - z[1] * x[0];

	// mpichler, 19950515 
	// cross product gives area of parallelogram, which is < 1.0 for
	// non-perpendicular unit-length vectors; so normalize x, y here
	
	mag = sqrtf(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
	if (mag) {
	  x[0] /= mag;
	  x[1] /= mag;
	  x[2] /= mag;
	}

	mag = sqrtf(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
	if (mag) {
	  y[0] /= mag;
	  y[1] /= mag;
	  y[2] /= mag;
	}


	//	setup matrx

#define M(row,col)  m_CameraMatrix.Get(row,col)
	M(0, 0) = x[0];
	M(0, 1) = x[1];
	M(0, 2) = x[2];
	M(0, 3) = 0.0;
	M(1, 0) = y[0];
	M(1, 1) = y[1];
	M(1, 2) = y[2];
	M(1, 3) = 0.0;
	M(2, 0) = z[0];
	M(2, 1) = z[1];
	M(2, 2) = z[2];
	M(2, 3) = 0.0;
	M(3, 0) = 0.0;
	M(3, 1) = 0.0;
	M(3, 2) = 0.0;
	M(3, 3) = 1.0;
#undef M

	
	//	quick hack!
	//	not sure why applying the translation doesnt work on the matrix...
	glPushMatrix();
	
	glLoadIdentity();
	glMultMatrixf(m_CameraMatrix);
	glTranslatef(-eye.x, -eye.y, -eye.z);
	glGetFloatv(GL_MODELVIEW_MATRIX,m_CameraMatrix);
	GDebug::CheckGLError();

	glPopMatrix();
	

	
}



/*
void GMatrix::glMultMatrix() const
{
	glMultMatrixf( _matrix );
}


void GMatrix::glLoadMatrix() const
{
	glLoadMatrixf( _matrix );
}
*/


void GCamera::SetupPerspective()
{
	//	same as gluPerspective()

	float Aspect = ViewportAspectRatio();
//	float Aspect = 1.f;
	float xmin, xmax, ymin, ymax;

	ymax = m_NearZ * tanf( m_FOV * ( PI / 360.f ) );
	ymin = -ymax;
	xmin = ymin * Aspect;
	xmax = ymax * Aspect;

	SetupFrustum( xmin, xmax, ymin, ymax, m_NearZ, m_FarZ );
	
}


void GCamera::SetupFrustum(float left, float right,float bottom, float top,float nearval, float farval)
{
	float x, y, a, b, c, d;
	float m[16];

	x = (2.0f * nearval) / (right - left);
	y = (2.0f * nearval) / (top - bottom);
	a = (right + left) / (right - left);
	b = (top + bottom) / (top - bottom);
	c = -(farval + nearval) / ( farval - nearval);
	d = -(2.0f * farval * nearval) / (farval - nearval);

#define M(row,col)  m[col*4+row]
	M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
	M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
	M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
	M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M

	glMultMatrixf(m);
}



void GCamera::SetupView(Bool SetupMatriciesOnly,GSkyBox* pSkyBox)
{
	//	this is now the active camera
	if ( !SetupMatriciesOnly )
	{
		g_pActiveCamera = this;
	}

	//	update projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	SetupPerspective();

	//	setup viewport
	if ( !SetupMatriciesOnly )
	{
		//	opengl viewport goes from bottom to top, so realign in the display's window
		int2 DisplaySize = g_Display->Window()->m_ClientSize;
		int4 DisplayViewport = m_Viewport;
		DisplayViewport.y = DisplaySize.y - ( m_Viewport.y + m_Viewport[3] );
		DisplayViewport[3] = m_Viewport[3];

		glViewport( DisplayViewport.x, DisplayViewport.y, DisplayViewport[2], DisplayViewport[3] );

		glScissor( DisplayViewport.x, DisplayViewport.y, DisplayViewport[2], DisplayViewport[3] );
		glEnable( GL_SCISSOR_TEST );

		//	clear screen
		glClearColor( m_ClearColour[0], m_ClearColour[1], m_ClearColour[2], m_ClearColour[3] );
		GLbitfield ClearMask = 0x0;
		if ( m_CameraFlags & GCameraFlags::Clear )			ClearMask |= GL_COLOR_BUFFER_BIT;
		if ( m_CameraFlags & GCameraFlags::ClearZ )			ClearMask |= GL_DEPTH_BUFFER_BIT;
		if ( m_CameraFlags & GCameraFlags::ClearStencil )	ClearMask |= GL_STENCIL_BUFFER_BIT;

		//	cancel clear if skybox doesnt want it cleared
		if ( pSkyBox )
		{
			if ( ! (pSkyBox->m_SkyBoxFlags & GSkyBoxFlags::ClearBackground) )
				ClearMask &= ~GL_COLOR_BUFFER_BIT;
		}

		glClear( ClearMask );
	}	


	//	update camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//	draw skybox
	if ( pSkyBox )
	{
		BeginOrtho();

		//	dont read or write to depth buffer
		glDepthFunc( GL_NEVER );
		glDisable( GL_DEPTH_TEST );
		glDisable( GL_CULL_FACE );

		pSkyBox->Draw( *this );

		glEnable( GL_CULL_FACE );
		glEnable( GL_DEPTH_TEST );
		EndOrtho();
	}
	glDepthFunc( GL_LESS );

	//	update and set camera matrix
	UpdateMatrix();	
	glMultMatrixf( m_CameraMatrix );

	GDebug::CheckGLError();
}



Bool GCamera::BeginOrtho()
{
	if ( m_OrthoMode )
	{
		GDebug_Break("Already in ortho mode");
		return FALSE;
	}
	
	//	change to ortho projection
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	//	change orthographic projection
	glOrtho( m_OrthoMin.x, m_OrthoMax.x, m_OrthoMin.y, m_OrthoMax.y, m_OrthoNearFar.x, m_OrthoNearFar.y );
	GDebug::CheckGLError();

	//	reset modelview
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	GDebug::CheckGLError();

	g_Display->Translate( m_OrthoOffset, &m_OrthoRotation );

	m_OrthoMode = TRUE;
	return TRUE;
}

void GCamera::EndOrtho()
{
	//	not in ortho mode, dont change anything
	if ( !m_OrthoMode )
		return;

	//	restore projection
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	GDebug::CheckGLError();

	//	restore modelview
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	GDebug::CheckGLError();

	m_OrthoMode = FALSE;
}



void GCamera::CaptureTexture(GAssetRef TextureRef)
{
	//	copies current screen buffer to texture
	GTexture* pTexture = GAssets::g_Textures.Find( TextureRef );
	if ( !pTexture )
	{
		GDebug_Break("Failed to find texture to capture to\n");
		return;
	}

	//	get texture index
	int TextureGLIndex = pTexture->m_GLIndex;
	if ( TextureGLIndex == 0 )
	{
		GDebug_Break("Texture is not uploaded. Cannot capture display\n");
		return;
	}

	//	capture current display
	pTexture->Select();
	glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_Viewport[0], m_Viewport[1], m_Viewport[2], m_Viewport[3], 0 );

}



void GCamera::CalcFrustumPlanes()
{
	//	save off current matricies
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	//	setup matricies
	SetupView(TRUE);


    float p[16];   // projection matrix
    float mv[16];  // model-view matrix
    float mvp[16]; // model-view-projection matrix
    float t;

    glGetFloatv( GL_PROJECTION_MATRIX, p );
    glGetFloatv( GL_MODELVIEW_MATRIX, mv );

	//	full amount of planes
	m_FrustumPlaneCount = 6;

    //
    // Concatenate the projection matrix and the model-view matrix to produce 
    // a combined model-view-projection matrix.
    //
    
    mvp[ 0] = mv[ 0] * p[ 0] + mv[ 1] * p[ 4] + mv[ 2] * p[ 8] + mv[ 3] * p[12];
    mvp[ 1] = mv[ 0] * p[ 1] + mv[ 1] * p[ 5] + mv[ 2] * p[ 9] + mv[ 3] * p[13];
    mvp[ 2] = mv[ 0] * p[ 2] + mv[ 1] * p[ 6] + mv[ 2] * p[10] + mv[ 3] * p[14];
    mvp[ 3] = mv[ 0] * p[ 3] + mv[ 1] * p[ 7] + mv[ 2] * p[11] + mv[ 3] * p[15];

    mvp[ 4] = mv[ 4] * p[ 0] + mv[ 5] * p[ 4] + mv[ 6] * p[ 8] + mv[ 7] * p[12];
    mvp[ 5] = mv[ 4] * p[ 1] + mv[ 5] * p[ 5] + mv[ 6] * p[ 9] + mv[ 7] * p[13];
    mvp[ 6] = mv[ 4] * p[ 2] + mv[ 5] * p[ 6] + mv[ 6] * p[10] + mv[ 7] * p[14];
    mvp[ 7] = mv[ 4] * p[ 3] + mv[ 5] * p[ 7] + mv[ 6] * p[11] + mv[ 7] * p[15];

    mvp[ 8] = mv[ 8] * p[ 0] + mv[ 9] * p[ 4] + mv[10] * p[ 8] + mv[11] * p[12];
    mvp[ 9] = mv[ 8] * p[ 1] + mv[ 9] * p[ 5] + mv[10] * p[ 9] + mv[11] * p[13];
    mvp[10] = mv[ 8] * p[ 2] + mv[ 9] * p[ 6] + mv[10] * p[10] + mv[11] * p[14];
    mvp[11] = mv[ 8] * p[ 3] + mv[ 9] * p[ 7] + mv[10] * p[11] + mv[11] * p[15];

    mvp[12] = mv[12] * p[ 0] + mv[13] * p[ 4] + mv[14] * p[ 8] + mv[15] * p[12];
    mvp[13] = mv[12] * p[ 1] + mv[13] * p[ 5] + mv[14] * p[ 9] + mv[15] * p[13];
    mvp[14] = mv[12] * p[ 2] + mv[13] * p[ 6] + mv[14] * p[10] + mv[15] * p[14];
    mvp[15] = mv[12] * p[ 3] + mv[13] * p[ 7] + mv[14] * p[11] + mv[15] * p[15];

    //
    // Extract the frustum's right clipping plane and normalize it.
    //

    m_FrustumPlanes[0][0] = mvp[ 3] - mvp[ 0];
    m_FrustumPlanes[0][1] = mvp[ 7] - mvp[ 4];
    m_FrustumPlanes[0][2] = mvp[11] - mvp[ 8];
    m_FrustumPlanes[0][3] = mvp[15] - mvp[12];

    t = (float) sqrtf( m_FrustumPlanes[0][0] * m_FrustumPlanes[0][0] + 
                      m_FrustumPlanes[0][1] * m_FrustumPlanes[0][1] + 
                      m_FrustumPlanes[0][2] * m_FrustumPlanes[0][2] );

    m_FrustumPlanes[0][0] /= t;
    m_FrustumPlanes[0][1] /= t;
    m_FrustumPlanes[0][2] /= t;
    m_FrustumPlanes[0][3] /= t;

    //
    // Extract the frustum's left clipping plane and normalize it.
    //

    m_FrustumPlanes[1][0] = mvp[ 3] + mvp[ 0];
    m_FrustumPlanes[1][1] = mvp[ 7] + mvp[ 4];
    m_FrustumPlanes[1][2] = mvp[11] + mvp[ 8];
    m_FrustumPlanes[1][3] = mvp[15] + mvp[12];

    t = (float) sqrtf( m_FrustumPlanes[1][0] * m_FrustumPlanes[1][0] + 
                      m_FrustumPlanes[1][1] * m_FrustumPlanes[1][1] + 
                      m_FrustumPlanes[1][2] * m_FrustumPlanes[1][2] );

    m_FrustumPlanes[1][0] /= t;
    m_FrustumPlanes[1][1] /= t;
    m_FrustumPlanes[1][2] /= t;
    m_FrustumPlanes[1][3] /= t;

    //
    // Extract the frustum's bottom clipping plane and normalize it.
    //

    m_FrustumPlanes[2][0] = mvp[ 3] + mvp[ 1];
    m_FrustumPlanes[2][1] = mvp[ 7] + mvp[ 5];
    m_FrustumPlanes[2][2] = mvp[11] + mvp[ 9];
    m_FrustumPlanes[2][3] = mvp[15] + mvp[13];

    t = (float) sqrtf( m_FrustumPlanes[2][0] * m_FrustumPlanes[2][0] + 
                      m_FrustumPlanes[2][1] * m_FrustumPlanes[2][1] + 
                      m_FrustumPlanes[2][2] * m_FrustumPlanes[2][2] );

    m_FrustumPlanes[2][0] /= t;
    m_FrustumPlanes[2][1] /= t;
    m_FrustumPlanes[2][2] /= t;
    m_FrustumPlanes[2][3] /= t;

    //
    // Extract the frustum's top clipping plane and normalize it.
    //

    m_FrustumPlanes[3][0] = mvp[ 3] - mvp[ 1];
    m_FrustumPlanes[3][1] = mvp[ 7] - mvp[ 5];
    m_FrustumPlanes[3][2] = mvp[11] - mvp[ 9];
    m_FrustumPlanes[3][3] = mvp[15] - mvp[13];

    t = (float) sqrtf( m_FrustumPlanes[3][0] * m_FrustumPlanes[3][0] + 
                      m_FrustumPlanes[3][1] * m_FrustumPlanes[3][1] + 
                      m_FrustumPlanes[3][2] * m_FrustumPlanes[3][2] );

    m_FrustumPlanes[3][0] /= t;
    m_FrustumPlanes[3][1] /= t;
    m_FrustumPlanes[3][2] /= t;
    m_FrustumPlanes[3][3] /= t;

    //
    // Extract the frustum's far clipping plane and normalize it.
    //

    m_FrustumPlanes[4][0] = mvp[ 3] - mvp[ 2];
    m_FrustumPlanes[4][1] = mvp[ 7] - mvp[ 6];
    m_FrustumPlanes[4][2] = mvp[11] - mvp[10];
    m_FrustumPlanes[4][3] = mvp[15] - mvp[14];

    t = (float) sqrtf( m_FrustumPlanes[4][0] * m_FrustumPlanes[4][0] +  
                      m_FrustumPlanes[4][1] * m_FrustumPlanes[4][1] + 
                      m_FrustumPlanes[4][2] * m_FrustumPlanes[4][2] );

    m_FrustumPlanes[4][0] /= t;
    m_FrustumPlanes[4][1] /= t;
    m_FrustumPlanes[4][2] /= t;
    m_FrustumPlanes[4][3] /= t;

    //
    // Extract the frustum's near clipping plane and normalize it.
    //

    m_FrustumPlanes[5][0] = mvp[ 3] + mvp[ 2];
    m_FrustumPlanes[5][1] = mvp[ 7] + mvp[ 6];
    m_FrustumPlanes[5][2] = mvp[11] + mvp[10];
    m_FrustumPlanes[5][3] = mvp[15] + mvp[14];

    t = (float) sqrtf( m_FrustumPlanes[5][0] * m_FrustumPlanes[5][0] + 
                      m_FrustumPlanes[5][1] * m_FrustumPlanes[5][1] + 
                      m_FrustumPlanes[5][2] * m_FrustumPlanes[5][2] );

    m_FrustumPlanes[5][0] /= t;
    m_FrustumPlanes[5][1] /= t;
    m_FrustumPlanes[5][2] /= t;
    m_FrustumPlanes[5][3] /= t;


	//	restore matricies
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


Bool GCamera::CullTest(float3& Pos, float Radius)
{
	//	returns TRUE if culled
	GIncCounter(CullTests,1);

	//	debug flag
	if ( m_CameraFlags & GCameraFlags::FailCullTests )
		return FALSE;

	/*
		todo: check if pos and radius is behind camera, or too far away
	*/
	
	//	check against frustum
	if( !SphereInsideFrustum( Pos, Radius) )
		return TRUE;

	return FALSE;
}

Bool GCamera::SphereInsideFrustum(float3& Pos, float Radius)
{
    for( int i=0;	i<m_FrustumPlaneCount;	++i )
    {
        if( m_FrustumPlanes[i][0] * Pos.x +
            m_FrustumPlanes[i][1] * Pos.y +
            m_FrustumPlanes[i][2] * Pos.z +
            m_FrustumPlanes[i][3] <= -Radius )
            return FALSE;
    }

    return TRUE;
}


Bool GCamera::PointInViewport(int2& Pos)
{
	return (	( Pos.x >= m_Viewport.x ) &&
				( Pos.x <= m_Viewport.x+m_Viewport[2] ) &&
				( Pos.y >= m_Viewport.y ) &&
				( Pos.y <= m_Viewport.y+m_Viewport[3] )
				);
}


void GCamera::CalcFrustumPlanes(int Planes, float3* pVerts, float3& ExternalCameraPosition )
{
	m_FrustumPlaneCount = Planes;
	
	int Nminus1;
	float3 edge1, edge2;
	float3 planesNormal;

    for ( int loop=0;	loop<Planes;	loop++ )
    {
        if (loop == 0)
            Nminus1 = Planes - 1;
        else
            Nminus1 = loop - 1;

        // get the normal from edges
        edge1.x = pVerts[loop].x - ExternalCameraPosition.x;
        edge1.y = pVerts[loop].y - ExternalCameraPosition.y;
        edge1.z = pVerts[loop].z - ExternalCameraPosition.z;
        edge2.x = pVerts[Nminus1].x - ExternalCameraPosition.x;
        edge2.y = pVerts[Nminus1].y - ExternalCameraPosition.y;
        edge2.z = pVerts[Nminus1].z - ExternalCameraPosition.z;

        planesNormal = edge1.CrossProduct( edge2 );
		
		m_FrustumPlanes[loop] = float4( planesNormal.x, planesNormal.y ,planesNormal.z, 0.f );
    }

}



//-------------------------------------------------------------------------
//	returns what portion of the world is viewable from 0..1 on x and y axis' by using the FOV values
//-------------------------------------------------------------------------
float2 GCamera::GetWorldVisibility()
{
	float2 Visiblity;
	Visiblity.x = FOVHorz() / 360.f;
	Visiblity.y = FOVVert() / 360.f;
	return Visiblity;
}

//-------------------------------------------------------------------------
//	returns scroll values for enviroment mapping based on the camera's viewing direction
//-------------------------------------------------------------------------
float2 GCamera::GetEnvMapScroll()
{
	float2 Scroll;

	float3 CameraVectorHorz( GetFowardVector() );
	float3 CameraVectorVert( CameraVectorHorz );

	//	get Y-Axis angle
	CameraVectorHorz.y = 0.f;
	CameraVectorHorz.Normalise();
	Scroll.x = RadToDeg( atan2f( CameraVectorHorz.x, CameraVectorHorz.z ) ) / 360.f;

	//	get elevation
	float2 xz( CameraVectorVert.x, CameraVectorVert.z );
	float y = CameraVectorVert.y;
	Scroll.y = RadToDeg( atan2f( xz.Length(), y ) );
	Scroll.y += 90.f;
	Scroll.y /= 360.f;

	return Scroll;
}


