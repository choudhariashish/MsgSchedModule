/*
 * MsgSched.h
 *
 *  Created on: Aug 1, 2013
 *      Author: Ashish Choudhari
 *	 	Real-Time Systems
 *		ECE Department
 *		Southern Illinois University, Carbondale, IL
 */
#ifndef MSGSCHED_H_
#define MSGSCHED_H_

#include <iostream>
#include <map>
#include <fstream>

///////////////////////////////////////////////////////////////
//	ADDITIONAL LIBRARY INCLUDES
//	Note: Remember to link libraries during compile time,
//			-lboost_thread
//			-lpthread
///////////////////////////////////////////////////////////////
#include <boost/thread/thread.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

///////////////////////////////////////////////////////////////
//	MODULE INCLUDES
///////////////////////////////////////////////////////////////
#include "MsgSched_Defines.h"

using namespace std;

class MS_Agent;
class MS_Tick;
class MS_Peer;

///////////////////////////////////////////////////////////////
//	MESSAGE SCHEDULING TICK
///////////////////////////////////////////////////////////////
// Singleton class to update MsgSchedModule's Tick
// Each Peer object will access this class to manage its message deadlines
class MS_Tick
{
private:
	bool Active;
	static MS_Tick *MS_Tick_Singleton;
	MS_Tick();
	// a thread to update Tick every predefined milliseconds
	boost::thread *Tick_Thread;

public:
	void Start();
	void Stop();
	void Reset();
	MS_Tick_Type Get_Current_Tick();
	static MS_Tick *Get_Instance();
	~MS_Tick();
};


///////////////////////////////////////////////////////////////
//	PEER CLASS
///////////////////////////////////////////////////////////////
class Peer
{
public:
	// need to set ID for Peer upon creation
	// (ID assigned by Cyber-Algo)
	Peer(int _PeerID, MS_Tick_Type _Phase_Time, MS_Tick_Type _Curr_RTT);
	~Peer();

	// can send message only if this function returns true
	int Get_Is_Invariant_State();
	// need to set this value before Node can send message
	void Set_Kmax(int _Kmax);

	/* following needs to be called from 'MS_Agent' when event occurs

	   1) Message received event
	   2) Acknowledgment received event
	   3) Message sent event
	   4) Message lost
	*/
	// need to call when a "Power chunk granted" message is received(command message to take out power)
	void Msg_Received(int _MsgID);
	/* calls->  Ack_Recv_Is_Better()  ,  if ack received is early
	   else->   Delete_Msg_Deadline() ,  if ack's message already missed its deadline
	   else->	Deadline_Met()
	*/
	// need to call when 'sent message' receives its 'acknowledgment'
	void Msg_Ack_Received(int _MsgID, int _ECN);
	// call when new message is sent
	void Msg_Sent(int _MsgID);
	// call when message is lost
	// if required a timeout mechanism can be included in this (MsgSched) Module
	void Msg_Lost(int _MsgID);

	// called from MS_Agent on behalf of MS_Tick
	// trigger to check deadline miss every 'Tick'
	void Check_Deadline_Miss();
	void Update_Period();

private:
	// need not have to call(useful for debugging)
	int Get_Curr_K();
	int GetID();
	// need not have to call(useful for debugging)
	int Get_Curr_Period();
	// need not have to call(useful for debugging)
	int Get_Curr_Deadline();
	// need not have to call(useful for debugging)
	int Get_Curr_RTT();


	MS_Tick* Tick;
	int My_ID;
	int Kmax;
	int Curr_K;
	MS_Tick_Type Phase_Time;
	// Main Invariant 'Is' is calculated for this Peer on function call 'Get_Is_Invariant_State()'
	// (Is = Ik * Ic * Ip)
	// Sub-Invariant, guard for maximum allowed outstanding messages, 'Kmax'
	bool Ik_Invariant;
	// Sub-Invariant, acknowledgments should receive before the end of Power-Transfer Phase
	bool Ic_Invariant;
	// Sub-Invariant, guard for avoiding period violation
	bool Ip_Invariant;

	MS_Tick_Type Last_Time_Sent;
	MS_Tick_Type Curr_Period;
	MS_Tick_Type Curr_Relative_Deadline;
	MS_Tick_Type Curr_RTT;
	// this variable is used to store average of Curr_RTT's until Better_RTT_Obs_Counter max-out
	MS_Tick_Type Obs_Avg_RTT;
	short Better_RTT_Obs_Counter;

	MS_Tick_Type Actual_RTT;

	// deadline data associated with every sent message is stored using this struct
	struct Deadline_Data
	{
		MS_Tick_Type Time_Sent;
		MS_Tick_Type Relative_Deadline;
		MS_Tick_Type Absolute_Deadline;
		Deadline_Data(MS_Tick_Type _Time_Sent, MS_Tick_Type _Relative_Deadline)
		{
			Time_Sent = _Time_Sent;
			Relative_Deadline = _Relative_Deadline;
			Absolute_Deadline = _Time_Sent + _Relative_Deadline;
//#ifdef DEBUG_PEER
//			cout<<" Tm_Snt:"<<Time_Sent<<" Rl_Dln:"<<Relative_Deadline<<" Abs_Dln:"<<Absolute_Deadline;
//#endif
		}
		~Deadline_Data(){};
	};

	map<int, Deadline_Data*> Deadline_Data_Map;
	// triggered by Msg_Sent()
	void Store_New_Msg_Deadline(int _MsgID);
	// triggered by Deadline_Miss(), Deadline_Met()
	void Delete_Msg_Deadline(int _MsgID);

