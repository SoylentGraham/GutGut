/*------------------------------------------------

  GWin32 Header file



-------------------------------------------------*/

#ifndef __GWIN32__H_
#define __GWIN32__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GApp.h"

#include <ZMouse.h>
#include <commdlg.h>
#include <commctrl.h>
#include <shellapi.h>
#include <winsock.h>

#pragma comment( lib, "comctl32.lib" )
#pragma comment( lib, "WSOCK32.LIB" )


//	Macros
//------------------------------------------------


typedef enum GPopupType
{
	Popup_OK=0,
	Popup_OKCancel,
	Popup_YesNo,
	Popup_YesNoCancel,
	Popup_AbortRetryIgnore,
};

typedef enum GPopupResult
{
	PopupResult_Error=0,
	PopupResult_OK,			//	retry, continue
	PopupResult_Cancel,		//	abort, try again
	PopupResult_Yes,
	PopupResult_No,
	PopupResult_Ignore,
};

//	Types
//------------------------------------------------

namespace GWin32
{
	class GRegistryEntry
	{
	public:
	   HKEY		m_RootKey;
	   GString	m_SubKey;
	   GString	m_ValueName;
	   GString	m_ValueData;

	public:
		
		Bool	Read();								//	reads key info into m_Data from registry
		Bool	Write(Bool WriteValue);				//	writes key & optionally the value to registry
		Bool	Remove(Bool RemoveValue);			//	remove key or just value from registry
	};

};


//	Declarations
//------------------------------------------------
namespace GWin32
{
	Bool			Init();
	Bool			Update();

	Bool			IsActiveWindow();
	Bool			OpenFileDialog( char* pFileBuffer, int BufferSize, const char* pFileFilter=NULL );	//	if no filter passed, defaults to all files
	Bool			SaveFileDialog( char* pFileBuffer, int BufferSize, const char* pFileFilter=NULL );	//	if no filter passed, defaults to all files
	Bool			OpenColourDialog(u32& Colour);

	GPopupResult	Popup(GPopupType Type, const char* pTitleString, const char* pString );
	GPopupResult	Popupf(GPopupType Type, const char* pTitleString, const char* pString, ... );
};






//	Inline Definitions
//-------------------------------------------------



#endif

