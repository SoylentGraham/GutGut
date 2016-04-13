/*------------------------------------------------

  float3 extra function defintions



-------------------------------------------------*/

#ifndef __GFLOAT3__H_
#define __GFLOAT3__H_



//	Declarations
//------------------------------------------------

template <class TYPE>
Type3<TYPE> Type3<TYPE>::CrossProduct(const Type3<TYPE>& v) const;

template <class TYPE>
void Type3<TYPE>::Reflect(const Type3<TYPE>& UpVector);






//	Inline Definitions
//-------------------------------------------------


template <class TYPE>
Type3<TYPE> Type3<TYPE>::CrossProduct(const Type3<TYPE>& v) const
{
	Type3<TYPE> xyz;

	xyz.x = ( y * v.z ) - ( z * v.y );
	xyz.y = ( z * v.x ) - ( x * v.z );
	xyz.z = ( x * v.y ) - ( y * v.x );

	return xyz;
}


template <class TYPE>
void Type3<TYPE>::Reflect(const Type3<TYPE>& UpVector)
{
	float Prod = UpVector.DotProduct( *this );
	(*this) -= ( UpVector * ( 2.f * Prod ) );
}


template <class TYPE>
float Type3<TYPE>::DotProduct(const Type3<TYPE>& v) const
{
	return( (x * v.x) + (y * v.y) + (z * v.z) );
}


template <class TYPE>
void Type3<TYPE>::RotateX(float RadAng)
{
	float y0 = y;	float z0 = z;
	y = ( y0 * cosf( RadAng ) ) - ( z0 * sinf( RadAng ) );  
	z = ( y0 * sinf( RadAng ) ) + ( z0 * cosf( RadAng ) );
};



template <class TYPE>
void Type3<TYPE>::RotateY(float RadAng)
{
	float x0 = x;	float z0 = z;
	x = ( x0 * cosf( RadAng ) ) + ( z0 * sinf( RadAng ) );
	z = ( z0 * cosf( RadAng ) ) - ( x0 * sinf( RadAng ) );
};



template <class TYPE>
void Type3<TYPE>::RotateZ(float RadAng)
{
	float x0 = x;	float y0 = y;
	x = ( x0 * cosf( RadAng ) ) - ( y0 * sinf( RadAng ) );  
	y = ( y0 * cosf( RadAng ) ) + ( x0 * sinf( RadAng ) );
};


template <class TYPE>
void Type3<TYPE>::SetRotation( float Angle, float Elevation, float Length )
{
	//	sets a position from rotation
	y = cosf( Elevation ) * Length;
	x = sinf( Angle ) * Length;
	z = cosf( Angle ) * Length;
}




/*
template <class TYPE>
Bool Type3<TYPE>::InsideTriangle(Type3<TYPE>& v0, Type3<TYPE>& v1, Type3<TYPE>& v2, GRPlaneEq& Plane)
{
	float U[3];
	float V[3];
	float pA = fabsf( Plane[0] );
	float pB = fabsf( Plane[1] );
	float pC = fabsf( Plane[2] );

	GRVector3 mv0 = v0 - (*this);
	GRVector3 mv1 = v1 - (*this);
	GRVector3 mv2 = v2 - (*this);

	if ( pA>pB && pA>pC )
	{
		U[0] = mv0.y;
		U[1] = mv1.y;
		U[2] = mv2.y;
		V[0] = mv0.z;
		V[1] = mv1.z;
		V[2] = mv2.z;
	}
	else if ( pB>pC )
	{
		U[0] = mv0.x;
		U[1] = mv1.x;
		U[2] = mv2.x;
		V[0] = mv0.z;
		V[1] = mv1.z;
		V[2] = mv2.z;
	}
	else
	{
		U[0] = mv0.x;
		U[1] = mv1.x;
		U[2] = mv2.x;
		V[0] = mv0.y;
		V[1] = mv1.y;
		V[2] = mv2.y;
	}

	float uvTest = Min( U[0], U[1] );
	if ( Min( U[2], uvTest ) > 0.0f ) return FALSE;
	uvTest = Max( U[0], U[1] );
	if ( Max( U[2], uvTest ) < 0.0f ) return FALSE;


	uvTest = Min( V[0], V[1] );
	if ( Min( V[2], uvTest ) > 0.0f ) return FALSE;
	uvTest = Max( V[0], V[1] );
	if ( Max( V[2], uvTest ) < 0.0f ) return FALSE;


	float Vi,Vj;
	float Ui,Uj;
	int i,j,Count=0;

	for ( i=0,j=1;	i<3;	i++,j++ )
	{
		if ( j==3 )	j=0;
		Vi = V[i];
		Vj = V[j];
		if ( Vi<0 && Vj>=0 || Vi>=0 && Vj<0 )
		{
			Ui = U[i];
			Uj = U[j];
			if ( Ui>=0 && Uj>=0 )
			{
				Count++;
			}
			else if ( Ui>=0 || Uj>=0 )
			{
				if ( Vj-Vi != 0 )
					if ( (Ui-Vi) * (Vj-Ui) / (Vj-Vi) > 0 )
						Count++;
			}
		}
	}

	return Count % 2;
}
*/





#endif

