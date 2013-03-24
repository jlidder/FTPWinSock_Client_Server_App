//    SERVER TCP PROGRAM
// revised and tidied up by
// J.W. Atwood
// 1999 June 30
// There is still some leftover trash in this code.

/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma once
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <winsock.h>
#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <fstream>
#include "aux_ser.h"
#include <sstream>



using namespace std;


	int main(void){

		WSADATA wsadata;

		try{        		 
			if (WSAStartup(0x0202,&wsadata)!=0){  
				cout<<"Error in starting WSAStartup()\n";
			}else{
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
			cout<<"hostname: "<<localhost<< endl;

			if((hp=gethostbyname(localhost)) == NULL) {
				cout << "gethostbyname() cannot get local host info?"
					<< WSAGetLastError() << endl; 
				exit(1);
			}

			//Create the server socket
			if((s = socket(AF_INET,SOCK_DGRAM,0))==INVALID_SOCKET) 
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

			int sa_len = sizeof(sa);
			
			
			//while(1)

		//	{

				//waiting to receiving handshake
				hndShake();
				
			
				if (msgFrame.command[0] == 'l' || msgFrame.command[0]== 'L')
				{
					//STEP 1 : SEND NO. OF FILES TO CLIENT.
                    HANDLE hFind;
                    WIN32_FIND_DATA data;
                    int no_of_files=0;

                    hFind = FindFirstFile("c:\\*.*", &data);
                    if (hFind != INVALID_HANDLE_VALUE)
                    {
                            do
                            {
                                printf("%s\n", data.cFileName);
                                no_of_files++;
                            }while (FindNextFile(hFind, &data));

                            FindClose(hFind);
                    }

                    char* no_of_files_char;
                    std::string tempstring;
                    std::stringstream out;
                    out << no_of_files;
                    sprintf(msgFrame.data,out.str().c_str());
                    sendNwait(msgFrame); //send no of files.

                    //STEP 3 : SEND FILE NAMES
                    string file_start_response( reinterpret_cast< char const* >(szbuffer) );

                    for(int counter=0; counter<no_of_files; counter++)
                    {
                            hFind = FindFirstFile("c:\\*.*", &data);
                            if (hFind != INVALID_HANDLE_VALUE)
                            {
                                    do
                                    {
                                            //printf("%s\n", data.cFileName);
                                            sprintf(msgFrame.data,data.cFileName);
                                            sendNwait(msgFrame);
                                    }while (FindNextFile(hFind, &data));

                                    FindClose(hFind);
                            }
                    }
				}

				else if (msgFrame.command[0] == 'g' || msgFrame.command[0] == 'G')
				{
					ifstream inFile(msgFrame.file_name, ios::in|ios::binary|ios::ate);
					if(inFile.is_open())
					{
						msgFrame.file_size = inFile.tellg();
						inFile.seekg(0, ios::beg);
						//sending file size
						sendNwait(msgFrame);
					
						//sending file
						int packet_amount = (int)ceil(((double)msgFrame.file_size/(double)PACKET_SIZE));
						for(int i=1;i<=packet_amount;i++)
						{
							inFile.read(msgFrame.data,PACKET_SIZE);//read from file		
							//send what is read
							sendNwait(msgFrame);
						}
					}
					else
					{
						msgFrame.file_size = -1;
						
						//sending no file found -1
						sendNwait(msgFrame);
						cout << "file doesn't exist";

					}
					inFile.close();					
				}//end of Get

				else if (msgFrame.command[0] == 'p' || msgFrame.command[0] == 'P')
				{
					
				}

				else if (msgFrame.command[0] == 'd' || msgFrame.command[0] == 'D')
				{
					string tempmsg = "send me file name!";
                    sprintf(msgFrame.data, tempmsg.c_str());
                    sendNwait(msgFrame); //send no of files.

                    //received file name
                    boolean fSuccess = DeleteFile(TEXT(msgFrame.data));

                    if (!fSuccess)
                    {
                            // Handle the error.
                            printf ("Delete File failed, error %d\n", GetLastError());
                            return (6);
                    }

                    else
                            printf ("Delete File is  OK!\n");
				} 

		//	}//wait loop

		} //try loop

		//Display needed error message.

		catch(char* str) { cerr<<str<<WSAGetLastError()<<endl;}	

		//close server socket
		closesocket(s);

		/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
		WSACleanup();
		return 0;
	}

void hndShake()
{//waits  for initiation	
	msgFrame.client_seqNo = -1;
	msgFrame.server_seqNo = -1;

	if((ibytesrecv = recvfrom(s,(char*)&msgFrame,sizeof(msgFrame),0,(SOCKADDR *)&SenderAddr,&SenderAddrSize)) == SOCKET_ERROR)
					throw "Receive error in server program\n";
	int serverNo = rand() % 256;
	msgFrame.server_number =serverNo;
	cout<<"received "<<msgFrame.client_number<< " and sending " <<msgFrame.server_number<<endl;
	
	do
	{
		if((ibytessent = sendto(s,(char*)&msgFrame,sizeof(msgFrame),0,(SOCKADDR *)&SenderAddr, sizeof (SenderAddr)))==SOCKET_ERROR)
					{throw "error in send in server program\n";}
		struct timeval tp;
		tp.tv_usec = 0;
		tp.tv_sec=1;
		FD_ZERO(&readfds); //initialize 
		FD_SET(s, &readfds); //put the socket in the set
		if((RetVal = select (1 , &readfds, NULL, NULL, &tp))==SOCKET_ERROR) 
		 {exit(EXIT_FAILURE);}
		else if(RetVal > 0)
		{//it now will wait for the msgFrame to begin data tranfer
			if((ibytesrecv = recvfrom(s,(char*) &msgFrame,sizeof(msgFrame),0, (SOCKADDR *) & SenderAddr, &SenderAddrSize)) == SOCKET_ERROR)
				throw "Receive failed\n";
		}
	}while (RetVal ==0 || serverNo != msgFrame.server_number);
	msgFrame.server_seqNo = msgFrame.server_number %2;//set the sequence number
}

//Stop and Wait functions

 void sendNwait(MESSAGE_FRAME &buff)
{//sends and waits for reply. If time out resends
	int seqNo = buff.client_seqNo;
	int newSeq =(msgFrame.client_seqNo+1) %2;//increase the seqNo
	msgFrame.client_seqNo=newSeq;
	do
	{		
		if((ibytessent = sendto(s,(char*)&buff,sizeof(buff),0,(SOCKADDR *)&SenderAddr, sizeof (SenderAddr)))==SOCKET_ERROR)
					{throw "error in send in server program\n";}
		struct timeval tp;
		tp.tv_usec =0;
		tp.tv_sec=1;
		FD_ZERO(&readfds); //initialize 
		FD_SET(s, &readfds); //put the socket in the set
		if((RetVal = select (1 , &readfds, NULL, NULL, &tp))==SOCKET_ERROR) 
		 {exit(EXIT_FAILURE);}
		else if(RetVal > 0)
		{
			if((ibytesrecv = recvfrom(s,(char*) &buff,sizeof(buff),0, (SOCKADDR *) & SenderAddr, &SenderAddrSize)) == SOCKET_ERROR)
				throw "Receive failed\n";
		}
			
	}while (RetVal ==0 || (seqNo == buff.client_seqNo));
	
}
