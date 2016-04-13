/*------------------------------------------------

  GQuaternion Header file



-------------------------------------------------*/

#ifndef __GQUATERNION__H_
#define __GQUATERNION__H_



//	Includes
//------------------------------------------------
#include "GMain.h"



//	Macros
//------------------------------------------------
#define ERROR_TOLERANCE 1e-6


//	Types
//------------------------------------------------
class GQuaternion
{
public:
	union
	{
		struct 
		{	
			float4		xyzw;
		};
		struct 
		{	
			float3		xyz;
			float		w;
		};
		struct
		{
			float	x;
			float	y;
			float	z;
			float	w;
		};
		struct
		{
			float	xyzw_a[4];
		};
	};

public:
	GQuaternion()			{	SetIdentity();	};
	GQuaternion(const float NewX,const float NewY,const float NewZ,const float NewW);
	GQuaternion(float3 Axis,float Angle);

	float&					operator[] (unsigned int Index) 	{	GDebug_CheckIndex(Index,0,4);	return xyzw_a[Index];	};
	void					operator () (const float NewX,const float NewY,const float NewZ);
	void					operator () (const float NewX,const float NewY,const float NewZ,const float NewW);
	void					operator () (const GQuaternion &Other);
	void					operator () (float3 Axis,float Angle);
	void					operator = (const GQuaternion &Other);// Afectação
	void					operator ~ ();// Conjugado
	void					SetValues(float NewX,float NewY,float NewZ,float NewW);

	Bool					operator == (const GQuaternion &Other) const;
	Bool					operator != (const GQuaternion &Other) const;

	GQuaternion				operator - () const;// Negação
	GQuaternion				operator + (const GQuaternion &Other) const;// Adição
	GQuaternion				operator - (const GQuaternion &Other) const;// Subtracção
	GQuaternion				operator * (const GQuaternion &Other) const;// Multiplicação

	void					operator += (const GQuaternion &Other);// Soma com afectação
	void					operator -= (const GQuaternion &Other);// Subtracção com afectação
	void					operator *= (const GQuaternion &Other);// Multiplicação com afectação

	void					operator /= (const float &Scalar);// Divisão com afectação
	void					operator *= (const float &Scalar);// Multiplicação com afectação

	void					SetEuler(float Pitch, float Yaw, float Roll);	//	radians
	GQuaternion&			Normalise();// Normalização dos vectores
	float					GetLength() const;
	float3&					GetAxis()			{	return xyz;	};
	inline Bool				IsValid() const		{	return (w!=0.f) && (xyz.LengthSq()!=0.f);	};

	inline void				SetIdentity()	{	xyzw.Set( 0, 0, 0, 1 );	};
	inline void				Set(const float3& Axis,const float Angle)	{	xyz = Axis;	w = Angle;	};
	inline void				Invert()		{	xyzw.Invert();	};
	void					LookAt(const float3& Dir,const float3& WorldUp=float3(0,1,0));
	void					RotateVector(float3& Vector) const;
};




//	Declarations
//------------------------------------------------
GQuaternion			operator * (const GQuaternion &First, const GQuaternion &Second);
GQuaternion			operator + (const GQuaternion &First, const GQuaternion &Second);
GQuaternion			operator - (const GQuaternion &First, const GQuaternion &Second);
GQuaternion			operator - (const GQuaternion &Quaternion);
GQuaternion			operator * (const GQuaternion& Quaternion,float Scalar);
GQuaternion			operator * (float Scalar,const GQuaternion& Quaternion);
GQuaternion			RotationArc(float3 V0,float3 V1);
inline float		DotProduct(const GQuaternion& First, const GQuaternion& Second);
GQuaternion			Slerp(const GQuaternion& From, const GQuaternion& To, float Interpolation, Bool AllowFlip);
GQuaternion			InterpQ(const GQuaternion& From, const GQuaternion& To, float Interpolation);
						   

void	QuaternionToMatrix(const GQuaternion& Quaternion,GMatrix& Matrix);
void	MatrixToQuaternion(const GMatrix& Matrix,GQuaternion& Quaternion);


//	Inline Definitions
//-------------------------------------------------





#endif

