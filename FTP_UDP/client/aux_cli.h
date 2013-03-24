
#ifndef AUX_CLI_H
#define AUX_CLI_H

#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>

#include <winsock.h>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <string.h>
#include <fstream>
#include <windows.h>
//user defined port number
#define REQUEST_PORT 7000
//#define packet size
#define PACKET_SIZE 1500
#define STOPNWAIT 1 //the window sequence size

int packet_amount;

int port=REQUEST_PORT;
fd_set readfds; //fd_set is a type

//socket data types
SOCKET s;
SOCKADDR_IN sa,sa3;         // filled by bind
SOCKADDR_IN sa_in;      // fill with server info, IP, port
sockaddr_in RecvAddr; //used for udp sendto

sockaddr_in SenderAddr; //used for udp recvfrom
int SenderAddrSize = sizeof (SenderAddr);

//handshake frame
struct THREE_WAY_HS{
	int client_number;
	int server_number;
	char command[20];
	char file_name[128];
}hsFrame;

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



//buffer data types
char szbuffer[PACKET_SIZE];

char *buffer;

int ibufferlen=0;

int ibytessent;
int ibytesrecv=0;
int RetVal;//used to hold select return value


//host data types
HOSTENT *hp;
HOSTENT *rp;

char localhost[11],
     remotehost[11];


//other

HANDLE test;

DWORD dwtest;


//Functions

 void sendNwait(MESSAGE_FRAME &buff);//for stop and wait protocol	

 void hndShake();	// for 3 way handshake

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




#endif

