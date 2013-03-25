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
                    sendNwait(); //send no of files.

                    //STEP 3 : SEND FILE NAMES
                    string file_start_response( reinterpret_cast< char const* >(szbuffer) );

                    for(int counter=0; counter<no_of_files; counter++)
                    {
                            hFind = FindFirstFile("c:\\*.*", &data);
                            if (hFind != INVALID_HANDLE_VALUE)
                            {
                                    do
                                    {
                                            sprintf(msgFrame.data,data.cFileName);
                                            sendNwait();

											string rcvd_msg( reinterpret_cast< char const* >(msgFrame.data) );
											char donelistchar[128] = "donelist";

											if(strcmp(rcvd_msg.c_str(),donelistchar))
											{
												//break;
											}

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

						FILE * logFile2;
						char name [100];
						logFile2 = fopen ("log.txt","a");

						ostringstream convert5;
						convert5 << msgFrame.file_size;// insert the textual representation of 'Number' in the characters in the stream
						string file_size_string = convert5.str(); // set 'Result' to the contents of the stream
						char file_transfer_msg[200]="File transfer initiated(GET)\n";
						fprintf(logFile2,file_transfer_msg);
						fclose(logFile2);

						msgFrame.file_size = inFile.tellg();
						inFile.seekg(0, ios::beg);
						//sending file size
						sendNwait();
					
						//sending file
						int packet_amount = (int)ceil(((double)msgFrame.file_size/(double)PACKET_SIZE));
						for(int i=1;i<=packet_amount;i++)
						{
							inFile.read(msgFrame.data,PACKET_SIZE);//read from file		
							//send what is read
							sendNwait();
						}

						FILE * logFile1;
						logFile1 = fopen ("log.txt","a");

						ostringstream convert55;
						convert55 << msgFrame.file_size;// insert the textual representation of 'Number' in the characters in the stream
						string file_size_string1212 = convert55.str(); // set 'Result' to the contents of the stream
						char file_size_msg[200]="Number of Bytes Sent to Client: ";
						strcat(file_size_msg,file_size_string1212.c_str());
						strcat(file_size_msg,"\n");
						fprintf(logFile1,file_size_msg);
						fclose(logFile1);
					}
					else
					{
						msgFrame.file_size = -1;
						
						//sending no file found -1
						sendNwait();
						cout << "file doesn't exist";

					}
					inFile.close();					
				}//end of Get

				else if (msgFrame.command[0] == 'p' || msgFrame.command[0] == 'P')
				{
					sprintf(msgFrame.data,"starttransfer");

					sendNwait();

					if(msgFrame.file_size <0 )
						throw "file does not exist on server!";

					else//receive file
					{//calulate packet number

						FILE * logFile2;
						char name [100];
						logFile2 = fopen ("log.txt","a");

						ostringstream convert5;
						convert5 << msgFrame.file_size;// insert the textual representation of 'Number' in the characters in the stream
						string file_size_string = convert5.str(); // set 'Result' to the contents of the stream
						char file_transfer_msg[200]="File transfer initiated(PUT)\n";
						fprintf(logFile2,file_transfer_msg);
						fclose(logFile2);

						int packet_amount = (int)ceil(((double)msgFrame.file_size/PACKET_SIZE));
						ofstream outFile( msgFrame.file_name, ios::binary | ios::out);

						for(int i=1;i<=packet_amount;i++)//needs to use select for final implementation
						{
							sendNwait();
							if(i < packet_amount)
							{
								outFile.write(msgFrame.data,PACKET_SIZE);
							}
							else
							{						
								int lastPacketSize = msgFrame.file_size - ((packet_amount - 1) * PACKET_SIZE);
						
								outFile.write(msgFrame.data,lastPacketSize);
								outFile.close();
								cout<<"File is completely received"<<endl;
							}					
						}

						FILE * logFile1;
						logFile1 = fopen ("log.txt","a");

						ostringstream convert55;
						convert55 << msgFrame.file_size;// insert the textual representation of 'Number' in the characters in the stream
						string file_size_string1212 = convert55.str(); // set 'Result' to the contents of the stream
						char file_size_msg[200]="Number of Bytes Downloaded to Server HDD: ";
						strcat(file_size_msg,file_size_string1212.c_str());
						strcat(file_size_msg,"\n");
						fprintf(logFile1,file_size_msg);
						fclose(logFile1);
					}
				}

				else if (msgFrame.command[0] == 'd' || msgFrame.command[0] == 'D')
				{
					string tempmsg = "send me file name!";
                    sprintf(msgFrame.data, tempmsg.c_str());
                    sendNwait(); //send no of files.

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

		//preparing log stuff.
		FILE * logFile;
		char name [100];
		logFile = fopen ("log.txt","a");
		char log_msg_sending[128] = "Client: sent packet: ";
		char log_msg_receiving[128] = "Client: received ACK for packet: ";
		char log_msg_3wayhandshake[128] = "Client: 3 way handshaking beginning\n";
		char log_msg_3wayhandshake_finished[128] = "Client: 3 way handshaking completed! \n";

		msgFrame.client_seqNo = -1;
		msgFrame.server_seqNo = -1;
		msgFrame.client_number = -1;
		msgFrame.server_number = -1;

		if((ibytesrecv = recvfrom(s,(char*)&msgFrame,sizeof(msgFrame),0,(SOCKADDR *)&SenderAddr,&SenderAddrSize)) == SOCKET_ERROR)
						throw "Receive error in server program\n";

		ostringstream convert5;
		convert5 << msgFrame.server_number;// insert the textual representation of 'Number' in the characters in the stream
		string pack_num_string16 = convert5.str(); // set 'Result' to the contents of the stream
		fprintf(logFile,strcat(log_msg_receiving,pack_num_string16.c_str()));
		fprintf(logFile,"\n");

		fprintf(logFile,log_msg_3wayhandshake);

		int serverNo = rand() % 256;
		msgFrame.server_number =serverNo;
		cout<<"received "<<msgFrame.client_number<< " and sending " <<msgFrame.server_number<<endl;
		//making a deep copy of original values in case of resending
		hldFrame.client_number = msgFrame.client_number; hldFrame.server_number = msgFrame.server_number; hldFrame.client_seqNo = msgFrame.client_seqNo;hldFrame.server_seqNo=msgFrame.server_seqNo;
		sprintf(hldFrame.command,msgFrame.command);sprintf(hldFrame.file_name,msgFrame.file_name);hldFrame.file_size=msgFrame.file_size;
		memcpy(hldFrame.data,msgFrame.data,PACKET_SIZE);
		bool wrongPacket = false;
		do
		{
			char log_msg_sending[128] = "Client: sent packet: ";
			char log_msg_receiving[128] = "Client: received ACK for packet: ";
			char log_msg_3wayhandshake[128] = "Client: 3 way handshaking beginning\n";
			char log_msg_3wayhandshake_finished[128] = "Client: 3 way handshaking completed! \n";

			if((ibytessent = sendto(s,(char*)&msgFrame,sizeof(msgFrame),0,(SOCKADDR *)&SenderAddr, sizeof (SenderAddr)))==SOCKET_ERROR)
						{throw "error in send in server program\n";}

			//log the packet sent in log file.
			string pack_num_string;          
			ostringstream convert4;  
			convert4 << msgFrame.client_number;
			string pack_num_string14 = convert4.str();
			fprintf(logFile,strcat(log_msg_sending,pack_num_string14.c_str()));
			fprintf(logFile,"\n");

			struct timeval tp;
			tp.tv_usec =300000;
			tp.tv_sec=0;
			FD_ZERO(&readfds); //initialize 
			FD_SET(s, &readfds); //put the socket in the set
			if((RetVal = select (1 , &readfds, NULL, NULL, &tp))==SOCKET_ERROR) 
			 {exit(EXIT_FAILURE);}
			else if(RetVal > 0)
			{//it now will wait for the msgFrame to begin data tranfer
				if((ibytesrecv = recvfrom(s,(char*) &msgFrame,sizeof(msgFrame),0, (SOCKADDR *) & SenderAddr, &SenderAddrSize)) == SOCKET_ERROR)
					throw "Receive failed\n";

				ostringstream convert3;
				convert3 << msgFrame.server_number;// insert the textual representation of 'Number' in the characters in the stream
				string pack_num_string9 = convert3.str(); // set 'Result' to the contents of the stream
				fprintf(logFile,strcat(log_msg_receiving,pack_num_string9.c_str()));
				fprintf(logFile,"\n");
		
				if((msgFrame.client_seqNo<0))
				{	//reverse msgFrame by deep copy
					msgFrame.client_number=hldFrame.client_number;msgFrame.server_number=hldFrame.server_number;msgFrame.client_seqNo=hldFrame.client_seqNo;msgFrame.server_seqNo=hldFrame.server_seqNo;
					sprintf(msgFrame.command,hldFrame.command);sprintf(msgFrame.file_name,hldFrame.file_name);msgFrame.file_size=hldFrame.file_size;
					memcpy(msgFrame.data,hldFrame.data,PACKET_SIZE);
					wrongPacket = true; // set the wrongPacket as true
				}
				else
					wrongPacket = false;
			}
		}while (RetVal ==0 ||wrongPacket );
		msgFrame.server_seqNo = (msgFrame.server_number %2);//create server sequence number
		
		fprintf(logFile,log_msg_3wayhandshake_finished);
		fclose(logFile);
	}

	//Stop and Wait functions

	 void sendNwait()
	{//sends and waits for reply. If time out resends

		FILE * logFile;
		char name [100];
		logFile = fopen ("log.txt","a");

		msgFrame.client_seqNo =(msgFrame.client_seqNo+1) %2;//increase the cleint seqNo
		//making a deep copy of original values in case of resending
		hldFrame.client_number = msgFrame.client_number; hldFrame.server_number = msgFrame.server_number; hldFrame.client_seqNo = msgFrame.client_seqNo;hldFrame.server_seqNo=msgFrame.server_seqNo;
		sprintf(hldFrame.command,msgFrame.command);sprintf(hldFrame.file_name,msgFrame.file_name);hldFrame.file_size=msgFrame.file_size;
		memcpy(hldFrame.data,msgFrame.data,PACKET_SIZE);
		bool wrongPacket = false;//control bool
		do
		{	//waits and resends if it times out.  Also resends if same packet is received
			if((ibytessent = sendto(s,(char*)&msgFrame,sizeof(msgFrame),0,(SOCKADDR *)&SenderAddr, sizeof (SenderAddr)))==SOCKET_ERROR)
						{throw "error in send in server program\n";}

			char log_msg_sending[128] = "Client: sent packet: ";
			char log_msg_receiving[128] = "Client: received ACK for packet: ";
			string pack_num_string;         
			ostringstream convert2; 
			convert2.clear();
			convert2 << msgFrame.client_seqNo;
			string pack_num_string7 = convert2.str(); 
			fprintf(logFile,strcat(log_msg_sending,pack_num_string7.c_str()));
			fprintf(logFile,"\n");

			struct timeval tp;
			tp.tv_usec =300000;//time out
			tp.tv_sec=0;
			FD_ZERO(&readfds); //initialize 
			FD_SET(s, &readfds); //put the socket in the set
			if((RetVal = select (1 , &readfds, NULL, NULL, &tp))==SOCKET_ERROR) 
			 {exit(EXIT_FAILURE);}
			else if(RetVal > 0)
			{
				if((ibytesrecv = recvfrom(s,(char*) &msgFrame,sizeof(msgFrame),0, (SOCKADDR *) & SenderAddr, &SenderAddrSize)) == SOCKET_ERROR)
					throw "Receive failed\n";

				string pack_num_string;         
				ostringstream convert1;
				convert1.clear();
				convert1 << msgFrame.server_seqNo;
				string pack_num_string6 = convert1.str(); 
				fprintf(logFile,strcat(log_msg_receiving,pack_num_string6.c_str()));
				fprintf(logFile,"\n");

				if(hldFrame.server_seqNo == msgFrame.server_seqNo|| msgFrame.server_seqNo<0)
				{	//reverse msgFrame by deep copy
					msgFrame.client_number=hldFrame.client_number;msgFrame.server_number=hldFrame.server_number;msgFrame.client_seqNo=hldFrame.client_seqNo;msgFrame.server_seqNo=hldFrame.server_seqNo;
					sprintf(msgFrame.command,hldFrame.command);sprintf(msgFrame.file_name,hldFrame.file_name);msgFrame.file_size=hldFrame.file_size;
					memcpy(msgFrame.data,hldFrame.data,PACKET_SIZE);
					wrongPacket = true; // set the wrongPacket as true
				}			
				else
					wrongPacket = false;
			}
		}while  (RetVal ==0 || wrongPacket);

		fclose(logFile);
	}



