/*
 * MsgSched_Tick.h
 *
 *  Created on: Aug 1, 2013
 *      Author: Ashish Choudhari
 *	 	Real-Time Systems
 *		ECE Department
 *		Southern Illinois University, Carbondale, IL
 */

#include "MsgSched.h"
#include "MsgSched_Defines.h"

MS_Tick_Type Current_Tick=0;

MS_Tick* MS_Tick::MS_Tick_Singleton = NULL;

// Note: this function is outside class MS_Tick
void MS_Tick_Func()
{
	// deliver tick change information to all peers in 'singleton' MS_Agent
	MS_Agent *Agent = MS_Agent::Get_Instance();

	while(1)
	{

#ifdef MEASURE_TICK_PRECISION
		boost::posix_time::ptime microsecT1 = boost::posix_time::microsec_clock::local_time();
#endif

		boost::this_thread::sleep(boost::posix_time::millisec(MS_Tick_Resolution));

		Current_Tick++;

		// deliver tick change information to MS_Agent
		Agent->Tick_Changed();

#ifdef MEASURE_TICK_PRECISION
		boost::posix_time::ptime microsecT2 = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration microsecDiff = microsecT2 - microsecT1;
		cout << "\nMS_Tick : Info: MS_Tick Correctness- "<<microsecDiff.total_milliseconds() << " MS_Tick_Current- "<<Current_Tick;
#endif
	}
}

MS_Tick* MS_Tick::Get_Instance()
{
	if(!MS_Tick_Singleton)
	{
#ifdef DEBUG_TICK
		cout << "\nMS_Tick : Info: creating new MS_Tick Instance";
#endif
		MS_Tick_Singleton = new MS_Tick();
	}
	return MS_Tick_Singleton;
}

void MS_Tick::Start()
{
	if(Active)
	{
		cerr << "\nMS_Tick : Warning: MS_Tick already Active\n";
	}
	else
	{
		Active = true;
		Current_Tick = 0;
		Tick_Thread = new boost::thread(boost::ref(MS_Tick_Func));
#ifdef DEBUG_TICK
		cout << "\nMS_Tick : Info: MS_Tick Start\n";
#endif
	}
}

void MS_Tick::Stop()
{
	if(Active)
	{
		Tick_Thread->interrupt();
		Tick_Thread->join();
		Active = false;
#ifdef DEBUG_TICK
		cout << "\nMS_Tick : Info: MS_Tick Stop\n";
#endif
	}
	else
	{
		cerr << "\nMS_Tick : Error: MS_Tick not Active, already in Stop\n";
	}
}

void MS_Tick::Reset()
{
	Current_Tick=0;

	if(!Active)
	{
#ifdef	DEBUG_TICK
		cout << "\nMS_Tick : Info: MS_Tick not Active, cannot Reset. Starting new Tick instead\n";
#endif
		Tick_Thread = new boost::thread(boost::ref(MS_Tick_Func));
		Active = true;
	}
}

MS_Tick_Type MS_Tick::Get_Current_Tick()
{
	return Current_Tick;
}


MS_Tick::MS_Tick()
{
	Active = false;
	Tick_Thread = NULL;
}


MS_Tick::~MS_Tick()
{
	Stop();
	Current_Tick=0;
}
