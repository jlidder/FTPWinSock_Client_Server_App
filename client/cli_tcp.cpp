// CLIENT TCP PROGRAM
// Revised and tidied up by
// J.W. Atwood
// 1999 June 30


//char* getmessage(char *);



/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>

#include <winsock.h>
#include <stdio.h>
#include <iostream>

#include <string.h>
#include <fstream>
#include <windows.h>
#include <sstream>

#include "aux_cli.h"

using namespace std;



int main(void){

	WSADATA wsadata;

	try {

		FILE * logFile;
		int n;
		char name [100];
		logFile = fopen ("log.txt","a");


		if (WSAStartup(0x0202,&wsadata)!=0){  
			cout<<"Error in starting WSAStartup()" << endl;
		} else {
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
		cout<<"Local host name is \"" << localhost << "\"" << endl;

		//log
		char log_msg[128] = "Client: Starting on host: ";
		fprintf(logFile,strcat(log_msg,localhost));
		fprintf(logFile,"\n");
		fclose(logFile);

		if((hp=gethostbyname(localhost)) == NULL)	
			throw "gethostbyname failed\n";

		//Ask for name of remote server

		cout << "please enter your remote server name :" << flush ;   
		cin >> remotehost ;
		cout << "Remote host name is: \"" << remotehost << "\"" << endl;

		if((rp=gethostbyname(remotehost)) == NULL)
			throw "remote gethostbyname failed\n";

		//Create the socket
		if((s = socket(AF_INET,SOCK_DGRAM,0))==INVALID_SOCKET) 
			throw "Socket failed\n";
		/* For UDP protocol replace SOCK_STREAM with SOCK_DGRAM */


		//Bind the server port

		//Fill-in Client Port and Address info.
		sa3.sin_family = AF_INET;
		sa3.sin_port = htons(5000); // 5000 is listening port on client.
		sa3.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(s,(LPSOCKADDR)&sa3,sizeof(sa3)) == SOCKET_ERROR)
			throw "can't bind the socket";
		cout << "Bind was successful" << endl;

		int sa_len = sizeof(sa);


		//Specify server address for client to connect to server.
		memset(&sa_in,0,sizeof(sa_in)); 
		memcpy(&sa_in.sin_addr,rp->h_addr,rp->h_length);
		sa_in.sin_family = rp->h_addrtype;   
		sa_in.sin_port = htons(port);

		//Display the host machine internet address

		cout << "What kind of command do you want? (list, get, put, delete)" << endl;
		cin >> msgFrame.command;
		if(msgFrame.command[0] == 'g' || msgFrame.command[0] == 'G'){
			cout << "What is the file name you want to get?" << endl;
			cin >> msgFrame.file_name;}

		if(msgFrame.command[0] == 'p' || msgFrame.command[0] == 'P')
		{
			cout << "What is the file you want to put?" << endl;
			cin >> msgFrame.file_name;
		}

		cout << "Handshaking beginning with remote host:";
		cout << inet_ntoa(sa_in.sin_addr) << endl;	
		hndShake();//initiate handshake


		//Commands begin
		if (msgFrame.command[0] == 'l' || msgFrame.command[0] == 'L')
		{
			msgFrame.data; //should receive total no of files.
            string list_size_string( reinterpret_cast< char const* >(msgFrame.data) );
			int list_size = atoi(list_size_string.c_str());

            sprintf(msgFrame.data,"startfilenames111111");

            char** File_Names_Array_PTRS = new char*[list_size];
            for(int i = 0; i < list_size; ++i)
            {
                    File_Names_Array_PTRS[i] = new char[128];
            }

            sendNwait(msgFrame);

            //will receive and ACK in this loop
            for(int counter=0; counter<list_size; counter++)
            {
                    //string filename_rcvd( reinterpret_cast< char const* >(mgFrame.data) );
                    cout << msgFrame.data << endl;
                    sprintf(msgFrame.data,"rcvdname");
                    sendNwait(msgFrame);
            }
			sprintf(msgFrame.data,"donelist");
			sendNwait(msgFrame);
		}

		else if (msgFrame.command[0] == 'g' || msgFrame.command[0] == 'G')
		{//received filesize
			
			if(msgFrame.file_size <0 )
				throw "file does not exist on server!";

			else//receive file
			{//calulate packet number
				packet_amount = (int)ceil(((double)msgFrame.file_size/PACKET_SIZE));
				ofstream outFile( msgFrame.file_name, ios::binary | ios::out);

				for(int i=1;i<=packet_amount;i++)//needs to use select for final implementation
				{
					sendNwait(msgFrame);
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
			}
		}//end of get loop

		else if (msgFrame.command[0] == 'p' || msgFrame.command[0] == 'P')
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
						//send what is readsendNwait(msgFrame);
						sendNwait(msgFrame);
					}
				}
				else
				{
					cout << msgFrame.file_name << endl;
					msgFrame.file_size = -1;
						
					//sending no file found -1
					sendNwait(msgFrame);
					cout << "file doesn't exist";

				}
				inFile.close();				
		}

		else if (msgFrame.command[0] == 'd' || msgFrame.command[0] == 'D')
		{
			sprintf(msgFrame.data,hsFrame.file_name);
			sendNwait(msgFrame);

			string delete_success_or_failure( reinterpret_cast< char const*>(msgFrame.data) );

			if(!strcmp(delete_success_or_failure.c_str() , "success"))
					throw "file was not deleted successfully on server :(";
		} 
	} // try loop

	//Display any needed error response.

	catch (char *str) { cerr<<str<<":"<<dec<<WSAGetLastError()<<endl;
						    wprintf(L" with 0x%x\n", GetLastError());}

	//close the client socket
	closesocket(s);

	/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
	WSACleanup();  

	return 0;
}


