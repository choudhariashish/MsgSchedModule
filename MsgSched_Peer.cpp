/*
 * MsgSched_Peer.h
 *
 *  Created on: Aug 1, 2013
 *      Author: Ashish Choudhari
 *	 	Real-Time Systems
 *		ECE Department
 *		Southern Illinois University, Carbondale, IL
 */

#include "MsgSched.h"
#include "MsgSched_Defines.h"

static bool ECN_Status = true;

int Peer::GetID()
{
	return My_ID;
}

// need not have to call(useful for debugging)
int Peer::Get_Curr_Period()
{
	return Curr_Period;
}
// need not have to call(useful for debugging)
int Peer::Get_Curr_Deadline()
{
	return Curr_Relative_Deadline;
}
// need not have to call(useful for debugging)
int Peer::Get_Curr_RTT()
{
	return Curr_RTT;
}
// can send message only if this function returns true
int Peer::Get_Is_Invariant_State()
{
	static bool first_time_check = true;
	bool cycle_end = false;

	if(first_time_check)
	{
/*
#ifdef DEBUG_PEER
	cout << "\nMS_Peer : Info: (Is)=(Ik & Ic & Ip):"<<(Ik_Invariant*Ic_Invariant*Ip_Invariant)<<"="<<Ik_Invariant<<"*"<<Ic_Invariant<<"*"<<Ip_Invariant;
#endif
*/
		first_time_check = false;
		return true;
	}
	else
	{
//#ifdef ALLOW_DELTA_VARY

//#else
		if(Curr_K < (Kmax-1))
//#endif
			Ik_Invariant = true;
		else
			Ik_Invariant = false;

		if( (Tick->Get_Current_Tick() + Curr_Relative_Deadline) < Phase_Time )
			Ic_Invariant = true;
		else
			cycle_end = true;

		if( (Tick->Get_Current_Tick() - Last_Time_Sent) >= (MS_Tick_Type)Curr_Period )
			Ip_Invariant = true;
		else
			Ip_Invariant = false;
/*
#ifdef DEBUG_PEER
	cout << "\nMS_Peer : Info: (Is)=(Ik & Ic & Ip):"<<(Ik_Invariant*Ic_Invariant*Ip_Invariant)<<"="<<Ik_Invariant<<"*"<<Ic_Invariant<<"*"<<Ip_Invariant;
#endif
*/
	}

	if(cycle_end){ return -1; }

	return (Ik_Invariant * Ic_Invariant * Ip_Invariant);
}
// need not have to call(useful for debugging)
int Peer::Get_Curr_K()
{
	return Curr_K;
}
// need to set this value before Node can send message,
void Peer::Set_Kmax(int _Kmax)
{
	Kmax = _Kmax;
}

/////////////////////////////////////////////////////////////////////
// Events:
/////////////////////////////////////////////////////////////////////

