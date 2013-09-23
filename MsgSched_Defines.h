/*
 * MsgSched_Defines.h
 *
 *  Created on: Aug 1, 2013
 *      Author: Ashish Choudhari
 *	 	Real-Time Systems
 *		ECE Department
 *		Southern Illinois University, Carbondale, IL
 */

#ifndef MSGSCHED_DEFINES_H_
#define MSGSCHED_DEFINES_H_

typedef	unsigned long int	MS_Tick_Type;

//	debug by print statements, comment the line below to disable
//#define DEBUG_TICK
//#define DEBUG_AGENT
//#define DEBUG_PEER

// dump Period of adaptation to file
#define DUMP_PERIOD

//	tick precision measurement, comment the line below to disable
//#define MEASURE_TICK_PRECISION

// in milliseconds eg. if '10' then 1 tick happens every 10ms
// stable at 10 (tested on Intel i5 processor)
#define MS_Tick_Resolution	10

/* declare it in 'Ticks', we can call it the 'time-step' by which the algorithm
     adapts period upon deadine_Miss()
 Note: RESPONSE_TIME_MARGIN + Curr_RTT = Curr_Relative_Deadline
 default: MAX_BETTER_OBSERVED_RTT = 2, RESPONSE_TIME_MARGIN = (2 * Msg_Sched_Tick_Resolution);
*/
#define RESPONSE_TIME_MARGIN	5 // in Ticks, 1 (Tick) =  MS_Tick_Resolution (milliseconds)
#define MAX_BETTER_OBS_RTT_COUNT	3

// if ENABLE_ECN is defined then Max_Better_Obs_RTT_Count will increase from
// MAX_BETTER_OBS_RTT_COUNT to
#define ENABLE_ECN

#define ALLOW_DELTA_VARY


#endif /* MSGSCHED_DEFINES_H_ */
