/*------------------------------------------------

  GDisplay.cpp

	Opengl/window handling wrapper


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GDisplay.h"
#include "GWin32.h"
#include "GDebug.h"
#include "GApp.h"
#include "GAsset.h"
#include "GStats.h"
#include "GShader.h"

//	globals
//------------------------------------------------

//	cached debug items
GList<GDebugPoint>		g_DebugPoints;
GList<GDebugSphere>		g_DebugSpheres;
GList<GDebugPosition>	g_DebugPositions;
GList<GDebugLine>		g_DebugLines;

float4		GDisplay::g_DebugColour( 1.0f, 0.5f, 0.0f, 1.f );
Bool		GDisplay::g_OpenglInitialised = FALSE;
GDisplay*	GDisplay::g_pDisplay = NULL;					//	global display object

//	initialises global default
GDrawInfo	GDrawInfo::g_Default(TRUE);	


GDeclareCounter(DrawCounter);
GDeclareCounter(CullTests);
GDeclareCounter(DebugPolys);
GDeclareCounter(DrawPolys);

//	Definitions
//------------------------------------------------


float3 ClosestPointOnLine(float3& a, float3& b, float3& p)
{
   //	Determine t (the length of the vector from ‘a’ to ‘p’)
   float3 c(p-a);
   float3 v(b-a);
   v.Normalise();
   
   float d = (a-b).Length();
   float t = (v*c).Length();

   // Check to see if ‘t’ is beyond the extents of the line segment
   if (t < 0)	return a;
   if (t > d)	return b;
 
   // Return the point between ‘a’ and ‘b’
	v *= t;	//	set length of V to t;
   
   return a + v;
}

float3 ClosestPointOnTriangle(float3& a, float3& b, float3& c, float3& p)
{
	float3 Rab = ClosestPointOnLine( a, b, p );
	float3 Rac = ClosestPointOnLine( a, c, p );
	float3 Rbc = ClosestPointOnLine( b, c, p );

	float rABtoP = (p - Rab).LengthSq();
	float rACtoP = (p - Rac).LengthSq();
	float rBCtoP = (p - Rbc).LengthSq();

	if ( rABtoP < rACtoP )
	{
		return rABtoP < rBCtoP ? Rab : Rbc;
	}
	else
	{
		return rACtoP < rBCtoP ? Rac : Rbc;
	}

	//return closest [Rab, Rbc, Rca] to p;
}



Bool SameSide(float3& p1, float3& p2, float3& a, float3& b)
{
    float3 cp1 = ( b - a ).CrossProduct( p1 - a );
    float3 cp2 = ( b - a ).CrossProduct( p2 - a );
    if ( cp1.DotProduct(cp2) >= 0.f )
		return TRUE;
    return FALSE;
}



Bool PointInsideTriangle(float3& Point, float3& v0, float3& v1, float3& v2, GPlane& Plane)
{

    if ( SameSide(Point,v0, v1,v2) &&
		 SameSide(Point,v1, v0,v2) &&
		 SameSide(Point,v2, v0,v1) )
		 return TRUE;

    return FALSE;

}
		
float TriangleSurfaceArea(float3& va, float3& vb, float3& vc)
{
	//	no surface area if any verts match positions
	if ( va == vb || vb == vc || vc == va )
		return 0.f;

	//	get side lengths
	float a = (va-vb).Length();	//	a-b
	float b = (vb-vc).Length();	//	b-c
	float c = (vc-va).Length();	//	c-a

	if ( a<=NEAR_ZERO )		return 0.f;
	if ( b<=NEAR_ZERO )		return 0.f;
	if ( c<=NEAR_ZERO )		return 0.f;

	//	heron/hero's formula
	//	If a, b, and c are the side lengths of a triangle, and we let s=(a+b+c)/2, then the area of the triangle is: 
	//	A = sqrt{s(s-a)(s-b)(s-c)}
	float s = ( a+b+c ) / 2.f;
	
	float area = sqrtf( s * (s-a) * (s-b) * (s-c) );
	
	return area;
}


void GPlane::CalcEquation( float3& v0, float3& v1, float3& v2 )
{
	float3 Cross = (v1-v0).CrossProduct( v2-v1 );
	x = Cross.x;
	y = Cross.y;
	z = Cross.z;
	w = 0.f;
	
	float rL = float3(x,y,z).Length();
	if ( fabsf( rL ) != 0.f )
	{
		(*this) /= rL;
		w = - DotProduct( float4( v0.x, v0.y, v0.z, 0.f ) );
	}
	else
	{
		Set( 1.f, 0.f, 0.f, 0.f );
	}
}

Bool GPlane::Intersection( float& IntLength, float3& Pos, float3& Dir )
{
	//	dir.w = 0
	//	pos.w = 1

	float Div = DotProduct( float4( Dir.x, Dir.y, Dir.z, 0.f ) );
	if ( fabsf( Div ) < 1.0e-10f )
	{
		return FALSE;
	}

	IntLength = - DotProduct( float4( Pos.x, Pos.y, Pos.z, 1.f ) ) / Div;
	return TRUE;
}




void GDrawInfo::InitDefault()
{
	TextureRef		= GAssetRef_Invalid;
	TextureRef2		= GAssetRef_Invalid;
	Flags			= 0x0;
	RGBA			= float4( 1,1,1,1 );

	Translation		= float3(0,0,0);
	WorldPos		= float3(0,0,0);
	pRotation		= NULL;
	pLight			= NULL;
	pShader			= NULL;
	pVertexColours	= NULL;
	pPixelShader	= NULL;
}




GAppWindow::GAppWindow()
{
	m_pOwnerApp	= NULL;
}


GAppWindow::~GAppWindow()
{
}
	



//------------------------------------------------

void GBounds::DebugDraw(float3& ThisPos,float4 Colour)
{
	if ( ! IsValid() )
		return;
	float3 Pos = ThisPos + m_Offset;
	g_Display->DebugSphere( Pos, Colour, m_Radius );
}


Bool GBounds::IsCulled(float3& ThisPos,GCamera* pCamera)
{
	//	no camera provided, use current actuve camera
	if ( !pCamera )
		pCamera = GCamera::g_pActiveCamera;
	
	if ( !pCamera )
	{
		GDebug_Break("No current active camera\n");
		return FALSE;
	}

	if ( ! IsValid() )
		return FALSE;

	//	test against active camera
	float3 Pos = ThisPos + m_Offset;

	return (pCamera->CullTest( Pos, m_Radius ));
}


Bool GBounds::Intersects(float3& ThisPos, GBounds& Bounds, float3& BoundsPos )
{
	//	does any part of this bounds overlap the bounds passed in?
	float3 PosA = ThisPos + m_Offset;
	float3 PosB = BoundsPos + Bounds.m_Offset;

	float Dist = (PosA - PosB).LengthSq();
	Dist -= (m_Radius+Bounds.m_Radius)*(m_Radius+Bounds.m_Radius);
	if ( Dist <= 0.f )
		return TRUE;

	return FALSE;
}


Bool GBounds::Inside(float3& ThisPos, float3& Position)
{
	//	is position inside this bounds?
	float3 Pos = ThisPos + m_Offset;
	float DistSq = (Pos - Position).LengthSq();

	if ( DistSq > (m_Radius*m_Radius) )
		return FALSE;
	
	return TRUE;
}


Bool GBounds::Inside(float3& ThisPos, GBounds& Bounds, float3& BoundsPos )
{
	//	is the bounds fully inside this bounds?

	//	if the radius of the bounds is bigger than our bounds, obviously it cant fit in
	if ( Bounds.m_Radius >= m_Radius )
		return FALSE;

	float3 PosA = ThisPos + m_Offset;
	float3 PosB = BoundsPos + Bounds.m_Offset;
	float DistSq = (PosA - PosB).LengthSq();
	float RadASq = (m_Radius * m_Radius);
	float RadBSq = (Bounds.m_Radius * Bounds.m_Radius);

	if ( DistSq + RadBSq > RadASq )
		return FALSE;

	return TRUE;
}




//------------------------------------------------

void GDebugPoint::Draw()
{
	glColor4f( Colour[0], Colour[1], Colour[2], Colour[3] );
	glPointSize( 3.f );
	
	//	draw a point
	glBegin( GL_POINTS );
		glVertex3fv( Pos );
	glEnd();
	
	//	draw an axis on this point
	glBegin( GL_LINES );
		glVertex3fv( Pos - float3(0,1,0) );
		glVertex3fv( Pos + float3(0,1,0) );

		glVertex3fv( Pos - float3(1,0,0) );
		glVertex3fv( Pos + float3(1,0,0) );

		glVertex3fv( Pos - float3(0,0,1) );
		glVertex3fv( Pos + float3(0,0,1) );
	glEnd();

	GIncCounter(DebugPolys,4);
}




void GDebugSphere::Draw()
{

	glColor4fv( Colour );
	glTranslatef( Pos.x, Pos.y, Pos.z );
	
	static u32	g_DebugSphereDisplayList = 0;
	static int	g_DebugPolysInDisplayList = 0;

	//	havent compiled a display list yet
	if ( g_DebugSphereDisplayList == 0 )
	{
		g_DebugSphereDisplayList = glGenLists(1);
		glNewList( g_DebugSphereDisplayList, GL_COMPILE );
		g_DebugPolysInDisplayList = 0;

		int CircleElements = 20;	//	lines to make up a circle
		float CircleElementAngleDelta = DegToRad( 360.f / CircleElements );
		int i,j;
		float3 LinePoint;

		LinePoint = float3( 0,1,0 );
		for ( j=0;	j<CircleElements;	j++ )
		{
			glBegin( GL_LINE_LOOP );
				for ( i=0;	i<CircleElements;	i++ )
				{
					LinePoint.RotateY( CircleElementAngleDelta );
					glVertex3fv( LinePoint );
				}				
				g_DebugPolysInDisplayList += CircleElements;
			glEnd();
			
			LinePoint.RotateX( CircleElementAngleDelta );
		}
		
		LinePoint = float3( 0,1,0 );
		for ( j=0;	j<CircleElements;	j++ )
		{
			glBegin( GL_LINE_LOOP );
				for ( i=0;	i<CircleElements;	i++ )
				{
					LinePoint.RotateZ( CircleElementAngleDelta );
					glVertex3fv( LinePoint );
				}				
				g_DebugPolysInDisplayList += CircleElements;
			glEnd();
			
			LinePoint.RotateX( CircleElementAngleDelta );
		}
		glEndList();
	}
	
	glScalef( Radius, Radius, Radius );
	glCallList( g_DebugSphereDisplayList );
	GIncCounter(DebugPolys,g_DebugPolysInDisplayList);
}

void GDebugPosition::Draw()
{
	glColor3fv( Colour );
	g_Display->Translate( Pos, &Rot );

	//	draw little arrow facing foward
	const float ArrowSize = 0.4f;
	glLineWidth( 2.f );
	glBegin( GL_LINE_LOOP );
		glVertex3f( -ArrowSize,0,0 );
		glVertex3f(  ArrowSize,0,0 );
		glVertex3f( 0,0,-ArrowSize*4.f );
	glEnd();

	GIncCounter(DebugPolys,3);
}

void GDebugLine::Draw()
{
	glColor3fv( Colour );
	
	glBegin( GL_LINES );
		glVertex3fv( Pos );
		glVertex3fv( PosTo );
	glEnd();

	GIncCounter(DebugPolys,1);
}

//------------------------------------------------


GDisplay::GDisplay()
{
	m_pWindow		= NULL;
	
	if ( GDisplay::g_pDisplay )
	{
		GDebug_Break("Display object already exists\n");
	}

	GDisplay::g_pDisplay = this;
}


GDisplay::~GDisplay()
{
	Shutdown();

	//	unset global display object
	if ( GDisplay::g_pDisplay == this )
		GDisplay::g_pDisplay = NULL;
}





Bool GDisplay::Init()
{
	//	can only make a display if we've been assigned an app
	if ( !Window() )
	{
		GDebug_Break("Window not assigned for display initialisation\n");
		return FALSE;
	}
	
	//	init extensions
	if ( !m_Extensions.Init() )
		return FALSE;

	return TRUE;
}



void GDisplay::Shutdown()
{
	//	clear out debug markers
	ClearDebugItems();
}



void GDisplay::Draw()
{
	//	initialise draw
	GIncCounter(DrawCounter,1);

	//	do game drawing
	if ( g_App )
	{
		g_App->GameDraw();
	}
	GDebug::CheckGLError();

	//	sync fps with hardware
	Bool SyncFps = TRUE;

	if ( g_App )
	{
		SyncFps = (g_App->m_AppFlags & GAppFlags::SyncFrames)!=0x0;
	}
	
	if ( Ext().HardwareEnabled( GHardware_SwapInterval ) )
	{
		if ( SyncFps )
			Ext().glSwapIntervalEXT()( 1 );
		else
			Ext().glSwapIntervalEXT()( 0 );
	}
	else
	{
		//	software frame limiter
		if ( SyncFps )
		{
			/*
			int FrameNo = g_StatsCounterList.GetCounterThisSecondValue("UpdateCounter");
			if ( FrameNo >= FIXED_FRAME_RATE-1 )
				FrameNo = 0;
			
			//	we should wait until this time to update
			float FrameTime = (float)FrameNo * (1.f/(float)(FIXED_FRAME_RATE-1));

			//	wait until we've reached the desired frame time
			float TimeInSec = m_pParentApp->m_Stats.GetTimeInSec();
			while ( TimeInSec < FrameTime )
			{
				TimeInSec = m_pParentApp->m_Stats.GetTimeInSec();
				GDebug::Print("%d: %4.4f < %4.4f\n", FrameNo, TimeInSec, FrameTime );
				//return TRUE;
				//Sleep(1);
			}
			*/
		}
	}

	//	flip
	SwapBuffers( Window()->m_HDC );
	GDebug::CheckGLError();

	//	clear out debug markers
	ClearDebugItems();

	//	once we've finished drawing, NULL the active camera, just in case anything tries to use it
	GCamera::g_pActiveCamera = NULL;


}







