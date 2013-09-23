/*
 * MsgSched_Agent.h
 *
 *  Created on: Aug 1, 2013
 *      Author: Ashish Choudhari
 *	 	Real-Time Systems
 *		ECE Department
 *		Southern Illinois University, Carbondale, IL
 */

#include "MsgSched.h"


MS_Agent* MS_Agent::MS_Agent_Singleton = NULL;



///////////////////////////////////////////////////////////////
// Private
///////////////////////////////////////////////////////////////
void MS_Agent::Distribute_KmaxLocal()
{
	if(Get_Number_Of_Peers() < 1)
		cerr << "\nMS_Agent: Error: 0 number of peers\n";
	else
	{
		// equal distribution
		int Kmax_for_each_Peer = (My_KmaxLocal/Get_Number_Of_Peers());
#ifdef DEBUG_AGENT
		cout << "\nMS_Agent: Info: Kmax_for_each_Peer: "<<Kmax_for_each_Peer;
#endif

		for(map<int, Peer*>::iterator it = All_Peer_Map.begin(); it != All_Peer_Map.end(); it++)
		{
			(*it).second->Set_Kmax(Kmax_for_each_Peer);
			(*it).second->Update_Period();
		}
	}
}
///////////////////////////////////////////////////////////////


// Set KmaxLocal for myself
void MS_Agent::Set_My_Kmax_And_Phase_Time(int _My_KmaxLocal, MS_Tick_Type _Phase_Time)
{
	My_KmaxLocal = _My_KmaxLocal;
	Phase_Time = _Phase_Time;
#ifdef DEBUG_AGENT
	cout << "\nMS_Agent: Info: My_KmaxLocal: " <<My_KmaxLocal<<" PhaseT:"<<Phase_Time \
			<<" RespTM:"<<RESPONSE_TIME_MARGIN;
#endif
}

#ifdef ALLOW_DELTA_VARY
void MS_Agent::Set_PCmin_PC_max(int _PCmin, int _PCmax)
{
	PCmin = _PCmin;
	PCmax = _PCmax;
}
#endif

void MS_Agent::Tick_Changed()
{
	if(All_Peer_Map.size())
	{
		for(map<int, Peer*>::iterator it = All_Peer_Map.begin(); it != All_Peer_Map.end();it++)
		{
			it->second->Check_Deadline_Miss();
		}
	}
}

///////////////////////////////////////////////////////////////
// EVENTS
///////////////////////////////////////////////////////////////

void MS_Agent::Event_Msg_Received(int _PeerID, int _MsgID)
{

}

void MS_Agent::Event_Msg_Ack_Received(int _PeerID, int _MsgID, int _ECN, int _Perror)
{
	if(All_Peer_Map.find(_PeerID) == All_Peer_Map.end())
		cerr << "\nMS_Agent: Error: invalid peer ID for Event ack receive\n";
	else
	{
		All_Peer_Map.at(_PeerID)->Msg_Ack_Received(_MsgID, _ECN, _Perror);
	}
}
void MS_Agent::Event_Msg_Sent(int _PeerID, int _MsgID)
{
	if(All_Peer_Map.find(_PeerID) == All_Peer_Map.end())
		cerr << "\nMS_Agent: Error: invalid peer ID for Event msg sent\n";
	else
		All_Peer_Map.at(_PeerID)->Msg_Sent(_MsgID);

}
void MS_Agent::Event_Msg_Lost(int _PeerID, int _MsgID)
{

}
///////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////
// INVARIANT CHECKING
///////////////////////////////////////////////////////////////
bool MS_Agent::Invariant_Check(int _PeerID)
{
	bool flag=0;
	int temp_state = 0;

	if(All_Peer_Map.find(_PeerID) == All_Peer_Map.end())
		cerr << "\nMS_Agent: Error: invalid ID for Invariant check\n";
	else
		temp_state = All_Peer_Map.at(_PeerID)->Get_Is_Invariant_State();

	if(temp_state == 0 || temp_state == -1) flag = false;
	if(temp_state == 1) flag = true;

	return flag;
}

