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
#include <string>
#include <vector>
#include <cstdio>

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
#define BUFFER_SIZE 128
char szbuffer[BUFFER_SIZE];
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

#include <fstream>

//filename
char filename[30];
char ch[128];

#ifdef USES_REAL_FILE_TYPE
#include <stdio.h>
#else
#define FILE void
#endif

//FTP Messages
#define CON "CON"
#define DONE "DONE"
#define GET "GET"
#define PUT "PUT"
#define DISC "DISC"
#define OK "OK"
#define LIST "LIST"
#define ERR "ERR"
#define FILE "FILE"
#define DEL "DEL"
#define SEND "SEND"

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
	sendMessage("Please Select a Number From the Menu:\r\n(1)Get File     (2)Put File     (3)List Files     (4)Exit FTP	(9)Delete File\r\n");
}

// Receive Message
void receiveMessage()
{
	// reset buffer
	memset(szbuffer, 0, sizeof szbuffer);

	if ((ibytesrecv = recv(s1, szbuffer, BUFFER_SIZE, 0)) == SOCKET_ERROR)
		throw "Receive error in server program\n";
}

// check if file exist
bool fileExist(const char *file)
{
	std::ifstream infile(file);
	return infile.good();
}

// List of Files
void getList()
{
	DIR *mydir = opendir("./Files/");

	struct dirent *entry = NULL;
	char files[1028] = "Here is the List of Files : \r\n";

	while ((entry = readdir(mydir)))
	{
		strcat(entry->d_name, "\r\n");
		strcat(files, entry->d_name);
	}

	sendMessage(files);

	closedir(mydir);

	sendMessage(DONE);
}

void getFile()
{
	sendMessage("GET");
	receiveMessage();
	sendMessage("[GET] Filename.ext: ");
	receiveMessage();

	char filename[30] = "Files/";
	strcat(filename, szbuffer);

	char *buffer;
	int ibufferlen = 0;
	int ibytessent;
	int ibytesrecv = 0;
	//host data types
	HOSTENT *hp;
	HOSTENT *rp;
			
	cout << "Sending file " << filename << endl;

	ifstream filedata;
	filebuf *pbuf;
	int filesize;

	try {

		// Open the file
		filedata.open(filename, ios::binary);

		if (filedata.is_open()){

			// Get pointer to file buffer and determine file size
			ifstream file(filename, ios::binary);
			file.seekg(0, ios::end);
			unsigned int filesize = file.tellg();
			file.close();

			cout << "File size: " << filesize << endl;

			// Send back an OK message to confirm receipt
			memset(szbuffer, 0, BUFFER_SIZE); // zero the buffer
			sprintf(szbuffer, "OK %d", filesize);
			ibufferlen = strlen(szbuffer);

			sendMessage(szbuffer);

			// Wait for confirmation 
			memset(szbuffer, 0, BUFFER_SIZE); // zero the buffer
			sendMessage(szbuffer);

			int count = 0;
			// Loop through the file and stream in chunks based on the buffer size
			while (!filedata.eof()){
				filedata.read(reinterpret_cast<char*>(szbuffer), BUFFER_SIZE);
				//reinterpret_cast<char*>(szbuffer)
				ibufferlen = sizeof(szbuffer);
				count += ibufferlen;
				cout << "Sent " << count << " bytes" << endl;
				if ((ibytessent = send(s1, szbuffer, (BUFFER_SIZE), 0)) == SOCKET_ERROR)
					throw "error in send in server program\n";

				memset(szbuffer, 0, BUFFER_SIZE); // zero the buffer
			}

			filedata.close();
		}
		else{

			cout << "File does not exist, sending decline" << endl;
			// Send back a NO to the client to indicate that the file does not exist
			memset(szbuffer, 0, BUFFER_SIZE); // zero the buffer
			sprintf(szbuffer, "NO -1");
			ibufferlen = strlen(szbuffer);

			if ((ibytessent = send(s1, szbuffer, ibufferlen, 0)) == SOCKET_ERROR)
				throw "error in send in server program\n";
		}
		// Print out any errors
	}
	catch (const char* str){
		cerr << str << WSAGetLastError() << endl;
	}
	memset(szbuffer, 0, BUFFER_SIZE); // zero the buffer
	choice = 0;
}

void createFile(char file[], char msg[])
{
	ofstream myfile;
	char path[40] = "Files/";
	strcat(path, file);
	myfile.open(path);
	myfile << msg;
	myfile.close();
}

void putFile()
{
	// send request for the file
	sendMessage(PUT);

	// receive the file that server request
	receiveMessage();

	char ch[30];
	sprintf(ch, szbuffer);

	sendMessage(OK);

	memset(szbuffer, 0, sizeof szbuffer);

	// get size
	receiveMessage();

	if (strcmp((char const*)szbuffer, ERR))
	{
		int filesize;
		sscanf(szbuffer, "%d", &filesize);

		sendMessage(OK);

		receiveMessage();

		int buffSize = 0;
		sscanf(szbuffer, "%d", &buffSize);

		sendMessage(OK);

		int nrPackages = (int)(ceil((double)filesize / (double)buffSize));

		char* msg = (char*)calloc(filesize, sizeof(char));

		for (int z = 0; z < (nrPackages); z++)
		{
			// receive the message
			receiveMessage();
			strcat(msg, szbuffer);
		}

		char path[40] = "Files/";
		strcat(path, ch);

		// create file if it does not exist
		while (fileExist(path))
		{
			memset(ch, 0, sizeof ch);
			memset(path, 0, sizeof path);
			sendMessage("EXIST");
			receiveMessage();
			sprintf(ch, szbuffer);
			sprintf(path, "Files/");
			strcat(path, szbuffer);
		}

		sendMessage("GOOD");
		// create file
		createFile(ch, msg);

		sendMessage(DONE);
		sendMessage(OK);
	}
	else{
		sendMessage(ERR);
		sendMessage(OK);
	}
	
}

// Delete File
void deleteFile()
{
	// send the DEL request
	sendMessage(DEL);

	receiveMessage();

	char path[40] = "Files/";
	strcat(path, szbuffer);

	if (fileExist(path))
	{
		remove(path);
		sendMessage(OK);
	}
	else
	{ 
		sendMessage(ERR);
	}

	sendMessage(DONE);
}

// Menu Choices Select
void menuSelect()
{
	// Convert selection to integer
	sscanf(szbuffer, "%d", &choice);

	switch (choice)
	{
	case 1:
		getFile();
		break;
	case 2:
		putFile();
		break;
	case 3:
		sendMessage(LIST);
		getList();
		break;
	case 4:
		sendMessage(DISC);
		cout << "Exiting Client" << endl;
		choice = 4;
		break;
	case 5:
		printMenu();
		break;
	case 9:
		deleteFile();
		break;
	default:
		printMenu();
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
				if (strcmp(szbuffer, OK))
				{
					menuSelect();
				}
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