int2 GDisplay::ScreenSize()
{
	int2 Size;
	Size.x = GetSystemMetrics(SM_CXSCREEN);
	Size.y = GetSystemMetrics(SM_CYSCREEN);
	return Size;
}



void GDisplay::Translate(float3& Transform, GQuaternion* pRotation, Bool DoTranslation, Bool DoRotation)
{
	//	apply transform and rotation to the modelview matrix

	//	apply translation
	if ( DoTranslation )
	{
		glTranslatef( Transform.x, Transform.y, Transform.z );
	}

	//	apply rotation
	if ( DoRotation && pRotation )
	{
		if ( pRotation->IsValid() )
		{
			Translate( *pRotation );
		}
	}
}


void GDisplay::Translate(GMatrix& Matrix)
{
	glMultMatrixf( Matrix );
}


void GDisplay::Translate(GQuaternion& Quaternion)
{
	GMatrix RotMatrix;
	QuaternionToMatrix( Quaternion, RotMatrix );
	Translate( RotMatrix );
	/*
	float scale,x,y,z,theta;
	scale = (pRotation->x*pRotation->x) + (pRotation->y*pRotation->y) + (pRotation->z*pRotation->z);
	if ( scale < NEAR_ZERO )
		scale = 1.f;
	x = pRotation->x / scale;
	y = pRotation->y / scale;
	z = pRotation->z / scale;
	theta = 2.f * acosf( pRotation->w );
	glRotatef( theta, x, y, z );			
	*/
}



