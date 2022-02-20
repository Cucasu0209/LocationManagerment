#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>

#include <iostream>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include "Connection.h"
#include "StringService.h"
#include "server-constants.h"

#pragma comment(lib, "Ws2_32.lib")


#define SPLIT_DELIMITER "\##"
#define ENDING_DELIMITER "\r\n"
#define PORT 6600
#define BUFF_SIZE 2048
#define SERVER_ADDR "127.0.0.1"

int Receive(SOCKET, char *, int, int);
int Send(SOCKET, char *, int, int);
void handleMessage(SOCKET, char *, int);
void handleLogin(SOCKET, char *);
void handleRegister(SOCKET, char *);
void handleListCategory(SOCKET, char *);
void handleListPlace(SOCKET, char *);
void handleUpdatePlace(SOCKET, char *);
void handleDeletePlace(SOCKET, char *);
void handleSharePlace(SOCKET, char *);
void handleAddPlace(SOCKET, char *);
void handleAddCategory(SOCKET, char *);
void handleGetStatusAccount(SOCKET, char *);



int main(int argc, char* argv[])
{
	getDbConnection();
	//handleListCategory(NULL, "LISTCA\##tokengenerate");
	//handleLogin(NULL, "LOGIN\r\nthangtv\r\n1");
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
	serverAddr.sin_port = htons(atoi(argv[1]));
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
			//need to change
			continue;
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
			else {
				for (i = 1; i < WSA_MAXIMUM_WAIT_EVENTS; i++)
					if (socks[i] == 0) {
						socks[i] = connSock;
						events[i] = WSACreateEvent();
						WSAEventSelect(socks[i], events[i], FD_READ | FD_CLOSE);
						nEvents++;
						break;
					}
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
				handleMessage(socks[index], recvBuff, ret);
				
				//reset event
				WSAResetEvent(events[index]);
			}
		}

		if (sockEvent.lNetworkEvents & FD_CLOSE) {
			if (sockEvent.iErrorCode[FD_CLOSE_BIT] != 0) {
				printf("FD_CLOSE failed with error %d\n", sockEvent.iErrorCode[FD_CLOSE_BIT]);
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

/*
* @function handleLogin : process message type "LOGIN"
* @param s : client socket
* @param buff: pointer request message 
*
*/
void handleLogin(SOCKET s, char * buff) {
	char * requestStr = (char *)malloc(sizeof(char) * BUFF_SIZE);
	strcpy(requestStr, buff);
	char * typeReq = strtok(requestStr, SPLIT_DELIMITER);
	char * username = strtok(NULL, SPLIT_DELIMITER);
	char * password = strtok(NULL, SPLIT_DELIMITER);
	printf("Handle request LOGIN: account=%s  password=%s\n", username, password);
	char sql[BUFF_SIZE];
	snprintf(sql, sizeof(sql), "SELECT token from User where username='%s' and password='%s'"
								, username, password);
	printf("Query: %s\n", sql);
	//query DB
	sql::Statement *stmt;
	sql::ResultSet *res;
	sql::Connection * con = getDbConnection();
	stmt = con->createStatement();
	try {
		res = stmt->executeQuery(sql);
	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: " << e.what() << std::endl;
		Send(s, LOGIN_NOK, strlen(LOGIN_NOK), 0);
		return;
	}
	char response[BUFF_SIZE];
	bool isSuccess = false;

	while (res->next()) {
		isSuccess = true;
		// send response with success code
		snprintf(response, sizeof(response), "%s%s%s"
			, LOGIN_OK,SPLIT_DELIMITER,res->getString("token").c_str());
	}
	if (isSuccess){
		Send(s, response, strlen(response), 0);

	}
	else {
		Send(s, LOGIN_NOK, strlen(LOGIN_NOK), 0);
	}
	delete res;
	delete stmt;
}

/*
* @function handleRegister : process message type "REGISTER"
* @param s : client socket
* @param buff: pointer request message
*
*/
void handleRegister(SOCKET s, char * buff) {
	char * requestStr = (char *)malloc(sizeof(char) * BUFF_SIZE);
	strcpy(requestStr, buff);
	char * typeReq = strtok(requestStr, SPLIT_DELIMITER);
	char * username = strtok(NULL, SPLIT_DELIMITER);
	char * password = strtok(NULL, SPLIT_DELIMITER);
	char * token = generateString(username);
	
	printf("Handle request Register: account=%s  password=%s\n", username, password);
	char sql[BUFF_SIZE];
	snprintf(sql, sizeof(sql), "INSERT INTO `user`(username, password, token) VALUES('%s', '%s', '%s')"
		, username, password, token);
	printf("Query: %s\n", sql);
	//query DB
	sql::Statement *stmt;
	sql::Connection * con = getDbConnection();
	stmt = con->createStatement();
	try {
		bool res = stmt->execute(sql);
		Send(s, REGIS_OK, strlen(REGIS_OK), 0);
	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: " << e.what() << std::endl;
		Send(s, REGIS_NOK, strlen(REGIS_NOK), 0);
	}

	
	delete stmt;
}
/*
* @function handleListCategory : process message type "LISTCA" (list category)
* @param s : client socket
* @param buff: pointer request message
*
*/
void handleListCategory(SOCKET s, char * buff) {
	char * requestStr = (char *)malloc(sizeof(char) * BUFF_SIZE);
	strcpy(requestStr, buff);
	char * typeReq = strtok(requestStr, SPLIT_DELIMITER);
	char * token = strtok(NULL, SPLIT_DELIMITER);
	printf("Handle request LIST_CATEGORY: token=%s\n", token);
	char sql[BUFF_SIZE];
	snprintf(sql, sizeof(sql), 
		"(select c.* from user u, category c where(c.user_create = u.username and u.token = '%s')) \
			UNION (select * from category ca where  ca.user_create is null) order by categoryid"
		, token);
	printf("Query: %s\n", sql);
	//query DB
	sql::Statement *stmt;
	sql::ResultSet *res;
	sql::Connection * con = getDbConnection();
	stmt = con->createStatement();
	try {
		res = stmt->executeQuery(sql);
	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: " << e.what() << std::endl;
		Send(s, LISTCA_NOK, strlen(LISTCA_NOK), 0);
		return;
	}
	char response[BUFF_SIZE];
	strcpy(response, LISTCA_OK);
	bool isSuccess = false;
	while (res->next()) {
		isSuccess = true;
		char buildCategoryStr[BUFF_SIZE*2];
		// send response with success code
		snprintf(buildCategoryStr, sizeof(buildCategoryStr), "%s%s%s%s"
			, SPLIT_DELIMITER, res->getString("categoryid").c_str(), SPLIT_DELIMITER, res->getString("name").c_str());
		strcat(response, buildCategoryStr);
	}
	if (isSuccess) {
		Send(s, response, strlen(response), 0);

	}
	else {
		Send(s, LISTCA_NOK, strlen(LISTCA_NOK), 0);
	}
	delete res;
	delete stmt;
}
/*
* @function handleLogin : process message type "LISTPL" (list place)
* @param s : client socket
* @param buff: pointer request message
*
*/
void handleListPlace(SOCKET s, char * buff) {
	char * requestStr = (char *)malloc(sizeof(char) * BUFF_SIZE);
	strcpy(requestStr, buff);
	char * typeReq = strtok(requestStr, SPLIT_DELIMITER);
	int categoryId = atoi(strtok(NULL, SPLIT_DELIMITER));
	char * token = strtok(NULL, SPLIT_DELIMITER);
	char sql[BUFF_SIZE];
	if (categoryId == 0) {
		printf("Handle request LIST_PLACE user be shared: token=%s\n", token);
		snprintf(sql, sizeof(sql),
			"select  p.placeid, p.name , up.user_share, up.categoryid from userplace up, `user` u, place p\
		where u.token = '%s' and p.placeid = up.placeid and u.username = up.username and up.categoryid=0 \
		order by p.placeid"
			, token);
	}
	else {
		printf("Handle request LIST_PLACE: categoryid=%d token=%s\n",categoryId, token);
		snprintf(sql, sizeof(sql),
			"select p.placeid, p.name, c.name, up.user_share from userplace up, `user` u, place p, category c \
		where u.token = '%s' and up.categoryid = c.categoryid and p.placeid = up.placeid \
		and u.username = up.username and up.categoryid=%d \
		order by p.placeid"
			, token, categoryId);
	}
	printf("Query: %s\n", sql);
	//query DB
	sql::Statement *stmt;
	sql::ResultSet *res;
	sql::Connection * con = getDbConnection();
	stmt = con->createStatement();
	try {
		res = stmt->executeQuery(sql);
	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: " << e.what() << std::endl;
		Send(s, LISTPL_NOK, strlen(LISTPL_NOK), 0);
		return;
	}
	char response[BUFF_SIZE];
	strcpy(response, LISTPL_OK);
	bool isSuccess = false;
	while (res->next()) {
		isSuccess = true;
		char buildCategoryStr[BUFF_SIZE * 2];
		// add space to end of format
		snprintf(buildCategoryStr, sizeof(buildCategoryStr), "%s%s%s%s%s%s%s%s "
			, SPLIT_DELIMITER, res->getString(1).c_str(), SPLIT_DELIMITER, res->getString(2).c_str(),
			SPLIT_DELIMITER, res->getString(3).c_str(), SPLIT_DELIMITER, res->getString(4).c_str());
		strcat(response, buildCategoryStr);
	}
	if (isSuccess) {
		Send(s, response, strlen(response), 0);

	}
	else {
		Send(s, LISTPL_NOK, strlen(LISTPL_NOK), 0);
	}
	delete res;
	delete stmt;
}
/*
* @function handleUpdatePlace : process message type "UPDATEPL"( update place)
* @param s : client socket
* @param buff: pointer request message
*
*/
void handleUpdatePlace(SOCKET s, char * buff) {
	char * requestStr = (char *)malloc(sizeof(char) * BUFF_SIZE);
	strcpy(requestStr, buff);
	char * typeReq = strtok(requestStr, SPLIT_DELIMITER);
	char * placeid = strtok(NULL, SPLIT_DELIMITER);
	char * placename = strtok(NULL, SPLIT_DELIMITER);
	char * categoryID = strtok(NULL, SPLIT_DELIMITER);
	char * token = strtok(NULL, SPLIT_DELIMITER);

	printf("Handle request UPDATE_PLACE: Placeid=%s  Placename=%s\  CategoryID=%s  token=%sn ", placeid, placename, categoryID, token);
	char sql[BUFF_SIZE];
	//step1: check perrmission to update name of place
	if (strcmp(placename, " ") != 0) {
		snprintf(sql, sizeof(sql), "select count(*) as count from userplace up, place p, `user` u \
		where up.username = u.username and u.token = '%s' \
		and up.placeid = p.placeid and up.user_share ='' and p.placeid = %d"
			, token, atoi(placeid));
		printf("Query: %s\n", sql);
		//query DB
		sql::Statement *stmt;
		sql::Connection * con = getDbConnection();
		stmt = con->createStatement();
		try {
			sql::ResultSet * res = stmt->executeQuery(sql);
			while (res->next()) {
				int count_valid = res->getInt(1);
				if (count_valid == 0) {
					Send(s, UPDATEPL_NON_PERSSION, strlen(UPDATEPL_NON_PERSSION), 0);
					return;
				}
			}
			//update place name after check permission
			snprintf(sql, sizeof(sql), "update place set name = '%s' where placeid = %d"
				, placename, atoi(placeid));
			printf("Query: %s\n", sql);
			try {
				bool res = stmt->execute(sql);
			}
			catch (sql::SQLException &e) {
				std::cout << "# ERR: " << e.what() << std::endl;
				Send(s, UPDATEPL_NOK, strlen(UPDATEPL_NOK), 0);
				return;
			}
			delete stmt;
		}
		catch (sql::SQLException &e) {
			std::cout << "# ERR: " << e.what() << std::endl;
			Send(s, UPDATEPL_NOK, strlen(UPDATEPL_NOK), 0);
			return;
		}
	}
	
	//step2: update category
	snprintf(sql, sizeof(sql), "update userplace set categoryid=%d \
	where placeid=%d and username = (select username from user where user.token = '%s')"
		, atoi(categoryID), atoi(placeid), token);
	printf("Query: %s\n", sql);
	//query DB
	sql::Statement *stmt;
	sql::Connection * con = getDbConnection();
	stmt = con->createStatement();
	try {
		bool res = stmt->execute(sql);
		Send(s, UPDATEPL_OK, strlen(UPDATEPL_OK), 0);
	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: " << e.what() << std::endl;
		Send(s, UPDATEPL_NOK, strlen(UPDATEPL_NOK), 0);
	}
	delete stmt;

	
}
/*
* @function handleDeletePlace : process message type "DELETEPL"( delete place)
* @param s : client socket
* @param buff: pointer request message
*
*/
void handleDeletePlace(SOCKET s, char * buff) {
	char * requestStr = (char *)malloc(sizeof(char) * BUFF_SIZE);
	strcpy(requestStr, buff);
	char * typeReq = strtok(requestStr, SPLIT_DELIMITER);
	char * placeid = strtok(NULL, SPLIT_DELIMITER);
	char * token = strtok(NULL, SPLIT_DELIMITER);

	printf("Handle request DELETE_PLACE: placeid=%s  token=%s\n", placeid, token);
	char sql[BUFF_SIZE];
	snprintf(sql, sizeof(sql), "delete from userplace where  placeid=%d \
					and username = (select username from `user` where user.token = '%s')"
		, atoi(placeid), token);
	printf("Query: %s\n", sql);
	//query DB
	sql::Statement *stmt;
	sql::Connection * con = getDbConnection();
	stmt = con->createStatement();
	try {
		bool res = stmt->execute(sql);
		Send(s, DELETEPL_OK, strlen(DELETEPL_OK), 0);
	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: " << e.what() << std::endl;
		Send(s, DELETEPL_NOK, strlen(DELETEPL_NOK), 0);
	}


	delete stmt;
}
/*
* @function handleSharePlace : process message type "SHAREPL"( share place)
* @param s : client socket
* @param buff: pointer request message
*
*/
void handleSharePlace(SOCKET s, char * buff) {
	char * requestStr = (char *)malloc(sizeof(char) * BUFF_SIZE);
	strcpy(requestStr, buff);
	char * typeReq = strtok(requestStr, SPLIT_DELIMITER);
	char * placeid = strtok(NULL, SPLIT_DELIMITER);
	char * user_be_shared = strtok(NULL, SPLIT_DELIMITER);
	char * token = strtok(NULL, SPLIT_DELIMITER);

	printf("Handle request SHARE_PLACE: placeid=%s user_share=%s token=%s\n", placeid, user_be_shared, token);
	char sql[BUFF_SIZE];
	snprintf(sql, sizeof(sql), "INSERT INTO userplace(username, placeid, categoryid, user_share) \
		VALUES('%s', %d, 0, (select username from `user` where user.token = '%s'))"
		, user_be_shared,  atoi(placeid), token);
	printf("Query: %s\n", sql);
	//query DB
	sql::Statement *stmt;
	sql::Connection * con = getDbConnection();
	stmt = con->createStatement();
	try {
		bool res = stmt->execute(sql);
		Send(s, SHAREPL_OK, strlen(SHAREPL_OK), 0);
	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: " << e.what() << std::endl;
		Send(s, SHAREPL_NOK, strlen(SHAREPL_NOK), 0);
	}


	delete stmt;
}

/*
* @function handleAddPlace : process message type "ADDPL"( add place)
* @param s : client socket
* @param buff: pointer request message
*
*/
void handleAddPlace(SOCKET s, char * buff) {
	char * requestStr = (char *)malloc(sizeof(char) * BUFF_SIZE);
	strcpy(requestStr, buff);
	char * typeReq = strtok(requestStr, SPLIT_DELIMITER);
	char * place_name = strtok(NULL, SPLIT_DELIMITER);
	char * category_id = strtok(NULL, SPLIT_DELIMITER);
	char * token = strtok(NULL, SPLIT_DELIMITER);

	printf("Handle request ADD_PLACE: place_name=%s category_id=%s token=%s\n", place_name, category_id, token);
	char sql[BUFF_SIZE];
	snprintf(sql, sizeof(sql), "INSERT INTO place(name) VALUES('%s')"
		, place_name);
	printf("Query: %s\n", sql);
	//query DB
	sql::Statement *stmt;
	sql::Connection * con = getDbConnection();
	stmt = con->createStatement();
	try {
		bool res = stmt->execute(sql);
		snprintf(sql, sizeof(sql), "INSERT INTO userplace(username, placeid, categoryid, user_share) \
			VALUES((select username from `user` where user.token = '%s'), (SELECT LAST_INSERT_ID()), %d, '');"
			, token, atoi(category_id));
		printf("Query: %s\n", sql);
		res = stmt->execute(sql);
		Send(s, ADDPL_OK, strlen(ADDPL_OK), 0);
	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: " << e.what() << std::endl;
		Send(s, ADDPL_NOK, strlen(ADDPL_NOK), 0);
	}


	delete stmt;
}
/*
* @function handleAddCategory : process message type "ADDCA" (add category)
* @param s : client socket
* @param buff: pointer request message
*
*/
void handleAddCategory(SOCKET s, char * buff) {
	char * requestStr = (char *)malloc(sizeof(char) * BUFF_SIZE);
	strcpy(requestStr, buff);
	char * typeReq = strtok(requestStr, SPLIT_DELIMITER);
	char * category_name = strtok(NULL, SPLIT_DELIMITER);
	char * token = strtok(NULL, SPLIT_DELIMITER);

	printf("Handle request ADD_CATEGORY: category_name=%s  token=%s\n", category_name, token);
	char sql[BUFF_SIZE];
	snprintf(sql, sizeof(sql), "INSERT INTO category(name, user_create) \
		VALUES('%s', (select username from `user` where user.token = '%s'))"
		, category_name, token);
	printf("Query: %s\n", sql);
	//query DB
	sql::Statement *stmt;
	sql::Connection * con = getDbConnection();
	stmt = con->createStatement();
	try {
		bool res = stmt->execute(sql);
		Send(s, CREATECA_OK, strlen(CREATECA_OK), 0);
	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: " << e.what() << std::endl;
		Send(s, CREATECA_NOK, strlen(CREATECA_NOK), 0);
	}


	delete stmt;
}
/*
* @function handleGetStatusAccount : process message type "STATUS" (status share locations)
* @param s : client socket
* @param buff: pointer request message
*
*/
void handleGetStatusAccount(SOCKET s, char * buff) {
	char * requestStr = (char *)malloc(sizeof(char) * BUFF_SIZE);
	strcpy(requestStr, buff);
	char * typeReq = strtok(requestStr, SPLIT_DELIMITER);
	char * token = strtok(NULL, SPLIT_DELIMITER);
	printf("Handle request STATUS: token=%s\n", token);
	char sql[BUFF_SIZE];
	snprintf(sql, sizeof(sql),
		"select  count(*) as count from userplace up, `user` u\
		where u.token = '%s'  and u.username = up.username and up.categoryid=0"
		, token);
	printf("Query: %s\n", sql);
	//query DB
	sql::Statement *stmt;
	sql::ResultSet *res;
	sql::Connection * con = getDbConnection();
	stmt = con->createStatement();
	try {
		res = stmt->executeQuery(sql);
	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: " << e.what() << std::endl;
		Send(s, STATUS_NOK,strlen(STATUS_NOK), 0);
		return;
	}
	char response[BUFF_SIZE];
	bool isSuccess = false;
	while (res->next()) {
		isSuccess = true;
		// send response with success code
		snprintf(response, sizeof(response), "%s%s%s"
			,STATUS_OK, SPLIT_DELIMITER, res->getString("count").c_str());
	}
	if (isSuccess) {
		Send(s, response, strlen(response), 0);

	}
	else {
		Send(s, STATUS_NOK, strlen(STATUS_NOK), 0);
	}
	delete res;
	delete stmt;
}
/*
* @function handleMessage : main process classify request type
* @param s : client socket
* @param recvBuff: pointer request message
* @param recv_size: length of recvBuff
*
*/
void handleMessage(SOCKET s, char * recvBuff, int recv_size){
	char * tmpSplitFunc = (char*)malloc(recv_size * sizeof(char));
	strcpy(tmpSplitFunc, recvBuff);
	char * typeReq = strtok(tmpSplitFunc, SPLIT_DELIMITER);
	if (strcmp(typeReq, "LOGIN") == 0) {
		handleLogin(s, recvBuff);
	}
	else if (strcmp(typeReq, "REGISTER") == 0) {
		handleRegister(s, recvBuff);
	}
	else if (strcmp(typeReq, "SHAREPL") == 0) {
		handleSharePlace(s, recvBuff);
	}
	else if (strcmp(typeReq, "SAVEPL") == 0) {
		handleAddPlace(s, recvBuff);
	}
	else if (strcmp(typeReq, "UPDATEPL") == 0) {
		handleUpdatePlace(s, recvBuff);
	}
	else if (strcmp(typeReq, "DELETEPL") == 0) {
		handleDeletePlace(s, recvBuff);
	}
	else if (strcmp(typeReq, "LISTPL") == 0) {
		handleListPlace(s, recvBuff);
	}
	else if (strcmp(typeReq, "LISTCA") == 0) {
		handleListCategory(s, recvBuff);
	}
	else if (strcmp(typeReq, "CREATECA") == 0) {
		handleAddCategory(s, recvBuff);
	}
	else if (strcmp(typeReq, "STATUS") == 0) {
		handleGetStatusAccount(s, recvBuff);
	}
}
