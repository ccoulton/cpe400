#include <iostream>
#include <cstdlib>
#include <time.h>
using namespace std;
/*
// simulate stop and wait, go back in
// and selective repeat.
// round time trip from 10 to 50ms
// timeout 45 ms
// size 100bytes 100 packets to transmit
// test from packet fail from 0 to .5 in 
// intervals of .1 window size 4
// test window from 4-16, interval 4 @ fail .2
// test at 1000 to transmit, compare retransmissions
// what is the best window size for from 4-16 and the sequence #
// range
/////////////////////////////////////////////////////////////*/
const int timeout = 45;

struct STAT{
	int retrans;
	unsigned int tottime;
	};
	
int findRTT(double failure);
STAT stopnwait(int numPckts, double FailRate);

	
struct PCKT{
	bool ACK;
	int RTT, squenceNUM;
	double failure;
	};
	
class reciever{
	public:
		reciever();
		reciever(int size);
		PCKT  SNWgetPCKT(PCKT &input);
		PCKT *GBNgetPCKT(PCKT input[]);
	private:
		int expected;
		int window;
		PCKT lastgood;
	};
	
reciever::reciever(){
	}
	
reciever::reciever(int size){
	expected = 0;
	window = size;
	}

PCKT reciever::SNWgetPCKT(PCKT &input){
	if (input.squenceNUM == expected){
		input.ACK = true;
		input.RTT = findRTT(input.failure);
		if (input.RTT == -1)
			input.ACK = false;
		else
			expected++;
		return(input);
		}
	else{
		input.ACK = false;
		input.RTT = 0;
		return(input);
		}
	}
	
PCKT *reciever::GBNgetPCKT(PCKT input[]){
	int RTT =0;
	for (int index =0; index<window; index++){
		if (input[index].squenceNUM == expected){
			RTT = findRTT(input[index].failure);
			if (RTT == -1){ //failed to arraive 
				input[index].ACK = false;
				input[index].RTT = 0;
				return(input);
				}
			else{
				input[index].ACK = true;
				input[index].RTT = RTT;
				expected++;
				}
			}
			
		else{
			input[index].ACK = false;
			input[index].RTT = findRTT(input[index].failure);
			return(input);
			}
		}
	return(input);
	}
	
int findRTT(double failure){
	int failrate = rand() % 100 +1;
	if (failrate <= (failure*100))
		return(-1);
	else{
		int RTT = rand() % 40 +10;
		return(RTT);
		}
	}

		
STAT stopnwait(int numPckts, double FailRate){
	/*send packet wait for ack if ack doesn't come failed
	  packets must time out before resent one packet at a time
	init variables*/
	reciever tester(1);
	PCKT input;
	input.failure = FailRate;
	int totaltime = 0;
	int retransmission = 0; 
	int squencenum= 0;
	int numACK =0;
	while(numACK <numPckts){
		input.squenceNUM = squencenum;
		input = tester.SNWgetPCKT(input);/*reciever to check if it fails to 
										connect aka findRTT() if didn't fail 	
										check if in seqence, should be cause 
										Stop and wait*/
		if ((input.RTT >= timeout)||(input.RTT == -1)){
			totaltime += timeout;
			retransmission++;
			}
		else{
			totaltime += input.RTT;
			squencenum++;
			numACK++;
			}
		}
	STAT output;
	output.retrans = retransmission;
	output.tottime = totaltime;
	return (output);
	}
	
//assuming each pckt takes 5ms to send so I can take off the approperate amount
STAT GoBackN(int numPckts, double FailRate, int Windowsize){
	reciever tester(Windowsize);
	PCKT input;
	//PCKT *out;
	int totaltime, retransmission, squencenum, numACK = 0;
	while (numACK <numPckts){
		for (int index=0; index< Windowsize; index++){
			input.failure = FailRate;
			input.squenceNUM = squencenum + index;
			if (input.squenceNUM > numPckts){
			    input.squenceNUM = numPckts;
			    }
			input = tester.SNWgetPCKT(input);
			if (input.ACK == true){ //pckt ACK
				totaltime += input.RTT - 5*index;
				squencenum++;
				numACK++;
				}
			else
				totaltime += timeout -5*index;
				retransmission++;
			}
			
		}
	STAT output;
	output.retrans = retransmission;
	output.tottime = totaltime;
	return (output);
	}
		
int main(int argc, char **argv)
	{
	srand(time(NULL));
	cout<<"testing StopnWait"<<endl;
	double fails = .0;
	STAT info;
	int size = 100;
	int Time, ReTrans;
	for (int i=0; i<= 5; i++){
		Time = ReTrans = 0;
		for (int j =0;	j<=10; j++){
			info = stopnwait(size, fails);
			Time += info.tottime;
			ReTrans += info.retrans;
			}
		cout<<"stop and wait for 10 tests average was"<<endl;
		cout<<"Failure rate: "<<fails<<endl;
		cout<<"Retransmission: "<<(ReTrans/10)<<endl;
		cout<<"average time: "<<(Time/10)<<endl;
	//bits as the other computer is recieving the file so only good packets
		cout<<"thoughput bits/sec: "<<(size*8)/(Time/(10*1000))<<endl
		<<endl;
		fails += .1;
		}
	fails +=0.0;
	for (int i=0; i<= 5; i++){
		Time = ReTrans =0;
		for (int j =0;	j<=10; j++){
			info = GoBackN(size, fails, 4);
			Time += info.tottime;
			ReTrans += info.retrans;
			}
		cout<<"Go back N for 10 tests average was"<<endl;
		cout<<"Failure rate: "<<fails<<endl;
		cout<<"Retransmission: "<<(ReTrans/10)<<endl;
		cout<<"average time: "<<(Time/10)<<endl;
	//bits as the other computer is recieving the file so only good packets
		cout<<"thoughput bits/sec: "<<(size*8)/(Time/(10*1000))<<endl
		<<endl;
		fails += .1;
		}
	}
