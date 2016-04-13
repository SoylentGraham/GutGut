/*------------------------------------------------

  GTypes Header file



-------------------------------------------------*/

#ifndef __GTYPES__H_
#define __GTYPES__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GDebug.h"



//	Macros
//------------------------------------------------

typedef enum GDebugColourBase
{
	GDebugColourBase_Any=0,
	GDebugColourBase_Red,
	GDebugColourBase_Green,
	GDebugColourBase_Blue,
	GDebugColourBase_Yellow,
	GDebugColourBase_Purple,
	GDebugColourBase_Cyan,
};





//	Types
//------------------------------------------------
class GMatrix;	//	external

template <class TYPE>
class Type2;
template <class TYPE>
class Type3;
template <class TYPE>
class Type4;


typedef Type2<int>		int2;
typedef Type3<int>		int3;
typedef Type4<int>		int4;

typedef Type2<s8>		s82;

typedef Type2<float>	float2;
typedef Type3<float>	float3;
typedef Type4<float>	float4;


//	32bit variable type variable
typedef struct
{
	union
	{
		void*	p;
		int		i;
		u32		u;
		struct
		{
			u16	a,b;
		};
	};
}
GVar32;


template <class TYPE>
class Type2
{
public:
	union
	{
		TYPE	_t[2];
		struct
		{
			TYPE	x,y;
		};
	};

public:
	Type2()											{	Set(0,0);	};
	Type2(const TYPE& a)							{	Set(a,a);	};
	Type2(const TYPE a)								{	Set(a,a);	};
	Type2(const TYPE a, const TYPE b)				{	Set(a,b);	};
	Type2(const Type2<TYPE>& t)						{	Set( t.x, t.y );	};

	inline void		Set(TYPE& a, TYPE& b)			{	_t[0] = a;	_t[1] = b;	};
	inline void		Set(const TYPE a, const TYPE b)	{	_t[0] = a;	_t[1] = b;	};

	inline TYPE		LengthSq() const				{	return (x*x) + (y*y);	};
	inline TYPE		Length() const					{	return sqrtf( LengthSq() );	};
	inline TYPE		Normalise()						{	return ((TYPE)1) / Length();	};

	inline Type2<TYPE>	operator *(const TYPE& v) const 		{	return Type2<TYPE>( x*v, y*v );	};
	inline Type2<TYPE>	operator *(const Type2<TYPE>& v) const	{	return Type2<TYPE>( x*v.x, y*v.y );	};
	inline Type2<TYPE>	operator +(const TYPE& v) const			{	return Type2<TYPE>( x+v, y+v );	};
	inline Type2<TYPE>	operator +(const Type2<TYPE>& v) const	{	return Type2<TYPE>( x+v.x, y+v.y );	};
	inline Type2<TYPE>	operator -(const TYPE& v) const			{	return Type2<TYPE>( x-v, y-v );	};
	inline Type2<TYPE>	operator -(const Type2<TYPE>& v) const	{	return Type2<TYPE>( x-v.x, y-v.y );	};
	inline Type2<TYPE>	operator /(const TYPE& v) const			{	return Type2<TYPE>( x/v, y/v );	};
	inline Type2<TYPE>	operator /(const Type2<TYPE>& v) const	{	return Type2<TYPE>( x/v.x, y/v.y );	};

	inline Bool		operator==(const Type2<TYPE>& v) const		{	return ( _t[0] == v._t[0] ) && ( _t[1] == v._t[1] );	};

	inline void		operator=(const Type2<TYPE>& v);
	inline void		operator=(const Type3<TYPE>& v);
	inline void		operator=(const Type4<TYPE>& v);
	
	inline void		operator+=(const TYPE& v)				{	x += v;		y += v;	};
	inline void		operator-=(const TYPE& v)				{	x -= v;		y -= v;	};
	inline void		operator*=(const TYPE& v)				{	x *= v;		y *= v;	};
	inline void		operator/=(const TYPE& v)				{	x /= v;		y /= v;	};

	inline void		operator+=(const Type2<TYPE>& v)		{	x += v.x;		y += v.y;	};
	inline void		operator-=(const Type2<TYPE>& v)		{	x -= v.x;		y -= v.y;	};
	inline void		operator*=(const Type2<TYPE>& v)		{	x *= v.x;		y *= v.y;	};
	inline void		operator/=(const Type2<TYPE>& v)		{	x /= v.x;		y /= v.y;	};

