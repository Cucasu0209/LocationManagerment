// SelectTCPServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define USER_PATH "account.txt"
#define LOG_PATH "log_20180188.txt"
#include "winsock2.h"
#include "stdlib.h"
#include "ws2tcpip.h"
#include <string>
#include <fstream>
#include <iostream>
#include <conio.h>
#include <map>
#include <time.h>
#pragma comment (lib,"ws2_32.lib")
#pragma warning(disable : 4996)
using namespace std;

#define SERVER_ADDR "127.0.0.2"
#define BUFF_SIZE 2048
#define DELIMITER "\r\n"
//store username status 
map<string, int> userstatus; //0-offline, 1-online

//struct manage login status
struct clientStatus {
	SOCKET connectSock;		// idenfication
	sockaddr_in clientAddr;	//address
	char username[30];		//username
	int status;			//login status
};

clientStatus clientAt[FD_SETSIZE];

/* clearClient function: refresh the client state
* @param cs :Pointer to clientStatus that needs to be refresh
* @return no return value
*/
void clearClient(clientStatus *status) {
	status->connectSock = 0;
	status->username[0] = 0;
	status->clientAddr = {};
	status->status =0;
}


/* checkEmpty function: check for the mess message is empty \r\n
* @param origin: char array needed check
* @return : true/false
*/
bool checkEmpty(char origin[]) {
	for (int i = 0; i < (int)strlen(origin); i++) {
		if (!(origin[i] == '\r' || origin[i + 1] == '\n' || origin[i] == 0)) return false;
	}
	return true;
}

/* setStatus function: set status for login
* @param name: client name
* @param status: client status now
* @return: void
*/
void setStatus(string name, int status) {
	for (map<string, int>::iterator i = userstatus.begin(); i != userstatus.end(); i++) {
		if (name == i->first) {
			i->second = status;
		}
	}
}


/* getMessage function: make up your message
* @param type: string behind
* @param data: string after
* @return: char*
*/
char* getMessage(string type, string data) {
	string rs = type + " " + data;
	int len = rs.length();
	char *c = new char[len + 1];
	for (int i = 0; i < len; i++) {
		c[i] = rs[i];
	}
	c[rs.length()] = '\0';
	return c;
}

/* stringToChar function: make string to char
* @param data: string input
* @return: char*
*/
char* stringToChar(string str) {
	int len = str.length();
	char *c = new char[len + 1];
	for (int i = 0; i < len; i++) {
		c[i] = str[i];
	}
	c[str.length()] = '\0';
	return c;
}

/* checkLogin function: check have user
* @param name: username needed to check
* @return: 1 if have user, 0 if not
*/
int checkHaveUser(string name) {
	for (map<string, int>::iterator i = userstatus.begin(); i != userstatus.end(); i++) {
		//cout << name << " ? = " << i->first << endl;
		name.erase(name.find_last_not_of(" \n\r\t") + 1);
		if (name == i->first) {
			return 1;
		}
	}
	return 0;
}





/* handleMsg function: analysis of received message
* @param in: Pointer to input message
* @param out: Pointer to output message
* out:
* 100: login success
* 101: login fail, user blocked
* 102: login fail, another client login to
* 103: login fail, has logged in yet
* 104: login fail, user not exist
* 200: post success
* 201: post fail ,didn't login
* 300: quit success
* 301: quit fail ,didn't login
* @param index: index of client
* @return: no return value
*/
void handleMsg(char *in, char *msgCode, int index) {
	string type = "";
	for (int i = 0; i < 4; i++) {
		type += in[i];
	}
	string data = in + 5;


	
}

/* recvMessage function: receive message from client
* @param s: socket idenfication
* @param buff: pointer to store output data
* @param size
* @param flags: 0
* @return : true if received data successfully , otherwise false
*/
int recvMessage(SOCKET s, char *buff, int size) {
	int n;
	buff[0] = 0;

	char data[BUFF_SIZE];
	n = recv(s, data, BUFF_SIZE, 0);

	if (n == SOCKET_ERROR) {
		printf("Error : %d.Cannot receive data.\n", WSAGetLastError());
		return -99;
	}
	else {
		data[n] = 0;
		strcat_s(buff, BUFF_SIZE, data);
		return n;
	}
}

/*sendMessage function: send message to client
* @param s: socket idenfication
* @param buff: pointer to store input data
* @param size: size of data
* @return : no return value
*/
void sendMessage(SOCKET s, char *buff, int size) {
	int n;

	char *tmp = buff;
	int lenLeft = size;
	while (lenLeft > 0) {

		n = send(s, tmp, lenLeft, 0);
		if (n == SOCKET_ERROR) {
			printf("Error: %d! Cannot send data.\n", WSAGetLastError());
			break;
		}
		lenLeft -= n;
		tmp += n;
	}
}



