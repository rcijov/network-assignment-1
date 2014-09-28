// CLIENT TCP PROGRAM
// Revised and tidied up by
// J.W. Atwood
// 1999 June 30

char* getmessage(char *);

/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <winsock.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <fstream>

#include <windows.h>

using namespace std;

//user defined port number
#define REQUEST_PORT 0x7070;
int port = REQUEST_PORT;


//socket data types
SOCKET s;
SOCKADDR_IN sa;         // filled by bind
SOCKADDR_IN sa_in;      // fill with server info, IP, port

//buffer data types
char szbuffer[128];
char *buffer;
int ibufferlen = 0;
int ibytessent;
int ibytesrecv = 0;

//host data types
HOSTENT *hp;
HOSTENT *rp;

char localhost[11],
remotehost[11];


//other
HANDLE test;
DWORD dwtest;

//choice
int choice;
char ch[128];

//FTP Messages
#define CON "CON"
#define FILENAME "FILENAME"
#define GET "GET"
#define PUT "PUT"
#define DISC "DISC"
#define LIST "LIST"
#define OK "OK"

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

// Compare Two Strings - 1 if they are equal
int strcmp(char *s1, char *s2)
{
	int i;
	for (i = 0; s1[i] == s2[i]; i++)
		if (s1[i] == '\0')
			return 1;
	return s1[i] - s2[i];
}

// Send Command
void sendMessage(char msg[])
{
	sprintf(szbuffer, ch);
	ibytessent = 0;
	ibufferlen = strlen(szbuffer);
	ibytessent = send(s, szbuffer, ibufferlen, 0);
	if (ibytessent == SOCKET_ERROR)
		throw "Send failed\n";
}

void setHost()
{
	//Ask for name of remote server
	cout << "Please enter your remote server name: " << flush;
	cin >> remotehost;
	cout << "Remote host name is: \"" << remotehost << "\"" << endl;

	if ((rp = gethostbyname(remotehost)) == NULL)
		throw "remote gethostbyname failed\n";
}

void setSocket()
{
	//Create the socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		throw "Socket failed\n";
	/* For UDP protocol replace SOCK_STREAM with SOCK_DGRAM */
}

void setConnection()
{
	//Specify server address for client to connect to server.
	memset(&sa_in, 0, sizeof(sa_in));
	memcpy(&sa_in.sin_addr, rp->h_addr, rp->h_length);
	sa_in.sin_family = rp->h_addrtype;
	sa_in.sin_port = htons(port);

	//Display the host machine internet address
	cout << "Connecting to remote host:";
	cout << inet_ntoa(sa_in.sin_addr) << endl;

	//Connect Client to the server
	if (connect(s, (LPSOCKADDR)&sa_in, sizeof(sa_in)) == SOCKET_ERROR)
		throw "connect failed\n";
}

void receiveMessage()
{
	// reset buffer
	memset(szbuffer, 0, sizeof szbuffer);

	if ((ibytesrecv = recv(s, szbuffer, 128, 0)) == SOCKET_ERROR)
		throw "Receive failed\n";
}

void createFile(char file[],char msg[])
{
	ofstream myfile;
	char path[40] = "../Files/";
	strcat(path, file);
	myfile.open(path);
	myfile << msg;
	myfile.close();
}

void getFile()
{
	// send request for the file
	sendMessage(FILENAME);

	// receive the file that server request
	receiveMessage();
	cout << szbuffer;

	// input the text file
	memset(ch, 0, sizeof ch);
	cin >> ch;
	sendMessage(ch);

	// receive the message
	receiveMessage();
	cout << szbuffer << endl;

	// create file
	createFile(ch, szbuffer);
}

void setHandShake()
{
	//append client message to szbuffer + send.
	sprintf(szbuffer, "Connect\r\n");
	ibytessent = 0;
	ibufferlen = strlen(szbuffer);
	ibytessent = send(s, szbuffer, ibufferlen, 0);
	if (ibytessent == SOCKET_ERROR)
		throw "Send failed\n";
}

int main(void){

	WSADATA wsadata;

	try {

		if (WSAStartup(0x0202, &wsadata) != 0){
			cout << "Error in starting WSAStartup()" << endl;
		}
		else {
			buffer = "WSAStartup was successful\n";
			WriteFile(test, buffer, sizeof(buffer), &dwtest, NULL);

			/* Display the wsadata structure */
			cout << endl
				<< "wsadata.wVersion " << wsadata.wVersion << endl
				<< "wsadata.wHighVersion " << wsadata.wHighVersion << endl
				<< "wsadata.szDescription " << wsadata.szDescription << endl
				<< "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
				<< "wsadata.iMaxSockets " << wsadata.iMaxSockets << endl
				<< "wsadata.iMaxUdpDg " << wsadata.iMaxUdpDg << endl;
		}


		//Display name of local host.

		gethostname(localhost, 10);
		cout << "Local host name is \"" << localhost << "\"" << endl;

		if ((hp = gethostbyname(localhost)) == NULL)
			throw "gethostbyname failed\n";

		while (1)
		{
			setHost();
			setSocket();
			setConnection();
			setHandShake();
			choice = 0;

			while (1)
			{
				/* Have an open connection, so, server is
				- waiting for the client request message
				- don't forget to append <carriage return>
				- <line feed> characters after the send buffer to indicate end-of file */

				//wait for reception of server response.
				ibytesrecv = 0;
				memset(szbuffer, 0, sizeof szbuffer);

				receiveMessage();

				if (!strcmp((char const*)szbuffer, DISC)){
					cout << endl;
					break;
				}
				else if (!strcmp((char const*)szbuffer, GET)){
					getFile();
				}
				else if (!strcmp((char const*)szbuffer, CON)){
					if ((ibytesrecv = recv(s, szbuffer, 128, 0)) == SOCKET_ERROR)
						throw "Receive failed\n";
					else
						cout << szbuffer;
				}
				else if (!strcmp((char const*)szbuffer, LIST)){
					sendMessage(LIST);
					memset(szbuffer, 0, sizeof szbuffer);
					receiveMessage();
					cout << szbuffer;
				}
				else{
					cout << "Your Command: ";
					cin >> ch;
					sendMessage(ch);
				}

			}
		} // try loop

		//Display any needed error response.
	}

	catch (char *str) { cerr << str << ":" << dec << WSAGetLastError() << endl; }

	//close the client socket
	closesocket(s);

	/* When done uninstall winsock.dll (WSACleanup()) and exit */
	WSACleanup();

	return 0;
}




