/*------------------------------------------------

  GMatrix Header file



-------------------------------------------------*/

#ifndef __GMATRIX__H_
#define __GMATRIX__H_



//	Includes
//------------------------------------------------
#include "GMain.h"



//	Macros
//------------------------------------------------



//	Types
//------------------------------------------------
class GQuaternion;

class GMatrix 
{
public:
	static float	g_Identity[16];

public:
	union
	{
		struct 
		{	
			float4	m_Column[4];	
		};
		struct 
		{	
			float	m_Matrix[16];	
		};
	};

public:
	GMatrix()											{	};
	GMatrix(const float Value);
	GMatrix(const float* pValues)						{	Copy( pValues );	};
	GMatrix(const GMatrix& Other)						{	Copy( &Other.m_Matrix[0] );	};
	GMatrix(const float4& col0,const float4& col1,const float4& col2,const float4& col3);

	static inline int	DataSize()		 				{	return ( sizeof(float) * 16);	};
	inline				operator float*()				{	return &m_Matrix[0];	};
	inline				operator const float*() const	{	return &m_Matrix[0];	};
	
//	float4				&operator [] (unsigned int Index);
//	const float4		&operator [] (unsigned int Index) const;
	void				operator =  (const GMatrix& Other)	{	Copy( &Other.m_Matrix[0] );	};
	void				operator =  (const float* pFloats)	{	Copy( pFloats );	};
	void				operator += (const GMatrix &Other);
	void				operator -= (const GMatrix &Other);
	void				operator *= (const GMatrix &Other);
	void				operator *= (float Float);
	void				operator /= (float Float);

	friend inline Bool	operator == (const GMatrix &V1,	const GMatrix &V2)	{	return V1.Compare(V2);	};
	friend inline Bool	operator != (const GMatrix &V1,	const GMatrix &V2)	{	return !V1.Compare(V2);	};
	friend GMatrix		operator + (const GMatrix &V1,	const GMatrix &V2);
	friend GMatrix		operator - (const GMatrix &V1,	const GMatrix &V2);
	friend GMatrix		operator * (const GMatrix &V1,	const GMatrix &V2);
	friend float3		operator * (const GMatrix &Other, const float3 &V1);
	friend float3		operator * (const float3 &V1,	const GMatrix &Other);
	friend float4		operator * (const GMatrix &Other, const float4 &V1);
	friend float4		operator * (const float4 &V1,	const GMatrix &Other);
	friend GMatrix		operator * (const GMatrix &Other, const float Float);
	friend GMatrix		operator * (const float Float,	const GMatrix &Other);

	inline void				Set(int r, int c, const float& f)					{	GDebug_CheckIndex(r,0,4);	GetCol(c)[r] = f;	};
	inline void				SetCol(int c, const float4& f)						{	GDebug_CheckIndex(c,0,4);	m_Column[c] = f;	};
	inline void				SetRow(int r, const float4& f)						{	GDebug_CheckIndex(r,0,4);	m_Column[0][r] = f.x;	m_Column[1][r] = f.y;	m_Column[2][r] = f.z;	m_Column[3][r] = f.w;	};
	inline void				SetCol(int c, const float3& f, const float w)		{	GDebug_CheckIndex(c,0,4);	m_Column[c].x = f.x;	m_Column[c].y = f.y;	m_Column[c].z = f.z;	m_Column[c].w = w;	};
	inline void				SetRow(int r, const float3& f, const float w)		{	GDebug_CheckIndex(r,0,4);	m_Column[0][r] = f.x;	m_Column[1][r] = f.y;	m_Column[2][r] = f.z;	m_Column[3][r] = w;	};
	inline float&			Get(int r, int c)				{	GDebug_CheckIndex(c,0,4);	GDebug_CheckIndex(r,0,4);	return m_Column[c][r];	};
	inline float4&			GetCol(int c)					{	GDebug_CheckIndex(c,0,4);	return m_Column[c];	};
	inline const float&		GetConst(int r, int c) const	{	GDebug_CheckIndex(c,0,4);	GDebug_CheckIndex(r,0,4);	return m_Column[c][r];	};
	inline const float4&	GetColConst(int c) const		{	GDebug_CheckIndex(c,0,4);	return m_Column[c];	};
	
	inline void			SetIdentity()					{	Copy( g_Identity );	};
	inline Bool			IsIdentity()					{	return Compare( g_Identity );	};

	void				Transpose();
	void				Invert();
	void				Rotate(const GQuaternion& q);			//	rotate by quaternion
	void				SetRotation(const GQuaternion& q);	//	set rotation from quaternion
	void				SetTranslate(const float3& xyz);
	void				SetRotationRadians(const float3& Angles);
	void				SetOrientation(float3& Foward,float3& Up,float3& Left);	//	construct orientation matrix

	void				InverseTranslateVect(float3& v);
	void				InverseRotateVect(float3& v);
	void				TransformVector(float3& v);

private:
	inline void			Copy(const float* pFloats)			{	memcpy( &m_Matrix[0], pFloats, sizeof(float)*16 );	};
	inline Bool			Compare(const float* pFloats) const		{	return ( memcmp( &m_Matrix[0], pFloats, sizeof(float)*16 ) == 0 );	};
};



//	Declarations
//------------------------------------------------
inline int GTemplate_DataSizeOf(GMatrix* p);





//	Inline Definitions
//-------------------------------------------------
inline int GTemplate_DataSizeOf(GMatrix* p)
{
	return GMatrix::DataSize();
}





#endif