void GDisplay::PushScene()
{
	glPushMatrix();
	glPushAttrib( GL_ALL_ATTRIB_BITS );
}


void GDisplay::PopScene()
{
	glPopAttrib();
	glPopMatrix();
}


void GDisplay::DebugPoint(float3& Pos, float4& Colour)
{
	GDebugPoint p;
	p.Pos		= Pos;
	p.Colour	= Colour;
	g_DebugPoints.Add( p );
}


void GDisplay::DebugPosition(float3& Pos, float4& Colour, GQuaternion& Rot)
{
	GDebugPosition p;
	p.Pos		= Pos;
	p.Colour	= Colour;
	p.Rot		= Rot;
	g_DebugPositions.Add( p );
}


void GDisplay::DebugSphere(float3& Pos, float4& Colour, float Radius)
{
	GDebugSphere p;
	p.Pos		= Pos;
	p.Colour	= Colour;
	p.Radius	= Radius;
	g_DebugSpheres.Add( p );
}


void GDisplay::DebugLine(float3& Pos, float4& Colour, float3& PosTo)
{
	GDebugLine p;
	p.Pos		= Pos;
	p.Colour	= Colour;
	p.PosTo		= PosTo;
	g_DebugLines.Add( p );
}


void GDisplay::DrawDebugItems()
{
	//	setup scene for debug display
	PushScene();
	glDisable( GL_DEPTH_TEST );
	glColor3f( 1,1,1 );


	int i;

	for ( i=0;	i<g_DebugPoints.Size();	i++ )
	{
		PushScene();
		g_DebugPoints[i].Draw();
		PopScene();
	}

	for ( i=0;	i<g_DebugSpheres.Size();	i++ )
	{
		PushScene();
		g_DebugSpheres[i].Draw();
		PopScene();
	}

	for ( i=0;	i<g_DebugPositions.Size();	i++ )
	{
		PushScene();
		g_DebugPositions[i].Draw();
		PopScene();
	}

	for ( i=0;	i<g_DebugLines.Size();	i++ )
	{
		PushScene();
		g_DebugLines[i].Draw();
		PopScene();
	}

	
	ClearDebugItems();
	PopScene();
}



