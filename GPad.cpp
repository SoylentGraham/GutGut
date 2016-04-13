/*------------------------------------------------

  GPad.cpp

	Gamepad input type


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GPad.h"
#include "GApp.h"
#include "GDebug.h"
#include "GWin32.h"

#include <mmsystem.h>
#pragma comment( lib, "Winmm.lib" )


//	globals
//------------------------------------------------
GPad				g_Pad;
extern const u32	g_ButtonMask[g_MaxPadButtons];


//	Definitions
//------------------------------------------------


const u32 g_ButtonMask[g_MaxPadButtons] = 
{
	JOY_BUTTON1,
	JOY_BUTTON2,
	JOY_BUTTON3,
	JOY_BUTTON4,
	JOY_BUTTON5,
	JOY_BUTTON6,
	JOY_BUTTON7,
	JOY_BUTTON8,
	JOY_BUTTON9,
	JOY_BUTTON10,
	JOY_BUTTON11,
	JOY_BUTTON12,
	JOY_BUTTON13,
	JOY_BUTTON14,
	JOY_BUTTON15,
	JOY_BUTTON16,
	JOY_BUTTON17,
	JOY_BUTTON18,
	JOY_BUTTON19,
	JOY_BUTTON20,
	JOY_BUTTON21,
	JOY_BUTTON22,
	JOY_BUTTON23,
	JOY_BUTTON24,
	JOY_BUTTON25,
	JOY_BUTTON26,
	JOY_BUTTON27,
	JOY_BUTTON28,
	JOY_BUTTON29,
	JOY_BUTTON30,
	JOY_BUTTON31,
	JOY_BUTTON32
};



GPad::GPad()
{
	m_PadAttatched = FALSE;
	m_AxisDeadzone = 0.1f;
	
	for ( int i=0;	i<g_MaxAxes;	i++ )
	{
		m_Axis[i].Enabled = FALSE;
	}
}



GPad::~GPad()
{

}


void GPad::Init()
{
	//	for now, we're only supporting one pad. but make sure its there!
	if ( joyGetNumDevs() < 1 )
	{
		GDebug::Print("No gamepad's installed.\n");
		GDebug::Print("--------------\n");

		m_PadAttatched = FALSE;
		return;
	}

	//	we have a pad!
	m_PadAttatched = TRUE;

	//	get pad's info
	JOYCAPS   joyCaps;

	if ( joyGetDevCaps(JOYSTICKID1, &joyCaps, sizeof(joyCaps)) != JOYERR_NOERROR )
	{
		GDebug::Print("Gamepad 1 not attatched.\n");
		GDebug::Print("--------------\n");
		PadError();
		return;
	}
	GDebug::Print("Pad found: \"%s\"\n", joyCaps.szPname );
	GDebug::Print("axes supported: %d\n", joyCaps.wMaxAxes );
	GDebug::Print("axes in use: %d\n", joyCaps.wNumAxes );
	GDebug::Print("buttons: %d\n", joyCaps.wMaxButtons );

	GDebug::Print("--------------\n");

	//	setup buttons
	int Buttons = joyCaps.wMaxButtons;
	if ( Buttons > g_MaxPadButtons )
		Buttons = g_MaxPadButtons;

	AllocButtons( Buttons );


	//	setup analog sticks
	u32 StickEnabledBit[g_MaxAxes] = 
	{
		0x0,
		0x0,
		JOYCAPS_HASZ,
		JOYCAPS_HASR,
		JOYCAPS_HASU,
		JOYCAPS_HASV,
	};

	u32* StickMin[g_MaxAxes] = 
	{
		&joyCaps.wXmin,
		&joyCaps.wYmin,
		&joyCaps.wZmin,
		&joyCaps.wRmin,
		&joyCaps.wUmin,
		&joyCaps.wVmin,
	};

	u32* StickMax[g_MaxAxes] = 
	{
		&joyCaps.wXmax,
		&joyCaps.wYmax,
		&joyCaps.wZmax,
		&joyCaps.wRmax,
		&joyCaps.wUmax,
		&joyCaps.wVmax,
	};

	for ( int i=0;	i<g_MaxAxes;	i++ )
	{
		//	check if the axis is enabled
		if ( StickEnabledBit[i] == 0x0 )
			m_Axis[i].Enabled = TRUE;
		else
			m_Axis[i].Enabled = joyCaps.wCaps & StickEnabledBit[i];

		m_Axis[i].DevMin = *StickMin[i];
		m_Axis[i].DevMax = *StickMax[i];
		m_Axis[i].DevMiddle = m_Axis[i].DevMin + ((m_Axis[i].DevMax - m_Axis[i].DevMin)/2);
	}
}



void GPad::Update()
{
	//	no pad
	if ( !m_PadAttatched )
		return;


	//	get all the button info off the pad
	JOYINFOEX joyInfoEx;
	ZeroMemory(&joyInfoEx, sizeof(joyInfoEx) );
	joyInfoEx.dwSize = sizeof(joyInfoEx);
	joyInfoEx.dwFlags = JOY_RETURNALL;
	if ( joyGetPosEx(JOYSTICKID1, &joyInfoEx) != JOYERR_NOERROR )
	{
		PadError();
		return;
	}

	//	update buttons
	for ( int b=0;	b<ButtonCount();	b++ )
	{
		UpdateButton( b, (joyInfoEx.dwButtons & g_ButtonMask[b])!=0 );
	}

	//	update sticks
	for ( int a=0;	a<g_MaxAxes;	a++ )
	{
		if ( !m_Axis[a].Enabled )
			continue;

		unsigned long* StickPos[g_MaxAxes] = 
		{
			&joyInfoEx.dwXpos,
			&joyInfoEx.dwYpos,
			&joyInfoEx.dwZpos,
			&joyInfoEx.dwRpos,
			&joyInfoEx.dwUpos,
			&joyInfoEx.dwVpos,
		};

		m_Axis[a].DevPos = *StickPos[a];
		m_Axis[a].Pos = CalcAxisValue(a);
	}



}


void GPad::Shutdown()
{

}



void GPad::PadError()
{
	m_PadAttatched = FALSE;
	DeleteButtons();
}



float GPad::CalcAxisValue(int a)
{
	if ( !AxisEnabled(a) )
		return 0.f;

	GPadAxis& Axis = m_Axis[a];
	if ( Axis.DevPos == Axis.DevMiddle )
		return 0.f;

	if ( Axis.DevPos < Axis.DevMiddle )
	{
		float MinRange = (float)(Axis.DevMiddle - Axis.DevMin);
		float PosInRange = (float)(Axis.DevPos - Axis.DevMin);
		float value = 1.f - (PosInRange / MinRange);
		if ( value <= m_AxisDeadzone )
			return 0.f;
		return -(value);
	}
	else
	{
		float MaxRange = (float)(Axis.DevMax - Axis.DevMiddle);
		float PosInRange = (float)(Axis.DevPos - Axis.DevMiddle);
		float value = ( PosInRange / MaxRange );
		if ( value <= m_AxisDeadzone )
			return 0.f;
		return value;
	}
}