/*handleTCPmsgAndSend function: Handle when receive byTCP socket and send
* @param rcvBuff: string recv
* @param size: recv length
* @param client: client socket
* @param index: client at index
* @return : no return value
*/
void handleTCPmsgAndSend(char *rcvBuff, int size, SOCKET client, int index) {
	rcvBuff[size] = 0;
	int clientPort = ntohs(clientAt[index].clientAddr.sin_port);
	string strBuffString = "";
	if (rcvBuff[1] == '\n' && rcvBuff[0] == '\r') {
		char *strBuff = stringToChar("");
		char sendBuff[BUFF_SIZE];
		printf("Recive from %d: %s\n", clientPort, strBuff);
		handleMsg(strBuff, sendBuff, index);


		printf("Send to %d: %s\n", clientPort, sendBuff);
		strcat(sendBuff, DELIMITER);
		Sleep(10);
		sendMessage(client, sendBuff, strlen(sendBuff));
	}
	for (int i = 0; i <= size; i++) {
		if (rcvBuff[i] == '\n' || rcvBuff[i] == '\r' || rcvBuff[i] == '\0') {
			if (rcvBuff[i] == '\r' && rcvBuff[i + 1] == '\n') {
				char *strBuff = stringToChar(strBuffString);
				if (checkEmpty(strBuff))continue;
				strBuffString = "";
				char sendBuff[BUFF_SIZE];
				printf("Recive from %d: %s\n", clientPort, strBuff);
				handleMsg(strBuff, sendBuff, index);


				printf("Send to %d: %s\n", clientPort, sendBuff);
				strcat(sendBuff, DELIMITER);
				sendMessage(client, sendBuff, strlen(sendBuff));
			}

		}
		else {
			strBuffString += rcvBuff[i];
		}
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	//readUserName();
	//Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported\n");

	//Step 2: Construct socket	
	SOCKET listenSock;

	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(_ttoi(argv[1]));
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);


	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error! Cannot bind this address.");
		return 0;
	}

	//Step 4: Listen request from client
	if (listen(listenSock, FD_SETSIZE)) {
		printf("Error! Cannot listen.");
		return 0;
	}

	printf("Server started at %d!\n", _ttoi(argv[1]));


	SOCKET client[FD_SETSIZE], connSock;
	fd_set readfds, initfds; //use initfds to initiate readfds at the begining of every loop step
	sockaddr_in clientAddr;
	int /*ret,*/ nEvents, clientAddrLen;
	char rcvBuff[BUFF_SIZE];

	for (int i = 0; i < FD_SETSIZE; i++)
		client[i] = 0;	// 0 indicates available entry

	FD_ZERO(&initfds);
	FD_SET(listenSock, &initfds);

	//Step 5: Communicate with clients
	while (1) {
		readfds = initfds;		/* structure assignment */
		nEvents = select(0, &readfds, 0, 0, 0);
		if (nEvents < 0) {
			printf("\nError! Cannot poll sockets: %d", WSAGetLastError());
			break;
		}

		//new client connection
		if (FD_ISSET(listenSock, &readfds)) {
			clientAddrLen = sizeof(clientAddr);
			if ((connSock = accept(listenSock, (sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
				printf("\nError! Cannot accept new connection: %d", WSAGetLastError());
				break;
			}
			else {
				printf("You got a connection from %s %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

				int i;
				for (i = 0; i < FD_SETSIZE; i++)
					if (client[i] == 0) {
						client[i] = connSock;
						clientAt[i].connectSock = client[i];
						clientAt[i].clientAddr = clientAddr;
						FD_SET(client[i], &initfds);
						break;
					}

				if (i == FD_SETSIZE) {
					printf("\nToo many clients.\n");
					closesocket(connSock);
				}

				if (--nEvents == 0)
					continue; //no more event
			}
		}

		//receive data from clients
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (client[i] == 0)
				continue;

			if (FD_ISSET(client[i], &readfds)) {
				int rs = recvMessage(client[i], rcvBuff, BUFF_SIZE);
				if (rs < 0) {
					FD_CLR(client[i], &initfds);
					setStatus(clientAt[i].username, 0);
					closesocket(client[i]);
					client[i] = 0;
					printf("%s %d : disconnected.\n", inet_ntoa(clientAt[i].clientAddr.sin_addr), ntohs(clientAt[i].clientAddr.sin_port));

					clientAt[i].status = 0;
					clearClient(&clientAt[i]);
				}
				else {

					handleTCPmsgAndSend(rcvBuff, rs, client[i], i);
				}
			}

			if (--nEvents <= 0)
				continue; //no more event
		}

	}

	closesocket(listenSock);
	WSACleanup();
	return 0;
}

