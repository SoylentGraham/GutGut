/*------------------------------------------------

  GMouse Header file



-------------------------------------------------*/

#ifndef __GMOUSE__H_
#define __GMOUSE__H_



//	Includes
//------------------------------------------------
#include "GInput.h"



//	Macros
//------------------------------------------------





//	Types
//------------------------------------------------
class GMouse: public GInput
{
public:
	int2	m_Pos;
	int2	m_PosChange;
	Bool	m_WheelScrolledUp;
	Bool	m_WheelScrolledDown;

	enum
	{
		Left=0,
		Right,
		Middle,
		WheelUp,	//	wheel scrolled fowards
		WheelDown,	//	wheel scrolled backwards

		MaxButtons,
	};

public:
	GMouse();
	~GMouse();

	inline int2&	Pos()		{	return m_Pos;	};
	inline int2&	PosChange()	{	return m_PosChange;	};
	virtual void	Update();
	virtual void	Init();

};




//	Declarations
//------------------------------------------------
extern GMouse	g_Mouse;



//	Inline Definitions
//-------------------------------------------------



#endif

