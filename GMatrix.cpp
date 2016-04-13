/*------------------------------------------------

  GMatrix.cpp

	Column major matrix class


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GMatrix.h"
#include "GQuaternion.h"
#include "GStats.h"


//	globals
//------------------------------------------------

float GMatrix::g_Identity[16] = 
{
   1,	0,	0,	0,
   0,	1,	0,	0,	
   0,	0,	1,	0,
   0,	0,	0,	1,
};

//#define USE_OPENGL_MATRIX_MULT
GDeclareCounter(MatrixMult);
GDeclareCounter(MatrixMultOperator);


//	Definitions
//------------------------------------------------


//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
GMatrix::GMatrix(float Value)
{
    GetCol(0).Set(Value, Value, Value, Value);
    GetCol(1).Set(Value, Value, Value, Value);
    GetCol(2).Set(Value, Value, Value, Value);
    GetCol(3).Set(Value, Value, Value, Value);

	#ifdef DEBUG_MATRIX
		m_Initialised = TRUE;
	#endif
}


//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
GMatrix::GMatrix(const float4& col0, const float4& col1, const float4& col2, const float4& col3)
{
    GetCol(0) = col0;
    GetCol(1) = col1;
    GetCol(2) = col2;
    GetCol(3) = col3;
}


//---------------------------------------------------------------------------
//  Operadores internos
//---------------------------------------------------------------------------
/*float4 &GMatrix::operator [] (unsigned int Index)
{
	GDebug_CheckIndex( Index, 0, 4 );
    return Column[Index];
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
const float4 &GMatrix::operator [] (unsigned int Index) const
{
	GDebug_CheckIndex( Index, 0, 4 );
	return Column[Index];
}*/


