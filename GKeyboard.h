/*------------------------------------------------

  GKeyboard Header file



-------------------------------------------------*/

#ifndef __GKEYBOARD__H_
#define __GKEYBOARD__H_



//	Includes
//------------------------------------------------
#include "GInput.h"



//	Macros
//------------------------------------------------
#define VK_LALT			VK_LMENU
#define VK_RALT			VK_RMENU
#define VK_PAGEUP		VK_PRIOR
#define VK_PAGEDOWN		VK_NEXT
#define VK_GRAVEACCENT	VK_OEM_3


//	Types
//------------------------------------------------
class GKeyboard : public GInput
{
public:

	enum
	{
		MaxKeys = 256,
	};

public:
	GKeyboard();
	~GKeyboard();

	virtual void	Update();
};




//	Declarations
//------------------------------------------------
extern GKeyboard	g_Keyboard;



//	Inline Definitions
//-------------------------------------------------



#endif

