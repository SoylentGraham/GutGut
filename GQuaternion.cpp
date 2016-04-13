/*------------------------------------------------

  GQuaternion.cpp

	Quaternion implementation. rotation always in x,y,z


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GQuaternion.h"
#include "GMatrix.h"


//	globals
//------------------------------------------------



//	Definitions
//------------------------------------------------

/*
void GQuaternion::DebugDraw(float Size)
{
	//	setup debug drawing
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glColor3f( 1,1,1 );
	glLinewidth( 1.f );

	float3 posAxis = GetAxis();
	posAxis *= Size;
	float3 negAxis = posAxis * -1.f;

	glBegin( GL_LINES );
		glVertex3fv( negAxis );
		glVertex3fv( posAxis );
	glEnd();

	float BoxRad = Size * 0.2f;
	glBegin( GL_LINE_LOOP );
		glVertex3f( -BoxRad, 0, -BoxRad );
		glVertex3f(  BoxRad, 0, -BoxRad );
		glVertex3f(  BoxRad, 0,  BoxRad );
		glVertex3f( -BoxRad, 0,  BoxRad );
	glEnd();

	//	todo, draw an arrow or something

	glPopAttrib();
}

*/


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

GQuaternion::GQuaternion(const float Newx,const float Newy,const float Newz,const float Neww)
{
	SetValues( x, y, z, w );
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

GQuaternion::GQuaternion(float3 Axis,float Angle)
{
	float Temp1,Temp2;
	Temp1 = Axis.LengthSq();
	
	if ( Temp1 == 0 )
	{
		//GDebug_Print("Quaternion Axis is zero!");
		Axis = float3(0,0,1);
		Temp1 = Axis.Length();
	}
	
	Temp2 = sinf(Angle * 0.5F) / Temp1;
	SetValues(Axis.x * Temp2,Axis.y * Temp2,Axis.z * Temp2,cosf(Angle * 0.5F));
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

void GQuaternion::operator () (const float Newx,const float Newy,const float Newz)
{
	SetValues(Newx,Newy,Newz,w);
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
void GQuaternion::operator () (const float Newx,const float Newy,const float Newz,const float Neww)
{
	SetValues(Newx,Newy,Newz,Neww);
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

void GQuaternion::operator () (const GQuaternion &Other)
{
	SetValues(Other.x,Other.y,Other.z,Other.w);
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

void GQuaternion::operator () (float3 Axis,float Angle)
{
	float Temp1,Temp2;
	Temp1 = Axis.Length();
	
	if ( Temp1 == 0.f )
	{
		GDebug_Break("Quaternion Axis is zero!");
	}

	Temp2 = sinf(Angle * 0.5F) / Temp1;
	SetValues(Axis.x * Temp2,Axis.y * Temp2,Axis.z * Temp2,cosf(Angle * 0.5F));
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
void GQuaternion::operator = (const GQuaternion &Other)
{
	SetValues(Other.x,Other.y,Other.z,Other.w);
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
void GQuaternion::operator ~ ()
{
	x=-x;
	y=-y;
	z=-z;
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
void GQuaternion::SetValues(float Newx,float Newy,float Newz,float Neww)
{
	x=Newx;
	y=Newy;
	z=Newz;
	w=Neww;
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

Bool GQuaternion::operator == (const GQuaternion &Other) const
{
	if ((x==Other.x) && (y==Other.y) && (z==Other.z) && (w==Other.w)) 
		return TRUE;
	return FALSE;
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

Bool GQuaternion::operator != (const GQuaternion &Other) const
{
	if ((x!=Other.x) || (y!=Other.y) || (z!=Other.z) || (w!=Other.w)) 
		return TRUE;
	return FALSE;
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

GQuaternion GQuaternion::operator - () const
{
	return GQuaternion(-x,-y,-z,-w);
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

GQuaternion GQuaternion::operator + (const GQuaternion &Other) const
{
	GQuaternion Temp;
	Temp(Other.x+x,Other.y+y,Other.z+z,Other.w+w);
	return Temp;
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

GQuaternion GQuaternion::operator - (const GQuaternion &Other) const
{
	GQuaternion Temp;
	Temp(Other.x-x,Other.y-y,Other.z-z,Other.w-w);
	return Temp;
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

GQuaternion GQuaternion::operator * (const GQuaternion &Other) const
{
	return GQuaternion(
		w * Other.x + x * Other.w + y * Other.z - z * Other.y,
		w * Other.y + y * Other.w + z * Other.x - x * Other.z,
		w * Other.z + z * Other.w + x * Other.y - y * Other.x,
		w * Other.w - x * Other.x - y * Other.y - z * Other.z); 
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

void GQuaternion::operator += (const GQuaternion &Other)
{
	x+=Other.x;
	y+=Other.y;
	z+=Other.z;
	w+=Other.w;
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

void GQuaternion::operator -= (const GQuaternion &Other)
{
	x-=Other.x;
	y-=Other.y;
	z-=Other.z;
	w-=Other.w;
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

void GQuaternion::operator *= (const GQuaternion &Other)
{
	Bool OtherValid = Other.IsValid();
	if ( !OtherValid )
		return;

	Bool ThisValid = this->IsValid();

	if ( !ThisValid )
	{
		*this = Other;
		return;
	}

	x=w * Other.x + x * Other.w + y * Other.z - z * Other.y,
	y=w * Other.y + y * Other.w + z * Other.x - x * Other.z,
	z=w * Other.z + z * Other.w + x * Other.y - y * Other.x,
	w=w * Other.w - x * Other.x - y * Other.y - z * Other.z; 
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

void GQuaternion::operator /= (const float& Scalar)
{
	SetValues( x/Scalar, y/Scalar, z/Scalar, w/Scalar );
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

void GQuaternion::operator *= (const float &Scalar)
{
	SetValues( x*Scalar, y*Scalar, z*Scalar, w*Scalar );
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

void GQuaternion::SetEuler(float Pitch, float Yaw, float Roll)
{
	/*
	float cosy = cosf(yaw / 2.0F);
	float siny = sinf(yaw / 2.0F);
	float cosP = cosf(Pitch / 2.0F);
	float sinP = sinf(Pitch / 2.0F);
	float cosR = cosf(Roll / 2.0F);
	float sinR = sinf(Roll / 2.0F);
	SetValues(
		cosR * sinP * cosy + sinR * cosP * siny,
		cosR * cosP * siny - sinR * sinP * cosy,
		sinR * cosP * cosy - cosR * sinP * siny,
		cosR * cosP * cosy + sinR * sinP * siny
		);
	return *this;

  */
	
	GQuaternion Qx( float3(0, 0, 1), Pitch );	Qx.Normalise();
	GQuaternion Qy( float3(0, 1, 0), Yaw );		Qx.Normalise();
	GQuaternion Qz( float3(1, 0, 0), Roll );	Qx.Normalise();

	SetValues(0,0,0,0);
	*this *= Qx;
	*this *= Qy;
	*this *= Qz;

	/*
	GQuaternion Qx( float3(sinf(a/2), 0, 0), cosf(a/2) );
	GQuaternion Qy( float3(0, sinf(b/2), 0), cosf(b/2) );
	GQuaternion Qz( float3(0, 0, sinf(c/2)), cosf(c/2) );

	*this = Qx;
	*this *= Qy;
	*this *= Qz;
	*/
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
	
GQuaternion &GQuaternion::Normalise(void)
	{
	float Length=GetLength();
	x/=Length;
	y/=Length;
	z/=Length;
	w/=Length;
	return *this;
	}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
float GQuaternion::GetLength() const
{
	return sqrtf(w * w + x * x + y * y + z * z);
}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
/*
GMatrix GQuaternion::GetMatrix(void)
{
	return GMatrix(
		float4(1 - 2*(y*y) - 2*(z*z), 2*(x*y+w*z), 2*(x*z-w*y), 0 ),
		float4(2*(x*y-w*z), 1 - 2*(x*x) - 2*(z*z), 2*(y*z+w*x), 0 ),
		float4(2*(x*z+w*y), 2*(y*z-w*x), 1 - 2*(x*x) - 2*(y*y), 0 ),
		float4(0,0,0,1)
		);
}
*/
//---------------------------------------------------------------------------
// Operadores externos
//---------------------------------------------------------------------------

GQuaternion operator * (const GQuaternion &First, const GQuaternion &Second)
	{
	return GQuaternion(
			First.w * Second.x + First.x * Second.w + First.y * Second.z - First.z * Second.y,
			First.w * Second.y + First.y * Second.w + First.z * Second.x - First.x * Second.z,
			First.w * Second.z + First.z * Second.w + First.x * Second.y - First.y * Second.x,
			First.w * Second.w - First.x * Second.x - First.y * Second.y - First.z * Second.z); 
	}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

GQuaternion operator + (const GQuaternion &First, const GQuaternion &Second)
	{
	return GQuaternion(First.x+Second.x,First.y+Second.y,First.z+Second.z,First.w+Second.w);
	}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

GQuaternion operator - (const GQuaternion &First, const GQuaternion &Second)
	{
	return GQuaternion(First.x-Second.x,First.y-Second.y,First.z-Second.z,First.w-Second.w);
	}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

GQuaternion operator - (const GQuaternion &Quaternion)
	{
	return GQuaternion(-Quaternion.x, -Quaternion.y, -Quaternion.z, -Quaternion.w);
	}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

GQuaternion operator * (GQuaternion Quaternion,float Scalar)
	{
	return GQuaternion(Quaternion.x*Scalar,Quaternion.y*Scalar,Quaternion.z*Scalar,Quaternion.w*Scalar);
	}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

GQuaternion operator * (float Scalar,GQuaternion Quaternion)
	{
	return GQuaternion(Quaternion.x*Scalar,Quaternion.y*Scalar,Quaternion.z*Scalar,Quaternion.w*Scalar);
	}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------

GQuaternion RotationArc(float3 V0,float3 V1)
	{
	GQuaternion Quaternion;
	// v0.normalize(); 
	// v1.normalize();  // If vector is already unit length then why do it again?
	float3 Temp = V0.CrossProduct(V1);

	float   d = V0.DotProduct(V1);
	float   s = (float)sqrt((1+d)*2);
	Quaternion.x = Temp.x / s;
	Quaternion.y = Temp.y / s;
	Quaternion.z = Temp.z / s;
	Quaternion.w = s /2.0f;
	return Quaternion;
	}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------

float DotProduct(const GQuaternion &First, const GQuaternion &Second)
	{
	return First.x * Second.x + First.y * Second.y + First.z * Second.z + First.w * Second.w;
	}

/*
//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
GQuaternion Slerp(const GQuaternion &From, const GQuaternion &To, float Interpolation)
{
	GQuaternion Temp;
	float omega, cosO, sinO;
	float scale0, scale1;

	cosO = DotProduct(From, To);

	if (cosO < 0.0)
	{
		cosO = -cosO;
		Temp = -To;
	}
	else
	{
		Temp = -To;
	}

	if ((1.0 - cosO) > ERROR_TOLERANCE)
	{
		omega = (float)acos(cosO);
		sinO = sinf(omega);
		scale0 = sinf((1.0f - Interpolation) * omega) / sinO;
		scale1 = sinf(Interpolation * omega) / sinO;
	}
	else
	{
		scale0 = 1.0f - Interpolation;
		scale1 = Interpolation;
	}
	return From*scale0 + Temp*scale1 ;
}
*/











#define kX	0
#define kY	1
#define kZ	2
#define kW	3



void QuaternionToMatrix(const GQuaternion &Quaternion,GMatrix &Matrix)
	{
	/*
	If Quaternion is guaranteed to be a unit CQuaternion, s will always
	be 1.  In that case, this calculation can be optimized out.
	*/
	float norm,s,xx,yy,zz,xy,xz,yz,wx,wy,wz;
	norm = Quaternion.x*Quaternion.x + Quaternion.y*Quaternion.y + Quaternion.z*Quaternion.z + Quaternion.w*Quaternion.w;
	s = (norm > 0) ? 2/norm : 0;
	
	/*
	Precalculate coordinate products
	*/
	xx = Quaternion.x * Quaternion.x * s;
	yy = Quaternion.y * Quaternion.y * s;
	zz = Quaternion.z * Quaternion.z * s;
	xy = Quaternion.x * Quaternion.y * s;
	xz = Quaternion.x * Quaternion.z * s;
	yz = Quaternion.y * Quaternion.z * s;
	wx = Quaternion.w * Quaternion.x * s;
	wy = Quaternion.w * Quaternion.y * s;
	wz = Quaternion.w * Quaternion.z * s;

	/*
	Calculate 3x3 matrix from orthonormal basis
	*/

	/*
	x axis
	*/
	Matrix.GetCol(kX)[kX] = 1.0f - (yy + zz);
	Matrix.GetCol(kY)[kX] = xy + wz;
	Matrix.GetCol(kZ)[kX] = xz - wy;
	
	/*
	y axis
	*/
	Matrix.GetCol(kX)[kY] = xy - wz;
	Matrix.GetCol(kY)[kY] = 1.0f - (xx + zz);
	Matrix.GetCol(kZ)[kY] = yz + wx;
	
	/*
	z axis
	*/
	Matrix.GetCol(kX)[kZ] = xz + wy;
	Matrix.GetCol(kY)[kZ] = yz - wx;
	Matrix.GetCol(kZ)[kZ] = 1.0f - (xx + yy);

	/*
	4th row and column of 4x4 matrix
	Translation and scale are not stored in quaternions, so these
	values are set to default (no scale, no translation).
	For systems where Matrix comes pre-loaded with scale and translation
	factors, this code can be excluded.
	*/
	Matrix.GetCol(kW)[kX] = 
	Matrix.GetCol(kW)[kY] = 
	Matrix.GetCol(kW)[kZ] = 
	Matrix.GetCol(kX)[kW] = 
	Matrix.GetCol(kY)[kW] = 
	Matrix.GetCol(kZ)[kW] = 0.0;
	Matrix.GetCol(kW)[kW] = 1.0f;
	}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------

void MatrixToQuaternion(const GMatrix &Matrix,GQuaternion &Quaternion)
{
	/*
	This code can be optimized for Matrix[kW][kW] = 1, which 
	should always be true.  This optimization is excluded
	here for clarity.
	*/
	
	float Tr = Matrix.GetColConst(kX)[kX] + Matrix.GetColConst(kY)[kY] + Matrix.GetColConst(kZ)[kZ] + Matrix.GetColConst(kW)[kW],fourD;
	int i,j,k;
	
	/*
	w >= 0.5 ?
	*/
	if (Tr >= 1.0f)
	{
		fourD = 2.0f * sqrtf(Tr);
		Quaternion.w = fourD/4.0f;
		Quaternion.x = (Matrix.GetColConst(kZ)[kY] - Matrix.GetColConst(kY)[kZ])/fourD;
		Quaternion.y = (Matrix.GetColConst(kX)[kZ] - Matrix.GetColConst(kZ)[kX])/fourD;
		Quaternion.z = (Matrix.GetColConst(kY)[kX] - Matrix.GetColConst(kX)[kY])/fourD;
	}
	else
	{
		/*
		Find the largest component.  
		*/
		if (Matrix.GetColConst(kX)[kX] > Matrix.GetColConst(kY)[kY])
		{
			i = kX;
		}
		else
		{
			i = kY;
		}

		if (Matrix.GetColConst(kZ)[kZ] > Matrix.GetColConst(i)[i])
		{
			i = kZ;
		}
		
		/*
		Set j and k to point to the next two components
		*/
		j = (i+1)%3;
		k = (j+1)%3;

		/*
		fourD = 4 * largest component
		*/
		fourD = 2.0f * sqrtf(Matrix.GetColConst(i)[i] - Matrix.GetColConst(j)[j] - Matrix.GetColConst(k)[k] + 1.0 );

		/*
		Set the largest component
		*/
		switch (i)
		{
			case 0:Quaternion.x=fourD/4.0f;
			case 1:Quaternion.y=fourD/4.0f;
			case 2:Quaternion.z=fourD/4.0f;
			case 3:Quaternion.w=fourD/4.0f;
		}
		
		/*
		Calculate remaining components
		*/
		switch (j)
		{
			case 0:Quaternion.x=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[j])/fourD;
			case 1:Quaternion.y=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[j])/fourD;
			case 2:Quaternion.z=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[j])/fourD;
			case 3:Quaternion.w=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[j])/fourD;
		}

		switch (k)
		{
			case 0:Quaternion.x=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[k])/fourD;
			case 1:Quaternion.y=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[k])/fourD;
			case 2:Quaternion.z=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[k])/fourD;
			case 3:Quaternion.w=(Matrix.GetColConst(j)[i] + Matrix.GetColConst(i)[k])/fourD;
		}
		
		Quaternion.w = (Matrix.GetColConst(k)[j] - Matrix.GetColConst(j)[k])/fourD;
	}
}



#undef kX
#undef kY
#undef kZ
#undef kW



/*
GQuaternion Slerp(GQuaternion &From, GQuaternion &To, float Interpolation, Bool AllowFlip)
{ 
	float cosAngle = (From[0]*To[0]) + (From[1]*To[1]) + (From[2]*To[2]) + (From[3]*To[3]);

	float c1, c2; 
	
	//	Linear interpolation for close orientations 
	if ((1.0 - fabsf(cosAngle)) < 0.01)  
	{ 
		c1 = 1.0f - Interpolation; 
		c2 = Interpolation; 
	} 
	else
	{  
		// Spherical interpolation 
		float angle    = acosf(fabsf(cosAngle)); 
		float sinAngle = sinf(angle); 
		c1 = sinf(angle * (1.0f - Interpolation)) / sinAngle; 
		c2 = sinf(angle * Interpolation) / sinAngle; 
	} 

	// Use the shortest path 
	if ((cosAngle < 0.0) && AllowFlip) 
		c1 = -c1; 

	return GQuaternion(c1*From[0] + c2*To[0], c1*From[1] + c2*To[1], c1*From[2] + c2*To[2], c1*From[3] + c2*To[3]); 
} 
  */  
  

GQuaternion InterpQ(const GQuaternion& From, const GQuaternion& To, float Interpolation)
{
	if ( From == To )
		return From;
	
	//represent the same rotation, but when doing interpolation they are in different places in the space used for interpolation, and produce different results.

	//The solution it to check how far apart the quaternions are, and if necessary flip one so they are closer. E.g. add the lines.

	GQuaternion FlipTo( To );

	//	flip to, if neccesary
	//if (Q1.w * Q2.w + Q1.x * Q2.x + Q1.y * Q2.y + Q1.z * Q2.z < 0)
	if ( From.xyzw.DotProduct( FlipTo.xyzw ) < 0)
	{   
		//Q2.w = -Q2.w;    Q2.x = -Q2.x;    Q2.y = -Q2.y;    Q2.z = -Q2.z;
		FlipTo.Invert();
	}

	float& t = Interpolation;
	float mint = 1.f - t;
	const GQuaternion& Q1 = From;
	const GQuaternion& Q2 = FlipTo;

	// Q = Q1*(1-Interpolation) + Q2*Interpolation;   

	GQuaternion Result;
	Result.x = ( Q1.x * mint) + ( Q2.x * t );
	Result.y = ( Q1.y * mint) + ( Q2.y * t );
	Result.z = ( Q1.z * mint) + ( Q2.z * t );
	Result.w = ( Q1.w * mint) + ( Q2.w * t );

	Result.Normalise();
	
	return Result;	
}




GQuaternion Slerp(GQuaternion& From, GQuaternion& To, float Interpolation, Bool AllowFlip)
{
	GQuaternion Result;

	// Decide if one of the quaternions is backwards
	int i;
	float a = 0, b = 0;
	for ( i = 0; i < 4; i++ )
	{
		a += ( From[i]-To[i] )*( From[i]-To[i] );
		b += ( From[i]+To[i] )*( From[i]+To[i] );
	}
	if ( a > b )
		To.Invert();

	float cosom = From[0]*To[0]+From[1]*To[1]+From[2]*To[2]+From[3]*To[3];
	float sclFrom, sclTo;

	if (( 1.0f+cosom ) > 0.00000001f )
	{
		if (( 1.0f-cosom ) > 0.00000001f )
		{
			float omega = acosf( cosom );
			float sinom = sinf( omega );
			sclFrom = sinf(( 1.0f-Interpolation )*omega )/sinom;
			sclTo = sinf( Interpolation*omega )/sinom;
		}
		else
		{
			sclFrom = 1.0f-Interpolation;
			sclTo = Interpolation;
		}

		GDebug::CheckFloat(sclFrom);
		GDebug::CheckFloat(sclTo);
		
		for ( i = 0; i < 4; i++ )
			Result[i] = (float)( sclFrom*From[i]+sclTo*To[i] );
	}
	else
	{
		Result[0] = -From[1];
		Result[1] = From[0];
		Result[2] = -From[3];
		Result[3] = From[2];

		sclFrom = sinf(( 1.0f-Interpolation )*0.5f*PI );
		sclTo = sinf( Interpolation*0.5f*PI );
		for ( i = 0; i < 3; i++ )
			Result[i] = (float)( sclFrom*From[i]+sclTo*Result[i] );
	}

	return Result;
}

void GQuaternion::LookAt(const float3& Dir,const float3& WorldUp)
{
	/*
	GMatrix m;
	
	float3 z(Dir);
	z.Normalise();

	float3 x = WorldUp.CrossProduct(z); // x = y cross z
	x.Normalise();
	
	float3 y = z.CrossProduct( x ); // y = z cross x
	
	float4 w( 0, 0, 0, 0 );
	
	m.SetCol( 0, x, 0.f );
	m.SetCol( 1, y, 0.f );
	m.SetCol( 2, z, 0.f );
	m.SetCol( 3, w );

	MatrixToQuaternion( m, *this );


	/*
	float3 z(Dir); 

	//z.norm(); 
	float3 x( WorldUp * z ); // x = y cross z 

	//x.norm(); 
	float3 y( z * x ); // y = z cross x 

	// here vectors are perpendicular ich other. 
	float tr = x.x + y.y + z.z; 
	Set( float3( y.z - z.y , z.x - x.z, x.y - y.x ), tr + 1.0f ); 

	//if we could multiply our vectors that thay became the same length or multiplay the quaternion components that it all quat was scaled by some scale, than we have only one SQRT. The question is how to find that multipliers ? may be you can solve this ? Remember that we can easy get dot product between "up" and "z" 
	Normalise(); 
	
	
	*/
	float3 DirNorm = Dir.Normal();
	float3 WorldUpNorm = WorldUp.Normal();

	if ( DirNorm == WorldUpNorm )
	{
		SetIdentity();
		return;
	}

	float3 crossZ( DirNorm );
	float3 crossX = WorldUpNorm.CrossProduct( crossZ ); // x = y cross z 
	crossX.Normalise();
	float3 crossY = crossZ.CrossProduct( crossX );	// y = z cross x 

	// here vectors are perpendicular ich other. 
	float tr = crossX.x + crossY.y + crossZ.z; 
	Set( float3( crossY.z - crossZ.y , crossZ.x - crossX.z, crossX.y - crossY.x ), tr + 1.0f ); 

	//if we could multiply our vectors that thay became the same length or multiplay the quaternion components that it all quat was scaled by some scale, than we have only one SQRT. The question is how to find that multipliers ? may be you can solve this ? Remember that we can easy get dot product between "up" and "z" 
	Normalise(); 
	
}

void GQuaternion::RotateVector(float3& Vector) const
{
	if ( !IsValid() )
		return;

	GMatrix RotationMatrix;
	RotationMatrix.SetRotation(*this);
	RotationMatrix.TransformVector( Vector );
/*	
	float3 This3 = float3( x, y, z );
	//<0,w> = inv(q) * <0,v> * q
	GQuaternion Inverse(*this);
	Inverse.Invert();

	Vector = Inverse * Vector * This3;
*/	
}
