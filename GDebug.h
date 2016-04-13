/*------------------------------------------------

  GDebug Header file



-------------------------------------------------*/

#ifndef __GDEBUG__H_
#define __GDEBUG__H_



//	Includes
//------------------------------------------------
#include "GMain.h"



//	Macros
//------------------------------------------------
#if (defined(_DEBUG) || defined(GUT_MAXSDK_SUPPORT) )
	#define ENABLE_DEBUG
#endif

#define MAX_ERR_LEN	512

//	break macros
#ifdef ENABLE_DEBUG

	#define GDebug_Break						{	GDebug_SetLastBreak();	}	GDebug::Break
	#define GDebug_Print						GDebug::Print
	#define GDebug_CheckIndex(i,f,l)			{	GDebug_SetLastBreak();	GDebug::CheckIndex((i),(f),(l) );	}
	#define GDebug_SetLastBreak()				{	GDebug::g_pLastBreakFile = __FILE__;	GDebug::g_LastBreakLine = __LINE__;	GDebug::g_pLastClass = typeid(this).name();	}

	#define GDebug_BreakGlobal					{	GDebug_SetLastBreakGlobal();	}	GDebug::Break
	#define GDebug_CheckIndexGlobal(i,f,l)		{	GDebug_SetLastBreakGlobal();	GDebug::CheckIndex((i),(f),(l) );	}
	#define GDebug_SetLastBreakGlobal()			{	GDebug::g_pLastBreakFile = __FILE__;	GDebug::g_LastBreakLine = __LINE__;	GDebug::g_pLastClass = NULL;	}

#else

	#define GDebug_Break				
	#define GDebug_Print				
	#define GDebug_CheckIndex(i,f,l)	
	#define GDebug_CheckIndexClass(i,f,l,c)	

	#define GDebug_BreakGlobal				
	#define GDebug_GlobalCheckIndexGlobal(i,f,l)	

#endif

//	Types
//------------------------------------------------
class GString;




//	Declarations
//------------------------------------------------
namespace GDebug
{
	extern const char*	g_pLastBreakFile;
	extern const char*	g_pLastClass;
	extern int			g_LastBreakLine;

	Bool				Init();									//	create a console to print errors to
	void				Shutdown();								//	cleanup console

	Bool				BreakPrompt(GString& ErrStr, const char* pSrcFile=NULL, int LineNo=-1);
	Bool				Break(const char* pText,...);			//	break and print out text
	Bool				Break(GString& String);					//	break and print out text

	void				Print(char* pText);						//	print text to console/stderr
	void				Print(const char* pText,...);			//	print formatted text

	void				Show();									//	show debug console
	void				Hide();									//	hide debug console

#ifdef ENABLE_DEBUG
	void				CheckGLError();							//	get last opengl error and break if error
	void				CheckWin32Error();						//	get last win32 error and break
	void				CheckWinsockError();					//	get last winsock error and break
	void				CheckMMSystemError(int ErrorCode);		//	break if mmsystem error code is an error

	void				CheckIndex(int Index,int Min, int Max);	//	quick bounds checking function
	void				CheckFloat(float f);					//	invalid float checking (infinate and NaN's)
#else
	inline void			CheckGLError()							{	};
	inline void			CheckWin32Error()						{	};
	inline void			CheckWinsockError()						{	};
	inline void			CheckIndex(int Index,int Min, int Max)	{	};
	inline void			CheckFloat(float f)						{	};
#endif
}



//	Inline Definitions
//-------------------------------------------------



#endif

