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
#include <windows.h>
#include "Headers\dirent.h"
#include <sys/types.h>

using namespace std;

//port data types

#define REQUEST_PORT 0x7070

int port = REQUEST_PORT;

//socket data types
SOCKET s;

SOCKET s1;
SOCKADDR_IN sa;      // filled by bind
SOCKADDR_IN sa1;     // fill with server info, IP, port
union {
	struct sockaddr generic;
	struct sockaddr_in ca_in;
}ca;

int calen = sizeof(ca);

//buffer data types
char szbuffer[128];

char *buffer;
int ibufferlen;
int ibytesrecv;

int ibytessent;

//host data types
char localhost[11];

HOSTENT *hp;

//wait variables
int nsa1;
int r, infds = 1, outfds = 0;
struct timeval timeout;
const struct timeval *tp = &timeout;

fd_set readfds;

//others
HANDLE test;
DWORD dwtest;

//menu choice
int choice;
#define BUFFER_SIZE 128

#include <fstream>

//filename
char filename[30];

//FTP Messages
#define CON "CON"
#define GET "GET"
#define PUT "PUT"
#define DISC "DISC"
#define OK "OK"
#define LIST "LIST"

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

// Send Message
void sendMessage(char msg[])
{
	sprintf(szbuffer, msg);
	ibytessent = 0;
	ibufferlen = strlen(szbuffer);
	if ((ibytessent = send(s1, szbuffer, ibufferlen, 0)) == SOCKET_ERROR)
		throw "error in send in server program\n";
}

// Print Menu
void printMenu()
{
	sendMessage("Please Select a Number From the Menu:\r\n(1)Get File     (2)Put File     (3)List Files     (4)Exit FTP\r\n");
}

// Receive Message
void receiveMessage()
{
	// reset buffer
	memset(szbuffer, 0, sizeof szbuffer);

	if ((ibytesrecv = recv(s1, szbuffer, 128, 0)) == SOCKET_ERROR)
		throw "Receive error in server program\n";
}

// List of Files
void getList()
{
	DIR *mydir = opendir("./Files/");

	struct dirent *entry = NULL;
	char files[128] = "Here is the List of Files : \r\n";

	while ((entry = readdir(mydir)))
	{
		strcat(entry->d_name, "\r\n");
		strcat(files, entry->d_name);
	}

	sendMessage(files);

	closedir(mydir);
}

void getFile()
{
	sendMessage("GET");
	receiveMessage();
	sendMessage("[GET] Filename.ext: ");
	receiveMessage();

	sprintf(filename, szbuffer);
	char filepath[30] = "Files/";
	strcat(filepath, filename);

	ifstream filedata;
	filebuf *pbuf;
	int filesize;

	try {

		// Open the file
		filedata.open(filepath);

		if (filedata.is_open()){

			// Get pointer to file buffer and determine file size
			pbuf = filedata.rdbuf();
			filesize = pbuf->pubseekoff(0, ios::end, ios::in);
			pbuf->pubseekpos(0, ios::in);

			cout << "File size: " << filesize << endl;

			int count = 0;
			// Loop through the file and stream in chunks based on the buffer size
			while (!filedata.eof()){
				memset(szbuffer, 0, sizeof szbuffer);
				filedata.read(szbuffer, BUFFER_SIZE - 1);
				ibufferlen = strlen(szbuffer);
				count += ibufferlen;
				cout << "Sent " << count << " bytes" << endl;
				sendMessage(szbuffer);
			}
			filedata.close();
		}
		else{
			cout << "File does not exist, sending decline" << endl;
		}
		// Print out any errors
	}
	catch (const char* str){
		cerr << str << WSAGetLastError() << endl;
	}
	memset(szbuffer, 0, BUFFER_SIZE); // zero the buffer
	sendMessage(OK);
}

// Menu Choices Select
void menuSelect()
{
	// Convert selection to integer
	sscanf(szbuffer, "%d", &choice);

	switch (choice)
	{
	case 1:
		cout << "one" << endl;
		getFile();
		break;
	case 2:
		cout << "two" << endl;
		sendMessage("Put File was Done Successfully.\r\n");
		break;
	case 3:
		sendMessage("LIST");
		getList();
		break;
	case 4:
		sendMessage("DISC");
		cout << "Exiting Client" << endl;
		break;
	}
}

int main(void){

	WSADATA wsadata;

	try{
		if (WSAStartup(0x0202, &wsadata) != 0){
			cout << "Error in starting WSAStartup()\n";
		}
		else{
			buffer = "WSAStartup was suuccessful\n";
			WriteFile(test, buffer, sizeof(buffer), &dwtest, NULL);

			/* display the wsadata structure */
			cout << endl
				<< "wsadata.wVersion " << wsadata.wVersion << endl
				<< "wsadata.wHighVersion " << wsadata.wHighVersion << endl
				<< "wsadata.szDescription " << wsadata.szDescription << endl
				<< "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
				<< "wsadata.iMaxSockets " << wsadata.iMaxSockets << endl
				<< "wsadata.iMaxUdpDg " << wsadata.iMaxUdpDg << endl;
		}

		//Display info of local host
		gethostname(localhost, 10);
		cout << "hostname: " << localhost << endl;

		if ((hp = gethostbyname(localhost)) == NULL) {
			cout << "gethostbyname() cannot get local host info?"
				<< WSAGetLastError() << endl;
			exit(1);
		}

		//Create the server socket
		if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
			throw "can't initialize socket";
		// For UDP protocol replace SOCK_STREAM with SOCK_DGRAM 


		//Fill-in Server Port and Address info.
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		sa.sin_addr.s_addr = htonl(INADDR_ANY);


		//Bind the server port

		if (bind(s, (LPSOCKADDR)&sa, sizeof(sa)) == SOCKET_ERROR)
			throw "can't bind the socket";
		cout << "Bind was successful" << endl;

		//Successfull bind, now listen for client requests.

		if (listen(s, 10) == SOCKET_ERROR)
			throw "couldn't  set up listen on socket";
		else cout << "Listen was successful" << endl;

		FD_ZERO(&readfds);

		//wait loop
		while (1)
		{

			FD_SET(s, &readfds);  //always check the listener

			if (!(outfds = select(infds, &readfds, NULL, NULL, tp))) {}

			else if (outfds == SOCKET_ERROR) throw "failure in Select";

			else if (FD_ISSET(s, &readfds))  cout << "got a connection request" << endl;

			//Found a connection request, try to accept. 

			if ((s1 = accept(s, &ca.generic, &calen)) == INVALID_SOCKET)
				throw "Couldn't accept connection\n";

			//Connection request accepted.
			cout << "accepted connection from " << inet_ntoa(ca.ca_in.sin_addr) << ":"
				<< hex << htons(ca.ca_in.sin_port) << endl;

			//Send the menu to the client
			sendMessage(CON);
			printMenu();
			sendMessage(OK);
			
			//Receive response
			receiveMessage();

			// reset choice
			choice = 0;

			while (choice != 4)
			{
				receiveMessage();
				//Print reciept of successful message. 
				cout << "This is message from client: " << szbuffer << endl;
				//Select From Menu
				menuSelect();
			}

		}//wait loop

	} //try loop

	//Display needed error message.

	catch (char* str) { cerr << str << WSAGetLastError() << endl; }

	//close Client socket
	closesocket(s1);

	//close server socket
	closesocket(s);

	/* When done uninstall winsock.dll (WSACleanup()) and exit */
	WSACleanup();
	return 0;
}



