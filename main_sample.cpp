/*
 * MsgSched.h
 *
 *  Created on: Aug 1, 2013
 *      Author: Ashish Choudhari
 *	 	Real-Time Systems
 *		ECE Department
 *		Southern Illinois University, Carbondale, IL
 */

#include "Tcp_Client.h"
#include "MsgSched.h"

#define ECN_TRANSPORT	10


MS_Agent *ms_Agent = MS_Agent::Get_Instance();
tcp_client client;

bool threadbreak = false;
bool PT_Done = false;
int peerID=8;
int TPT=20000, PT=0, PA=0;
int PCsize = 10;
int PCmin=10, PCmax=30;

int parseValue(char buff[], int hashPos)
{
    int value[4]={0,0,0,0};
    sscanf(buff, "*%d#%d#%d#%d#", &value[0], &value[1], &value[2], &value[3]);
    return value[hashPos];
}

void receiverFunc()
{
    char indata[100];
    int it=0;
    int i=0;
    string ch;

	// flush buffers
	for(int k=0;k<100;k++)
		indata[k]=' ';

    while(1)
    {
    	ch = client.receive(1);

    	if(ch[0]=='*')
    	{
    		while(1)
    		{
    			if(ch[0] == '#') i++;
    			indata[it] = ch[0];
    			if(i==4) break;
    			it++;
    			ch[0] = ' ';
    			ch = client.receive(1);
    		}
    		cout<<"\n"<<indata<<"\n";
    		PA = parseValue(indata,2);
    		ms_Agent->Event_Msg_Ack_Received(peerID, parseValue(indata,1), parseValue(indata,3), PT-PA);

    	}
    	// flush buffers
    	for(int k=0;k<100;k++)
    		indata[k]=' ';
    	i=0;
    	it=0;
    	ch[0]=' ';

    	if(threadbreak)
    	{
    		client.closeSocket();
    		break;
    	}
    	if(PA == TPT)
    	{
    		PT_Done = true;
    		break;
    	}
    }
}


int main()
{
	MS_Tick_Type PhaseTime=14000, Obs_RTT=400;
    char data[100];
    int msgID=1;
    int MyKmax=20;

    //connect to host
    client.conn( "localhost" , 4242 );

    ms_Agent->Set_My_Kmax_And_Phase_Time(MyKmax, PhaseTime);
    ms_Agent->Set_PCmin_PC_max(PCmin, PCmax);
    ms_Agent->Add_Peer(peerID, Obs_RTT);

    boost::thread *receiverThread;
    receiverThread = new boost::thread(boost::ref(receiverFunc));

    while(1)
    {
    	if(ms_Agent->Invariant_Check(peerID))
    	{
    		if(PT < TPT)
    		{
    			sprintf(data, "*%d#%d#%d#%d#", peerID, msgID, PCsize, ECN_TRANSPORT);

    			client.send_data(data);

    			ms_Agent->Event_Msg_Sent(peerID, msgID);

    			msgID++;
    			PT += PCsize;
    		}
        	if(ms_Agent->Get_Current_Tick() == PhaseTime)
        	{
        		cout << "\n\nPhaseTime Over\n\n";
        		threadbreak = true;
        		break;
        	}
        	if(PT_Done)
        	{
        		cout << "\n\nAll Power Transfered\n\n";
        		threadbreak = true;
        		break;
        	}
    	}
    }

    cout<<"\n\n\n\nPT:"<<PT<<" PA:"<<PA;
	receiverThread->interrupt();
	receiverThread->join();
    ms_Agent->~MS_Agent();
	cout << "\n\nloop break\n";

    return 0;
}
