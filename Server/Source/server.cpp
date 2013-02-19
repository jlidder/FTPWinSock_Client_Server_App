/*a small file Server
Usage: suppose Server is running on sd1.encs.concordia.ca and server is running on sd2.encs.concordia.ca
.Also suppose there is a file called test.txt on the server.
In the Server,issuse "Server sd2.encs.concordia.ca test.txt size" and you can get the size of the file.
In the Server,issuse "Server sd2.encs.concordia.ca test.txt time" and you can get creation time of the file
*/

#pragma once
#pragma comment( linker, "/defaultlib:ws2_32.lib" ) //This is your basic WINSOCK shell
#include <winsock.h>
#include <windows.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <process.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include "Thread.h"
#include "server.h"
#include "string"
#include <windows.h>

using namespace std;

struct PACKET
{
    char data[1300];
	bool FIRST_PACKET_OF_STREAM;
    bool LAST_PACKET_OF_STREAM;
};

union 
{
	struct sockaddr generic;
	struct sockaddr_in ca_in;}ca;
	int calen=sizeof(ca); 

//port data types
#define RREQUEST_PORT 0x7070
int port=RREQUEST_PORT;

//buffer data types
char szbuffer[2000];
char *buffer;
int ibufferlen;
int ibytesrecv;
int ibytessent;

//socket data types
SOCKET s;
SOCKET s1;
SOCKADDR_IN sa;      // filled by bind
SOCKADDR_IN sa1;     // fill with server info, IP, port

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

//------------------------------------------------------------------------------------------------------------------------------------
//FUNCTIONS TO BE USED BY MAIN()------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
void WriteToFile(char * filename, char * data)
{
	//write bytes into file
	ofstream outputFile;
	std::string filename_string_format( reinterpret_cast< char const* >(filename) );
	outputFile.open(filename_string_format); 

	outputFile << data;

	outputFile.close();
}
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------

TcpServer::TcpServer()
{
	WSADATA wsadata;
	if (WSAStartup(0x0202,&wsadata)!=0)
		TcpThread::err_sys("Starting WSAStartup() error\n");
	
	//Display name of local host
	if(gethostname(servername,HOSTNAME_LENGTH)!=0) //get the hostname
		TcpThread::err_sys("Get the host name error,exit");
	
	//printf("Multi-threaded FTP Server Started :) \n");
	//printf("Server: %s waiting to be contacted for time/size request...\n",servername); //ftpd_tcp starting at host: [sun] waiting to be contacted for transferring files...
	printf("ftpd_tcp starting at host: %s \n",servername);
	printf("waiting to be contacted for transferring files... \n"); 
	
	//Create the server socket
	if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		TcpThread::err_sys("Create socket error,exit");
	
	//Fill-in Server Port and Address info.
	ServerPort=RREQUEST_PORT;
	memset(&ServerAddr, 0, sizeof(ServerAddr));      /* Zero out structure */
	ServerAddr.sin_family = AF_INET;                 /* Internet address family */
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
	ServerAddr.sin_port = htons(ServerPort);         /* Local port */
	
	//Bind the server socket
    if (bind(serverSock, (struct sockaddr *) &ServerAddr, sizeof(ServerAddr)) < 0)
		TcpThread::err_sys("Bind socket error,exit");
	
	//Successfull bind, now listen for Server requests.
	if (listen(serverSock, MAXPENDING) < 0)
		TcpThread::err_sys("Listen socket error,exit");
}

TcpServer::~TcpServer()
{
	WSACleanup();
}


void TcpServer::start()
{
	for (;;) /* Run forever */
	{
		/* Set the size of the result-value parameter */
		clientLen = sizeof(ServerAddr);

		/* Wait for a Server to connect */
		if ((clientSock = accept(serverSock, (struct sockaddr *) &ClientAddr, 
			&clientLen)) < 0)
			TcpThread::err_sys("Accept Failed ,exit");
		
        /* Create a Thread for this new connection and run*/
		TcpThread * pt=new TcpThread(clientSock,serverSock);
		pt->start();
	}
}