	inline TYPE&	operator[](const int Index)		{	GDebug_CheckIndex(Index,0,2);	return _t[Index];	};
	inline			operator TYPE*()				{	return &_t[0];	};
	inline			operator const TYPE*() const	{	return &_t[0];	};
};





template <class TYPE>
class Type3
{
public:
	union
	{
		TYPE	_t[3];
		struct 
		{
			TYPE	x,y,z;
		};
	};

public:
	Type3()											{	Set(0,0,0);	};
	Type3(const TYPE& a)							{	Set(a,a,a);	};
	Type3(const TYPE a)								{	Set(a,a,a);	};
	Type3(const TYPE a, const TYPE b, const TYPE c)	{	Set(a,b,c);	};
	Type3(const Type3<TYPE>& t)						{	Set( t.x, t.y, t.z );	};
	Type3(const Type4<TYPE>& t);

	inline void		Set(TYPE& a, TYPE& b, TYPE& c)					{	_t[0] = a;	_t[1] = b;	_t[2] = c;	};
	inline void		Set(const TYPE a, const TYPE b, const TYPE c)	{	_t[0] = a;	_t[1] = b;	_t[2] = c;	};

	inline TYPE		LengthSq() const				{	return (x*x) + (y*y) + (z*z);	};
	inline TYPE		Length() const					{	return sqrtf( LengthSq() );	};

	inline Bool		operator==(const Type3<TYPE>& v) const		{	return ( _t[0] == v._t[0] ) && ( _t[1] == v._t[1] ) && ( _t[2] == v._t[2] );	};
	
	inline void		operator=(const Type2<TYPE>& v);
	inline void		operator=(const Type3<TYPE>& v);
	inline void		operator=(const Type4<TYPE>& v);

	inline void		operator+=(const TYPE v)				{	_t[0] += v;		_t[1] += v;		_t[2] += v;	};
	inline void		operator-=(const TYPE v)				{	_t[0] -= v;		_t[1] -= v;		_t[2] -= v;	};
	inline void		operator*=(const TYPE v)				{	_t[0] *= v;		_t[1] *= v;		_t[2] *= v;	};
	inline void		operator/=(const TYPE v)				{	_t[0] /= v;		_t[1] /= v;		_t[2] /= v;	};

	inline void		operator+=(const Type3<TYPE>& v)		{	x += v.x;		y += v.y;		z += v.z;	};
	inline void		operator-=(const Type3<TYPE>& v)		{	x -= v.x;		y -= v.y;		z -= v.z;	};
	inline void		operator*=(const Type3<TYPE>& v)		{	x *= v.x;		y *= v.y;		z *= v.z;	};
	inline void		operator/=(const Type3<TYPE>& v)		{	x /= v.x;		y /= v.y;		z /= v.z;	};

	inline void		operator+=(const Type4<TYPE>& v)		{	x += v.x;		y += v.y;		z += v.z;	};
	inline void		operator-=(const Type4<TYPE>& v)		{	x -= v.x;		y -= v.y;		z -= v.z;	};
	inline void		operator*=(const Type4<TYPE>& v)		{	x *= v.x;		y *= v.y;		z *= v.z;	};
	inline void		operator/=(const Type4<TYPE>& v)		{	x /= v.x;		y /= v.y;		z /= v.z;	};

	inline Type3<TYPE>	operator *(const TYPE v) const		{	return Type3<TYPE>( x*v, y*v, z*v );	};
	inline Type3<TYPE>	operator /(const TYPE v) const		{	return Type3<TYPE>( x/v, y/v, z/v );	};

	inline Type3<TYPE>	operator *(const Type3<TYPE>& v) const	{	return Type3<TYPE>( x*v.x, y*v.y, z*v.z );	};
	inline Type3<TYPE>	operator /(const Type3<TYPE>& v) const	{	return Type3<TYPE>( x/v.x, y/v.y, z/v.z );	};
	inline Type3<TYPE>	operator -(const Type3<TYPE>& v) const	{	return Type3<TYPE>( x-v.x, y-v.y, z-v.z );	};
	inline Type3<TYPE>	operator +(const Type3<TYPE>& v) const	{	return Type3<TYPE>( x+v.x, y+v.y, z+v.z );	};

