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
			cout << " PA: "<<atoi(indata)<<"\n\n";
			PA = atoi(indata);

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
		if(PA == TPT)
		{
			PT_Done = true;
			break;
		}

	}
}


int main()
{
	MS_Tick_Type PhaseTime=100000, Obs_RTT=400;
    char data[100];
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

    		sprintf(data, "*%d#%d#%d#%d#", peerID, msgID, PCsize, ECN_TRANSPORT);

    		client.send_data(data);

      		ms_Agent->Event_Msg_Sent(peerID, msgID);

    		msgID++;
    		PT += PCsize;

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