/* following needs to be called from 'MS_Agent' when event occurs

   1) Message received event
   2) Acknowledgment received event
   3) Message sent event
   4) Message lost
*/
// need to call when a Power message is received(command message to take out power)
void Peer::Msg_Received(int _MsgID)
{

}
/* calls->  Ack_Recv_Is_Better()  ,  if ack received is early
   else->   Delete_Msg_Deadline() ,  if ack's message already missed its deadline
   else->	Deadline_Met()
*/
void Peer::Msg_Ack_Received(int _MsgID, int _ECN, int _PA)
{
	// practically 'Relative_Deadline' should never be '0'
	if(Deadline_Data_Map.at(_MsgID)->Relative_Deadline != 0)
	{
		Curr_K--;

#ifdef ENABLE_ECN
		// '11' because ECN bits are 11 if cogestion is detected
		if(_ECN >= 11 )
		{
			Detected_ECN_CE();
			ECN=true;
		}
		else
			ECN=false;
#endif

		Set_PA(_PA);

		Actual_RTT= Tick->Get_Current_Tick()-Deadline_Data_Map.at(_MsgID)->Time_Sent;

#ifdef DEBUG_PEER
		cout << "\n\nMS_Peer : Info: EVENT: ACK RECEIVED";
		cout << "\nMS_Peer : Info: RTT: "<<Tick->Get_Current_Tick()-Deadline_Data_Map.at(_MsgID)->Time_Sent<<" PeerID:"<<My_ID<<" msgID:"<<_MsgID;
#endif

		if(Deadline_Data_Map.at(_MsgID)->Absolute_Deadline > Tick->Get_Current_Tick())
		{
			Deadline_Met(_MsgID);
		}
		else
		{
#ifdef DEBUG_PEER
			cout << "\n\nMS_Peer : Info: EVENT: ALREADY MISSED DEADLINE";
			cout << "\nMS_Peer : Info: PeerID:"<<My_ID<<" msgID:"<<_MsgID;
#endif

			if( (Tick->Get_Current_Tick()-Deadline_Data_Map.at(_MsgID)->Time_Sent) \
					> Curr_Relative_Deadline)
			{
#ifdef DEBUG_PEER
	cout << ", have high RTT"<<".....(adapting)";
#endif
				Curr_RTT = Tick->Get_Current_Tick()-Deadline_Data_Map.at(_MsgID)->Time_Sent;
				Curr_Relative_Deadline = Curr_RTT + RESPONSE_TIME_MARGIN;

				/*
				 * #################################################################
				 */
				Update_Period();
				/*
				 * #################################################################
				 */

#ifdef DEBUG_PEER
	cout << "\nMS_Peer : Info: PeerID:"<<My_ID \
			<<" NEW P:"<<Curr_Period<<" RTT:" \
			<<Curr_RTT<<" Rl_Dln:" \
			<<Curr_Relative_Deadline \
			<<"................................(adapting)";
#endif
			}
			Delete_Msg_Deadline(_MsgID);
		}
#ifdef DEBUG_PEER
		cout << "\nMS_Peer : Info: Curr_K: "<<Curr_K;
#endif
	}
	else
		cerr << "\nMS_Peer : Error: oops..! MsgID:"<<_MsgID<<" does not exist," \
				" should call 'MS_Agent->Event_Msg_Sent(PeerID, MsgID)' when msg was sent";
}
void Peer::Msg_Sent(int _MsgID)
{
	if(Deadline_Data_Map.find(_MsgID) == Deadline_Data_Map.end())
	{
		Store_New_Msg_Deadline(_MsgID);
		Last_Time_Sent = Tick->Get_Current_Tick();
		Curr_K++;
//#ifdef DEBUG_PEER
//		cout << "\nMS_Peer : Info: Curr_K: "<<Curr_K;
//#endif
	}
	else
		cerr << "\nMS_Peer : Error: MsgID:"<<_MsgID<<" already exists," \
				" messages should have unique ID's\n";

}

void Peer::Msg_Lost(int _MsgID)
{

}

// executed every 'Tick' to see if some msg missed its deadline
void Peer::Check_Deadline_Miss()
{

	bool miss = false;
	for(map<int, Deadline_Data*>::iterator it=Deadline_Data_Map.begin(); it!=Deadline_Data_Map.end();it++)
	{
		if( it->second->Absolute_Deadline == Tick->Get_Current_Tick() )
		{
			if(!miss)
			{
#ifdef DEBUG_PEER
				cout << "\n\nMS_Peer : Info: EVENT: DEADLINE MISSED";
				cout << "\nMS_Peer : Info: deadline miss, PeerID:"<<My_ID<<" Msg's:";
#endif
				Deadline_Miss();
				miss = true;
			}
#ifdef DEBUG_PEER
			cout<<" "<<it->first;
#endif
		}
	}

#ifdef ENABLE_ECN
	if(Max_Better_Obs_RTT_Count_ECN_Fallback_Time==Tick->Get_Current_Tick())
	{
		ECN_Status = true;
		Max_Better_Obs_RTT_Count_ECN=MAX_BETTER_OBS_RTT_COUNT;
#ifdef DEBUG_PEER
		cout << "\n\nMS_Peer : Info: EVENT: COUNTER FALLBACK";
		cout << "\nMS_Peer : Info: Counter:"<<Max_Better_Obs_RTT_Count_ECN;
#endif
	}

	// dump statistics
	pfile<<Tick->Get_Current_Tick()<<";"<<Curr_Period<<";"<<Curr_K<<";"<<Actual_RTT<<";"<<PA<<";"<<ECN<<"\n";
#else
	pfile<<Tick->Get_Current_Tick()<<";"<<Curr_Period<<";"<<Curr_K<<";"<<Actual_RTT<<";"<<PA<<"\n";
#endif

	pfile.flush();
}

