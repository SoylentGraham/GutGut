/*------------------------------------------------

  GListStack Header file

  fixed size array that acts like it grows and shrinks
  but rather than allocating new data we use the fixed
  array, which when used as a local var will just be 
  on the stack


-------------------------------------------------*/

#ifndef __GLISTSTACK__H_
#define __GLISTSTACK__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GDebug.h"
#include "GListTemplate.h"

//	Macros
//------------------------------------------------



//	Types
//------------------------------------------------

template <class TYPE,int MAXSIZE>
class GListStack : public GListTemplate<TYPE>
{
protected:
	int					m_Size;				//	running size
	TYPE				m_Data[MAXSIZE];	//	actual data

public:
	GListStack();													//	default constructor
	GListStack(const GListStack<TYPE,MAXSIZE>& CopyList);			//	construct from another list of this type
	
	virtual int			Size() const								{	return m_Size;	};
	virtual TYPE*		Data() const								{	return &m_Data[0];	};

	//	array
	virtual void		Resize(int size)							{	m_Size = Size;	};	//	
	virtual void		Realloc(int size)							{	};	//	shouldnt be called on fixed array

	virtual TYPE&		ElementAt(int Index) const					{	GDebug_CheckIndex(Index,0,SIZE); return m_Data[Index];	};
	virtual const TYPE&	ElementAtConst(int Index) const				{	GDebug_CheckIndex(Index,0,SIZE); return m_Data[Index];	};
	
	//	operators
	inline TYPE&		operator[](int Index)						{	return ElementAt(Index);	}
	inline TYPE&		operator[](u32 Index)						{	return ElementAt(Index);	}
	inline const TYPE&	operator[](int Index) const					{	return ElementAt(Index);	}
	inline const TYPE&	operator[](u32 Index) const					{	return ElementAt(Index);	}
	inline void			operator+=(const GListStack<TYPE,MAXSIZE>& Array)		{	Add(Array);	};
	inline void			operator+=(const TYPE val)					{	Add(val);	};
	inline void			operator+=(const TYPE& val)					{	Add(val);	};
	inline void			operator+=(const TYPE* val)					{	Add(val);	};
};			




//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------


template <class TYPE,int MAXSIZE>
GListStack<TYPE,MAXSIZE>::GListStack()
{
	m_Size = 0;
}


template <class TYPE,int MAXSIZE>
GListStack<TYPE,MAXSIZE>::GListStack(const GListStack<TYPE,MAXSIZE>& CopyList)
{
	GListStack();
	Copy(CopyList);
}






#endif

