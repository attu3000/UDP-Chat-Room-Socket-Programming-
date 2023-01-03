#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <udt.h>
#include <thread>
using namespace std;

bool exit_flag=false;

void msend(int soc);
void rsend(int soc);
thread sendThread;
thread receiveThread;



int main(int argc, char* argv[])
{
   if ((argc < 10) || (0 == atoi(argv[9])))
   {
      cout << "usage: -join -host <hostname> -username<username> -passcode <passcode> -port 5001" << endl;
      return -1;
   }

   UDT::startup();

   UDTSOCKET soc = UDT::socket(AF_INET, SOCK_STREAM, 0);

   sockaddr_in addre;
   addre.sin_family = AF_INET;
   addre.sin_port = htons(atoi(argv[9]));
   inet_pton(AF_INET, "127.0.0.1", &addre.sin_addr);

   memset(&(addre.sin_zero), '\0', 8);

   if (UDT::ERROR == UDT::connect(soc, (sockaddr*)&addre, sizeof(addre)))
   {
      return -1;
   }
   // send password to server
      int len = strlen(argv[7]);

   if (UDT::ERROR == UDT::send(soc, argv[7], strlen(argv[7]), 0))
   {
      return -1;
   }
   // send username to server
   len = strlen(argv[5]);
   if (UDT::ERROR == UDT::send(soc, argv[5], len, 0))
   {
      return -1;
   }
   // receive password verification from server
      char res[1024];
   
      if (UDT::ERROR == UDT::recv(soc, res, 1024, 0)) {
      return 0;
      }
      std::string str = res;
      std::string icr = "Incorrect passcode";
      if (icr.compare(str) == 0){
         cout <<"Incorrect passcode"<< endl;
         UDT::close(soc);
         UDT::cleanup();
         return -1;
      }
      cout << "Connected to " << argv[3] << " on port " << argv[9]<<endl;
   //create thread
	thread st(msend, soc);
	thread rt(rsend, soc);

   //move
	sendThread=move(st);
	receiveThread=move(rt);


   //join
	if(sendThread.joinable())
		sendThread.join();
	if(receiveThread.joinable())
		receiveThread.join();
			
	return 0;
}

void rsend(int soc)
{
	while(true)
	{
      char val[1024];
      if (UDT::ERROR == UDT::recv(soc, val, sizeof(val), 0)) {
         return;
      }
      if(strcmp(val,"Exit")==0)
		{
			receiveThread.detach();	
			UDT::close(soc);
         UDT::cleanup();
         return;
		}	
      string s(val);
      cout<<val<<endl;
	}	
}

void msend(int soc)
{
	while(true)
	{
		char val[1024];
		cin.getline(val,1024);
      if (UDT::ERROR == UDT::send(soc, val, sizeof(val), 0)) {
         
         return;
         }
     
		if(strcmp(val,"Exit")==0)
		{
			exit_flag=true;
			UDT::cleanup();
			UDT::close(soc);
         return;
		}	
	}		
}

