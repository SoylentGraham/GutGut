/*------------------------------------------------

  GMain Header file



-------------------------------------------------*/

#ifndef __GMAIN__H_
#define __GMAIN__H_



//	Includes
//------------------------------------------------
#pragma comment( lib, "Opengl32.lib" )
#pragma comment( lib, "glu32.lib" )

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Windowsx.h>
#include <math.h>
#include <stdio.h>
#include <typeinfo>
#include "glsdk/gl.h"
#include "glsdk/glext.h"
#include "glsdk/wglext.h"
//#include "glsdk/glxext.h"




//	Macros
//------------------------------------------------
#define TRUE	1
#define FALSE	0

#define	PI				( (float)3.14159265358979323846f )
#define	PI_Deg			RadToDeg( PI )
#define TWO_PI			((float)6.283185307f)
#define HALF_PI			((float)1.570796326794895f)
#define DEG_TO_RAD		(PI/(float)180.f)
#define RAD_TO_DEG		((float)180.f/PI)
#define DegToRad(deg)	(((float)deg)*DEG_TO_RAD)
#define RadToDeg(rad)	(((float)rad)*RAD_TO_DEG)

#define g_WorldUp		float3(0,1,0)
#define g_WorldFoward	float3(0,0,1)
#define g_WorldRight	float3(1,0,0)


#define NEAR_ZERO		0.0001f

#define GMAX_STRING		2048

#define ARRAY_SIZE(a)	((int) sizeof(a)/sizeof(a[0]) )
#define GDelete(p)	\
	if ( p )		\
	{				\
		delete p;	\
		p = NULL;	\
	}				\


#define GDeleteArray(p)	\
	if ( p )			\
	{					\
		delete[] p;		\
		p = NULL;		\
	}					\


#define			GMin( a, b )			( (a) < (b) ? (a) : (b) )
#define			GMax( a, b )			( (a) > (b) ? (a) : (b) )

#define BIT(b)					(1<<(b))

#define DEFINE_U8_BITS (a,b,c,d,e,f,g,h)					( (a<<0)|(b<<1)|(c<<2)|(d<<3)|(e<<4)|(f<<5)|(g<<6)|(h<<7) )
#define DEFINE_U16_BITS(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)	( (a<<0)|(b<<1)|(c<<2)|(d<<3)|(e<<4)|(f<<5)|(g<<6)|(h<<7)|(i<<8)|(j<<9)|(k<<10)|(l<<11)|(m<<12)|(n<<13)|(o<<14)|(p<<15) )
#define DEFINE_U16_U8(a,b)									( (a<<0)|(b<<8) )					//	u8 --> u16
#define DEFINE_U32_U8(a,b,c,d)								( (a<<0)|(b<<8)|(c<<16)|(d<<24) )	//	u8 --> u32
#define DEFINE_U32_U16(a,b)									( (a<<0)|(b<<16) )					//	u16 --> u32

#define DLL_EXPORT		extern "C" __declspec(dllexport)
#define FORCE_INLINE	__forceinline

#ifdef _MSC_VER

	#define START_PACK_STRUCT		\
	#pragma pack( push, packing )	\
	#pragma pack( 1 )
	
	#define END_PACK_STRUCT			\
	#pragma pack( pop, packing )	\
	
	#define PACK_STRUCT

#elif defined( __GNUC__ )

	#define START_PACK_STRUCT
	#define END_PACK_STRUCT
	#define PACK_STRUCT	__attribute__((packed))

#endif


#define GDataSizeOf(TYPE)	GTemplate_DataSizeOf( (TYPE*)0x0 )
#define GDataSizeOfVar(Var)	GTemplate_DataSizeOf( &Var )




//	Types
//------------------------------------------------
typedef GLboolean	Bool;
typedef	GLuint		u32;
typedef	GLushort	u16;
typedef	GLubyte		u8;
typedef	GLint		s32;
typedef	GLshort		s16;
typedef	GLbyte		s8;

#include "GTypes.h"





//	Declarations
//------------------------------------------------
int APIENTRY	WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

u32				GNearestPower( int Size );				//	returns the nearest n^2 number from the number entered. rounds down
inline Bool		IsPower2(u32 Val);						//	returns true if power-of-two

char*			ExportString(const char* pString);		//	copies a string to a permanant (static) buffer then returns it
char*			ExportStringf(const char* pString,...);	//	exports a formatted string using ExportString
char*			AppendStringf(char* pOrigString, const char* pString,...);	
u16*			ExportWString(const char* pString);		//	converts a string to a unicode string




//	Inline Definitions
//-------------------------------------------------


//-------------------------------------------------------------------------
//	template function for sorting comparisons
//-------------------------------------------------------------------------
template<class TYPE>
Bool CompareIsLess(TYPE& a, TYPE& b)
{
	return a < b;
}

inline Bool IsPower2(u32 Val)
{
	return ((Val & (Val - 1)) == 0x0);
}

//-------------------------------------------------------------------------
//	template function for getting the size of a type, in case a type wants to overload it
//-------------------------------------------------------------------------
template<class TYPE>
inline int GTemplate_DataSizeOf(TYPE* p)
{
	return sizeof( TYPE );
}

//-------------------------------------------------------------------------
//	interp 2 values
//-------------------------------------------------------------------------
template<class TYPE>
inline TYPE Interp(TYPE& a,TYPE& b, float Rate)
{
	TYPE Result;
	Result = a + ( ( b - a ) * Rate );
	return Result;
}


//-------------------------------------------------------------------------
//	overloaded copy class for copying multiple elements. usually we can memcpy but 
//	in certain cases we need to do a special copy mode
//-------------------------------------------------------------------------
template<class TYPE>
inline void GCopyData(TYPE* pNewData, const TYPE* pOldData, int Elements)
{
	memcpy( pNewData, pOldData, sizeof(TYPE) * Elements );
}

//-------------------------------------------------------------------------
//	limit a variable between a min and max
//-------------------------------------------------------------------------
template<class TYPE>
inline TYPE& GLimit(TYPE& Var,const TYPE& Min, const TYPE& Max )
{
	if ( Var < Min )
		Var = Min;
	else if ( Var > Max )
		Var = Max;

	return Var;
}

//-------------------------------------------------------------------------
//	when a value goes over the max, wrap it back to the min. eg. GWrap( 13, 4, 10 ) becomes 7 [4+(13-10)]
//-------------------------------------------------------------------------
template<class TYPE>
inline TYPE& GWrap(TYPE& Var,const TYPE& Min, const TYPE& Max )
{
	TYPE Diff = Max - Min;

	while ( Var < Min )
		Var += Diff;

	while ( Var > Max )
		Var -= Diff;

	return Var;
}




#endif