	inline Type3<TYPE>	operator *(const Type4<TYPE>& v) const	{	return Type3<TYPE>( x*v.x, y*v.y, z*v.z );	};
	inline Type3<TYPE>	operator /(const Type4<TYPE>& v) const	{	return Type3<TYPE>( x/v.x, y/v.y, z/v.z );	};
	inline Type3<TYPE>	operator -(const Type4<TYPE>& v) const	{	return Type3<TYPE>( x-v.x, y-v.y, z-v.z );	};
	inline Type3<TYPE>	operator +(const Type4<TYPE>& v) const	{	return Type3<TYPE>( x+v.x, y+v.y, z+v.z );	};

	inline TYPE&	operator[](const int Index)		{	GDebug_CheckIndex(Index,0,3);	return _t[Index];	};
	inline			operator TYPE*()				{	return &_t[0];	};
	inline			operator const TYPE*() const	{	return &_t[0];	};


	//	vector functions
	inline void		Normalise()						{	(*this) *= 1.f/Length();	};			//	normalises vector
	Type3<TYPE>		Normal() const					{	return (*this) * (1.f/Length());	};	//	returns the normal of thsi vector
	inline void		Invert()						{	x=-x;	y=-y;	z=-z;	};
	inline void		Reflect(const Type3<TYPE>& UpVector);
	Type3<TYPE>		CrossProduct(const Type3<TYPE>& v) const;
	float			DotProduct(const Type3<TYPE>& v) const;
	//Bool			InsideTriangle(Type3<TYPE>& v0, Type3<TYPE>& v1, Type3<TYPE>& v2, GRPlaneEq& Plane);
	void			RotateX(float RadAng);
	void			RotateY(float RadAng);
	void			RotateZ(float RadAng);
	void			SetRotation( float Angle, float Elevation, float Length );


};




template <class TYPE>
class Type4
{
public:
	union
	{
		TYPE	_t[4];
		struct
		{
			TYPE	x,y,z,w;
		};
	};
	

public:
	Type4()											{	Set(0,0,0,0);	};
	Type4(const TYPE& a)							{	Set(a,a,a,a);	};
	Type4(const TYPE a)								{	Set(a,a,a,a);	};
	Type4(const TYPE a, const TYPE b, const TYPE c, const TYPE d)	{	Set(a,b,c,d);	};
	Type4(const Type4<TYPE>& t)						{	Set( t.x, t.y, t.z, t.w );	};
	Type4(const Type3<TYPE>& t)						{	Set( t.x, t.y, t.z, 0.f );	};
	Type4(TYPE* p)									{	Set( p[0], p[1], p[2], p[3] );	};

//	inline void		Set(TYPE& a, TYPE& b, TYPE& c, TYPE& d)						{	_t[0] = a;	_t[1] = b;	_t[2] = c;	_t[3] = d;	};
	inline void		Set(const TYPE a, const TYPE b, const TYPE c, const TYPE d)	{	_t[0] = a;	_t[1] = b;	_t[2] = c;	_t[3] = d;	};
	inline void		Set(const Type3<TYPE>& v, const TYPE d)						{	Set( v.x, v.y, v.z, d );	};
	inline void		Set(const Type4<TYPE>& v)									{	Set( v.x, v.y, v.z, v.w );	};

	inline Bool		operator==(const Type4<TYPE>& v) const		{	return ( _t[0] == v._t[0] ) && ( _t[1] == v._t[1] ) && ( _t[2] == v._t[2] ) && ( _t[3] == v._t[3] );	};

	inline void		operator=(const Type2<TYPE>& v);
	inline void		operator=(const Type3<TYPE>& v);
	inline void		operator=(const Type4<TYPE>& v);

