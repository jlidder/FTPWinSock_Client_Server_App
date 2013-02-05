// CLIENT TCP PROGRAM
// Revised and tidied up by
// J.W. Atwood & Jaspreet Singh Lidder
// 1999 June 30 // 2013 January 29

char* getmessage(char *);
/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>
#include <winsock.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include "string"

using namespace std;

//user defined port number
#define REQUEST_PORT 0x7070;
int port=REQUEST_PORT;

//socket data types
SOCKET s;
SOCKADDR_IN sa;         // filled by bind
SOCKADDR_IN sa_in;      // fill with server info, IP, port

//buffer data types
char szbuffer[128]; //1500 buffer size. We expect to use 1200 for actual raw data + 300 saved for header info and other info.
char *buffer;
int ibufferlen=0;
int ibytessent;
int ibytesrecv=0;

//host data types
HOSTENT *hp;
HOSTENT *rp;

char localhost[11],
     remotehost[11];

//other
HANDLE test;
DWORD dwtest;

						//reference for used structures

						/*  * Host structure

							struct  hostent {
							char    FAR * h_name;             official name of host *
							char    FAR * FAR * h_aliases;    alias list *
							short   h_addrtype;               host address type *
							short   h_length;                 length of address *
							char    FAR * FAR * h_addr_list;  list of addresses *
						#define h_addr  h_addr_list[0]            address, for backward compat *
						};

						 * Socket address structure

						 struct sockaddr_in {
						 short   sin_family;
						 u_short sin_port;
						 struct  in_addr sin_addr;
						 char    sin_zero[8];
						 }; */

struct PACKET
{
    string Header;
    char data[1300];
	bool FIRST_PACKET_OF_STREAM;
    bool LAST_PACKET_OF_STREAM;
};