//////////////////////////////TcpThread Class //////////////////////////////////////////
void TcpThread::err_sys(char * fmt,...)
{     
	perror(NULL);
	va_list args;
	va_start(args,fmt);
	fprintf(stderr,"error: ");
	vfprintf(stderr,fmt,args);
	fprintf(stderr,"\n");
	va_end(args);
	exit(1);
}
unsigned long TcpThread::ResolveName(char name[])
{
	struct hostent *host;            /* Structure containing host information */
	
	if ((host = gethostbyname(name)) == NULL)
		err_sys("gethostbyname() failed");
	
	/* Return the binary, network byte ordered address */
	return *((unsigned long *) host->h_addr_list[0]);
}

/*
msg_recv returns the length of bytes in the msg_ptr->buffer,which have been recevied successfully.
*/
int TcpThread::msg_recv(int sock,Msg * msg_ptr)
{
	int rbytes,n;
	
	for(rbytes=0;rbytes<MSGHDRSIZE;rbytes+=n)
		if((n=recv(sock,(char *)msg_ptr+rbytes,MSGHDRSIZE-rbytes,0))<=0)
			err_sys("Recv MSGHDR Error");
		
		for(rbytes=0;rbytes<msg_ptr->length;rbytes+=n)
			if((n=recv(sock,(char *)msg_ptr->buffer+rbytes,msg_ptr->length-rbytes,0))<=0)
				err_sys( "Recevier Buffer Error");
			
			return msg_ptr->length;
}

/* msg_send returns the length of bytes in msg_ptr->buffer,which have been sent out successfully
*/
int TcpThread::msg_send(int sock,Msg * msg_ptr)
{
	int n;
	if((n=send(sock,(char *)msg_ptr,MSGHDRSIZE+msg_ptr->length,0))!=(MSGHDRSIZE+msg_ptr->length))
		err_sys("Send MSGHDRSIZE+length Error");
	return (n-MSGHDRSIZE);
	
}

