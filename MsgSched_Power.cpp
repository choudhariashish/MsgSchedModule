/*
 * MS_Power.cpp
 *
 *  Created on: Aug 1, 2013
 *      Author: Ashish Choudhari
 *	 	Real-Time Systems
 *		ECE Department
 *		Southern Illinois University, Carbondale, IL
 */

#include "MsgSched_Power.h"

MS_Power::MS_Power(MS_Tick_Type _Phase_Time, int _TPT)
{
	Phase_Time = _Phase_Time;
	TPT = _TPT;
}
MS_Power::~MS_Power()
{
}

int MS_Power::Calculate_Delta(MS_Tick_Type _Curr_Period, MS_Tick_Type _Curr_RTT, MS_Tick_Type _t, int _PT)
{
	int Delta=0;
	int Total_Msgs=0;

	MS_Tick_Type LTT = Get_Last_Transfer_Time(_Curr_RTT, _t);

	if(!LTT)
		return 0;

	Total_Msgs = (LTT - _t)/ _Curr_Period;

	Delta = (TPT - _PT) / Total_Msgs;

	return Delta;
}

MS_Tick_Type MS_Power::Get_Last_Transfer_Time(MS_Tick_Type _Curr_RTT, MS_Tick_Type _t)
{
	if( (_t + _Curr_RTT) > Phase_Time )
	{
		cout << "\n\nMS_Power: PHASE TIME OVER: cannot transfer power\n\n";
		return 0;
	}
	else
	{
		return (Phase_Time - _Curr_RTT);
	}
}
