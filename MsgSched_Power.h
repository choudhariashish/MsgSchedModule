/*
 * MsgSched_Power.h
 *
 *  Created on: Aug 1, 2013
 *      Author: Ashish Choudhari
 *	 	Real-Time Systems
 *		ECE Department
 *		Southern Illinois University, Carbondale, IL
 */

#include "MsgSched.h"

#ifndef MSGSCHED_POWER_H_
#define MSGSCHED_POWER_H_

class MS_Power
{
public:
	MS_Power(MS_Tick_Type _Phase_Time, int _TPT);
	~MS_Power();
	int Calculate_Delta(MS_Tick_Type _Curr_Period, MS_Tick_Type _Curr_RTT, MS_Tick_Type _t, int _PT);

private:
	int TPT;
	MS_Tick_Type Phase_Time;
	MS_Tick_Type Get_Last_Transfer_Time(MS_Tick_Type _Curr_RTT, MS_Tick_Type _t);

};





#endif /* MSGSCHED_POWER_H_ */
