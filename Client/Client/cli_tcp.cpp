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
#include <string>

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
#define BUFFER_SIZE 128
char szbuffer[BUFFER_SIZE];
char *buffer;
int ibufferlen = 0;
int ibytessent;
int ibytesrecv = 0;

//host data types
HOSTENT *hp;
HOSTENT *rp;

char localhost[11],
remotehost[11];

//filename
char filename[30];

//other
HANDLE test;
DWORD dwtest;

//choice
int choice;
char ch[128];

//FTP Messages
#define CON "CON"
#define DONE "DONE"
#define FILE "FILE"
#define GET "GET"
#define PUT "PUT"
#define DISC "DISC"
#define LIST "LIST"
#define OK "OK"
#define ERR "ERR"
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

/*  returns 1 iff str ends with suffix  */
int str_ends_with(const char * str, const char * suffix) {

	if (str == NULL || suffix == NULL)
		return 0;

	size_t str_len = strlen(str);
	size_t suffix_len = strlen(suffix);

	if (suffix_len > str_len)
		return 0;

	return 0 == strncmp(str + str_len - suffix_len, suffix, suffix_len);
}

// Send Command
void sendMessage(char msg[])
{
	//memset(szbuffer, 0, sizeof szbuffer);
	sprintf(szbuffer, msg);
	ibytessent = 0;
	ibufferlen = strlen(szbuffer);
	ibytessent = send(s, szbuffer, ibufferlen, 0);
	if (ibytessent == SOCKET_ERROR)
		throw "Send failed\n";
}

void getCommand()
{
	std::cout << "Your Command: ";
	cin >> ch;
	sendMessage(ch);
}

// Compare Two Strings (1 if they are equal)
int strcmp(char *s1, char *s2)
{
	int i;
	for (i = 0; s1[i] == s2[i]; i++)
		if (s1[i] == '\0')
			return 1;
	return s1[i] - s2[i];
}

void setHost()
{
	//Ask for name of remote server
	std::cout << "Please enter your remote server name: " << flush;
	cin >> remotehost;
	std::cout << "Remote host name is: \"" << remotehost << "\"" << endl;

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
	std::cout << "Connecting to remote host:";
	std::cout << inet_ntoa(sa_in.sin_addr) << endl;

	//Connect Client to the server
	if (connect(s, (LPSOCKADDR)&sa_in, sizeof(sa_in)) == SOCKET_ERROR)
		throw "connect failed\n";
}

void receiveMessage()
{
	// reset buffer
	memset(szbuffer, 0, sizeof szbuffer);

	if ((ibytesrecv = recv(s, szbuffer, BUFFER_SIZE, 0)) == SOCKET_ERROR)
		throw "Receive failed\n";
}

void createFile(char file[],char msg[])
{
	ofstream myfile;
	char path[40] = "Files/";
	strcat(path, file);
	myfile.open(path, ios::binary);
	myfile << msg;
	myfile.close();
}

// check if file exist
bool fileExist(const char *file)
{
	std::ifstream infile(file);
	return infile.good();
}

void remove(char *pString, char letter)
{
	// ok we're mixing C and C++ here - it's legal a little ugly but... 
	char * temp = new char[strlen(pString)];
	char * sourceStr = pString;
	char * destStr = temp;

	while (*sourceStr != 0)
	{
		if (*sourceStr != letter)
		{
			*destStr = *sourceStr;
			destStr++;
			*destStr = 0; // always keep it terminated 
		}
		sourceStr++;
	}
	// done 
	strcpy(pString, destStr);
	delete destStr;
}

