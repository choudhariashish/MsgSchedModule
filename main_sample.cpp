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
int peerID=8;

void receiverFunc()
{
    char indata[10];
    int msgID=0, ECN=0;
    int it=0;
    string ch;

	while(1)
	{
		ch = client.receive(1);

		if(ch[0] == '*')
		{
			// Peer ID
			ch = client.receive(1);
			while(ch[0] != '#')
			{
				indata[it++] = ch[0];
				ch[0] = ' ';
				ch = client.receive(1);
			}
//			cout << "\n\nACK: peerID: "<<atoi(indata);
			for(int k=0;k<10;k++)
				indata[k]=' ';
			it=0;

			// Message ID
			ch = client.receive(1);
			while(ch[0] != '#')
			{
				indata[it++] = ch[0];
				ch[0] = ' ';
				ch = client.receive(1);
			}
//			cout << " msgID: "<<atoi(indata);
			msgID = atoi(indata);
			for(int k=0;k<10;k++)
				indata[k]=' ';
			it=0;

			// Power Chunk Size
			ch = client.receive(1);
			while(ch[0] != '#')
			{
				indata[it++] = ch[0];
				ch[0] = ' ';
				ch = client.receive(1);
			}
//			cout << " pC: "<<atoi(indata)<<"\n\n";
			for(int k=0;k<10;k++)
				indata[k]=' ';
			it=0;

			// ECN Detection
			ch = client.receive(1);
			while(ch[0] != '#')
			{
				indata[it++] = ch[0];
				ch[0] = ' ';
				ch = client.receive(1);
			}
			cout << "\nECN: "<<atoi(indata)<<": ";
			if( atoi(indata) == 11 )
			{
//				cout << "detected ECN_CE\n";
				ECN = atoi(indata);
			}
			for(int k=0;k<10;k++)
				indata[k]=' ';
			it=0;

			ms_Agent->Event_Msg_Ack_Received(peerID, msgID, ECN);
			ECN=0;
			msgID=0;
		}
		else
			ch[0] = ' ';
		if(threadbreak)
		{
			client.closeSocket();
			break;
		}

	}
}


int main()
{
	MS_Tick_Type PhaseTime=100000, Obs_RTT=1000;
    char data[10];
    int TotalPower=20000, PowerTransfered=0;
    int PowerChunkSize = 10;
    int msgID=1;
    int MyKmax=20;

    //connect to host
    client.conn( "localhost" , 4242 );

    ms_Agent->Set_My_Kmax_And_Phase_Time(MyKmax, PhaseTime);
    ms_Agent->Add_Peer(peerID, Obs_RTT);

    boost::thread *receiverThread;
    receiverThread = new boost::thread(boost::ref(receiverFunc));

    while(1)
    {
    	if(ms_Agent->Invariant_Check(peerID))
    	{

    		sprintf(data, "*%d#%d#%d#%d#", peerID, msgID, PowerChunkSize, ECN_TRANSPORT);

    		client.send_data(data);

      		ms_Agent->Event_Msg_Sent(peerID, msgID);

    		msgID++;
    		PowerTransfered += PowerChunkSize;
//        	cout << "\n\npower transfered: "<<PowerTransfered;

        	if(PowerTransfered == TotalPower)
        	{
        		cout << "\n\npower transfered\n\n";
        		threadbreak = true;
        		break;
        	}

    	}
    }

	receiverThread->interrupt();
	receiverThread->join();
    ms_Agent->~MS_Agent();
	cout << "\n\nloop break\n";

    return 0;
}
