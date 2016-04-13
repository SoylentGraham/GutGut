/*------------------------------------------------

  GKeyboard.cpp

	Keyboard input type


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GKeyboard.h"
#include "GApp.h"
#include "GWin32.h"


//	globals
//------------------------------------------------
GKeyboard	g_Keyboard;



//	Definitions
//------------------------------------------------

GKeyboard::GKeyboard()
{
	AllocButtons( GKeyboard::MaxKeys );
}



GKeyboard::~GKeyboard()
{

}



void GKeyboard::Update()
{
	//	ignore input if game isnt main window
	Bool ActiveWindow = GWin32::IsActiveWindow();

	for ( int k=0;	k<GKeyboard::MaxKeys;	k++ )
	{
		Bool keystate = GetAsyncKeyState( k ) >> 8;
		UpdateButton( k, ActiveWindow ? keystate : FALSE );
	}
}

