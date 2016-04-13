/*------------------------------------------------

  GWinDialog Header file



-------------------------------------------------*/

#ifndef __GWINDIALOG__H_
#define __GWINDIALOG__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GWinControl.h"
#include "GString.h"


//	Macros
//------------------------------------------------
namespace GWinDialogFlags
{
	//	style flags
	const u32	Modal						= WS_POPUP;
};


//	Types
//------------------------------------------------

//-------------------------------------------------------------------------
//	base dialog handling class
//-------------------------------------------------------------------------
class GWinDialog
{
public:
	HWND		m_Hwnd;		//	dialog's window handle

public:
	GWinDialog();
	virtual ~GWinDialog();

	//	control-specific stuff
	virtual int				DialogResource()							{	return -1;	};		//	which resource index do we create the dialog from?

	//	win32 notify messages
	virtual Bool			OnButtonPress(int ButtonID)					{	return TRUE;	};	//	return TRUE to exit dialog
	virtual void			OnInit()									{	};
	virtual Bool			OnActivate()								{	return FALSE;	};	//	return TRUE if handled

	//	generic dialog stuff
	virtual Bool			ShowDialog(GWinControl* pOwner);			//	create dialog and wait for result

	//	general win32 interfacing calls which dont need to be updated
	inline HWND				Hwnd()										{	return m_Hwnd;	};
	Bool					GetDialogString(GString& String, int DialogItem);
	Bool					SetDialogString(GString& String, int DialogItem);
	Bool					GetDialogInt(int& Integer, int DialogItem);
	Bool					SetDialogInt(int& Integer, int DialogItem);
	HWND					GetDialogItem(int DialogItem);
	LRESULT					SendDialogItemMessage(int DialogItem, int Message, WPARAM wParam, LPARAM lParam);

	//	callback function, each derived class should implement its own and update this virtual func
	virtual DLGPROC			GetDialogCallback()							{	return &GWinDialog::DialogCallback;	};
	static int CALLBACK		DialogCallback(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);	//	default callbacl

};



//	Declarations
//------------------------------------------------


//	Inline Definitions
//-------------------------------------------------



#endif

