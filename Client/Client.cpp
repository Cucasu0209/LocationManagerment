

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "winsock2.h"
#include "ws2tcpip.h"
#include "string"
#include <windows.h>

#define BUFF_SIZE  2048
#pragma comment (lib, "Ws2_32.lib")
using namespace std;

//MODEL
struct Location {
	char placeID[BUFF_SIZE];
	char placeName[BUFF_SIZE];
	char categoryID[BUFF_SIZE];
	char categoryName[BUFF_SIZE];
};


SOCKET client;
bool isLogin = false;
char username[BUFF_SIZE];


//CONTROLLER
void copyCharArray(char *a, char *b, int size) {
	for (int i = 0; i < size; i++) {
		a[i] = b[i];
	}
}

int convertCharArrayToInt(char *a) {
	int res = 0;
	for (int i = 0; a[i] != 0; i++) {
		if (a[i] >= 48 && a[i] <= 57)res = res * 10 + a[i] - 48;
		else
		{
			return -1;
		}
	}
	return res;
}

int getInputOption() {
	char key[BUFF_SIZE];
	gets_s(key, BUFF_SIZE);
	return convertCharArrayToInt(key);
}

bool loginUser(char* username, char* password) {
	return username[0] == 'q';
}

bool registerUser(char* username, char* password) {
	return username[0] == 'q';
}

bool saveLocation(char* location, char* categoryId) {
	return true;
}

bool logoutUser() {
	return true;
}

bool updateLocation(char* locationID, char* locationName, char* categoryId) {
	return true;
}

bool deleteLocation(char* locationID) {
	return true;
}

bool saveLocationShared(char* locationID) {
	return true;
}

//VIEW
bool yesNoQuestion() {
	printf("1. Yes         2. No\n");
	printf("You select: ");
	while (true) {
		int sel = getInputOption();
		if (sel == 1)return true;
		if (sel == 2)return false;
		printf("Select again: ");
	}
	return true;

}

void showLoginScene() {
	printf("\n");
	char _userName[BUFF_SIZE], _password[BUFF_SIZE];
	printf("Username: ");
	gets_s(_userName, BUFF_SIZE);
	printf("Password: ");
	gets_s(_password, BUFF_SIZE);
	if (loginUser(_userName, _password)) {
		copyCharArray(username, _userName, BUFF_SIZE);
		printf("-LOG: Login Success.\n");
		isLogin = true;
	}
	else {
		printf("-LOG: Login Fail.\n");
	}

}

void showRegisterScene() {
	printf("\n");
	char _userName[BUFF_SIZE], _password[BUFF_SIZE];
	printf("Username: ");
	gets_s(_userName, BUFF_SIZE);
	printf("Password: ");
	gets_s(_password, BUFF_SIZE);
	if (registerUser(_userName, _password)) {
		printf("-LOG: Register Success.\n");
	}
	else {
		printf("-LOG: Exist Username.\n");
	}

}

char* showCategorySelector() {
	printf("\n");
	printf("================    OPTION      ================\n");
	printf("Please Select one category.\n");
	printf("1. category 1.\n");
	printf("2. category 1.\n");
	printf("2. category 1.\n");
	printf("2. category 1.\n");
	printf("2. category 1.\n");
	printf("==================================================\n");
	printf("You Select 1 category: ");
	char retu[BUFF_SIZE];
	while (true) {
		int in = getInputOption();
		switch (in)
		{
		case 1:
			return  retu;
		case 2:
			return  retu;
		default:
			printf("-LOG: Wrong. Select Again: ");
			break;
		}
	}

	return retu;
}

void showAddNewLocationScene() {
	printf("\n");
	char _locationName[BUFF_SIZE], _categoryID[BUFF_SIZE];
	printf("Location Name: ");
	gets_s(_locationName, BUFF_SIZE);

	char* selector = showCategorySelector();
	copyCharArray(_categoryID, selector, BUFF_SIZE);

	printf("Do you want to save?.\n");
	bool isSave = yesNoQuestion();
	if (isSave) {
		if (saveLocation(_locationName, _categoryID)) {
			printf("-LOG: Add New Location Success.\n");
		}
		else
		{
			printf("-LOG: Add New Location Fail.\n");
		}
	}

}

void showUpdateLocationDetail(char* locationID) {
	printf("\n");
	char _locationName[BUFF_SIZE], _categoryID[BUFF_SIZE];
	char* selector;
	printf("Update Location Option:\n");

	while (true) {

		printf("1.Location Name      2.Category      3.Save    4.Don't Save\n");
		int in = getInputOption();
		switch (in)
		{
		case 1:
			printf("New Location Name: ");
			char _newLocationName[BUFF_SIZE];
			gets_s(_newLocationName, BUFF_SIZE);
			copyCharArray(_locationName, _newLocationName, BUFF_SIZE);
			break;
		case 2:
			char  _newCategoryID[BUFF_SIZE];
			selector = showCategorySelector();
			copyCharArray(_newCategoryID, selector, BUFF_SIZE);
			copyCharArray(_categoryID, _newCategoryID, BUFF_SIZE);
			break;
		case 3:
			if (updateLocation(locationID, _locationName, _categoryID)) {
				printf("-LOG: Update Location Success.\n");
			}
			else {
				printf("-LOG: Update Location Fail.\n");
			}
			return;
		case 4:
			printf("Don't Save and Back To Home?.\n");
			if (yesNoQuestion()) {
				return;
			}
		default:
			printf("-LOG: Wrong. Select Again: ");
			return;
		}
	}
}

