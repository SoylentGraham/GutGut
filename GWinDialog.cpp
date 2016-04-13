/*------------------------------------------------

  GWinDialog.cpp

	Class for handling dialogs
	

-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GWinDialog.h"
#include "GApp.h"


//	globals
//------------------------------------------------
GList<GWinDialog*>	g_ActiveWinDialogs;		//	list of existing dialogs. used to match HWND's with dialogs

//	Definitions
//------------------------------------------------


GWinDialog::GWinDialog()
{
}


GWinDialog::~GWinDialog()
{
}


//-------------------------------------------------------------------------
//	create dialog
//-------------------------------------------------------------------------
Bool GWinDialog::ShowDialog(GWinControl* pOwner)
{
	int DialogResult = IDCANCEL;

	//	add dialog to list
	g_ActiveWinDialogs.Add( this );

	//	create dialog
	DialogResult = DialogBoxParam( GApp::g_HInstance, MAKEINTRESOURCE( DialogResource() ), pOwner->Hwnd(), GetDialogCallback(), (u32)this );

	if ( DialogResult == -1 )
	{
		GDebug::CheckWin32Error();
	}

	//	remove dialog from list
	int Index = g_ActiveWinDialogs.FindIndex(this);
	g_ActiveWinDialogs.RemoveAt( Index );

	return (DialogResult == IDOK);
}

//-------------------------------------------------------------------------
//	default dialog callback
//-------------------------------------------------------------------------
/*static*/int CALLBACK GWinDialog::DialogCallback(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//	get dialog with this hwnd
	GWinDialog* pDialog = NULL;
	for ( int d=0;	d<g_ActiveWinDialogs.Size();	d++ )
		if ( g_ActiveWinDialogs[d]->Hwnd() == hwndDlg )
			pDialog = g_ActiveWinDialogs[d];

	//	handle message
	switch ( message )
	{
		case WM_INITDIALOG:
			//	address of dialog is lParam
			pDialog = (GWinDialog*)lParam;
			pDialog->m_Hwnd = hwndDlg;
			pDialog->OnInit();
			return TRUE;
			break;

		case WM_COMMAND:
			//	button pressed, if TRUE returned, exit dialog
			if ( pDialog->OnButtonPress( LOWORD(wParam) ) )
			{
				EndDialog( hwndDlg, LOWORD(wParam) );
			}
			return TRUE;
			break;

		case WM_ACTIVATE:
			if ( LOWORD( wParam ) == WA_ACTIVE )
			{
				if ( pDialog->OnActivate() )
					return 0;
			}
			break;

		case WM_SETFONT:
		case WM_WINDOWPOSCHANGING:
		case WM_WINDOWPOSCHANGED:
		case WM_NCACTIVATE:
		case WM_USER:
		//case WM_CHANGEUISTATE:
		//case WM_UPDATEUISTATE:
		//case WM_QUERYUISTATE:
			break;
	};

	return FALSE;
}

//-------------------------------------------------------------------------
//	get the string from a dialog text object
//-------------------------------------------------------------------------
Bool GWinDialog::GetDialogString(GString& String, int DialogItem)
{
	String.Chars().Resize(200);
	String.Chars()[0] = 0;
	
	if ( !GetDlgItemText( Hwnd(), DialogItem, String.Data(), String.Length() ) )
	{
		String.Clear();
		return FALSE;
	}

	return TRUE;
}

//-------------------------------------------------------------------------
//	set the string on a dialog text item
//-------------------------------------------------------------------------
Bool GWinDialog::SetDialogString(GString& String, int DialogItem)
{
	if ( !SetDlgItemText( Hwnd(), DialogItem, (char*)String ) )
	{
		return FALSE;
	}

	return TRUE;
}

//-------------------------------------------------------------------------
//	get the string from a dialog text object
//-------------------------------------------------------------------------
Bool GWinDialog::GetDialogInt(int& Integer, int DialogItem)
{
	int Success = FALSE;

	Integer = GetDlgItemInt( Hwnd(), DialogItem, &Success, TRUE );

	return Success;
}

//-------------------------------------------------------------------------
//	set the string on a dialog text item
//-------------------------------------------------------------------------
Bool GWinDialog::SetDialogInt(int& Integer, int DialogItem)
{
	if ( !SetDlgItemInt( Hwnd(), DialogItem, Integer, TRUE ) )
	{
		return FALSE;
	}

	return TRUE;
}

//-------------------------------------------------------------------------
//	return handle of an item on the dialog
//-------------------------------------------------------------------------
HWND GWinDialog::GetDialogItem(int DialogItem)
{
	return GetDlgItem( Hwnd(), DialogItem );
}

//-------------------------------------------------------------------------
//	send a message to a dialog item
//-------------------------------------------------------------------------
LRESULT GWinDialog::SendDialogItemMessage(int DialogItem, int Message, WPARAM wParam, LPARAM lParam)
{
	return SendDlgItemMessage( Hwnd(), DialogItem, Message, wParam, lParam );
}