/////////////////////////////////////////////////////////////////////
// private:
/////////////////////////////////////////////////////////////////////

// triggered by Message_Sent()
void Peer::Store_New_Msg_Deadline(int _MsgID)
{
#ifdef DEBUG_PEER
	cout << "\n\nMS_Peer : Info: EVENT: NEW MESSAGE SENT";
	cout << "\nMS_Peer : Info: Store Deadline, PeerID:"<<My_ID<<" msgID:"<<_MsgID;
#endif
	Deadline_Data *new_Deadline = new Deadline_Data(Tick->Get_Current_Tick(), Curr_Relative_Deadline);

	Deadline_Data_Map[_MsgID] = new_Deadline;

#ifdef DEBUG_PEER
	Display_All_Stored_Deadlines();
#endif
}

// triggered by Deadline_Miss(), Deadline_Met()
void Peer::Delete_Msg_Deadline(int _MsgID)
{
	Deadline_Data_Map.at(_MsgID)->~Deadline_Data();
	Deadline_Data_Map.erase(_MsgID);
#ifdef DEBUG_PEER
	Display_All_Stored_Deadlines();
#endif
}


// calls Delete_Msg_Deadline()
void Peer::Deadline_Met(int _MsgID)
{
#ifdef DEBUG_PEER
	cout << "\nMS_Peer : Info: deadline met, PeerID:"<<My_ID<<" msgID:"<<_MsgID;
#endif

	static int count=0;

	// check for Better RTT
	MS_Tick_Type temp_MsgRTT = Tick->Get_Current_Tick()-Deadline_Data_Map.at(_MsgID)->Time_Sent;

//	cout << "\ntemp_MsgRTT: "<<temp_MsgRTT<<" Curr_RTT: "<<Curr_RTT<<"\n";

	if( temp_MsgRTT < Curr_RTT )
	{
		if( (Curr_RTT - temp_MsgRTT) > RESPONSE_TIME_MARGIN )
		{
//			cout << "\n(Curr_RTT-temp_MsgRTT): "<<(Curr_RTT - temp_MsgRTT);

			Better_RTT_Obs_Counter++;

			if(Obs_Avg_RTT <=1)
				Obs_Avg_RTT = temp_MsgRTT;

			Obs_Avg_RTT = (temp_MsgRTT + Obs_Avg_RTT) / 2;

			if(Better_RTT_Obs_Counter == MAX_BETTER_OBS_RTT_COUNT)
			{
				Ack_Recv_Is_Better();
				// reset Obs_Avg_RTT to accumulate next
				// MAX_BETTER_OBS_RTT_COUNT averages
				Obs_Avg_RTT = 0;
				Better_RTT_Obs_Counter = 0;
			}
		}
		else
		{
			// we need continuous good RTT's to adapt
			Better_RTT_Obs_Counter = 0;
			Obs_Avg_RTT = 0;
		}
	}
	Delete_Msg_Deadline(_MsgID);
}