	inline void		operator+=(const Type4<TYPE>& v)		{	_t[0] += v[0];		_t[1] += v[1];		_t[2] += v[2];		_t[3] += v[3];	};
	inline void		operator-=(const Type4<TYPE>& v)		{	_t[0] -= v[0];		_t[1] -= v[1];		_t[2] -= v[2];		_t[3] -= v[3];	};
	inline void		operator*=(const Type4<TYPE>& v)		{	_t[0] *= v[0];		_t[1] *= v[1];		_t[2] *= v[2];		_t[3] *= v[3];	};
	inline void		operator/=(const Type4<TYPE>& v)		{	_t[0] /= v[0];		_t[1] /= v[1];		_t[2] /= v[2];		_t[3] /= v[3];	};

	inline void		operator+=(const TYPE v)				{	_t[0] += v;		_t[1] += v;		_t[2] += v;		_t[3] += v;	};
	inline void		operator-=(const TYPE v)				{	_t[0] -= v;		_t[1] -= v;		_t[2] -= v;		_t[3] -= v;	};
	inline void		operator*=(const TYPE v)				{	_t[0] *= v;		_t[1] *= v;		_t[2] *= v;		_t[3] *= v;	};
	inline void		operator/=(const TYPE v)				{	_t[0] /= v;		_t[1] /= v;		_t[2] /= v;		_t[3] /= v;	};

	inline TYPE&	operator[](const int Index)		{	/*GDebug_CheckIndex(Index,0,4);*/	return _t[Index];	};
	inline			operator TYPE*()				{	return &_t[0];	};
	inline			operator const TYPE*() const	{	return &_t[0];	};

	inline void		Invert()						{	x=-x;	y=-y;	z=-z;	w=-w;	};	
	//	vector functions
	inline TYPE		LengthSq() const				{	return (x*x) + (y*y) + (z*z) + (w*w);	};
	inline TYPE		Length() const					{	return sqrtf( LengthSq() );	};
	float			DotProduct(const Type4<TYPE>& v) const;

	//	2d functions
	inline Bool		PointInside(const Type2<TYPE>& Point) const	{	return ( Point.x >= x && Point.y >= y && Point.x < z && Point.y < w );	};
};



namespace GDebug
{
	void				CheckFloat(float2& f2);
	void				CheckFloat(float3& f3);
	void				CheckFloat(float4& f4);
};




//	Declarations
//------------------------------------------------
float3	GetDebugColour(int ColourIndex,GDebugColourBase ColourBase=GDebugColourBase_Any);		//	generate a colour from an index




//	Inline Definitions
//-------------------------------------------------

//	type2
template <class TYPE>
inline void Type2<TYPE>::operator=(const Type2<TYPE>& v)	{	x=v.x;	y=v.y;	};
template <class TYPE>
inline void Type2<TYPE>::operator=(const Type3<TYPE>& v)	{	x=v.x;	y=v.y;	};
template <class TYPE>
inline void Type2<TYPE>::operator=(const Type4<TYPE>& v)	{	x=v.x;	y=v.y;	};

//	type3
template <class TYPE>
inline void Type3<TYPE>::operator=(const Type2<TYPE>& v)	{	x=v.x;	y=v.y;	};
template <class TYPE>
inline void Type3<TYPE>::operator=(const Type3<TYPE>& v)	{	x=v.x;	y=v.y;	z=v.z;	};
template <class TYPE>
inline void Type3<TYPE>::operator=(const Type4<TYPE>& v)	{	x=v.x;	y=v.y;	z=v.z;	};
template <class TYPE>
Type3<TYPE>::Type3<TYPE>(const Type4<TYPE>& t)					{	Set( t.x, t.y, t.z );	};


//	type4
template <class TYPE>
inline void Type4<TYPE>::operator=(const Type2<TYPE>& v)	{	x=v.x;	y=v.y;	};
template <class TYPE>
inline void Type4<TYPE>::operator=(const Type3<TYPE>& v)	{	x=v.x;	y=v.y;	z=v.z;	};
template <class TYPE>
inline void Type4<TYPE>::operator=(const Type4<TYPE>& v)	{	x=v.x;	y=v.y;	z=v.z;	w=v.w;	};


template <class TYPE>
float Type4<TYPE>::DotProduct(const Type4<TYPE>& v) const
{
	return( (x * v.x) + (y * v.y) + (z * v.z) + (w * v.w) );
}



//	all float3/Type3 additional function definitions in a seperate header
#include "GFloat3.h"



#endif