// return in milliseconds, EWT: Expected Wait Time for Invariant to be true
int MS_Agent::Invariant_Get_EWT(int _PeerID)
{
	return 0;
}

void MS_Agent::Wait(int _tick)
{
	int count = Tick->Get_Current_Tick();
	int tick = _tick + count;

	while(count < tick)
	{
		count = Tick->Get_Current_Tick();
	}
}

// wait until Invariant is true
bool MS_Agent::Invariant_WU_True(int _PeerID)
{
	int temp_state = 0;

	while(true)
	{
		if(All_Peer_Map.at(_PeerID)->Get_Is_Invariant_State() == 1)
		{
			temp_state = 1;
			break;
		}
		if(All_Peer_Map.at(_PeerID)->Get_Is_Invariant_State() == -1)
		{
#ifdef DEBUG_AGENT
	cout << "\nMS_Agent: Info: End of Cycle Time";
#endif
			temp_state = 0;
			break;
		}
		Wait(2);
	}
	return temp_state;
}
///////////////////////////////////////////////////////////////


// add a new peer to the map, needs ID
void MS_Agent::Add_Peer(int _PeerID, MS_Tick_Type _Obs_RTT)
{
	Peer *new_Peer = new Peer(_PeerID, Phase_Time, _Obs_RTT);

	if(!new_Peer)
		cerr << "\nMS_Agent: Error: cannot allocate memory for new Peer\n";
	else
	{
		All_Peer_Map[_PeerID] = new_Peer;
#ifdef DEBUG_AGENT
		cout << "\nMS_Agent: Info: new Peer added to map with ID: "<<_PeerID;
#endif
		Distribute_KmaxLocal();
	}
}

// delete a peer from the map, needs ID
void MS_Agent::Delete_Peer(int _PeerID)
{
#ifdef DEBUG_AGENT
	cout << "\nMS_Agent: Info: deleting peer ID: "<<_PeerID;
#endif

	if(All_Peer_Map.find(_PeerID) == All_Peer_Map.end())
		cerr << "\nMS_Agent: Error: invalid ID for peer delete\n";
	else
	{
		All_Peer_Map.at(_PeerID)->~Peer();
		All_Peer_Map.erase(_PeerID);
	}

	Distribute_KmaxLocal();
}

// delete all Peers
void MS_Agent::Delete_All_Peers()
{
#ifdef DEBUG_AGENT
	cout << "\nMS_Agent: Info: deleting all peers";
#endif

	int temp_PeerID=0;

	for(map<int, Peer*>::iterator it = All_Peer_Map.begin(); it != All_Peer_Map.end();it++)
	{
		temp_PeerID = it->first;
		it->second->~Peer();
		All_Peer_Map.erase(temp_PeerID);
	}
#ifdef DEBUG_AGENT
	cout << "\nMS_Agent: Info: no of peers: "<<	Get_Number_Of_Peers();
#endif

}

// returns total number of peers available communicate
// useful for Debugging
int MS_Agent::Get_Number_Of_Peers()
{
	return All_Peer_Map.size();
}

MS_Agent::MS_Agent():
My_KmaxLocal(0),
Phase_Time(0),
PCmin(10),
PCmax(10)
{
	Tick = MS_Tick::Get_Instance();
	Tick->Start();
}

MS_Agent* MS_Agent::Get_Instance()
{
	if(!MS_Agent_Singleton)
	{
#ifdef DEBUG_AGENT
		cout << "\nMS_Agent: Info: creating new MS_Agent Instance";
#endif
		MS_Agent_Singleton = new MS_Agent();
	}
	return MS_Agent_Singleton;
}


MS_Agent::~MS_Agent()
{
	Delete_All_Peers();
	All_Peer_Map.clear();
	Tick->~MS_Tick();
}

