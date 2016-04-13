/*------------------------------------------------

  GListFixed Header file

  fixed-size array with bounds checking


-------------------------------------------------*/

#ifndef __GLISTFIXED__H_
#define __GLISTFIXED__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GDebug.h"
#include "GListTemplate.h"

//	Macros
//------------------------------------------------



//	Types
//------------------------------------------------

template <class TYPE,int SIZE>
class GListFixed : public GListTemplate<TYPE>
{
private:
	TYPE				m_Data[SIZE];	//	actual data

public:
	GListFixed()													{	};	//	default constructor
	GListFixed(const GListFixed<TYPE,SIZE>& CopyList);				//	construct from another list of this type
	
	virtual int			Size() const								{	return SIZE;	};
	virtual TYPE*		Data() const								{	return &m_Data[0];	};

	//	array
	virtual void		Resize(int size)							{	};	//	shouldnt be called on fixed array
	virtual void		Realloc(int size)							{	};	//	shouldnt be called on fixed array

	virtual TYPE&		ElementAt(int Index)						{	GDebug_CheckIndex(Index,0,SIZE); return m_Data[Index];	};
	virtual const TYPE&	ElementAtConst(int Index) const				{	GDebug_CheckIndex(Index,0,SIZE); return m_Data[Index];	};
	
	//	operators
	inline TYPE&		operator[](int Index)						{	return ElementAt(Index);	}
	inline TYPE&		operator[](u32 Index)						{	return ElementAt(Index);	}
	inline const TYPE&	operator[](int Index) const					{	return ElementAt(Index);	}
	inline const TYPE&	operator[](u32 Index) const					{	return ElementAt(Index);	}
	inline void			operator+=(const GListFixed<TYPE,SIZE>& Array)		{	Add(Array);	};
	inline void			operator+=(const TYPE val)					{	Add(val);	};
	inline void			operator+=(const TYPE& val)					{	Add(val);	};
	inline void			operator+=(const TYPE* val)					{	Add(val);	};
};			




//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------

/*
template <class TYPE,int SIZE>
GListFixed<TYPE,SIZE>::GListFixed(const GListFixed<TYPE,SIZE>& CopyList)
{
	Copy(CopyList);
}
*/





#endif