// triggered by timing thread when some message misses
// its absolute deadline
void Peer::Deadline_Miss()
{
	Curr_RTT = Curr_RTT + RESPONSE_TIME_MARGIN;
	Curr_Relative_Deadline = Curr_RTT + RESPONSE_TIME_MARGIN;

	/*
	 * #################################################################
	 */
	Update_Period();
	/*
	 * #################################################################
	 */

#ifdef DEBUG_PEER
	cout << "\nMS_Peer : Info: PeerID:"<<My_ID \
			<<" NEW P:"<<Curr_Period<<" RTT:" \
			<<Curr_RTT<<" Rl_Dln:" \
			<<Curr_Relative_Deadline \
			<<".....(adapting)\n";
#endif
}

// this function gets triggered when observed RTT
// is good MAX_BETTER_OBSERVED_RTT number of times
void Peer::Ack_Recv_Is_Better()
{
#ifdef DEBUG_PEER
	cout << ", ack is better";
#endif

#ifdef ENABLE_ECN
	if(Max_Better_Obs_RTT_Count_ECN==MAX_BETTER_OBS_RTT_COUNT)
#endif
	{
		cout << ", ack is better";
	// reset counter for next Better_RTT
	Better_RTT_Obs_Counter = 0;

#ifdef ENABLE_ECN
	Curr_RTT =  Curr_RTT - RESPONSE_TIME_MARGIN;
	Curr_Relative_Deadline = Curr_RTT + RESPONSE_TIME_MARGIN;
#else
	Curr_RTT = Obs_Avg_RTT + RESPONSE_TIME_MARGIN;
	Curr_Relative_Deadline = Obs_Avg_RTT + RESPONSE_TIME_MARGIN;
#endif

	cout<<"  Curr_RTT: "<<Curr_RTT<<"\n";

	/*
	 * #################################################################
	 */
	Update_Period();
	/*
	 * #################################################################
	 */

#ifdef DEBUG_PEER
	cout << "\n\nMS_Peer : Info: EVENT: ACK IS BETTER";
	cout << "\nMS_Peer : Info: PeerID:"<<My_ID \
			<<" NEW P:"<<Curr_Period<<" RTT:" \
			<<Curr_RTT<<" Rl_Dln:" \
			<<Curr_Relative_Deadline \
			<<".....(adapting)\n";
#endif
	}

}

#ifdef ENABLE_ECN

void Peer::Detected_ECN_CE()
{
	static MS_Tick_Type Time_To_Activate_ECN = 0;

	if( ECN_Status )
	{

		cout << "\nECN_STATUS: "<<ECN_Status<<"\n";

		Time_To_Activate_ECN = Calculate_ECN_Activate_Time();
		Max_Better_Obs_RTT_Count_ECN_Fallback_Time = Time_To_Activate_ECN;

		Max_Better_Obs_RTT_Count_ECN = Calculate_ECN_Counter();

		Curr_RTT = Curr_RTT + 2*Curr_K*RESPONSE_TIME_MARGIN;
		Curr_Relative_Deadline = Curr_RTT + RESPONSE_TIME_MARGIN;

		Update_Period();

		ECN_Status = false;

#ifdef DEBUG_PEER
	cout << "\n\nMS_Peer : Info: EVENT: ECN_CE DETECTED" \
			"\nDisabling adaptation due to better RTT's";
	cout << "\nMS_Peer : Info: PeerID:"<<My_ID \
			<<" NEW P:"<<Curr_Period<<" RTT:" \
			<<Curr_RTT<<" Rl_Dln:" \
			<<Curr_Relative_Deadline \
			<<" Counter:"<<Max_Better_Obs_RTT_Count_ECN<<"\n";
#endif

	}
	else
	{
#ifdef DEBUG_PEER
		cout << "\n\nMS_Peer : Info: EVENT: ECN_CE DETECTED" \
				"\nUpdate due to ECN_CE deactivated\n";
#endif
	}
}

int Peer::Calculate_ECN_Counter()
{
	return Curr_K*2;
}

MS_Tick_Type Peer::Calculate_ECN_Activate_Time()
{
	return Tick->Get_Current_Tick()+Curr_RTT;
}

#endif