	// calls Delete_Msg_Deadline()
	void Deadline_Met(int _MsgID);
	// triggered by 'Check_Deadline_Miss()' when some message misses
	// its absolute deadline
	void Deadline_Miss();

	// this function gets triggered when observed Curr_RTT
	// is good MAX_BETTER_OBSERVED_RTT number of times
	void Ack_Recv_Is_Better();

	// used if 'ENABLE_ECN' is defined
	// this function will increase the 'MAX_BETTER_OBS_RTT_COUNT'
	// limit and make it more tough for
	void Detected_ECN_CE();
	int Calculate_ECN_Counter();
	MS_Tick_Type Calculate_ECN_Activate_Time();
	int Max_Better_Obs_RTT_Count_ECN;
	MS_Tick_Type Max_Better_Obs_RTT_Count_ECN_Fallback_Time;

#ifdef DEBUG_PEER
	void Display_All_Stored_Deadlines()
	{
		cout << "\nMS_Peer : Info: PeerID: "<<My_ID<<" stored MsgID's:";
		map<int, Deadline_Data*>::iterator it;
		for(it=Deadline_Data_Map.begin(); it!= Deadline_Data_Map.end();it++)
		{
			cout <<" "<<it->first;
		}
	}
#endif


//#ifdef DUMP_PERIOD
    // dump Period adaptation to a log file
	  ofstream pfile;
//#endif

};


///////////////////////////////////////////////////////////////
//	MESSAGE SCHEDULING AGENT
///////////////////////////////////////////////////////////////
class MS_Agent
{
private:
	int My_KmaxLocal;
	MS_Tick_Type Phase_Time;
	// distribute 'My_KmaxLocal' among the Peers ready to receive power
	// distribution policy can be changed, now its configured to distribute evenly
	void Distribute_KmaxLocal();
	MS_Tick *Tick;
	// hold all peers
	map<int, Peer*> All_Peer_Map;
	static MS_Agent *MS_Agent_Singleton;
	MS_Agent();

public:
	///////////////////////////////////////////////////////////////
	// SETTING KMAX LOCAL
	///////////////////////////////////////////////////////////////
	// set My_Kmax_Local: this 'My_Kmax_Local' value will be distributed among
	// the Peers(short of power), considering this 'Node' has excess power.
	// My_Kmax_Local value has to be set/recalculated after every SC(state collection)
	///////////////////////////////////////////////////////////////
	void Set_My_Kmax_And_Phase_Time(int _MS_Agent_My_KmaxLocal, MS_Tick_Type _Phase_Time);

	///////////////////////////////////////////////////////////////
	// FOR TICK INCREMENT
	///////////////////////////////////////////////////////////////
	// MS_Tick calls this function in order to tell all Peers about tick change
	// which(Peers) in turn check for deadline miss
	///////////////////////////////////////////////////////////////
	void Tick_Changed();


	///////////////////////////////////////////////////////////////
	// EVENTS
	///////////////////////////////////////////////////////////////
	// following needs to be called from Cyber-Algo when event occurs
	///////////////////////////////////////////////////////////////
	// 1) Message received event
	// 2) Acknowledgment received event
	// 3) Message sent event
	// 4) Message lost
	///////////////////////////////////////////////////////////////
	// Note: As of now, there is confusion between 1 and 2
	// 	   	 This module is not sure whether Cyber-Algo will parse Message and
	// 	   	 explicitly tell this module whether message is "Acknowledgment" OR "Receive Power"
	///////////////////////////////////////////////////////////////
	void Event_Msg_Received(int _PeerID, int _MsgID);
	void Event_Msg_Ack_Received(int _PeerID, int _MsgID, int _ECN);
	void Event_Msg_Sent(int _PeerID, int _MsgID);
	void Event_Msg_Lost(int _PeerID, int _MsgID);

	///////////////////////////////////////////////////////////////
	// INVARIANT CHECKING
	///////////////////////////////////////////////////////////////
	//	before any power message is sent this Invariant_Check()
	//	has to be done. Only migrate power if return is true.
	//
	//	new feature of Estimated Wait Time (MS_Agent_Invariant_Get_EWT())
	//	and Wait Until 'Is' true (Invariant_WU_True()) can be added.
	//
	//	Invariant_WU_True would not return unless Invariant is true or Cycle end time.
	///////////////////////////////////////////////////////////////
	bool Invariant_Check(int _PeerID);
	// return in milliseconds
	int Invariant_Get_EWT(int _PeerID);
	bool Invariant_WU_True(int _PeerID);

	///////////////////////////////////////////////////////////////
	// PEER MANAGEMENT
	///////////////////////////////////////////////////////////////
	// adds new peer with Cyber-Algo assigned ID, UUID should work! (if 'int')
	// Obs_RTT is the round trip time observed in the state collection phase (SC)
	void Add_Peer(int _PeerID, MS_Tick_Type _Obs_RTT);
	// delete peer from the map, needs ID
	void Delete_Peer(int _PeerID);
	// returns total number of peers available communicate
	// useful for debugging
	int Get_Number_Of_Peers();
	// returns a pointer to map
	map<int, Peer*> *Get_Map_Of_Peers();
	// delete all Peers
	void Delete_All_Peers();

	// temporary function for testing from main()
	MS_Tick_Type Get_Current_Tick(){return Tick->Get_Current_Tick();}

	void Wait(int _tick);

	static MS_Agent* Get_Instance();
	~MS_Agent();
};

#endif /* MSGSCHED_H_ */
