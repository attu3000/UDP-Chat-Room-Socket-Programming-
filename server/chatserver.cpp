

#include <cstdlib>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <udt.h>
#include <cstdlib>
#include <netdb.h>
#include <cstring>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <vector>

using namespace std;

struct structValues
{
	int id;
	int socket;
	thread th;
};
vector<structValues> clientInfo;

int clientID = 0;

void shared_print(string str, bool endLine);
int messageClients(string message, int sender_id);
void chatclient(int soc, int id);

string password;
int main(int argc, char* argv[])
{
   if ((argc < 5) || ((2 == argc) && (0 == atoi(argv[2]))))
   {
      cout << "usage: chatclient [server_port]" << endl;
      return 0;
   }


   UDT::startup();

   addrinfo hints;
   addrinfo* peer;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_flags = AI_PASSIVE;
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;

   string service("9000");
   if (argc == 5)
      service = argv[2];

   if (0 != getaddrinfo(NULL, service.c_str(), &hints, &peer))
   {
      cout << "illegal port number or port is busy." << endl;
      return 0;
   }
   password = argv[4];

   UDTSOCKET serv = UDT::socket(peer->ai_family, peer->ai_socktype, peer->ai_protocol);
   


   if (UDT::ERROR == UDT::bind(serv, peer->ai_addr, peer->ai_addrlen))
   {
      cout << "bind: " << UDT::getlasterror().getErrorMessage() << endl;
      
      return 0;
   }

   freeaddrinfo(peer);

   cout << "Server started on port " << service <<". Accepting connections"<< endl;

   UDT::listen(serv, 10);

   sockaddr_storage caddr;
   int addrlen = sizeof(caddr);
   UDTSOCKET soc;

	while(1)
	{
      if (UDT::INVALID_SOCK == (soc = UDT::accept(serv, (sockaddr*)&caddr, &addrlen))){
         cout << "accept: " << UDT::getlasterror().getErrorMessage() << endl;
         return 0;
      }

		clientID++;
		thread th(chatclient,soc,clientID);


		clientInfo.push_back({clientID,soc,(move(th))});
	}

	for(int i = 0; i < (int)clientInfo.size(); i++) {
		if(clientInfo[i].th.joinable()) {
			clientInfo[i].th.join();
      }
	}

	UDT::close(serv);
	return 0;
}


int messageClients(string sendClient, int sender_id) {

	char val[1024];
	strcpy(val, sendClient.c_str());
	for(int i = 0; i < (int)clientInfo.size(); i++)
	{
		if(clientInfo[i].id != sender_id)
		{
         if (UDT::ERROR == UDT::send(clientInfo[i].socket, val, sendClient.length() + 1, 0)) {
            cout << UDT::getlasterror().getErrorMessage() << endl;
         return -1;
         }
		}
	}		
   return 0;
}





void chatclient(int soc, int id)
{

   char clientPassword[1024], clientUsername[1024];

   if (UDT::ERROR == UDT::recv(soc, clientPassword, 1024, 0))
   {
      cout << UDT::getlasterror().getErrorMessage() << endl;
      return;
   }

   if (UDT::ERROR == UDT::recv(soc, clientUsername, 1024, 0))
   {
      cout << UDT::getlasterror().getErrorMessage() << endl;
      return;
      
   }

   std::string usr = clientUsername;
   std::string pas = clientPassword;
   //send incorrect if incorrect
   char inc[1024]= "Incorrect passcode";
   if (password.compare(pas) != 0){
      //cout << "incorrect password"<< endl;
      if (UDT::ERROR == UDT::send(soc, inc , 1024, 0)){
      cout << UDT::getlasterror().getErrorMessage() << endl;
      }
      return;
   }
   
   else{
   const char* enter = "joined";
   if(UDT::ERROR == UDT::send(soc, enter, strlen(enter), 0)){
      cout << UDT::getlasterror().getErrorMessage() << endl;
   }
      string usrjoined = string(clientUsername) + " joined the chatroom";
      messageClients(usrjoined,id);	
      cout <<usr<<" joined the chatroom"<<endl;
     
   }

	while(1)
	{
      char str[1024];
      
      if (UDT::ERROR == UDT::recv(soc, str, 1024, 0)) {
      return;
      }
      string response = string(clientUsername) + ": "+str;


		if(strcmp(str,"Exit")==0)
		{
			string message=string(usr)+string(" has left");		
         const char* exit = "Exit";
         if(UDT::ERROR == UDT::send(soc, exit, strlen(exit), 0)){
            cout << UDT::getlasterror().getErrorMessage() << endl;
         }
					
			messageClients(response,id);	
			return;
		}
      if(strcmp(str,":)")==0){
         response = string(clientUsername) + ": " + "[feeling happy]";
      }
      if(strcmp(str,":(")==0){
         response = string(clientUsername) + ": " + "[feeling sad]";
      }
      if(strcmp(str,":mytime")==0){
            time_t currTime = time(0);
            char* correctTime = ctime(&currTime);
            response = string(clientUsername) + ": " + correctTime;
      }
      if(strcmp(str,":+1hr")==0){
            time_t currTime = time(0) + 3600;
            char* correctTime = ctime(&currTime);
            response = string(clientUsername) + ": " + correctTime;
      }
		messageClients(response,id);					
      cout<<response<<endl;
	}	
}