void Peer::Update_Period()
{
#ifdef ALLOW_DELTA_VARY
	int temp_Curr_Delta=0;
#endif

	if(Curr_K == Kmax)
		Curr_Period = Curr_RTT;
	else
		Curr_Period = Curr_Relative_Deadline/(Kmax-Curr_K);

// calculate new delta
#ifdef ALLOW_DELTA_VARY
	temp_Curr_Delta = Power->Calculate_Delta(Curr_Period, Curr_RTT, Tick->Get_Current_Tick(), PA);

	if(temp_Curr_Delta >= DeltaMax)
		Curr_Delta = DeltaMax;
	else if(temp_Curr_Delta <= DeltaMin)
		Curr_Delta = DeltaMin;
	else
		Curr_Delta = temp_Curr_Delta;
#endif
}

void Peer::Set_PA(int _PA)
{
	PA = _PA;
}

#ifdef ALLOW_DELTA_VARY
void Peer::Set_DeltaMin_DeltaMax(int _deltaMin, int _deltaMax)
{
	DeltaMin = _deltaMin;
	DeltaMax = _deltaMax;
}
int Peer::Get_Curr_Delta()
{
	return Curr_Delta;
}
#endif

void Peer::Set_TPT(int _TPT)
{
	TPT = _TPT;
#ifdef ALLOW_DELTA_VARY
	Power = new MS_Power(Phase_Time, TPT);
#endif
}

// need to set ID for Peer upon creation
// (ID assigned by Cyber-Algo)
Peer::Peer(int _PeerID, MS_Tick_Type _Phase_Time, MS_Tick_Type _Curr_RTT=800 )
{
	My_ID = _PeerID;
	Kmax = 0;
	Curr_K = 0;
	Phase_Time = _Phase_Time;

	Ik_Invariant = 1;
	Ic_Invariant = 1;
	Ip_Invariant = 1;

	Last_Time_Sent = 0;
	Curr_RTT = _Curr_RTT;
	Curr_Relative_Deadline = Curr_RTT + RESPONSE_TIME_MARGIN;
	Obs_Avg_RTT = Curr_RTT;

	Actual_RTT = Curr_RTT;
/*
 * Period is updated by Peer::Update_Period()
 * when called from MS_Agent::Distribute_KmaxLocal()
 */
	Curr_Period = 0;

	Better_RTT_Obs_Counter = 0;

#ifdef ENABLE_ECN
	Max_Better_Obs_RTT_Count_ECN=MAX_BETTER_OBS_RTT_COUNT;
	Max_Better_Obs_RTT_Count_ECN_Fallback_Time=0;
	ECN=false;
#endif

	// power specific
	PA=0;
#ifdef ALLOW_DELTA_VARY
	DeltaMin = 10;
	Curr_Delta = DeltaMin;
	DeltaMax = DeltaMin;
	// TPT is set by MS_Agent::Add_Peer just after creating a new peer
	TPT = 0;
#endif

	Tick = MS_Tick::Get_Instance();

//#ifdef DUMP_PERIOD
    pfile.open("/home/ashish/workspace/Graph/LiveGraph.1.14.Application.bin/MSMOutput.lgdat");
    pfile << "##;##\n";
    pfile << "@LiveGraph test file.\n";
#ifdef ENABLE_ECN
    pfile << "Tick;Period;Curr_K;Actual_RTT;PA;ECN\n";
#else
    pfile << "Tick;Period;Curr_K;Actual_RTT;PA\n";
#endif
    pfile.flush();
//#endif
}

Peer::~Peer()
{
#ifdef	DEBUG_PEER
	cout << "\nMS_Peer : Info: delete PeerID:"<<My_ID;
#endif
	map<int, Deadline_Data*>::iterator it;
	for(it=Deadline_Data_Map.begin(); it!= Deadline_Data_Map.end();it++)
	{
		it->second->~Deadline_Data();
	}
	Deadline_Data_Map.clear();
#ifdef ALLOW_DELTA_VARY
	Power->~MS_Power();
#endif
//#ifdef DUMP_PERIOD
	pfile.close();
//#endif
}
