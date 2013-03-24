#ifndef SER_TCP_H
#define SER_TCP_H
#define HOSTNAME_LENGTH 20
#define RESP_LENGTH 40
#define FILENAME_LENGTH 20
#define REQUEST_PORT 5001
#define BUFFER_LENGTH 1024 
#define MAXPENDING 10
#define MSGHDRSIZE 8 //Message Header Size


typedef enum{
	REQ_SIZE=1,REQ_TIME,RESP //Message type
} Type;

typedef struct  
{
	char hostname[HOSTNAME_LENGTH];
	char filename[FILENAME_LENGTH];
} Req;  //request

typedef struct  
{
	char response[RESP_LENGTH];
} Resp; //response


typedef struct 
{
	Type type;
	int  length; //length of effective bytes in the buffer
	char buffer[BUFFER_LENGTH];
} Msg; //message format used for sending and receiving


class TcpServer
{
	int serverSock,clientSock;     /* Socket descriptor for server and client*/
	struct sockaddr_in ClientAddr; /* Client address */
	struct sockaddr_in ServerAddr; /* Server address */
	unsigned short ServerPort;     /* Server port */
	int clientLen;            /* Length of Server address data structure */
	char servername[HOSTNAME_LENGTH];

public:
		TcpServer();
		~TcpServer();
		void TcpServer::start();
};

class TcpThread :public Thread
{

	int CLIENT_SOCKET;
	int SERVER_SOCKET;
public: 
	TcpThread(int clientsocket, int serversocket):CLIENT_SOCKET(clientsocket),SERVER_SOCKET(serversocket)
	{}
	virtual void run();
    int msg_recv(int ,Msg * );
	int msg_send(int ,Msg * );
	unsigned long ResolveName(char name[]);
    static void err_sys(char * fmt,...);
};

#endif