void GDisplay::ClearDebugItems()
{
	g_DebugPoints.Empty();
	g_DebugSpheres.Empty();
	g_DebugPositions.Empty();
	g_DebugLines.Empty();
}


float3 CalcNormal(float3& va, float3& vb, float3& vc )
{
	//	calculate normal
	float3 p,q;
	p = float3( vb.x - va.x,	vb.y - va.y,	vb.z - va.z	);
	q = float3( vc.x - va.x,	vc.y - va.y,	vc.z - va.z	);

	GDebug::CheckFloat(p);
	GDebug::CheckFloat(q);

	float3 Normal = p.CrossProduct(q);
	
	GDebug::CheckFloat(Normal);

/*	if ( Normal.Length() < NEAR_ZERO )
	{
		//GDebug_Break("Normal length is zero\n");
		GPlane Plane;
		Plane.CalcEquation( va, vb, vc );
		Normal = float3( Plane.f() );
		
		if ( Normal.Length() < NEAR_ZERO )
		{
			GDebug_Break("Normal length is zero\n");
		}
	}
*/
	Normal.Normalise();
	GDebug::CheckFloat(Normal);
	
	return Normal;
}


float2 GDisplay::ObjectToScreen(float3& WorldPos) 
{
	float4 Pos4 = float4( WorldPos.x, WorldPos.y, WorldPos.z, 0.f );
    
    // Get the matrices and viewport

    float modelView[16];
    float projection[16];
    float viewport[4];
    float depthRange[2];

    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glGetFloatv(GL_VIEWPORT, viewport);
    glGetFloatv(GL_DEPTH_RANGE, depthRange);

    // Compose the matrices into a single row-major transformation


    float4 T[4];
    int r, c, i;
    for (r = 0; r < 4; ++r) {
        for (c = 0; c < 4; ++c) {
            T[r][c] = 0;
            for (i = 0; i < 4; ++i) {
                // OpenGL matrices are column major


                T[r][c] += projection[r + i * 4] * modelView[i + c * 4];
            }
        }
    }

    // Transform the vertex

    float4 result;
    for (r = 0; r < 4; ++r) {
        result[r] = T[r].DotProduct(Pos4);
    }

    // Homogeneous divide

    const float rhw = 1 / result.w;

	float4 Result(	(1 + result.x * rhw) * viewport[2] / 2 + viewport[0],
					(1 - result.y * rhw) * viewport[3] / 2 + viewport[1],
					(result.z * rhw) * (depthRange[1] - depthRange[0]) + depthRange[0],
					rhw );

	return float2( Result.x, Result.y );
} 