void getFile()
{
	// send request for the file
	sendMessage(FILE);

	// receive the file that server request
	receiveMessage();
	std::cout << szbuffer;

	// input the text file
	memset(ch, 0, sizeof ch);
	cin >> ch;
	sendMessage(ch);

	char filename[40] = "Files/";
	strcat(filename, szbuffer);

	char *buffer;
	int ibufferlen = 0;
	int ibytessent;
	int ibytesrecv = 0;

	//host data types
	HOSTENT *hp;
	HOSTENT *rp;

	// Perform a get request
	// Parse response and filesize from server
	const char * response = new char[2];
	int filesize;

	try {
		//wait for reception of server response.
		ibytesrecv = 0;
		if ((ibytesrecv = recv(s, szbuffer, BUFFER_SIZE, 0)) == SOCKET_ERROR)
			throw "Receive failed\n";

		cout << "Server responded with " << szbuffer << endl;

		sscanf(szbuffer, "%s %d", response, &filesize);
		cout << "Response " << response << " filesize " << filesize << endl;

		// Ensure the response from the socket is OK
		if (!strcmp((const char*)response, OK)){

			// Send ack to start data transfer
			memset(szbuffer, 0, BUFFER_SIZE);
			sprintf(szbuffer, SEND);
			ibufferlen = strlen(szbuffer);

			sendMessage(szbuffer);

			// Intermediary buffer for formatting incoming data
			int count = 0;

			ofstream output_file;
			output_file.open(filename, ios::binary | ios::out);

			if (output_file.is_open()){

				// Read data from the server until we have received the file
				while (count < filesize){
					if ((ibytesrecv = recv(s, szbuffer, BUFFER_SIZE, 0)) == SOCKET_ERROR)
						throw "Receive failed\n";

					output_file.write(szbuffer, sizeof(szbuffer));

					count += sizeof(szbuffer);

					cout << "Received " << count << " bytes" << endl;
					// Sanitize buffer
					memset(szbuffer, 0, BUFFER_SIZE);
				}

				// Close our output file
				output_file.close();
			}
			memset(szbuffer, 0, BUFFER_SIZE);
		}
		else{
			// Make a note that the file does not exist
			cout << "Requested file does not exist" << endl;
		}
	}
	catch (const char* str){
		cerr << str << WSAGetLastError() << endl;
	}

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

void putFile()
{
	std::cout << "[PUT] Filename.ext: ";
	cin >> ch;

	sprintf(filename, ch);
	char filepath[30] = "Files/";
	strcat(filepath, filename);

	ifstream filedata;
	filebuf *pbuf;
	int filesize;

	// send the file name
	sendMessage(ch);
	receiveMessage();

	try {

		// Open the file
		filedata.open(filepath);

		if (filedata.is_open()){

			// Get pointer to file buffer and determine file size
			pbuf = filedata.rdbuf();
			filesize = pbuf->pubseekoff(0, ios::end, ios::in);
			pbuf->pubseekpos(0, ios::in);

			std::string s = std::to_string(filesize);
			char const *psize = s.c_str();

			// send the size of the file
			sendMessage((char *)psize);
			receiveMessage();

			std::string sa = std::to_string(BUFFER_SIZE);
			char const *pasize = sa.c_str();

			// send the buffer size
			sendMessage((char *)pasize);
			receiveMessage();

			int count = 0;

			// Loop through the file and stream in chunks based on the buffer size
			while (!filedata.eof()){
				memset(szbuffer, 0, sizeof szbuffer);
				filedata.read(szbuffer, BUFFER_SIZE - 1);
				ibufferlen = strlen(szbuffer);
				count += ibufferlen;
				sendMessage(szbuffer);
			}
			filedata.close();

			// check if exists
			receiveMessage();
			while (strcmp((char const*)szbuffer, "GOOD"))
			{
				memset(szbuffer, 0, sizeof szbuffer);
				std::cout << "File Name Exists, New Name File: ";
				cin >> ch;
				sendMessage(ch);
				receiveMessage();
			}
		}
		else{
			std::cout << "File does not exist, sending decline" << endl;
			sendMessage(ERR);
		}
		// Print out any errors
	}
	catch (const char* str){
		cerr << str << WSAGetLastError() << endl;
	}
	memset(szbuffer, 0, BUFFER_SIZE); // zero the buffer
	sendMessage(OK);

	receiveMessage();
	if (!strcmp((char const*)szbuffer, DONE))
	{
		std::cout << "File has been uploaded successfully." << endl;
	}
	memset(szbuffer, 0, sizeof szbuffer);
}

void delFile()
{
	std::cout << "[DEL] Filename.ext: ";
	cin >> ch;

	char* an = new char[1];

	std::cout << "Are you sure that you want to delete " << ch << " ? (Y)es or (N)o : ";
	cin >> an;
	
	if (!strcmp((char const*)an, "y"))
	{
		sendMessage(ch);
		receiveMessage();

		if (strcmp((char const*)szbuffer, ERR))
		{
			std::cout << "Filename " << ch << " has been deleted." << endl;
		}
		else{
			std::cout << "Filename " << ch << " does not exist." << endl;
		}
	}
	else if (!strcmp((char const*)an, "n"))
	{
		std::cout << "Delete Canceled." << endl;
	}
	else{
		std::cout << "Invalid Command." << endl;
	}

	sendMessage(OK);
}

void menuSelect()
{
	if (!strcmp((char const*)szbuffer, DISC)){
		std::cout << endl;
		sprintf(ch, DISC);
		sendMessage("4");
	}
	else if (!strcmp((char const*)szbuffer, PUT)){
		putFile();
	}
	else if (!strcmp((char const*)szbuffer, DEL)){
		delFile();
	}
	else if (!strcmp((char const*)szbuffer, GET)){
		getFile();
	}
	else if (!strcmp((char const*)szbuffer, CON)){
		if ((ibytesrecv = recv(s, szbuffer, 128, 0)) == SOCKET_ERROR)
			throw "Receive failed\n";
		else
			std::cout << szbuffer;
	}
	else if (!strcmp((char const*)szbuffer, LIST)){
		//sendMessage(LIST);
		memset(szbuffer, 0, sizeof szbuffer);
		receiveMessage();
		std::cout << szbuffer;
	}
	else{
		getCommand();
	}
}

int main(void){

	WSADATA wsadata;

	try {

		if (WSAStartup(0x0202, &wsadata) != 0){
			std::cout << "Error in starting WSAStartup()" << endl;
		}
		else {
			buffer = "WSAStartup was successful\n";
			WriteFile(test, buffer, sizeof(buffer), &dwtest, NULL);

			/* Display the wsadata structure */
			std::cout << endl
				<< "wsadata.wVersion " << wsadata.wVersion << endl
				<< "wsadata.wHighVersion " << wsadata.wHighVersion << endl
				<< "wsadata.szDescription " << wsadata.szDescription << endl
				<< "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
				<< "wsadata.iMaxSockets " << wsadata.iMaxSockets << endl
				<< "wsadata.iMaxUdpDg " << wsadata.iMaxUdpDg << endl;
		}


		//Display name of local host.
		gethostname(localhost, 10);
		std::cout << "Local host name is \"" << localhost << "\"" << endl;

		if ((hp = gethostbyname(localhost)) == NULL)
			throw "gethostbyname failed\n";

		while (1)
		{
			setHost();
			setSocket();
			setConnection();
			setHandShake();
			choice = 0;
			memset(ch, 0, sizeof ch);

			while (strcmp((char const*)ch, DISC))
			{
				/* Have an open connection, so, server is
				- waiting for the client request message
				- don't forget to append <carriage return>
				- <line feed> characters after the send buffer to indicate end-of file */

				//wait for reception of server response.
				ibytesrecv = 0;
				memset(szbuffer, 0, sizeof szbuffer);

				do{
					receiveMessage();
					menuSelect();
				} while (strcmp((char const*)ch, DISC));

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




