/*------------------------------------------------

  GStats Header file



-------------------------------------------------*/

#ifndef __GSTATS__H_
#define __GSTATS__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GList.h"



//	Macros
//------------------------------------------------
//	desired overall framerate
#define FRAMES_PER_SECOND	60

//	use this to compensate slowdown
#define FRAMERATE	(gr_Stats.m_DrawsLastSecond)


#define FRAMERATEF	((float)FRAMERATE)
#define TIME_DELTA	(1.f/(float)FRAMERATEF)

#define g_Stats		(g_App->m_Stats)

#define GDeclareCounter(name)	GStatsTempCounter _DeclareCounter_##name(#name)
#define GIncCounter(name,i)		{	GStatsTempCounter _TmpCounter_##name(#name,i);	}
#define GResetCounter(name)		{	GStatsCounter* pCounter = GStats::g_CounterList.GetCounter(#name);	if ( pCounter )	pCounter->Reset();	};

#define GDeclareTimer(name)		GStatsTempTimer _DeclareTimer_##name(#name,TRUE)
#define GLocalTimer(name)		GStatsTempTimer _TmpTimer_##name(#name);


//	Types
//------------------------------------------------

//-------------------------------------------------------------------------
//	debug counter.
//	GStatsTempCounter	temporary variable for incrementing a counter
//	GStatsCounter		stored counter type for the list
//	GStatsCounterList	List of counters
//-------------------------------------------------------------------------

class GStatsCounter
{
public:
	int			m_Counter;				//	cumulitive for this frame
	int			m_CounterThisSecond;	//	cumulitive for this second
	int			m_CounterLastSecond;	//	cumulitive last second
	const char*	m_pName;

public:
	GStatsCounter()							
	{	
		m_Counter = 0;
		m_CounterThisSecond = 0;
		m_CounterLastSecond = 0;
	};

	inline void	Reset()						{	m_Counter = 0;	};
	inline void	ResetThisSecond()			{	m_CounterLastSecond = m_CounterThisSecond;	m_CounterThisSecond = 0;	};
	inline void	ResetThisFrame()			{	m_CounterThisSecond += m_Counter;	m_Counter = 0;	};
	
	inline void	Increment(int Increment)	{	m_Counter += Increment;	};
	inline void	Decrement(int Decrement)	{	m_Counter -= Decrement;	};
};

class GStatsTempCounter
{
public:
	GStatsTempCounter(const char* pCounterName, int Increment);	//	increments counter
	GStatsTempCounter(const char* pCounterName);				//	decalares counter
};

class GStatsCounterList : public GList<GStatsCounter>
{
public:
	
public:
	void			Reset();						//	resets all counters
	void			ResetThisSecond();				//	resets all counters
	void			ResetThisFrame();				//	resets all counters
	
	GStatsCounter*	GetCounter(const char* pName);	//	find counter with name
	GStatsCounter*	AddCounter(const char* pName);	//	add new counter
	
	inline int		GetCounterValue(const char* pName)
	{
		GStatsCounter* pCounter = GetCounter(pName);
		return pCounter ? pCounter->m_Counter : -1;
	}

	inline int		GetCounterLastSecondValue(const char* pName)
	{
		GStatsCounter* pCounter = GetCounter(pName);
		return pCounter ? pCounter->m_CounterLastSecond : -1;
	}
	
	inline int		GetCounterThisSecondValue(const char* pName)
	{
		GStatsCounter* pCounter = GetCounter(pName);
		return pCounter ? pCounter->m_CounterThisSecond : -1;
	}
};


//-------------------------------------------------------------------------
//	debug timer
//	GStatsTempTimer		temporary variable for adding to a timer
//	GStatsTimer			stored timer type for the list
//	GStatsTimerList		List of timers
//-------------------------------------------------------------------------


class GStatsTimer
{
public:
	u32			m_Time;
	u32			m_TimeThisSecond;
	u32			m_TimeAverage;
	const char*	m_pName;

public:
	GStatsTimer()					{	Reset();	};

	inline void	Reset()				{	m_Time = 0;	};
	inline void	ResetThisSecond()	{	m_TimeThisSecond = 0;	};
	inline void	UpdateThisSecond()	{	m_TimeThisSecond += m_Time;	};
	inline void	SetAverage(int Avg)	{	m_TimeAverage = m_TimeThisSecond / Avg;	};
	inline void	AddTime(u32 Time)	{	m_Time += Time;	};
	inline void	Divide(u32 div)		{	m_Time /= div;	};
};

class GStatsTempTimer
{
public:
	GStatsTimer*	m_pTimer;	//	local pointer to the timer
	u32				m_StartTime;
	
public:
	GStatsTempTimer(const char* pTimerName,Bool DeclareTimer);
	GStatsTempTimer(const char* pTimerName);
	~GStatsTempTimer()	
	{
		Stop();
	}

	void		Start();
	void		Stop();
};

class GStatsTimerList : public GList<GStatsTimer>
{
public:
	
public:
	void			Reset();						//	resets all counters
	void			ResetThisSecond();				//	resets all counters
	void			UpdateThisSecond();				//	resets all counters
	void			SetAverage(int Avg);
	void			Divide(int div);				//	divides all timers
	GStatsTimer*	GetTimer(const char* pName);	//	find timer with name
	GStatsTimer*	AddTimer(const char* pName);	//	add new timer
	
	inline int		GetTimerValue(const char* pName)
	{
		GStatsTimer* pTimer = GetTimer(pName);
		return pTimer ? pTimer->m_Time : -1;
	}
};





//-------------------------------------------------------------------------
//	general stats controller
//-------------------------------------------------------------------------
class GStats
{
public:
	int			m_UpdateCounter;	//	updates so far this second
	int			m_DrawCounter;		//	draws so far this second
	float		m_FrameDelta;
	
private:
	
	int			m_UpdatesLastSecond;		//	
	int			m_DrawsLastSecond;			//	frames rendered per second, used for physics timing, never have this zero

	u32			m_LastTimeMil;				//	time at last stats update


	int			m_CurrentFrame;				//	which frame should we be on (out of FRAMES_PER_SECOND)
	int			m_LastFrame;				//	which frame were we on on last update
	int			m_FramesDropped;			//	how many frames need to be made up

	Bool		m_NewSecond;				//	current frame is a new second

	int			m_FirstTime;				//	GetTimeMil when app first started

public:
	GStats();
	~GStats();

	void			Init();					//	resets stats for first update

	inline Bool		NewSecond()				{	return m_NewSecond;	};			//	is this frame a new second?
	inline int		UpdatesPerSecond()		{	return m_UpdatesLastSecond;	};
	inline int		DrawsPerSecond()		{	return m_DrawsLastSecond;	};
	inline int		FramesDropped()			{	return m_FramesDropped;	};
	inline void		ReduceFramesDropped()	{	m_FramesDropped--;	};
	
	inline int		Uptime()				{	return GetTimeMil() - m_FirstTime;	};	//	return how many milsecs we've been runnning
	void			Update();				//	updates counters, timers etc
	void			OnNewSecond();
	void			OnNewFrame();

	u32				GetTimeMil();			//	get time in milliseconds from windows
	float			GetTimeInSec();			//	returns the time between 0..1 in this second

	void			Debug();				//	print out current debug stats
};
					






//	Declarations
//------------------------------------------------
extern GStatsCounterList	g_StatsCounterList;		//	list of global counters
extern GStatsTimerList		g_StatsTimerList;		//	list of global timers



//	Inline Definitions
//-------------------------------------------------


#endif

