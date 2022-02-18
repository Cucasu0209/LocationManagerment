#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#pragma comment(lib, "Ws2_32.lib")

#define ENDING_DELIMITER "\r\n"
#define PORT 6600
#define BUFF_SIZE 2048
#define SERVER_ADDR "127.0.0.1"

int Receive(SOCKET, char *, int, int);
int Send(SOCKET, char *, int, int);
char* getSubStr(char *, int , int );
void handleLogin(char *);
sql::Connection * getDbConnection();



int main(int argc, char* argv[])
{
	handleLogin("LOGIN\r\nthang\r\nthangtv32");
	DWORD		nEvents = 0;
	DWORD		index;
	SOCKET		socks[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT	events[WSA_MAXIMUM_WAIT_EVENTS];
	WSANETWORKEVENTS sockEvent;

	//Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported\n");
		return 0;
	}

	//Step 2: Construct LISTEN socket	
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	socks[0] = listenSock;
	events[0] = WSACreateEvent(); //create new events
	nEvents++;

	// Associate event types FD_ACCEPT and FD_CLOSE
	// with the listening socket and newEvent   
	WSAEventSelect(socks[0], events[0], FD_ACCEPT | FD_CLOSE);


	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error %d: Cannot associate a local address with server socket.", WSAGetLastError());
		return 0;
	}

	//Step 4: Listen request from client
	if (listen(listenSock, 10)) {
		printf("Error %d: Cannot place server socket in state LISTEN.", WSAGetLastError());
		return 0;
	}

	printf("Server started!\n");

	char sendBuff[BUFF_SIZE], recvBuff[BUFF_SIZE];
	SOCKET connSock;
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	int ret, i;

	for (i = 1; i < WSA_MAXIMUM_WAIT_EVENTS; i++) {
		socks[i] = 0;
	}
	while (1) {
		//wait for network events on all socket
		index = WSAWaitForMultipleEvents(nEvents, events, FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED) {
			printf("Error %d: WSAWaitForMultipleEvents() failed\n", WSAGetLastError());
			break;
		}

		index = index - WSA_WAIT_EVENT_0;
		WSAEnumNetworkEvents(socks[index], events[index], &sockEvent);

		if (sockEvent.lNetworkEvents & FD_ACCEPT) {
			if (sockEvent.iErrorCode[FD_ACCEPT_BIT] != 0) {
				printf("FD_ACCEPT failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
				break;
			}

			if ((connSock = accept(socks[index], (sockaddr *)&clientAddr, &clientAddrLen)) == SOCKET_ERROR) {
				printf("Error %d: Cannot permit incoming connection.\n", WSAGetLastError());
				break;
			}

			//Add new socket into socks array
			int i;
			if (nEvents == WSA_MAXIMUM_WAIT_EVENTS) {
				printf("\nToo many clients.");
				closesocket(connSock);
			}
			else
				for (i = 1; i < WSA_MAXIMUM_WAIT_EVENTS; i++)
					if (socks[i] == 0) {
						socks[i] = connSock;
						events[i] = WSACreateEvent();
						WSAEventSelect(socks[i], events[i], FD_READ | FD_CLOSE);
						nEvents++;
						break;
					}

			//reset event
			WSAResetEvent(events[index]);
		}

		if (sockEvent.lNetworkEvents & FD_READ) {
			//Receive message from client
			if (sockEvent.iErrorCode[FD_READ_BIT] != 0) {
				printf("FD_READ failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
				break;
			}

			ret = Receive(socks[index], recvBuff, BUFF_SIZE, 0);

			//Release socket and event if an error occurs
			if (ret <= 0) {
				closesocket(socks[index]);
				socks[index] = 0;
				WSACloseEvent(events[index]);
				nEvents--;
			}
			else {
				//echo to client
				recvBuff[ret] = 0;
				char * typeReq = strtok(recvBuff, ENDING_DELIMITER);
				if (strcmp(typeReq, "LOGIN") == 0) {
					handleLogin(recvBuff);
				}
				else if (strcmp(typeReq, "REGISTER") == 0) {

				}
				else if (strcmp(typeReq, "LISTFR") == 0) {

				}
				else if (strcmp(typeReq, "SAVEPL") == 0) {

				}
				else if (strcmp(typeReq, "UPDATEPL") == 0) {

				}
				else if (strcmp(typeReq, "DELETEPL") == 0) {

				}
				else if (strcmp(typeReq, "LISTPL") == 0) {

				}
				else if (strcmp(typeReq, "LISTCA") == 0) {

				}
				else if (strcmp(typeReq, "CREATECA") == 0) {

				}
				else if (strcmp(typeReq, "SHARE") == 0) {

				}
				Send(socks[index], recvBuff, strlen(recvBuff), 0);

				//reset event
				WSAResetEvent(events[index]);
			}
		}

		if (sockEvent.lNetworkEvents & FD_CLOSE) {
			if (sockEvent.iErrorCode[FD_CLOSE_BIT] != 0) {
				printf("FD_CLOSE failed with error %d\n", sockEvent.iErrorCode[FD_CLOSE_BIT]);
				break;
			}
			//Release socket and event
			closesocket(socks[index]);
			socks[index] = 0;
			WSACloseEvent(events[index]);
			nEvents--;
		}
	}
	return 0;
}

/* The recv() wrapper function */
int Receive(SOCKET s, char *buff, int size, int flags) {
	int n;
	n = recv(s, buff, size, flags);
	buff[n] = 0;
	printf("Receive data %s\n", buff);
	if (n == SOCKET_ERROR)
		printf("Error %d: Cannot receive data.\n", WSAGetLastError());
	else if (n == 0)
		printf("Client disconnects.\n");
	return n;
}

/* The send() wrapper function*/
int Send(SOCKET s, char *buff, int size, int flags) {
	int n;
	printf("Send data %s\n", buff);
	n = send(s, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error %d: Cannot send data.\n", WSAGetLastError());

	return n;
}

char* getSubStr(char * buff, int from, int length) {
	char* subbuff = (char *)malloc((length + 1) * sizeof(char));
	memcpy(subbuff, &buff[from], length);
	subbuff[length] = '\0';
	return subbuff;
}

sql::Connection * getDbConnection() {
	sql::Driver *driver;
	sql::Connection *con;
	/* Create a connection */
	driver = get_driver_instance();
	con = driver->connect("tcp://127.0.0.1:3306", "root", "root");
	/* Connect to the MySQL database */
	con->setSchema("quickstartdb");
	return con;
}
void handleLogin(char * buff) {
	char * requestStr = (char *)malloc(sizeof(char) * BUFF_SIZE);
	strcpy(requestStr, buff);
	char * typeReq = strtok(requestStr, ENDING_DELIMITER);
	char * username = strtok(NULL, "\r\n");
	char * password = strtok(NULL, "\r\n");
	printf("Handle request LOGIN: %s    %s", username, password);


	//query DB
	sql::Statement *stmt;
	sql::ResultSet *res;
	sql::Connection * con = getDbConnection();
	stmt = con->createStatement();
	res = stmt->executeQuery("SELECT 'Hello World!' AS _message");
	while (res->next()) {
		printf("MySQl reply .... \n");
		/* Access column data by alias or column name */
		printf("Account logined: %s", res->getString("_message"));
	}
	delete res;
	delete stmt;
	delete con;

	// xư lý response code và send lại cho client
}