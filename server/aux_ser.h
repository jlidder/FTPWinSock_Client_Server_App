



#ifndef AUX_SER_H

#define AUX_SER_H
   #pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <winsock.h>
#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <fstream>

//port data types

#define REQUEST_PORT 5001
//#define REQUEST_PORT 0x7070 
#define PACKET_SIZE 1500
int port=REQUEST_PORT;
#define STOPNWAIT 1 //the window sequence size

//socket data types
SOCKET s;
SOCKADDR_IN sa;      // filled by bind
SOCKADDR_IN sa1;     // fill with server info, IP, port recvfrom
SOCKADDR_IN SenderAddr;
int SenderAddrSize = sizeof (sa1);//size of address for udp recvfrom
sockaddr_in RecvAddr;//udp sendto

//hold data to be sent
struct HolderFrame{
	int client_number;
	int server_number;
	char command[20];
	int client_seqNo;
	int server_seqNo;
	char file_name[128];
	int file_size;
	char data[PACKET_SIZE];//or error message
}hldFrame;

//data frames
struct MESSAGE_FRAME {
	int client_number;
	int server_number;
	char command[20];
	int client_seqNo;
	int server_seqNo;
	char file_name[128];
	int file_size;
	char data[PACKET_SIZE];//or error message
} msgFrame;


union {struct sockaddr generic;
	struct sockaddr_in ca_in;}ca;

	int calen=sizeof(ca); 

	//buffer data types
	char szbuffer[PACKET_SIZE];

	char *buffer;
	int ibufferlen;
	int ibytesrecv;

	int ibytessent;
	int RetVal;//used to hold select return value

	//host data types
	char localhost[11];

	HOSTENT *hp;

	//wait variables
	int nsa1;
	int r,infds=1, outfds=0;
	
	fd_set readfds;

	//others
	HANDLE test;

	DWORD dwtest;

//functions
void sendNwait();//for stop and wait protocol
void hndShake(); //for 3 way handshake












#endif
