/*------------------------------------------------

  GInput Header file



-------------------------------------------------*/

#ifndef __GINPUT__H_
#define __GINPUT__H_



//	Includes
//------------------------------------------------
#include "GMain.h"



//	Macros
//------------------------------------------------


//	Types
//------------------------------------------------
class GInput
{
public:
	int		m_ButtonCount;	//	number of buttons allocated in m_pButtons
	u32*	m_pButtons;		//	button state for each button


	enum	//	button state flags. added to an emum so we can reuse the GInput namespace
	{
		Down		= 1<<0,
		Pressed		= 1<<1,
		Released	= 1<<2,
	};

public:
	GInput();
	~GInput();

	inline int		ButtonCount()					{	return m_ButtonCount;	};
	virtual void	AllocButtons(int ButtonCount);
	virtual void	DeleteButtons();

	inline Bool		ValidButton(int b)				{	return b < ButtonCount();	};
	inline u32&		ButtonState(int b)				{	return m_pButtons[b];	};
	inline Bool		ButtonDown(int b)				{	return ValidButton(b) ? ButtonState(b) & Down : FALSE;	};
	inline Bool		ButtonPressed(int b)			{	return ValidButton(b) ? ButtonState(b) & Pressed : FALSE;	};
	inline Bool		ButtonReleased(int b)			{	return ValidButton(b) ? ButtonState(b) & Released : FALSE;	};

	void			UpdateButton(int b, Bool down);	//	update individual button state 

	virtual void	Init();			//	device specific init
	virtual void	Shutdown();		//	device specific shutdown
	virtual void	Update();		//	device specific update, query device for frame
};



//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------



#endif

