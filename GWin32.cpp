/*------------------------------------------------

  GWin32.cpp

	Win32 specific calls


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GWin32.h"
#include "GApp.h"
#include "GDebug.h"
#include "GMouse.h"
#include "GWinControl.h"
#include "GWinTreeView.h"



//	globals
//------------------------------------------------

const char*	g_AllFileFilters	= "All files (*.*)\0*.*\0\0";





//	Definitions
//------------------------------------------------

Bool GWin32::Init()
{
	InitCommonControls();

	//	register mouse wheel
	if ( GWinControl::g_MouseWheelMsg == 0 )
	{
		GWinControl::g_MouseWheelMsg = RegisterWindowMessage( MSH_MOUSEWHEEL );

		if ( GWinControl::g_MouseWheelMsg == 0 )
			GDebug::CheckWin32Error();
	}

	return TRUE;
}


Bool GWin32::Update()
{
	static MSG msg;
	Sleep(0);
	
	//	win32 style app update
	if ( g_App->m_AppFlags & GAppFlags::Win32Wait )
	{
		//	wait for message
		while (GetMessage(&msg, NULL, 0, 0)) 
		{
			if ( g_App->m_AppFlags & GAppFlags::QuitApp )
				return FALSE;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	}
	else
	{
		//	process windows messages if there are any
		while ( PeekMessage(&msg,NULL,0,0,PM_REMOVE) )
		{
		//	GetMessage(&msg,NULL,0,0);
			if ( msg.message == WM_QUIT )
				return FALSE;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}


	return TRUE;
}






Bool GWin32::IsActiveWindow()
{
	return ( GetActiveWindow() == GApp::g_pApp->Hwnd() );
}




void InitFileDialog( OPENFILENAME* pDialog, HWND hwnd, char* pFileString, u32 FileNameLen, const char* pFilterString )
{
	pDialog->lStructSize = sizeof(OPENFILENAME);
	pDialog->hwndOwner = (HWND)hwnd;
//	ofn.lpstrFilter = "gba rom's (*.gba;*.bin;*.agb)\0*.gba;*.bin;*.agb\0All Files (*.*)\0*.*\0\0";
	pDialog->lpstrFilter = pFilterString;
	pDialog->nMaxFile = FileNameLen;
	pDialog->lpstrDefExt = "*";
	pDialog->Flags = OFN_EXPLORER	|	OFN_PATHMUSTEXIST	|	OFN_HIDEREADONLY	|	OFN_OVERWRITEPROMPT;
	pDialog->lpstrFile = pFileString;
}
	

Bool GWin32::OpenFileDialog( char* pFileBuffer, int BufferSize, const char* pFileFilter )
{
	if ( pFileFilter == NULL )
		pFileFilter = g_AllFileFilters;

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ZeroMemory( pFileBuffer, BufferSize );

	InitFileDialog( &ofn, g_App->Hwnd(), pFileBuffer, BufferSize, pFileFilter );

	return ( GetOpenFileName(&ofn) );
}


Bool GWin32::SaveFileDialog( char* pFileBuffer, int BufferSize, const char* pFileFilter )
{
	if ( pFileFilter == NULL )
		pFileFilter = g_AllFileFilters;

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ZeroMemory( pFileBuffer, BufferSize );

	InitFileDialog( &ofn, g_App->Hwnd(), pFileBuffer, BufferSize, pFileFilter );
	
	return ( GetSaveFileName(&ofn) );
}


Bool GWin32::OpenColourDialog(u32& Colour)
{
	//	setup dialog
	CHOOSECOLOR ChooseColour;
	ZeroMemory(&ChooseColour, sizeof(CHOOSECOLOR));
	ChooseColour.lStructSize = sizeof(CHOOSECOLOR);
	ChooseColour.hwndOwner = g_App->Hwnd();
		
	ChooseColour.Flags |= CC_RGBINIT;	//	use an initial colour
	ChooseColour.Flags |= CC_ANYCOLOR;	//	any type of colour
	ChooseColour.Flags |= CC_FULLOPEN;	//	dialog fully opened
		
//	COLORREF ColourRef;
//	ColourRef = RGB( RGBA_Red(Colour), RGBA_Green(Colour), RGBA_Blue(Colour) );
//	ChooseColour.rgbResult = ColourRef;
	ChooseColour.rgbResult = Colour;

	COLORREF acrCustClr[16];
	ChooseColour.lpCustColors = acrCustClr;

	//	open dialog
	if ( !ChooseColor( &ChooseColour ) )
		return FALSE;

	//	convert colour back to a u32 color
	Colour = ChooseColour.rgbResult;


	return TRUE;
}



GPopupResult GWin32::Popup(GPopupType Type, const char* pTitle, const char* pText )
{
	u32 Flags = 0x0;
	
	switch ( Type )
	{
		case Popup_OK:					Flags |= MB_OK|MB_ICONASTERISK;			break;
		case Popup_OKCancel:			Flags |= MB_OKCANCEL|MB_ICONQUESTION;	break;
		case Popup_YesNo:				Flags |= MB_YESNO|MB_ICONQUESTION;		break;
		case Popup_YesNoCancel:			Flags |= MB_YESNOCANCEL|MB_ICONQUESTION;	break;
		case Popup_AbortRetryIgnore:	Flags |= MB_ABORTRETRYIGNORE|MB_ICONHAND;	break;
	};

	int Result = MessageBox( NULL, pText, pTitle, Flags );

	if ( Result == 0 )
	{
		GDebug::CheckWin32Error();
		return PopupResult_Error;
	}

	switch ( Result )
	{
		case IDRETRY:	return PopupResult_OK;		break;
		case IDOK:		return PopupResult_OK;		break;
		case IDYES:		return PopupResult_Yes;		break;
		case IDNO:		return PopupResult_No;		break;
		case IDIGNORE:	return PopupResult_Ignore;	break;
		case IDCANCEL:	return PopupResult_Cancel;	break;
		case IDABORT:	return PopupResult_Cancel;	break;
		#if(WINVER >= 0x0400)
			case IDCLOSE:	return PopupResult_Cancel;	break;
		#endif
		#if(WINVER >= 0x0500)
			case IDCONTINUE:	return PopupResult_OK;		break;
			case IDTRYAGAIN:	return PopupResult_Cancel;	break;
		#endif
	};

	return PopupResult_Error;
}



GPopupResult GWin32::Popupf(GPopupType Type, const char* pTitle, const char* pText, ... )
{
	char TxtOut[1024];
	if ( strlen( pText ) >= 1024 )
	{
		GDebug_BreakGlobal("Formatted debug string too long");
	}

	va_list v;
	va_start( v, pText );
	int iChars = vsprintf((char*)TxtOut,pText, v );
	return Popup( Type, pTitle, TxtOut );
}



//-------------------------------------------------------------------------
//	reads key info into m_Data from registry
//-------------------------------------------------------------------------
Bool GWin32::GRegistryEntry::Read()
{
	GDebug_Break("GRegistryEntry::Read not implemented\n");
	return FALSE;
}

//-------------------------------------------------------------------------
//	writes key & optionally the value to registry
//-------------------------------------------------------------------------
Bool GWin32::GRegistryEntry::Write(Bool WriteValue)
{
	HKEY ResultKey = NULL;
	ULONG KeyDisposition = NULL;
	HRESULT Result = RegCreateKeyEx(	m_RootKey,
										(char*)m_SubKey,
										0,
										NULL,
										REG_OPTION_NON_VOLATILE,
										KEY_WRITE,
										NULL,
										&ResultKey,
										&KeyDisposition );

	//	bad sub key
	if ( Result == ERROR_BAD_PATHNAME )
	{
		GDebug_Break("Error writing registry invalid subkey \"%s\"", (char*)m_SubKey );
		return FALSE;
	}

	//	error
	if ( Result != ERROR_SUCCESS )
	{
		return FALSE;
	}

	//	print info
	if ( KeyDisposition == REG_CREATED_NEW_KEY )
	{
		GDebug_Print("GRegistryEntry::Write: Wrote new key\n");
	}

	//	print info
	if ( KeyDisposition == REG_OPENED_EXISTING_KEY )
	{
		GDebug_Print("GRegistryEntry::Write: Key already exists\n");
	}

	//	write value in key too
	if ( WriteValue )
	{
		Result = RegSetValueEx(	ResultKey, 
								(char*)m_ValueName,
								0,
								REG_SZ,
								(u8*)(m_ValueData.Data()),
								m_ValueData.Length() );
		RegCloseKey(ResultKey);
	
		if ( Result != ERROR_SUCCESS )
		{
			return FALSE;
		}
	}

	return TRUE;
}

//-------------------------------------------------------------------------
//	remove key or just value from registry
//-------------------------------------------------------------------------
Bool GWin32::GRegistryEntry::Remove(Bool RemoveValue)
{
	GDebug_Break("GRegistryEntry::Remove not implemented\n");
	return FALSE;
}


