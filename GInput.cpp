/*------------------------------------------------

  GInput.cpp

	Generic input type that can be adapted for keyboard/mouse/pad


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GInput.h"


//	globals
//------------------------------------------------



//	Definitions
//------------------------------------------------

GInput::GInput()
{
	m_ButtonCount = 0;
	m_pButtons = NULL;
}


GInput::~GInput()
{
	DeleteButtons();
}



void GInput::AllocButtons(int ButtonCount)	
{	
	if ( ButtonCount == m_ButtonCount )
		return;

	DeleteButtons();
	if ( ButtonCount > 0 )
	{
		m_ButtonCount = ButtonCount;	
		m_pButtons = new u32[m_ButtonCount];
	}
}


void GInput::DeleteButtons()				
{
	if ( m_pButtons )	
		delete[] m_pButtons;	
	m_pButtons = NULL;	
	m_ButtonCount = 0;
}


	
void GInput::UpdateButton(int b, Bool down)
{
	//	get old state before we change it, so we can tell if button has been pressed or released
	Bool WasDown = ButtonDown(b);

	//	reset button state
	u32 NewState = down ? GInput::Down : 0x0;
	
	if ( !WasDown && down )
		NewState |= GInput::Pressed;

	if ( WasDown && !down )
		NewState |= GInput::Released;

	ButtonState(b) = NewState;
}


void GInput::Update()
{
	//	device specific
}


void GInput::Init()
{
	//	device specific
}



void GInput::Shutdown()
{
	//	device specific
}



