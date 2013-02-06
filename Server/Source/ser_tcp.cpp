// SERVER TCP PROGRAM
// revised and tidied up by
// J.W. Atwood & Jaspreet Singh Lidder
// 1999 June 30 // 2013 January 29

#pragma once
#pragma comment( linker, "/defaultlib:ws2_32.lib" ) //This is your basic WINSOCK shell
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <winsock.h>
#include <iostream>
#include <windows.h>
#include <string.h>
#include <fstream>
#include "string"

using namespace std;

//port data types
#define REQUEST_PORT 0x7070
int port=REQUEST_PORT;

//socket data types
SOCKET s;
SOCKET s1;
SOCKADDR_IN sa;      // filled by bind
SOCKADDR_IN sa1;     // fill with server info, IP, port

struct PACKET
{
    string Header;
    char data[1300];
	bool FIRST_PACKET_OF_STREAM;
    bool LAST_PACKET_OF_STREAM;
};

union 
{
	struct sockaddr generic;
	struct sockaddr_in ca_in;}ca;
	int calen=sizeof(ca); 

	//buffer data types
	char szbuffer[2000];
	char *buffer;
	int ibufferlen;
	int ibytesrecv;
	int ibytessent;

	//file buffer data
	PACKET * packet_collection;
	int amount_of_packets;

	//host data types
	char localhost[11];
	HOSTENT *hp;

	//wait variables
	int nsa1;
	int r,infds=1, outfds=0;
	struct timeval timeout;
	const struct timeval *tp=&timeout;
	fd_set readfds;

	//others
	HANDLE test;
	DWORD dwtest;

	int main(void)
	{
		WSADATA wsadata;

		try
		{        		 
			if (WSAStartup(0x0202,&wsadata)!=0)  
				cout<<"Error in starting WSAStartup()\n";
			
			else
			{
				buffer="WSAStartup was suuccessful\n";   
				WriteFile(test,buffer,sizeof(buffer),&dwtest,NULL); 

				/* display the wsadata structure */
				cout<< endl
					<< "wsadata.wVersion "       << wsadata.wVersion       << endl
					<< "wsadata.wHighVersion "   << wsadata.wHighVersion   << endl
					<< "wsadata.szDescription "  << wsadata.szDescription  << endl
					<< "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
					<< "wsadata.iMaxSockets "    << wsadata.iMaxSockets    << endl
					<< "wsadata.iMaxUdpDg "      << wsadata.iMaxUdpDg      << endl;
			}  

			//Display info of local host
			gethostname(localhost,10);
			cout<<"ftpd_tcp starting at host: "<<localhost<< endl;

			if((hp=gethostbyname(localhost)) == NULL) 
			{
				cout << "gethostbyname() cannot get local host info?"
					<< WSAGetLastError() << endl; 
				exit(1);
			}

			//Create the server socket
			if((s = socket(AF_INET,SOCK_STREAM,0))==INVALID_SOCKET) 
				throw "can't initialize socket";
			// For UDP protocol replace SOCK_STREAM with SOCK_DGRAM 


			//Fill-in Server Port and Address info.
			sa.sin_family = AF_INET;
			sa.sin_port = htons(port);
			sa.sin_addr.s_addr = htonl(INADDR_ANY);


			//Bind the server port
			if (bind(s,(LPSOCKADDR)&sa,sizeof(sa)) == SOCKET_ERROR)
				throw "can't bind the socket";
			cout << "Bind was successful" << endl;

			//Successfull bind, now listen for client requests.
			if(listen(s,10) == SOCKET_ERROR)
				throw "couldn't  set up listen on socket";
			else cout << "Listen was successful" << endl;

			FD_ZERO(&readfds);

			int close_command=0;
			//wait loop

			cout << "waiting to be contacted for transferring files..." << endl;

			while(1)
			{
					FD_SET(s,&readfds);  //always check the listener

					if(!(outfds=select(infds,&readfds,NULL,NULL,tp))) {}

					else if (outfds == SOCKET_ERROR) throw "failure in Select";

					else if (FD_ISSET(s,&readfds))  cout << "got a connection request" << endl; 

					//Found a connection request, try to accept. 
					if((s1=accept(s,&ca.generic,&calen))==INVALID_SOCKET)
							throw "Couldn't accept connection\n";

					//Connection request accepted.
					cout<<"accepted connection from "<<inet_ntoa(ca.ca_in.sin_addr)<<":"
						<<hex<<htons(ca.ca_in.sin_port)<<endl;

					//Fill in szbuffer from accepted request.
					if((ibytesrecv = recv(s1,szbuffer,128,0)) == SOCKET_ERROR) //CLIENT SENDS THIS SERVER A REQUEST TYPE.
						throw "Receive error in server program\n";

					std::string msg_converted( reinterpret_cast< char const* >(szbuffer) ); 

					if(msg_converted == "get") //THE REQUEST IS A GET-TYPE
					{
							char response_msg[128];
							sprintf_s(response_msg,"ok");

							if(send(s1,response_msg,128,0)==SOCKET_ERROR) //SERVER SENDS A OK CONFIRMATION TO CLIENT.
								throw "Send error in server program\n";

							char filenamebuffer[128];
							if((ibytesrecv = recv(s1,filenamebuffer,128,0)) == SOCKET_ERROR)  //CLIENT SENDS FILE-NAME TO SERVER.
								throw "Receive error in server program\n";

							std::string file_name_converted( reinterpret_cast< char const* >(filenamebuffer) ); //should be receiving the filename

							//------------------------------------------------------------------------------------------------------------------
							//PROCESS FILE AND SEND IT BACK....
							//------------------------------------------------------------------------------------------------------------------
							ifstream file (file_name_converted, ios::in|ios::binary|ios::ate);
							if (file.is_open())
							{
									int filesize = file.tellg();
									char * memblock = new char [filesize];
									file.seekg (0, ios::beg);
									file.read (memblock, filesize);
									file.close();

									//CREATE PACKETS BASED ON SIZE OF FILE
									amount_of_packets = ceil((filesize/1300.0));
									int position_of_buffer = 0;
									packet_collection = new PACKET[amount_of_packets];
									int packet_number=0;
									int byte_in_packet=0;
									int dummy=0;

									for(packet_number=0; packet_number< amount_of_packets; packet_number++)
									{
										for(byte_in_packet=0; byte_in_packet<1300; ++byte_in_packet)
										{
											if(position_of_buffer < filesize)
											{
												memcpy (&packet_collection[packet_number].data[byte_in_packet], &memblock[position_of_buffer++],1);
												if(byte_in_packet==1299)
													memcpy (&packet_collection[packet_number].data[byte_in_packet+1], "\0" ,1); //LAST CHAR MUST BE A END
												continue;
											}
										}
									}
									delete[] memblock; //get rid of the in-memory buffer storage.
							}

							else 
									throw "Unable to open file";
							//------------------------------------------------------------------------------------------------------------------
							//------------------------------------------------------------------------------------------------------------------
							//------------------------------------------------------------------------------------------------------------------
							//NOW AFTER BREAKING THE BINARY FILE IN PARTS, WE WILL NOW SEND IT IN PIECES. 
							// 1. SEND NUMBER OF PACKETS TO SEND.
							// 2. CLIENT SAYS "start"
							// 3. SERVER STARTS SENDING PACKET. ONE BY ONE.
							// 4. CLIENT RECEIVES 1 DATA PIECE.
							// 5. CLIENT SENDS A RESPONSE MSG.
							// 5. 5.1 - IF RESPONSE == "ACCEPTED", THEN SEND ANOTHER PACKET
							//    5.2 - IF RESPONSE == "FAILED", THEN SEND THE SAME PACKET OVER AGAIN. (BUT TCP ENSURES SUCCESS, SO NOT SURE IF WE WILL DO STEP 5.2)
							// 4. REPEAT STEP 5 UNTIL LAST PACKET.
							sprintf_s(response_msg,"ok");

							char int_to_char[128];
							itoa (amount_of_packets,int_to_char,10); //convert int to char array
								
							if(send(s1,int_to_char,128,0)==SOCKET_ERROR) //SERVER SENDS THE TOTAL # OF DATA CHUNKS TO SEND.
									throw "Send error in server program\n";

							char start_command[128];
							if(recv(s1,start_command,128,0) == SOCKET_ERROR) // WAIT FOR 'start' command from client
									throw "get data failed 1\n";

							std::string start_command_string_form( reinterpret_cast< char const* >(start_command) );

							if(start_command_string_form=="start")
							{
									// in a for loop, we send each packet, piece by piece.
									for(int packet_counter=0; packet_counter < amount_of_packets; packet_counter++)
									{
											if(send(s1,packet_collection[packet_counter].data,1300,0)==SOCKET_ERROR) //SERVER SENDS data.
													throw "Send error in server program\n";

											char receive_msg[128];

											if((ibytesrecv = recv(s1,receive_msg,128,0)) == SOCKET_ERROR) // WAIT FOR 'received' msg from client
													throw "get data failed 2\n";
									}
							}											
					}

					else if(msg_converted == "put")
					{
							//TODO
					}

			}//wait loop
		} //end of try 

		//Display needed error message.
		catch(char* str)
		{ 
			cerr<<str<<WSAGetLastError()<<endl;
			int test;
			cin >> test;
		}

		//close Client socket
		closesocket(s1);		

		//close server socket
		closesocket(s);
		cout << "SERVER IS DONE" << endl;
		/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
		WSACleanup();
		return 0;
	}// END OF MAIN
	