//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
void GMatrix::operator += (const GMatrix &Other)
{
	GetCol(0) += Other[0];
	GetCol(1) += Other[1];
	GetCol(2) += Other[2];
	GetCol(3) += Other[3];
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
void GMatrix::operator -= (const GMatrix &Other)
{
	GetCol(0) -= Other[0];
	GetCol(1) -= Other[1];
	GetCol(2) -= Other[2];
	GetCol(3) -= Other[3];
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
void GMatrix::operator *= (const GMatrix &Matrix)
{
	GIncCounter(MatrixMultOperator,1);

	#ifdef USE_OPENGL_MATRIX_MULT

		glPushMatrix();
		glLoadMatrixf(Float());
		glMultMatrixf(Matrix);
		glGetFloatv( GL_MODELVIEW_MATRIX, Float() );
		glPopMatrix();

	#else

		GMatrix t;
		for ( int r = 0; r < 4; r++)
		{
			for ( int c = 0; c < 4; c++)
			{
				float f = 0;

				f += (GetColConst(0)[r]) * (Matrix.GetColConst(c)[0]);
				f += (GetColConst(1)[r]) * (Matrix.GetColConst(c)[1]);
				f += (GetColConst(2)[r]) * (Matrix.GetColConst(c)[2]);
				f += (GetColConst(3)[r]) * (Matrix.GetColConst(c)[3]);
				t.GetCol(c)[r] = f;
			}
		}

		*this = t;

	#endif
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
void GMatrix::operator *= (float Float)
{
	GetCol(0) *= Float;
	GetCol(1) *= Float;
	GetCol(2) *= Float;
	GetCol(3) *= Float;
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
void GMatrix::operator /= (float Float)
{
	GetCol(0) /= Float;
	GetCol(1) /= Float;
	GetCol(2) /= Float;
	GetCol(3) /= Float;
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
GMatrix operator + (const GMatrix &V1, const GMatrix &V2)
{
	GMatrix Return(V1);
	Return += V2;
	return Return;
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
GMatrix operator - (const GMatrix &V1, const GMatrix &V2)
{
	GMatrix ret(V1);
	ret -= V2;
	return ret;
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
GMatrix operator * (const GMatrix &V1, const GMatrix &V2)
{
	GIncCounter(MatrixMult,1);

	#ifdef USE_OPENGL_MATRIX_MULT
	{
		glPushMatrix();
		glLoadMatrixf(V1);
		glMultMatrixf(V2);
		GMatrix Result;
		glGetFloatv( GL_MODELVIEW_MATRIX, Result );
		glPopMatrix();

		return Result;
	}
	#else
	{
		GMatrix ret(V1);
		ret *= V2;
		return ret;
	}
	#endif
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
float3 operator * (const GMatrix& Other, const float3& V1)
{
//	float4 ret( V1.x, V1.y, V1.z, 0.f );
	float4 ret( V1 );
	ret = Other * ret;
	return float3(ret.x, ret.y, ret.z);
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
float3 operator * (const float3 &V1, const GMatrix &Other)
{
	float4 ret(V1);
	ret = ret * Other;
	return float3(ret.x, ret.y, ret.z);
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------

float4 operator * (const GMatrix &Other, const float4 &V1)
{
	float4 ret;
	ret.x = V1.x * Other.GetColConst(0)[0] + V1.y * Other.GetColConst(1)[0] + V1.z * Other.GetColConst(2)[0] + V1.w * Other.GetColConst(3)[0];
	ret.y = V1.x * Other.GetColConst(0)[1] + V1.y * Other.GetColConst(1)[1] + V1.z * Other.GetColConst(2)[1] + V1.w * Other.GetColConst(3)[1];
	ret.z = V1.x * Other.GetColConst(0)[2] + V1.y * Other.GetColConst(1)[2] + V1.z * Other.GetColConst(2)[2] + V1.w * Other.GetColConst(3)[2];
	ret.w = V1.x * Other.GetColConst(0)[3] + V1.y * Other.GetColConst(1)[3] + V1.z * Other.GetColConst(2)[3] + V1.w * Other.GetColConst(3)[3];
	return ret;
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
float4 operator * (const float4 &V1, const GMatrix &Other)
{
	float4 ret;
	ret.x = Other.GetColConst(0).DotProduct( V1 );
	ret.y = Other.GetColConst(1).DotProduct( V1 );
	ret.z = Other.GetColConst(2).DotProduct( V1 );
	ret.w = Other.GetColConst(3).DotProduct( V1 );
	return ret;
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------

GMatrix operator * (const GMatrix &Other, float f)
	{
	GMatrix ret(Other);
	ret *= f;
	return ret;
	}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
GMatrix operator * (float f, const GMatrix &Other)
	{
	GMatrix ret(Other);
	ret *= f;
	return ret;
	}



//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
void GMatrix::Transpose()
{
	float t;

	for ( int c = 0; c < 4; c++)
	{
		for ( int r = c + 1; r < 4; r++)
		{
			t = GetCol(c)[r];
			GetCol(c)[r] = GetCol(r)[c];
			GetCol(r)[c] = t;
		} 
	} 
}

//---------------------------------------------------------------------------
//  
//---------------------------------------------------------------------------
void GMatrix::Invert()
{
	GMatrix V1( *this );
	GMatrix V2( g_Identity );

	int r, c;
	int cc;
	int RowMax; // Points to max abs value row in this column
	int row;
	float tmp;

	// Go through columns
	for (c=0; c<4; c++)
	{
		// Find the row with max value in this column
		RowMax = c;
		for (r=c+1; r<4; r++)
		{
			if (fabs(V1.GetCol(c)[r]) > fabs(V1.GetCol(c)[RowMax]))
			{
				RowMax = r;
			}
		}

		// If the max value here is 0, we can't invert.  Return identity.
		if (V1.GetCol(RowMax)[c] == 0.0F)
		{
			SetIdentity();
			return;
		}

		// Swap row "RowMax" with row "c"
		for (cc=0; cc<4; cc++)
		{
			tmp = V1.GetCol(cc)[c];
			V1.GetCol(cc)[c] = V1.GetCol(cc)[RowMax];
			V1.GetCol(cc)[RowMax] = tmp;
			tmp = V2.GetCol(cc)[c];
			V2.GetCol(cc)[c] = V2.GetCol(cc)[RowMax];
			V2.GetCol(cc)[RowMax] = tmp;
		}

		// Now everything we do is on row "c".
		// Set the max cell to 1 by dividing the entire row by that value
		tmp = V1.GetCol(c)[c];
		for (cc=0; cc<4; cc++)
		{
			V1.GetCol(cc)[c] /= tmp;
			V2.GetCol(cc)[c] /= tmp;
		}

		// Now do the other rows, so that this column only has V1 1 and 0's
		for (row = 0; row < 4; row++)
		{
			if (row != c)
			{
				tmp = V1.GetCol(c)[row];
				for (cc=0; cc<4; cc++)
				{
					V1.GetCol(cc)[row] -= V1.GetCol(cc)[c] * tmp;
					V2.GetCol(cc)[row] -= V2.GetCol(cc)[c] * tmp;
				}
			}
		}
	}

	*this = V2;
}


void GMatrix::Rotate(const GQuaternion& q)
{
	//	make a matrix to multiply by
	GMatrix RotMatrix;
	QuaternionToMatrix( q, RotMatrix );

	//	have to do this twice...
	*this *= RotMatrix;
	*this *= RotMatrix;
}


void GMatrix::SetRotation(const GQuaternion& q)
{
	QuaternionToMatrix( q, *this );
}



void GMatrix::SetTranslate(const float3& xyz) 
{
	//SetIdentity();
	GetCol(3)[0] = xyz.x;
	GetCol(3)[1] = xyz.y;
	GetCol(3)[2] = xyz.z;
}



void GMatrix::InverseTranslateVect(float3& v)
{
	v[0] = v[0] - m_Matrix[12];
	v[1] = v[1] - m_Matrix[13];
	v[2] = v[2] - m_Matrix[14];
}


void GMatrix::InverseRotateVect(float3& v)
{
	float3 vec;

	vec[0] = v[0] * m_Matrix[0] + v[1] * m_Matrix[1] + v[2] * m_Matrix[2];
	vec[1] = v[0] * m_Matrix[4] + v[1] * m_Matrix[5] + v[2] * m_Matrix[6];
	vec[2] = v[0] * m_Matrix[8] + v[1] * m_Matrix[9] + v[2] * m_Matrix[10];

	memcpy( &v, vec, sizeof(float3) );
}


void GMatrix::TransformVector(float3& v)
{
	float vector[4];

	vector[0] = v[0]*m_Matrix[0]+v[1]*m_Matrix[4]+v[2]*m_Matrix[8]+m_Matrix[12];
	vector[1] = v[0]*m_Matrix[1]+v[1]*m_Matrix[5]+v[2]*m_Matrix[9]+m_Matrix[13];
	vector[2] = v[0]*m_Matrix[2]+v[1]*m_Matrix[6]+v[2]*m_Matrix[10]+m_Matrix[14];
	vector[3] = v[0]*m_Matrix[3]+v[1]*m_Matrix[7]+v[2]*m_Matrix[11]+m_Matrix[15];

	v[0] = vector[0];
	v[1] = vector[1];
	v[2] = vector[2];
//	v[3] = vector[3];
}



void GMatrix::SetRotationRadians(const float3& Angles)
{
	float cr = cosf( Angles[0] );
	float sr = sinf( Angles[0] );
	float cp = cosf( Angles[1] );
	float sp = sinf( Angles[1] );
	float cy = cosf( Angles[2] );
	float sy = sinf( Angles[2] );

	SetIdentity();

	m_Matrix[0] = ( float )( cp*cy );
	m_Matrix[1] = ( float )( cp*sy );
	m_Matrix[2] = ( float )( -sp );

	float srsp = sr*sp;
	float crsp = cr*sp;

	m_Matrix[4] = ( float )( srsp*cy-cr*sy );
	m_Matrix[5] = ( float )( srsp*sy+cr*cy );
	m_Matrix[6] = ( float )( sr*cp );

	m_Matrix[8] = ( float )( crsp*cy+sr*sy );
	m_Matrix[9] = ( float )( crsp*sy-sr*cy );
	m_Matrix[10] = ( float )( cr*cp );
}


//-------------------------------------------------------------------------
//	construct orientation matrix
//-------------------------------------------------------------------------
void GMatrix::SetOrientation(float3& Foward,float3& Up,float3& Left)
{

}