int main(void)
{
	WSADATA wsadata;
	try 
	{
		if (WSAStartup(0x0202,&wsadata)!=0)
			cout<<"Error in starting WSAStartup()" << endl;

		else 
		{
			buffer="WSAStartup was successful\n";   
			WriteFile(test,buffer,sizeof(buffer),&dwtest,NULL); 

			/* Display the wsadata structure */
			cout<< endl
				<< "wsadata.wVersion "       << wsadata.wVersion       << endl
				<< "wsadata.wHighVersion "   << wsadata.wHighVersion   << endl
				<< "wsadata.szDescription "  << wsadata.szDescription  << endl
				<< "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
				<< "wsadata.iMaxSockets "    << wsadata.iMaxSockets    << endl
				<< "wsadata.iMaxUdpDg "      << wsadata.iMaxUdpDg      << endl;
		}  

		//Display name of local host.
		gethostname(localhost,10);
		cout<<"ftp_tcp starting on host: " << localhost << endl;

		if((hp=gethostbyname(localhost)) == NULL) 
			throw "gethostbyname failed\n";

		//Ask for name of remote server

		cout << "Type name of ftp server :" << flush ;   
		cin >> remotehost ;
		//cout << "Remote host name is: \"" << remotehost << "\"" << endl;
		if((rp=gethostbyname(remotehost)) == NULL)
			throw "remote gethostbyname failed\n";

		//Create the socket
		if((s = socket(AF_INET,SOCK_STREAM,0))==INVALID_SOCKET) 
			throw "Socket failed\n";
		/* For UDP protocol replace SOCK_STREAM with SOCK_DGRAM */

		//Specify server address for client to connect to server.
		memset(&sa_in,0,sizeof(sa_in));
		memcpy(&sa_in.sin_addr,rp->h_addr,rp->h_length);
		sa_in.sin_family = rp->h_addrtype;   
		sa_in.sin_port = htons(port);

		//Display the host machine internet address
		cout << "Connecting to remote host:";
		cout << inet_ntoa(sa_in.sin_addr) << endl;

		//Connect Client to the server
		if (connect(s,(LPSOCKADDR)&sa_in,sizeof(sa_in)) == SOCKET_ERROR)
			throw "connect failed\n";

		/* Have an open connection, so, server is 

		   - waiting for the client request message
		   - don't forget to append <carriage return> 
		   - <line feed> characters after the send buffer to indicate end-of file */

		  ifstream::pos_type filesize;
		  char * memblock;
		  int amount_of_packets;
		  PACKET * packet_collection;
		  char filename[128];
		  string type_of_transfer;

		  cout << "Type name of file to be transferred: ";
		  cin >> filename;

		  cout << "Type direction of transfer: ";
		  cin >> type_of_transfer;
		  cout << "Sent requst to "<<remotehost<<". Handshaking...."<<endl;

		  //*******************************************************************************************************************************
		  //*******************************************************************************************************************************
		  //*******************************************************************************************************************************
		  //GET REQUEST FROM CLIENT TO SERVER
		  //GETTING A FILE FROM THE SERVER.
		  if(type_of_transfer == "get")
		  {
            char request_msg[128]; 

			sprintf_s(request_msg,"get");
			
			ibytessent=0; 
			ibufferlen = strlen(request_msg);
			if (send(s,request_msg, 128, 0) == SOCKET_ERROR) // TELL THE SERVER WE WANT TO INITIATE A FILE GET REQUEST
				throw "get failed\n";  

			char server_request_response[128];
			if((ibytesrecv = recv(s,server_request_response,128,0)) == SOCKET_ERROR) // SERVER SHOULD RESPOND WITH "OK" RESPONSE TO GET REQUEST
				throw "get failed\n";
			else
			{
				std::string server_msg_converted( reinterpret_cast< char const* >(server_request_response) );
				if(server_msg_converted=="ok")
				{
					if (send(s,filename, 128, 0) == SOCKET_ERROR)// TELL THE SERVER, THE FILE NAME TO BE TRANSFERED OVER.
						throw "file transfer initiation failed\n";  

					char file_content[128];

					if((ibytesrecv = recv(s,file_content,128,0)) == SOCKET_ERROR) // SERVER SHOULD RESPOND WITH FILE CONTENT
						throw "Server could not find file!";

					else
					{
							/*
							 * FILE RECEIVE LOGIC GOES HERE! (probably a for loop. LOOP(request <--> response) )
							 */

					}
				}
				else
					throw "Server Denied request!";
			}
		  }
		  //*******************************************************************************************************************************
		  //*******************************************************************************************************************************
		  //*******************************************************************************************************************************

		  //PUTTING A FILE ON THE SERVER.
		  else if(type_of_transfer == "put")
		  {
			 /* 
		      ifstream file (filename, ios::in|ios::binary|ios::ate);
			  if (file.is_open())
			  {
					filesize = file.tellg();
					memblock = new char [filesize];
					cout << filesize;
					file.seekg (0, ios::beg);
					file.read (memblock, filesize);
					file.close();

					cout << "the complete file content is in memory";

					//CREATE PACKETS BASED ON SIZE OF FILE
					memblock[3];

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
									memcpy (&packet_collection[packet_number].data[byte_in_packet+1], "\0" ,1);
								continue;
							}
						}
					}

					cout << "finished packaging file!";

					delete[] memblock; //get rid of the in-memory buffer storage.
			  }

			  else 
				  throw "Unable to open file";

			  //sprintf_s(szbuffer,"hello world!\r\n"); 
			  cout << "AMOUNT OF PACKETS: " << amount_of_packets << endl;
			  for(int packet_counter=0; packet_counter < 1 ; packet_counter++)
			  {
				cout << packet_collection[packet_counter].data << endl;
				ibytessent=0;    
				ibufferlen = strlen(packet_collection[100].data);
				ibytessent = send(s,packet_collection[100].data,ibufferlen,0);
				if (ibytessent == SOCKET_ERROR)
					throw "Send failed\n";  
			  }
			  //cout << "END OF FOR LOOP!!!!" << endl;

			  //wait for reception of server response.
			  ibytesrecv=0; 
			  //cout << "WE REACHED THE WAITING LINE...." << endl;
			  if((ibytesrecv = recv(s,szbuffer,128,0)) == SOCKET_ERROR)
				  throw "Receive failed\n";
			  else
				cout << "Packet Received by Server Successfully - Msg from server:" << szbuffer << endl;
			  */
		  }//end of if-put block


			
	} // try loop

	//Display any needed error response
	catch (char *str)
	{
		cerr<<str<<":"<<dec<<WSAGetLastError()<<endl;
	}

	//close the client socket
	closesocket(s);

	/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
	WSACleanup();  
	return 0;
}