void hndShake()
{///initialize to non seqNo

	//preparing log stuff.
	FILE * logFile;
	char name [100];
	logFile = fopen ("log.txt","a");
	char log_msg_sending[128] = "Client: sent packet: ";
	char log_msg_receiving[128] = "Client: received ACK for packet: ";
	char log_msg_3wayhandshake[128] = "Client: 3 way handshaking beginning\n";
	char log_msg_3wayhandshake_finished[128] = "Client: 3 way handshaking completed! \n";

	fprintf(logFile,log_msg_3wayhandshake);

	msgFrame.client_seqNo = -1;
	msgFrame.server_seqNo = -1;
	int serverNo = rand() % 256; //rand number 0 to 255
	msgFrame.client_number = serverNo;
	cout<<"Sending " << msgFrame.client_number;
	do
	{//resends packet until it receives responds

		string pack_num_string;          // string which will contain the result
		ostringstream convert;   // stream used for the conversion
		convert << msgFrame.client_number;// insert the textual representation of 'Number' in the characters in the stream
		pack_num_string = convert.str(); // set 'Result' to the contents of the stream
		fprintf(logFile,strcat(log_msg_sending,pack_num_string.c_str()));
	    fprintf(logFile,"\n");

		if((ibytessent = sendto(s,(char*)&msgFrame,sizeof(msgFrame),0,(sockaddr *)&sa_in, sizeof(sa_in)))==SOCKET_ERROR)
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

			ostringstream convert1;
			convert1 << msgFrame.server_number;// insert the textual representation of 'Number' in the characters in the stream
			pack_num_string = convert1.str(); // set 'Result' to the contents of the stream
			fprintf(logFile,strcat(log_msg_receiving,pack_num_string.c_str()));
			fprintf(logFile,"\n");
		}
	}while (RetVal ==0 ||serverNo != msgFrame.client_number);	
	cout << " Received " << msgFrame.server_number<<endl;
	msgFrame.client_seqNo = msgFrame.client_number % 2;//sets the sequence number
	
	cout<<"Handshake complete";
	int seqNo = msgFrame.server_seqNo;

	do
	{
		string pack_num_string1;          // string which will contain the result
		ostringstream convert2;   // stream used for the conversion
		convert2 << msgFrame.client_number;// insert the textual representation of 'Number' in the characters in the stream
		pack_num_string1 = convert2.str(); // set 'Result' to the contents of the stream
		fprintf(logFile,strcat(log_msg_sending,pack_num_string1.c_str()));
	    fprintf(logFile,"\n");

		ibytessent = sendto(s,(char*)&msgFrame,sizeof(msgFrame),0,(sockaddr *)&sa_in, sizeof(sa_in));
		if (ibytessent == SOCKET_ERROR)
			throw "Send failed\n";
		struct timeval tp;
		tp.tv_usec =0;
		tp.tv_sec=1;
		FD_ZERO(&readfds); //initialize 
		FD_SET(s, &readfds); //put the socket in the set
		if((RetVal = select (1 , &readfds, NULL, NULL, &tp))==SOCKET_ERROR) 
		 {exit(EXIT_FAILURE);}
		else if(RetVal > 0)
		{
			if((ibytesrecv = recvfrom(s,(char*) &msgFrame,sizeof(msgFrame),0, (SOCKADDR *) & SenderAddr, &SenderAddrSize)) == SOCKET_ERROR)
				throw "Receive failed\n";

			ostringstream convert3;
			convert3 << msgFrame.server_number;// insert the textual representation of 'Number' in the characters in the stream
			string pack_num_string2 = convert3.str(); // set 'Result' to the contents of the stream
			fprintf(logFile,strcat(log_msg_receiving,pack_num_string2.c_str()));
			fprintf(logFile,"\n");
		}	
	}while (RetVal ==0 ||(msgFrame.server_seqNo==-1));

	fprintf(logFile,log_msg_3wayhandshake_finished);
	fclose(logFile);
}

//Stop and Wait functions
 void sendNwait(MESSAGE_FRAME &buff)
{//sends and waits for reply. If time out resends
	FILE * logFile;
	char name [100];
	logFile = fopen ("log.txt","a");

	int seqNo = buff.server_seqNo;
	int newSeq = (msgFrame.server_seqNo+1) %2;
	msgFrame.server_seqNo=newSeq;
	do
	{
		ibytessent = sendto(s,(char*)&buff,sizeof(buff),0,(sockaddr *)&sa_in, sizeof(sa_in));
		if (ibytessent == SOCKET_ERROR)
			throw "Send failed\n";

		char log_msg_sending[128] = "Client: sent packet: ";
		char log_msg_receiving[128] = "Client: received ACK for packet: ";

		string pack_num_string;          // string which will contain the result
		ostringstream convert;   // stream used for the conversion
		convert << msgFrame.client_seqNo;// insert the textual representation of 'Number' in the characters in the stream
		pack_num_string = convert.str(); // set 'Result' to the contents of the stream
		fprintf(logFile,strcat(log_msg_sending,pack_num_string.c_str()));
	    fprintf(logFile,"\n");

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

			string pack_num_string;          // string which will contain the result
			ostringstream convert;   // stream used for the conversion
			convert << msgFrame.server_seqNo;// insert the textual representation of 'Number' in the characters in the stream
			pack_num_string = convert.str(); // set 'Result' to the contents of the stream
			fprintf(logFile,strcat(log_msg_receiving,pack_num_string.c_str()));
			fprintf(logFile,"\n");
		}	
	}while (RetVal ==0 || (seqNo == buff.server_seqNo));	

	fclose(logFile);
}