void showLocationDetail(char* locationID) {
	printf("\n");
	printf("Detail balebal..........\n");
	printf("1.Update      2.Delete      3.Cancel\n");
	while (true) {
		int in = getInputOption();
		switch (in)
		{
		case 1:
			showUpdateLocationDetail(locationID);
			return;
		case 2:
			printf("Do you want to delete?\n");
			if (yesNoQuestion()) {
				if (deleteLocation(locationID)) {
					printf("-LOG: Delete Success.\n");
				}
				else {
					printf("-LOG: Delete Fail.\n");
				}
			}
			return;
		case 3:
			return;
		default:
			printf("-LOG: Wrong. Select Again: ");
			break;
		}
	}
}

void showLocationDetailShared(char* locationID) {
	printf("\n");
	printf("Detail Shared balebal..........\n");
	printf("1.Save      2.Delete      3.Cancel\n");
	printf("You Select: ");
	while (true) {
		int in = getInputOption();
		switch (in)
		{
		case 1:
			if (saveLocationShared(locationID)) {

			}
			return;
		case 2:
			printf("Do you want to delete?\n");
			if (yesNoQuestion()) {
				if (deleteLocation(locationID)) {
					printf("-LOG: Delete Success.\n");
				}
				else {
					printf("-LOG: Delete Fail.\n");
				}
			}
			return;
		case 3:
			return;
		default:
			printf("-LOG: Wrong. Select Again: ");
			break;
		}
	}
}

void showLocationsSharedScene() {
	printf("\n");
	printf("================    LOCATION SHARED LIST      ==========\n");
	printf("0. Back.\n");
	printf("1. Location 1.\n");
	printf("2. Location 1.\n");
	printf("2. Location 1.\n");
	printf("2. Location 1.\n");
	printf("2. Location 1.\n");
	printf("========================================================\n");
	printf("You Select: ");
	while (true) {
		int in = getInputOption();
		switch (in)
		{
		case 0:
			return;
		case 1:
			char locationID[BUFF_SIZE];
			showLocationDetailShared(locationID);
			return;
		default:
			printf("-LOG: Wrong. Select Again: ");
			break;
		}
	}
}

void showLocationsScene() {
	printf("\n");
	printf("================    LOCATION LIST      ================\n");
	printf("0. Back.\n");
	printf("1. Your friends shared to you 5 locations.\n");
	printf("2. Location 1.\n");
	printf("2. Location 1.\n");
	printf("2. Location 1.\n");
	printf("2. Location 1.\n");
	printf("2. Location 1.\n");
	printf("========================================================\n");
	printf("You Select: ");
	while (true) {
		int in = getInputOption();
		switch (in)
		{
		case 0:
			return;
		case 1:
			showLocationsSharedScene();
			return;
		case 2:
			char locationID[BUFF_SIZE];
			showLocationDetail(locationID);
			return;
		default:
			printf("-LOG: Wrong. Select Again: ");
			break;
		}
	}
}

void showFirstScene() {
	printf("\n");
	printf("================              MENU              =========\n");
	printf("1. Log in\n");
	printf("2. Sign up\n");
	printf("=========================================================\n");
	printf("You Select: ");
	while (true) {
		int in = getInputOption();
		switch (in)
		{
		case 1:
			showLoginScene();
			return;
		case 2:
			showRegisterScene();
			return;
		default:
			printf("-LOG: Wrong. Select Again: ");
			break;
		}
	}

}

void showHomeScene() {
	printf("\n");
	printf("================       Hello %s        ================\n", username);
	printf("1. Add new Location\n");
	printf("2. See all Location\n");
	printf("3. Log out\n");
	printf("================================================================\n");
	printf("You Select: ");
	while (true) {
		int in = getInputOption();
		switch (in)
		{
		case 1:
			showAddNewLocationScene();
			return;
		case 2:
			showLocationsScene();
			return;
		case 3:
			printf("Are you sure to Logout?\n");
			if (yesNoQuestion()) {
				if (logoutUser()) {
					printf("-LOG: Log out Success.\n");
					isLogin = false;
				}
				else
				{
					printf("-LOG: Log out Fail.\n");
				}
			}

			return;
		default:
			printf("-LOG: Wrong. Select Again: ");
			break;
		}
	}
}

//SOCKET
int sendMessage(char* msg) {
	char sendBuff[BUFF_SIZE];
	int i;
	for(i = 0; msg[i] != 0; i++) {
		sendBuff[i] = msg[i];
	}
	sendBuff[i] = '\r';
	sendBuff[i] = '\n';
	int msgLength = i + 2;
	return send(client, sendBuff, msgLength, 0);
}

int receiveMessage(char* buff) {
	int ret = recv(client, buff, BUFF_SIZE, 0);
	if (ret == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAETIMEDOUT)
			printf("Time out!");
		else printf("Error : %d.Cannot receive data.\n", WSAGetLastError());
	}
	else if (strlen(buff) > 0) {
		buff[ret] = 0;
		printf("%s\n", buff);
	}
	return ret;
}

//MAIN
int main(int argc, char* argv[])
{
	// TODO tranthang2404: delete after none debug
	Sleep(1000);
	//Step 1: Inittiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Version is not supported.\n");
		return 0;
	}

	//Step 2: Construct socket
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		printf("Error %d: cannot create client socket.\n", WSAGetLastError());
		return 0;
	}

	//Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &serverAddr.sin_addr);

	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error: %d. Cannot connect to server\n", WSAGetLastError());
		return 0;
	}
	printf("Connected to server.\n");
	//Step 4: Communicate with server
	while (1) {
		if (!isLogin) {
			showFirstScene();
		}
		else {
			showHomeScene();
		}
	}

	//Step 5: Close socket
	closesocket(client);
	//Step 6: Terminate Winsock
	WSACleanup();
	return 0;
}