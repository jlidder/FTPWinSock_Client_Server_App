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

		if(msgFrame.command[0] == 'd' || msgFrame.command[0] == 'D'){
			cout << "What is the file name you want to delete?" << endl;
			cin >> msgFrame.file_name;}

		

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

            sendNwait();

            //will receive and ACK in this loop
            for(int counter=0; counter<list_size; counter++)
            {
                    //string filename_rcvd( reinterpret_cast< char const* >(mgFrame.data) );
                    cout << msgFrame.data << endl;
                    sprintf(msgFrame.data,"rcvdname");
                    sendNwait();
            }
			sprintf(msgFrame.data,"donelist");
			sendNwait();
			system("PAUSE");
		}

		else if (msgFrame.command[0] == 'g' || msgFrame.command[0] == 'G')
		{//received filesize
			
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
				char file_transfer_msg[200]="File transfer initiated(GET)\n";
				fprintf(logFile2,file_transfer_msg);
				fclose(logFile2);

				packet_amount = (int)ceil(((double)msgFrame.file_size/PACKET_SIZE));
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
			}
		}//end of get loop

		else if (msgFrame.command[0] == 'p' || msgFrame.command[0] == 'P')
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
					char file_transfer_msg[200]="File transfer initiated(PUT)\n";
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
						//send what is readsendNwait(msgFrame);
						sendNwait();
					}
				}
				else
				{
					cout << msgFrame.file_name << endl;
					msgFrame.file_size = -1;
					cout << "file doesn't exist. Please restart client and server :)";

					//sending no file found -1
					sendNwait();
					

				}
				inFile.close();				
		}

		else if (msgFrame.command[0] == 'd' || msgFrame.command[0] == 'D')
		{
			sprintf(msgFrame.data,hldFrame.file_name);
			sendNwait();

			string delete_success_or_failure( reinterpret_cast< char const*>(msgFrame.data) );

			if(!strcmp(delete_success_or_failure.c_str() , "success"))
					throw "file was not deleted successfully on server :(";
			cout<<"file deleted";
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
{
	//preparing log stuff.
	FILE * logFile;
	char name [100];
	logFile = fopen ("log.txt","a");
	char log_msg_sending[128] = "Client: sent packet: ";
	char log_msg_receiving[128] = "Client: received ACK for packet: ";
	char log_msg_3wayhandshake[128] = "Client: 3 way handshaking beginning\n";
	char log_msg_3wayhandshake_finished[128] = "Client: 3 way handshaking completed! \n";

	fprintf(logFile,log_msg_3wayhandshake);

	///initialize to non seqNo no
	msgFrame.client_seqNo = -1;
	msgFrame.server_seqNo = -1;
	msgFrame.client_number = -1;
	msgFrame.server_number = -1;
	int clientNo = rand() % 256; //rand number 0 to 255
	msgFrame.client_number = clientNo;
	cout<<"Sending " << msgFrame.client_number;
	do
	{//resends packet until it receives responds
		char log_msg_sending[128] = "Client: sent packet: ";
		char log_msg_receiving[128] = "Client: received ACK for packet: ";
		char log_msg_3wayhandshake[128] = "Client: 3 way handshaking beginning\n";
		char log_msg_3wayhandshake_finished[128] = "Client: 3 way handshaking completed! \n";

		if((ibytessent = sendto(s,(char*)&msgFrame,sizeof(msgFrame),0,(sockaddr *)&sa_in, sizeof(sa_in)))==SOCKET_ERROR)//initial hs with client Number
					{throw "error in send in server program\n";}

		//log the packet sent in log file.
		string pack_num_string;          
		ostringstream convert;  
		convert << msgFrame.client_number;
		pack_num_string = convert.str();
		fprintf(logFile,strcat(log_msg_sending,pack_num_string.c_str()));
	    fprintf(logFile,"\n");

		struct timeval tp;
		tp.tv_usec = 0;
		tp.tv_sec=1;
		FD_ZERO(&readfds); //initialize 
		FD_SET(s, &readfds); //put the socket in the set
		if((RetVal = select (1 , &readfds, NULL, NULL, &tp))==SOCKET_ERROR) 
		 {exit(EXIT_FAILURE);}
		else if(RetVal > 0)
		{//it now will wait for the msgFrame to begin data tranfer
			if((ibytesrecv = recvfrom(s,(char*) &msgFrame,sizeof(msgFrame),0, (SOCKADDR *) & SenderAddr, &SenderAddrSize)) == SOCKET_ERROR)//wating for server to reply with servernumber&
				throw "Receive failed\n";																								//client number

			ostringstream convert1;
			convert1 << msgFrame.server_number;// insert the textual representation of 'Number' in the characters in the stream
			pack_num_string = convert1.str(); // set 'Result' to the contents of the stream
			fprintf(logFile,strcat(log_msg_receiving,pack_num_string.c_str()));
			fprintf(logFile,"\n");
		}
	}while (RetVal ==0 ||msgFrame.server_number <0);	

	cout << " Received " << msgFrame.server_number<<endl;
	msgFrame.client_seqNo = msgFrame.client_number % 2;//create client sequence number
	
	cout<<"Handshake complete\n";
	//making a deep copy of original values in case of resending
	hldFrame.client_number = msgFrame.client_number; hldFrame.server_number = msgFrame.server_number; hldFrame.client_seqNo = msgFrame.client_seqNo;hldFrame.server_seqNo=msgFrame.server_seqNo;
	sprintf(hldFrame.command,msgFrame.command);sprintf(hldFrame.file_name,msgFrame.file_name);hldFrame.file_size=msgFrame.file_size;
	memcpy(hldFrame.data,msgFrame.data,PACKET_SIZE);
	bool wrongPacket = false;//control bool
	//loop for resending if not received
		do 
	{//waits for server first packet reply for action selected with servers packet number
		ibytessent = sendto(s,(char*)&msgFrame,sizeof(msgFrame),0,(sockaddr *)&sa_in, sizeof(sa_in));//send client_aeq
		if (ibytessent == SOCKET_ERROR)
			throw "Send failed\n";

		//log the send
		string pack_num_string1;         
		ostringstream convert2;  
		convert2 << msgFrame.client_number;
		pack_num_string1 = convert2.str(); 
		fprintf(logFile,strcat(log_msg_sending,pack_num_string1.c_str()));
	    fprintf(logFile,"\n");


		struct timeval tp;
		tp.tv_usec =300000;
		tp.tv_sec=0;
		FD_ZERO(&readfds); //initialize 
		FD_SET(s, &readfds); //put the socket in the set
		if((RetVal = select (1 , &readfds, NULL, NULL, &tp))==SOCKET_ERROR) 
		 {exit(EXIT_FAILURE);}
		else if(RetVal > 0)
		{
			if((ibytesrecv = recvfrom(s,(char*) &msgFrame,sizeof(msgFrame),0, (SOCKADDR *) & SenderAddr, &SenderAddrSize)) == SOCKET_ERROR)
				throw "Receive failed\n";//waiting for server seq

			ostringstream convert3;
			convert3 << msgFrame.server_number;
			string pack_num_string2 = convert3.str(); 
			fprintf(logFile,strcat(log_msg_receiving,pack_num_string2.c_str()));
			fprintf(logFile,"\n");
			
			if((msgFrame.server_seqNo<0))
			{	//reverse msgFrame by deep copy and discard packet
				msgFrame.client_number=hldFrame.client_number;msgFrame.server_number=hldFrame.server_number;msgFrame.client_seqNo=hldFrame.client_seqNo;msgFrame.server_seqNo=hldFrame.server_seqNo;
				sprintf(msgFrame.command,hldFrame.command);sprintf(msgFrame.file_name,hldFrame.file_name);msgFrame.file_size=hldFrame.file_size;
				memcpy(msgFrame.data,hldFrame.data,PACKET_SIZE);
				wrongPacket = true; // set the wrongPacket as true
			}
			else
				wrongPacket = false;
		}
	}while (RetVal ==0 ||wrongPacket);	

	fprintf(logFile,log_msg_3wayhandshake_finished);
	fclose(logFile);
}


//Stop and Wait functions
 void sendNwait()
{//sends and waits for reply. If time out resends
	FILE * logFile;
	char name [100];
	logFile = fopen ("log.txt","a");

	msgFrame.server_seqNo = (msgFrame.server_seqNo+1) %2;//increase server seqNo
	//making a deep copy of original values in case of resending
	hldFrame.client_number = msgFrame.client_number; hldFrame.server_number = msgFrame.server_number; hldFrame.client_seqNo = msgFrame.client_seqNo;hldFrame.server_seqNo=msgFrame.server_seqNo;
	sprintf(hldFrame.command,msgFrame.command);sprintf(hldFrame.file_name,msgFrame.file_name);hldFrame.file_size=msgFrame.file_size;
	memcpy(hldFrame.data,msgFrame.data,PACKET_SIZE);
	bool wrongPacket = false;//control bool
	do
	{//waits and resends if it times out.  Also resends if same packet is received
		ibytessent = sendto(s,(char*)&msgFrame,sizeof(msgFrame),0,(sockaddr *)&sa_in, sizeof(sa_in));
		if (ibytessent == SOCKET_ERROR)
			throw "Send failed\n";

		char log_msg_sending[128] = "Client: sent packet: ";
		char log_msg_receiving[128] = "Client: received ACK for packet: ";

		string pack_num_string;         
		ostringstream convert; 
		convert << msgFrame.client_seqNo;
		pack_num_string = convert.str(); 
		fprintf(logFile,strcat(log_msg_sending,pack_num_string.c_str()));
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
			if(hldFrame.client_seqNo == msgFrame.client_seqNo)
			{
				//reverse msgFrame by deep copy ready to send again
				msgFrame.client_number=hldFrame.client_number;msgFrame.server_number=hldFrame.server_number;msgFrame.client_seqNo=hldFrame.client_seqNo;msgFrame.server_seqNo=hldFrame.server_seqNo;
				sprintf(msgFrame.command,hldFrame.command);sprintf(msgFrame.file_name,hldFrame.file_name);msgFrame.file_size=hldFrame.file_size;
				memcpy(msgFrame.data,hldFrame.data,PACKET_SIZE);
				wrongPacket = true; // set the wrongPacket as true
			}
			else
				wrongPacket = false;
			
			string pack_num_string;         
			ostringstream convert;   
			convert << msgFrame.server_seqNo;
			pack_num_string = convert.str(); 
			fprintf(logFile,strcat(log_msg_receiving,pack_num_string.c_str()));
			fprintf(logFile,"\n");
		}	
	}while (RetVal ==0 || wrongPacket);

	fclose(logFile);
}