void TcpThread::run() 
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

				/* display the wsadata structure 
				cout<< endl
					<< "wsadata.wVersion "       << wsadata.wVersion       << endl
					<< "wsadata.wHighVersion "   << wsadata.wHighVersion   << endl
					<< "wsadata.szDescription "  << wsadata.szDescription  << endl
					<< "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
					<< "wsadata.iMaxSockets "    << wsadata.iMaxSockets    << endl
					<< "wsadata.iMaxUdpDg "      << wsadata.iMaxUdpDg      << endl;*/
			}  

			//Display info of local host 
			gethostname(localhost,128);
			//cout<<"ftpd_tcp starting at host: "<<localhost<< endl;

			if((hp=gethostbyname(localhost)) == NULL) 
			{
				cout << "gethostbyname() cannot get local host info?"
					<< WSAGetLastError() << endl; 
				exit(1);
			}

			//Successfull bind, now listen for client requests.
			if(listen(SERVER_SOCKET,10) == SOCKET_ERROR)
				throw "couldn't  set up listen on socket";
			//else cout << "Listen was successful" << endl;

			FD_ZERO(&readfds);

			int close_command=0;
			//wait loop

			//cout << "waiting to be contacted for transferring files..." << endl;

				FD_SET(SERVER_SOCKET,&readfds);  //always check the listener

				if(!(outfds=select(infds,&readfds,NULL,NULL,tp))) {}

				else if (outfds == SOCKET_ERROR) throw "failure in Select";

				else if (FD_ISSET(s,&readfds))  cout << "got a connection request" << endl; 

				//-------------------------------------------------------------------------------------------------------
				//receive user name....
				char user_name[128];
				if((ibytesrecv = recv(CLIENT_SOCKET,user_name,128,0)) == SOCKET_ERROR) // SERVER SHOULD RESPOND WITH "OK" RESPONSE
					throw "get failed\n";

				char user_name_response[128];
				sprintf_s(user_name_response,"ok");
				if (send(CLIENT_SOCKET,user_name_response, 128, 0) == SOCKET_ERROR) // TELL THE SERVER THE USER NAME
					throw "get failed\n";  

				std::string user_name_converted( reinterpret_cast< char const* >(user_name) ); //should be receiving the filename

				//-------------------------------------------------------------------------------------------------------

				//Fill in szbuffer from accepted request.
				if((ibytesrecv = recv(CLIENT_SOCKET,szbuffer,128,0)) == SOCKET_ERROR) //CLIENT SENDS THIS SERVER A REQUEST TYPE.
					throw "Receive error in server program\n";

				std::string msg_converted( reinterpret_cast< char const* >(szbuffer) ); 

				if(msg_converted == "get") //THE REQUEST IS A GET-TYPE
				{
						char response_msg[128];
						sprintf_s(response_msg,"ok");

						if(send(CLIENT_SOCKET,response_msg,128,0)==SOCKET_ERROR) //SERVER SENDS A OK CONFIRMATION TO CLIENT.
							throw "Send error in server program\n";

						char filenamebuffer[128];
						if((ibytesrecv = recv(CLIENT_SOCKET,filenamebuffer,128,0)) == SOCKET_ERROR)  //CLIENT SENDS FILE-NAME TO SERVER.
							throw "Receive error in server program\n";

						std::string file_name_converted( reinterpret_cast< char const* >(filenamebuffer) ); //should be receiving the filename

						//------------------------------------------------------------------------------------------------------------------
						//CHECK IF FILE EXISTS , THEN PROCESS FILE AND SEND IT BACK. IF FILE DOES NOT EXIST, THEN WE SEND A ERROR BACK TO CLIENT.
						//------------------------------------------------------------------------------------------------------------------
						ifstream file (file_name_converted, ios::out|ios::binary|ios::ate);
						if (file.is_open())
						{
								char confirmation_msg[128];
								sprintf_s(confirmation_msg,"fileexists");

								if(send(CLIENT_SOCKET,confirmation_msg,128,0)==SOCKET_ERROR)
									throw "Send error in server program\n";

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
									for(byte_in_packet=0; byte_in_packet<1300;byte_in_packet++)
									{
										
										//if(position_of_buffer <= filesize)
										//{
											//if(byte_in_packet==1299)
											//		memcpy (&packet_collection[packet_number].data[byte_in_packet], "\0" ,1); //LAST CHAR MUST BE A END
											if(position_of_buffer == filesize)
											{
												memcpy (&packet_collection[packet_number].data[byte_in_packet], "\0" ,1);
												break;
											}
											else
												memcpy (&packet_collection[packet_number].data[byte_in_packet], &memblock[position_of_buffer++],1);

										}				
									}
								//}//end of buffer for loop
								delete[] memblock; //get rid of the in-memory buffer storage.
						}//end of packet for loop

						else 
						{
							//ERROR CASE: Check if file exists....
							char error_msg[128];
							sprintf_s(error_msg,"nofile");

							if(send(CLIENT_SOCKET,error_msg,128,0)==SOCKET_ERROR) 
								throw "Send error in server program\n";

							throw "Unable to open file";
						}

						char hostname[128];
						gethostname(hostname,CLIENT_SOCKET); //user_name
						cout << "User '"<< user_name_converted << "' requested file " << file_name_converted << " to be sent." << endl;

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
								
						if(send(CLIENT_SOCKET,int_to_char,128,0)==SOCKET_ERROR) //SERVER SENDS THE TOTAL # OF DATA CHUNKS TO SEND.
								throw "Send error in server program\n";

						char start_command[128];
						if(recv(CLIENT_SOCKET,start_command,128,0) == SOCKET_ERROR) // WAIT FOR 'start' command from client
								throw "get data failed 1\n";

						std::string start_command_string_form( reinterpret_cast< char const* >(start_command) );

						if(start_command_string_form=="start")
						{
								cout << "Sending file to "<< user_name_converted << ", waiting........"<<endl;
								// in a for loop, we send each packet, piece by piece.
								for(int packet_counter=0; packet_counter < amount_of_packets; packet_counter++)
								{
										if(send(CLIENT_SOCKET,packet_collection[packet_counter].data,1300,0)==SOCKET_ERROR) //SERVER SENDS data.
												throw "Send error in server program\n";

										char receive_msg[128];

										if((ibytesrecv = recv(CLIENT_SOCKET,receive_msg,128,0)) == SOCKET_ERROR) // WAIT FOR 'received' msg from client
												throw "get data failed 2\n";
								}
								cout << "File Transfer to "<< user_name_converted << " is complete !!!"<<endl;;
						}											
				}

				//-----------------------------------
				else if(msg_converted == "put")
				{
						char request_response[128]; 

						sprintf_s(request_response,"ok");
			
						ibytessent=0; 
						ibufferlen = strlen(request_response);
						if (send(CLIENT_SOCKET,request_response, 128, 0) == SOCKET_ERROR) // TELL THE CLIENT WE GOT THE REQUEST.
							throw "get failed\n";  

						char file_name[128];
						if((ibytesrecv = recv(CLIENT_SOCKET,file_name,128,0)) == SOCKET_ERROR) // WAIT FOR THE FILE NAME FROM THE CLIENT
							throw "get failed\n";

						std::string file_name_converted( reinterpret_cast< char const* >(file_name) );

						//message
						char hostname[128];
						gethostname(hostname,CLIENT_SOCKET);
						cout << "User "<< user_name_converted << " requested to put the file " << file_name_converted << " on the server." << endl;

						if (send(CLIENT_SOCKET,request_response, 128, 0) == SOCKET_ERROR) // TELL THE CLIENT WE GOT THE REQUEST.
							throw "get failed\n";  

						char size_chunk[128];
						if((ibytesrecv = recv(CLIENT_SOCKET,size_chunk,128,0)) == SOCKET_ERROR) // WAIT FOR THE # OF PACKETS.
							throw "get failed\n";


						//setup file chunks to be received
						std::string data_chunk_string_form( reinterpret_cast< char const* >(size_chunk) );
						int data_chunk_int_form = atoi(data_chunk_string_form.c_str());

						packet_collection = new PACKET[data_chunk_int_form]; //setting up the data collection
						int total_bytes_of_file = data_chunk_int_form*1300;

						sprintf_s(request_response,"start");

						if (send(CLIENT_SOCKET,request_response, 128, 0) == SOCKET_ERROR)// TELL THE SERVER, THE FILE NAME TO BE TRANSFERED OVER.
							throw "file transfer initiation failed\n";  

						//message
						cout << "Receiving file from "<< user_name_converted << ".........."<<endl;

						for(int receive_loop=0; receive_loop < data_chunk_int_form; receive_loop++) // RECEIVE EACH DATA CHUNK
						{
							/*
							 * FILE RECEIVE LOGIC GOES HERE! (probably a for loop. LOOP(request <--> response) )
							 */
							char data_chunk[1300];
							if((ibytesrecv = recv(CLIENT_SOCKET,data_chunk,1300,0)) == SOCKET_ERROR) // WAIT FOR THE DATA CHUNK TO BE SENT.
								throw "get data failed\n";

							for(int data_byte=0; data_byte<1300; data_byte++) //GO THROUGH EACH BYTE IN DATA CHUNK AND DEEP COPY IT OVER TO ITS RESPECTIVE PACKET.
							{
								memcpy (&packet_collection[receive_loop].data[data_byte], &data_chunk[data_byte],1);
								//if(receive_loop==1299)
									//memcpy (&packet_collection[receive_loop].data[receive_loop+1], "\0" ,1);
							}
					
							char receive_msg[128];
							sprintf_s(receive_msg,"received");

							if (send(CLIENT_SOCKET,receive_msg, 128, 0) == SOCKET_ERROR)// TELL THE SERVER, THE FILE NAME TO BE TRANSFERED OVER.
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
						WriteToFile(file_name,file_data_buffer_TOBEWRITTEN);
						//###################################################################################
						
						//message
						cout << "File "<< file_name << " from "<< user_name_converted << " received !"<<endl;
				}
		} //end of try 

		//Display needed error message.
		catch(char* str)
		{ 
			cerr<<str<<WSAGetLastError()<<endl;
		}

		//close Client socket
		closesocket(CLIENT_SOCKET);		

		/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
		WSACleanup();

		cout << "waiting for new connection......" << endl;
}


int main(void)
{
	
	TcpServer ts;
	ts.start();
	
	return 0;
}

