// CLIENT TCP PROGRAM
// Revised and tidied up by
// J.W. Atwood & Jaspreet Singh Lidder
// 1999 June 30 // 2013 January 29

char* getmessage(char *);

#pragma comment( linker, "/defaultlib:ws2_32.lib" ) // basic WINSOCK shell
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
#include <vector>

using namespace std;

//user defined port number
#define REQUEST_PORT 0x7070;
int port=REQUEST_PORT;

//socket data types
SOCKET s;
SOCKADDR_IN sa;         // filled by bind
SOCKADDR_IN sa_in;      // fill with server info, IP, port

//buffer data types
char szbuffer[128]; 
char *buffer;
int ibufferlen=0;
int ibytessent;
int ibytesrecv=0;

struct PACKET
{
    string Header;
    char data[1300];
	bool FIRST_PACKET_OF_STREAM;
    bool LAST_PACKET_OF_STREAM;
};

//file buffer data
PACKET * packet_collection;
int amount_of_packets;

//host data types
HOSTENT *hp;
HOSTENT *rp;

char localhost[11],
     remotehost[11];

//other
HANDLE test;
DWORD dwtest;

//------------------------------------------------------------------------------------------------------------------------------------
//FUNCTIONS TO BE USED BY MAIN()------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
void WriteToFile(char * filename, char * data)
{
	//write bytes into file
	ofstream outputFile;
	std::string filename_string_format( reinterpret_cast< char const* >(filename) );
	outputFile.open(filename_string_format); //test.pdf is a temp var. We will get rid of this in the next commit or something

	outputFile << data;

	outputFile.close();
}
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
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
		   - don't forget to append <carriage return>  */

		  ifstream::pos_type filesize;
		  char * memblock;
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

			std::string server_msg_converted( reinterpret_cast< char const* >(server_request_response) );
			if(server_msg_converted=="ok")//server responded with OK. we can now prepare for file transfer.
			{
				if (send(s,filename, 128, 0) == SOCKET_ERROR)// TELL THE SERVER, THE FILE NAME TO BE TRANSFERED OVER.
					throw "file transfer initiation failed\n";  

				char num_of_data_chunks_to_receive[128];

				if((ibytesrecv = recv(s,num_of_data_chunks_to_receive,128,0)) == SOCKET_ERROR) // SERVER SHOULD RESPOND WITH HOW MANY DATA CHUNKS IT HAS PREPARED TO SEND TO US. 
					throw "Server could not receive";                                          // THIS WILL HELP US DEFINE A LOOP FOR CONTINIOUS RECEIVE AND SEND MSG'S.

				//setup file chunks to be received
				std::string data_chunk_string_form( reinterpret_cast< char const* >(num_of_data_chunks_to_receive) );
				int data_chunk_int_form = atoi(data_chunk_string_form.c_str());

				packet_collection = new PACKET[data_chunk_int_form]; //setting up the data collection

				char start_msg[128];
				sprintf_s(start_msg,"start");

				if (send(s,start_msg, 128, 0) == SOCKET_ERROR)// TELL THE SERVER, THE FILE NAME TO BE TRANSFERED OVER.
					throw "file transfer initiation failed\n";  

				int total_bytes_of_file = data_chunk_int_form*1300;

				for(int receive_loop=0; receive_loop < data_chunk_int_form; receive_loop++) // RECEIVE EACH DATA CHUNK
				{
					/*
					 * FILE RECEIVE LOGIC GOES HERE! (probably a for loop. LOOP(request <--> response) )
					 */
					char data_chunk[1300];
					if((ibytesrecv = recv(s,data_chunk,1300,0)) == SOCKET_ERROR) // WAIT FOR THE DATA CHUNK TO BE SENT.
						throw "get data failed\n";

					for(int data_byte=0; data_byte<1300; data_byte++) //GO THROUGH EACH BYTE IN DATA CHUNK AND DEEP COPY IT OVER TO ITS RESPECTIVE PACKET.
					{
						memcpy (&packet_collection[receive_loop].data[data_byte], &data_chunk[data_byte],1);
						if(receive_loop==1299)
							memcpy (&packet_collection[receive_loop].data[receive_loop+1], "\0" ,1);
					}
					
					char receive_msg[128];
					sprintf_s(receive_msg,"received");

					if (send(s,receive_msg, 128, 0) == SOCKET_ERROR)// TELL THE SERVER, THE FILE NAME TO BE TRANSFERED OVER.
						throw "file transfer initiation failed\n";  
				}//end of for-loop which waits for receiving each data chunk

				//finished file transfer over. We need to marshal the data into its respective file type
				char * file_data_buffer_TOBEWRITTEN = new char[total_bytes_of_file];

				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
				//take all fragmented data and put it all together. (unify data chunks)
				int marshalcounter=0;

				for (int packet_collection_counter=0; packet_collection_counter < data_chunk_int_form; packet_collection_counter++)
				{
					for(int bytes_in_packet_counter=0;bytes_in_packet_counter<1300;bytes_in_packet_counter++)
					{
						memcpy (&file_data_buffer_TOBEWRITTEN[marshalcounter], &packet_collection[packet_collection_counter].data[bytes_in_packet_counter],1);
						marshalcounter++;
					}
				}
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

				//###################################################################################
				//write bytes into file
				WriteToFile(filename,file_data_buffer_TOBEWRITTEN);
				//###################################################################################
			}
			else
				throw "Server Denied request!";
		  }
		  //*******************************************************************************************************************************
		  //*******************************************************************************************************************************
		  //*******************************************************************************************************************************

		  //*******************************************************************************************************************************
		  //*******************************************************************************************************************************
		  //*******************************************************************************************************************************
		  //PUT REQUEST FROM CLIENT TO SERVER
		  //PUTTING A FILE ON THE SERVER.
		  else if(type_of_transfer == "put")
		  {
			//TODO
		  }//end of if-put block
		  //*******************************************************************************************************************************
		  //*******************************************************************************************************************************
		  //*******************************************************************************************************************************
	} // try loop

	//Display any needed error response
	catch (char *str)
	{
		cerr<<str<<":"<<dec<<WSAGetLastError()<<endl;
		cout << "error occured....";
		int testvar;
		cin >> testvar;
	}

	//close the client socket
	closesocket(s);

	/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
	WSACleanup();  
	return 0;
}