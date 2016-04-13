/*------------------------------------------------

  GStats.cpp

	Time information


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GStats.h"
#include "GApp.h"

#include <MMSystem.h>
#pragma comment( lib, "Winmm.lib" )



//	globals
//------------------------------------------------


//	Definitions
//------------------------------------------------

GStatsTempCounter::GStatsTempCounter(const char* pCounterName)
{
	g_StatsCounterList.AddCounter( pCounterName );
}


GStatsTempCounter::GStatsTempCounter(const char* pCounterName, int Increment)
{
	GStatsCounter* pStatsCounter = g_StatsCounterList.GetCounter(pCounterName);
	if ( pStatsCounter )
		pStatsCounter->Increment( Increment );
}

void GStatsCounterList::Reset()
{
	for ( int i=0;	i<Size();	i++ )
		ElementAt(i).Reset();
}

void GStatsCounterList::ResetThisSecond()
{
	for ( int i=0;	i<Size();	i++ )
		ElementAt(i).ResetThisSecond();
}

void GStatsCounterList::ResetThisFrame()
{
	for ( int i=0;	i<Size();	i++ )
		ElementAt(i).ResetThisFrame();
}


GStatsCounter* GStatsCounterList::GetCounter(const char* pName)
{
	for ( int i=0;	i<Size();	i++ )
	{
		if ( strcmp( ElementAt(i).m_pName, pName ) == 0 )
		{
			return &ElementAt(i);
		}
	}
	return NULL;
}

GStatsCounter* GStatsCounterList::AddCounter(const char* pName)
{
	if ( GetCounter(pName) )
	{
		GDebug::Break("Counter with the name %s already exists.\n", pName );
		return GetCounter(pName);
	}

	GStatsCounter Counter;
	Counter.m_pName = pName;
	Counter.m_Counter = 0;
	Add( Counter );
	return &ElementLast();
}

void GStatsTimerList::Reset()
{
	for ( int i=0;	i<Size();	i++ )
		ElementAt(i).Reset();
}

void GStatsTimerList::ResetThisSecond()
{
	for ( int i=0;	i<Size();	i++ )
		ElementAt(i).ResetThisSecond();
}

void GStatsTimerList::UpdateThisSecond()
{
	for ( int i=0;	i<Size();	i++ )
		ElementAt(i).UpdateThisSecond();
}

void GStatsTimerList::SetAverage(int Avg)
{
	if ( Avg != 0 )
		for ( int i=0;	i<Size();	i++ )
			ElementAt(i).SetAverage(Avg);
}

void GStatsTimerList::Divide(int div)
{
	for ( int i=0;	i<Size();	i++ )
		ElementAt(i).Divide(div);
}

GStatsTimer* GStatsTimerList::GetTimer(const char* pName)
{
	for ( int i=0;	i<Size();	i++ )
		if ( strcmp( ElementAt(i).m_pName, pName ) == 0 )
			return &ElementAt(i);
	return NULL;
}

GStatsTimer* GStatsTimerList::AddTimer(const char* pName)
{
	if ( GetTimer(pName) )
	{
		GDebug::Break("Timer with the name %s already exists.\n", pName );
		return GetTimer(pName);
	}

	GStatsTimer Timer;
	Timer.m_pName = pName;
	Timer.m_Time = 0;
	Add( Timer );
	return &ElementLast();
}

GStatsTempTimer::GStatsTempTimer(const char* pTimerName,Bool DeclareTimer)
{
	g_StatsTimerList.AddTimer( pTimerName );
}

GStatsTempTimer::GStatsTempTimer(const char* pTimerName)
{
	m_pTimer = g_StatsTimerList.GetTimer(pTimerName);
	Start();
}


void GStatsTempTimer::Start()	
{	
	if ( m_pTimer )
		m_StartTime = g_Stats.GetTimeMil();	
};

void GStatsTempTimer::Stop()	
{	
	if ( m_pTimer && m_StartTime > 0 )	
		m_pTimer->AddTime( g_Stats.GetTimeMil() - m_StartTime );	
	m_StartTime = 0;	
};



//------------------------------------------------


GStats::GStats()
{
	m_UpdatesLastSecond	= 0;
	m_DrawsLastSecond	= FRAMES_PER_SECOND;	//	initialise to full frame rate
	m_NewSecond			= FALSE;
	m_LastTimeMil		= 0;
	m_CurrentFrame		= 0;
	m_LastFrame			= 0;
	m_FramesDropped		= 0;
	m_FrameDelta		= 0.f;

	m_FirstTime			= GetTimeMil();
}



GStats::~GStats()
{
}





u32 GStats::GetTimeMil()
{
	//	win32 specific
	return timeGetTime();
}
	

float GStats::GetTimeInSec()
{
	u32 Time = GetTimeMil();
	u32 TimeMil = Time % 1000;

	return (float)TimeMil / 1000.f;
}

	

void GStats::Update()
{
	//	new time
	u32 NewTimeMil = GetTimeMil();

	//	millisecond remainders
	int LastMil	= m_LastTimeMil % 1000;
	int NewMil	= NewTimeMil % 1000;

	//	elapsed to a new second
	if ( NewMil < LastMil )
	{
		//	do new second stuff
		OnNewSecond();
		m_NewSecond = TRUE;
	}
	else
	{
		//	reset value
		m_NewSecond = FALSE;
	}




	//	get timedelta
	float MilPerFrame = 1000.f / FIXED_FRAME_RATEF;
	u32 TimeDiff = NewTimeMil - m_LastTimeMil;
	float TimeDiff_f = (float)TimeDiff / MilPerFrame;
	TimeDiff_f *= 1.f/FIXED_FRAME_RATEF;
	m_FrameDelta = TimeDiff_f;
	//GDebug_Print("%1.5f delta. %1.5f\n", m_FrameDelta, 1.f / FIXED_FRAME_RATEF );
	
	//m_FrameDelta = 1.f / FIXED_FRAME_RATEF;
	if ( m_FrameDelta > 5.f / FIXED_FRAME_RATEF )
		m_FrameDelta = 5.f / FIXED_FRAME_RATEF;

	static float TotalFrameDelta = 0.f;
	TotalFrameDelta += m_FrameDelta;


	//	which frame should we be on? this second
	m_LastFrame = m_CurrentFrame;
	int MilPerSec = 1000 / FRAMES_PER_SECOND;
	m_CurrentFrame = NewMil / MilPerSec;

	//	check if we've dropped any frames
	if ( !NewSecond() )
	{
		if ( m_CurrentFrame - m_LastFrame > 1 )
			m_FramesDropped += ( m_CurrentFrame - m_LastFrame ) - 1;
	}
	else
	{
		//	starting new second above frame 0?
		if ( m_CurrentFrame > 0 )
			m_FramesDropped = m_CurrentFrame-1;

		//GDebug_Print("Total timedelta %2.4f\n",TotalFrameDelta);
		TotalFrameDelta=0.f;
	}

	
	//	update time mil
	m_LastTimeMil = NewTimeMil;

}
				


	
void GStats::OnNewSecond()
{
	//	upate counters
	m_UpdatesLastSecond	= m_UpdateCounter;
	m_DrawsLastSecond	= m_DrawCounter;
	if ( m_DrawsLastSecond <= 0 )
		m_DrawsLastSecond = 1;

	//	update/reset poly counters

	g_StatsCounterList.ResetThisSecond();
	g_StatsTimerList.ResetThisSecond();

	m_UpdateCounter	= 0;
	m_DrawCounter	= 0;
	m_FramesDropped = 0;
}


void GStats::OnNewFrame()
{
	g_StatsCounterList.ResetThisFrame();
}




void GStats::Debug()
{
	int DrawCounter = g_StatsCounterList.GetCounterLastSecondValue("DrawCounter");
	if ( DrawCounter == 0 )
		DrawCounter = 1;

	GDebug::Print("------ Average counters for second #%d (%d)--------\n", Uptime()/1000 );
	for ( int c=0;	c<g_StatsCounterList.Size();	c++)
	{
		GDebug::Print("%20s: Current: %5d; LastSecond: %5d (%d)\n", g_StatsCounterList[c].m_pName, g_StatsCounterList[c].m_Counter, g_StatsCounterList[c].m_CounterLastSecond, g_StatsCounterList[c].m_CounterLastSecond/DrawCounter );
	}

}


void GStats::Init()
{
	m_LastTimeMil = GetTimeMil();
	OnNewSecond();
	m_LastFrame = 0;
	m_CurrentFrame = 0;
}


