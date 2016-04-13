/*------------------------------------------------

  GPad Header file



-------------------------------------------------*/

#ifndef __GPAD__H_
#define __GPAD__H_



//	Includes
//------------------------------------------------
#include "GInput.h"



//	Macros
//------------------------------------------------
const int	g_MaxPadButtons	= 32;
const int	g_MaxAxes		= 6;




//	Types
//------------------------------------------------
typedef struct
{
	Bool	Enabled;	//	this stick exists
	float	Pos;		//	calculated on update (-1..1)
	
	u32		DevPos;		//	between min and max
	u32		DevMax;		//	max axis value
	u32		DevMin;		//	min axis value
	u32		DevMiddle;	//	middle axis value

} GPadAxis;


class GPad: public GInput
{
public:
	Bool		m_PadAttatched;
	GPadAxis	m_Axis[g_MaxAxes];
	float		m_AxisDeadzone;		//	anything less than this is 0

public:
	GPad();
	~GPad();

	virtual void	Init();
	virtual void	Shutdown();
	virtual void	Update();

	void			PadError();
	float			CalcAxisValue(int a);

	inline Bool		AxisEnabled(int a)	{	return m_Axis[a].Enabled;	};
	inline float	GetAxis(int a)		{	return m_Axis[a].Pos;	};
};




//	Declarations
//------------------------------------------------
extern GPad		g_Pad;



//	Inline Definitions
//-------------------------------------------------



#endif

