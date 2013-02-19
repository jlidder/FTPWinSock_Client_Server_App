// CLIENT TCP PROGRAM
// Revised and tidied up by
// J.W. Atwood & Jaspreet Singh Lidder
// 1999 June 30 // 2013 January 29

//char* getmessage(char *);

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
#include <windows.h>
#include <Lmcons.h>

using namespace std;

#define USERNAMEBUFFER 128

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
	ofstream outputFile(filename,ios::in|ios::binary);
	string filename_string_format( reinterpret_cast< char const* >(filename) );
	outputFile.open(filename_string_format); 

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
			//WriteFile(test,buffer,sizeof(buffer),&dwtest,NULL); 

			/* Display the wsadata structure 
			cout<< endl
				<< "wsadata.wVersion "       << wsadata.wVersion       << endl
				<< "wsadata.wHighVersion "   << wsadata.wHighVersion   << endl
				<< "wsadata.szDescription "  << wsadata.szDescription  << endl
				<< "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
				<< "wsadata.iMaxSockets "    << wsadata.iMaxSockets    << endl
				<< "wsadata.iMaxUdpDg "      << wsadata.iMaxUdpDg      << endl;*/
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
		//cout << "Connecting to remote host:";
		//cout << inet_ntoa(sa_in.sin_addr) << endl;

		//Connect Client to the server
		if (connect(s,(LPSOCKADDR)&sa_in,sizeof(sa_in)) == SOCKET_ERROR)
			throw "connect failed\n";

		/* Have an open connection, so, server is 
		   - don't forget to append <carriage return>  */

		  ifstream::pos_type filesize;
		  char filename[128];
		  string type_of_transfer;
	
		  cout << "Type name of file to be transferred: ";
		  cin >> filename;

		  cout << "Type direction of transfer: ";
		  cin >> type_of_transfer;
		  
		  TCHAR username [ UNLEN + 1 ];
		  DWORD size = UNLEN + 1;
		  GetUserName( (TCHAR*)username, &size );

		  //send user name....
		  if (send(s,username, 128, 0) == SOCKET_ERROR) // TELL THE SERVER THE USER NAME
			throw "get failed\n";  

		  char server_request_response[128];
		  if((ibytesrecv = recv(s,server_request_response,128,0)) == SOCKET_ERROR) // SERVER SHOULD RESPOND WITH "OK" RESPONSE
			throw "get failed\n";

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

			string server_msg_converted( reinterpret_cast< char const* >(server_request_response) );
			if(server_msg_converted=="ok")//server responded with OK. we can now prepare for file transfer.
			{
				if (send(s,filename, 128, 0) == SOCKET_ERROR)// TELL THE SERVER, THE FILE NAME TO BE TRANSFERED OVER.
					throw "file transfer initiation failed\n";  

				//--------------------------------------------------
				//ERROR CHECK: server says if file exists or not....
				char file_check_response[128];
				if((ibytesrecv = recv(s,file_check_response,128,0)) == SOCKET_ERROR) // WAIT FOR response if file exists
					throw "get data failed\n";

				string file_server_check( reinterpret_cast< char const* >(file_check_response) );

				if(file_server_check == "nofile")
				{
					cout << "File does not exist on server!" << endl;
					throw "this file does not exist on server!";
				}
				//--------------------------------------------------

				char num_of_data_chunks_to_receive[128];

				if((ibytesrecv = recv(s,num_of_data_chunks_to_receive,128,0)) == SOCKET_ERROR) // SERVER SHOULD RESPOND WITH HOW MANY DATA CHUNKS IT HAS PREPARED TO SEND TO US. 
					throw "Server could not receive";                                          // THIS WILL HELP US DEFINE A LOOP FOR CONTINIOUS RECEIVE AND SEND MSG'S.

				//setup file chunks to be received
				string data_chunk_string_form( reinterpret_cast< char const* >(num_of_data_chunks_to_receive) );
				int data_chunk_int_form = atoi(data_chunk_string_form.c_str());

				packet_collection = new PACKET[data_chunk_int_form]; //setting up the data collection

				char start_msg[128];
				sprintf_s(start_msg,"start");

				if (send(s,start_msg, 128, 0) == SOCKET_ERROR)// TELL THE SERVER, THE FILE NAME TO BE TRANSFERED OVER.
					throw "file transfer initiation failed\n";  

				int total_bytes_of_file = data_chunk_int_form*1300;
				char *memblock = new char[total_bytes_of_file];
				int data_byte=0;

				for(int receive_loop=0; receive_loop < data_chunk_int_form; receive_loop++) // RECEIVE EACH DATA CHUNK
				{
					/*
					 * FILE RECEIVE LOGIC GOES HERE! (probably a for loop. LOOP(request <--> response) )
					 */
					char data_chunk[1300];
					if((ibytesrecv = recv(s,data_chunk,1300,0)) == SOCKET_ERROR) // WAIT FOR THE DATA CHUNK TO BE SENT.
						throw "get data failed\n";
										
					memcpy (&memblock[data_byte], &data_chunk,1300);
						
					char receive_msg[128];
					sprintf_s(receive_msg,"received");

					if (send(s,receive_msg, 128, 0) == SOCKET_ERROR)// TELL THE SERVER, THE FILE NAME TO BE TRANSFERED OVER.
						throw "file transfer initiation failed\n";  
				}//end of for-loop which waits for receiving each data chunk

				//finished file transfer over. We need to marshal the data into its respective file type
				char * file_data_buffer_TOBEWRITTEN = new char[total_bytes_of_file];

				
 
				//###################################################################################
				//write bytes into file
				WriteToFile(filename,file_data_buffer_TOBEWRITTEN);
				//###################################################################################
				cout << "File has arrived!" << endl;
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
			    char request_msg[128]; 
				sprintf_s(request_msg,"put");

				if(send(s,request_msg,128,0)==SOCKET_ERROR) //CIENT TELLS SERVER OF A PUT REQUEST.
					throw "Send error in server program\n";

				char response[128];
				if((ibytesrecv = recv(s,response,128,0)) == SOCKET_ERROR)  //CLIENT SENDS FILE-NAME TO SERVER.
					throw "Receive error in server program\n";

				string response_converted1( reinterpret_cast< char const* >(response) ); //should be receiving the filename

				//if response is not OK
				if(response_converted1 != "ok")
					throw "Server did not respond to a put request";
				
				if(send(s,filename,128,0)==SOCKET_ERROR) //SEND FILENAME TO SERVER.
					throw "Send error in server program\n";

				if((ibytesrecv = recv(s,response,128,0)) == SOCKET_ERROR)  //CLIENT SENDS FILE-NAME TO SERVER.
					throw "Receive error in server program\n";

				string response_converted2( reinterpret_cast< char const* >(response) ); //should be receiving the filename

				if(response_converted2 != "ok") //server is going along...
					throw "File Name response from server failed.";

				//------------------------------------------------------------------------------------------------------------------
				//PROCESS FILE AND SEND IT TO SERVER.
				//------------------------------------------------------------------------------------------------------------------
				ifstream file (filename, ios::in|ios::binary|ios::ate);
				if (file.is_open())
				{
						int filesize = (int)file.tellg();
						char * memblock = new char [filesize];
						file.seekg (0, ios::beg);
						file.read (memblock, filesize);
						file.close();

						//CREATE PACKETS BASED ON SIZE OF FILE
						amount_of_packets = (int)ceil((filesize/1300.0));
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
											if(position_of_buffer<filesize-1)
											{
											memcpy (&packet_collection[packet_number].data[byte_in_packet], &memblock[position_of_buffer++],1);
											if(byte_in_packet==1299)
												memcpy (&packet_collection[packet_number].data[byte_in_packet+1], "\0" ,1); //LAST CHAR MUST BE A END
											continue;
											}
											else 
											{
												memcpy (&packet_collection[packet_number].data[byte_in_packet], "\0" ,1);
												continue;
											}
										}
							}
						}
						delete[] memblock; //get rid of the in-memory buffer storage.
				}

				else 
				{
					cout << "could not open file!" << endl;
					throw "Unable to open file";
				}
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
				sprintf_s(request_msg,"ok");

				char int_to_char[128];
				itoa (amount_of_packets,int_to_char,10); //convert int to char array
								
				if(send(s,int_to_char,128,0)==SOCKET_ERROR) //SERVER SENDS THE TOTAL # OF DATA CHUNKS TO SEND.
						throw "Send error in server program\n";

				char start_command[128];
				if(recv(s,start_command,128,0) == SOCKET_ERROR) // WAIT FOR 'start' command from client
						throw "get data failed 1\n";

				std::string start_command_string_form( reinterpret_cast< char const* >(start_command) );

				if(start_command_string_form!="start")
					throw "file size information not received by server";
				
				char receive_msg[128];
				// in a for loop, we send each packet, piece by piece.
				for(int packet_counter=0; packet_counter < amount_of_packets; packet_counter++)
				{
						if(send(s,packet_collection[packet_counter].data,1300,0)==SOCKET_ERROR) //SERVER SENDS data.
								throw "Send error in server program\n";

						if((ibytesrecv = recv(s,receive_msg,128,0)) == SOCKET_ERROR) // WAIT FOR 'received' msg from client
								throw "get data failed 2\n";
				}	
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
	}

	//close the client socket
	closesocket(s);

	/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
	WSACleanup();  
	return 0;
}
