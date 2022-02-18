#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include "winsock2.h"
#include "ws2tcpip.h"
#include "string"
#include <windows.h>

#define BUFF_SIZE  2048
#define SPLIT_DELIMITER "\##"
#define ENDING_DELIMITER "\r\n"
#pragma comment (lib, "Ws2_32.lib")
using namespace std;

int sendMessage(char* msg);
int receiveMessage(char* buff);
char* getSubStr(char *, int, int);
char * formatStr(char*);
void showLogoutScene();
void showRegisterScene();

void showLoginScene();
struct Location {
	char placeID[BUFF_SIZE];
	char placeName[BUFF_SIZE];
	char categoryID[BUFF_SIZE];
	char categoryName[BUFF_SIZE];
};


SOCKET client;
bool isLogin = false;
char username[BUFF_SIZE];
char secretKey[BUFF_SIZE];


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

bool registerUser(char* username, char* password) {
	return username[0] == 'q';
}

bool saveLocation(char* location, char* categoryId) {
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

void showAllLocationByType(int categoryid, bool isShareType=false){
	char request[BUFF_SIZE];
	if (isShareType) {
		printf("danh sach dia diem chua gan category \n");
		snprintf(request, sizeof(request), "LISTPL%s%c%s%s"
			, SPLIT_DELIMITER, '0', SPLIT_DELIMITER,secretKey);
	}
	else {
		printf("danh sach dia diem categoryid=%d \n", categoryid);
		snprintf(request, sizeof(request), "LISTPL%s%d%s%s"
			, SPLIT_DELIMITER, categoryid, SPLIT_DELIMITER, secretKey);
	}

	int ret = sendMessage(request);
	if (ret == SOCKET_ERROR)
		printf("Error %d\n", WSAGetLastError());
	char response[BUFF_SIZE];
	ret = receiveMessage(response);
	printf("response from server %s", response);
}
void showNotifyLocationScene(){
	showAllLocationByType(0, true);
}

int showCategorySelector() {
	char request[BUFF_SIZE];
	snprintf(request, sizeof(request), "LISTCA%s%s"
		, SPLIT_DELIMITER, secretKey);
	int ret = sendMessage(request);
	if (ret == SOCKET_ERROR)
		printf("Error %d\n", WSAGetLastError());
	char response[BUFF_SIZE];
	ret = receiveMessage(response);
	if (ret > 0) {
		printf("Reponse from server: %s \n", response);
	}
	char * typeReq = strtok(response, SPLIT_DELIMITER);
	if (strcmp(typeReq, "800") == 0) {
		printf("\n");
		printf("Please Select one category.\n");
		printf("CATEGORY_ID\t NAME\n");
		char * value = strtok(NULL, SPLIT_DELIMITER);
		while (value != NULL) {
			printf("%s\t\t%s\n", value, strtok(NULL, SPLIT_DELIMITER));
			value = strtok(NULL, SPLIT_DELIMITER);
		}
		printf("==================================================\n");
		printf("You Select 1 category: ");
		while (true) {
			int in = getInputOption();
			if (in < 0) {
				printf("\n-LOG: Wrong. Select Again: ");
				continue;
			}
			else if (in == 0) {
				return -1;
			}
			else {
				return in;
			}
		}

	}
	else if (strcmp(typeReq, "801") == 0) {
		printf("No Category found!\n");
	}
	return -1;
	
}

void showAddNewLocationScene() {
	printf("\n");
	char _locationName[BUFF_SIZE], _categoryID[BUFF_SIZE];
	printf("Location Name: ");
	gets_s(_locationName, BUFF_SIZE);

	/*char* selector = showCategorySelector();
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
	}*/

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
		/*	selector = showCategorySelector();
			copyCharArray(_newCategoryID, selector, BUFF_SIZE);
			copyCharArray(_categoryID, _newCategoryID, BUFF_SIZE)*/;
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
	printf("================    CATEGORY LIST      ================\n");
	printf("0. Back.\n");
	int input = showCategorySelector();
	if (input == -1) {
		return;
	}
	else {
		showAllLocationByType(input);
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
	printf("1. You has X locations share don't has category!\n");
	printf("2. Add new Location\n");
	printf("3. See all Location\n");
	printf("4. Log out\n");
	printf("================================================================\n");
	printf("You Select: ");
	while (true) {
		int in = getInputOption();
		switch (in)
		{
		case 1:
			showNotifyLocationScene();
			return;
		case 2:
			showAddNewLocationScene();
			return;
		case 3:
			showLocationsScene();
			return;
		case 4:
			showLogoutScene();
			return;
		default:
			printf("-LOG: Wrong. Select Again: ");
			break;
		}
	}
}



//MAIN
int main(int argc, char* argv[])
{
	isLogin = true;
	strcpy(secretKey, "tokengenerate");
	// TODO tranthang2404: delete after none debug
	Sleep(500);
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


//SOCKET
int sendMessage(char* msg) {
	char sendBuff[BUFF_SIZE];
	int i;
	for (i = 0; msg[i] != 0; i++) {
		sendBuff[i] = msg[i];
	}
	/*sendBuff[i] = '\r';
	sendBuff[i+1] = '\n';*/
	return send(client, sendBuff, i, 0);
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
	}
	return ret;
}

char* getSubStr(char * buff, int from, int length) {
	char* subbuff = (char *)malloc((length + 1) * sizeof(char));
	memcpy(subbuff, &buff[from], length);
	subbuff[length] = '\0';
	return subbuff;
}

char * formatStr(char* input) {
	int lenStr = 0;
	for (int i = 0; i < strlen(input); i++) {
		if (input[i] == '\0') {
			lenStr = i;
		}
	}
	char * formatedStr = getSubStr(input, 0, lenStr);
	return formatedStr;
}

void showLogoutScene() {
	printf("Are you sure to Logout?\n");
	if (yesNoQuestion()) {
		printf("-LOG: Log out Success.\n");
		isLogin = false;
		strcpy(secretKey, "");
	}
}
void showLoginScene() {
	char _userName[BUFF_SIZE], _password[BUFF_SIZE];
	printf("\nUsername: ");
	gets_s(_userName, BUFF_SIZE);
	printf("\nPassword: ");
	gets_s(_password, BUFF_SIZE);
	char request[BUFF_SIZE];
	snprintf(request, sizeof(request), "LOGIN%s%s%s%s"
		, SPLIT_DELIMITER, _userName, SPLIT_DELIMITER, _password);
	int ret = sendMessage(request);
	if (ret == SOCKET_ERROR)
		printf("Error %d\n", WSAGetLastError());
	char response[BUFF_SIZE];
	ret = receiveMessage(response);
	if (ret > 0) {
		printf("Reponse from server: %s \n", response);
	}
	char * typeReq = strtok(response, SPLIT_DELIMITER);
	if (strcmp(typeReq, "100") == 0) {
		char * token = strtok(NULL, SPLIT_DELIMITER);
		isLogin = true;
		strcpy(secretKey, token);
	}
	else if (strcmp(typeReq, "101") == 0) {
		isLogin = false;
		printf("Login failed, account or password not correct! \n");
	}
}


void showRegisterScene() {
	printf("\n");
	char _userName[BUFF_SIZE], _password1[BUFF_SIZE], _password2[BUFF_SIZE];
	printf("Username: ");
	gets_s(_userName, BUFF_SIZE);
	printf("Password: ");
	gets_s(_password1, BUFF_SIZE);
	printf("Password Again: ");
	gets_s(_password2, BUFF_SIZE);
	if (strcmp(_password1, _password2) == 0) {
		char request[BUFF_SIZE];
		snprintf(request, sizeof(request), "REGISTER%s%s%s%s"
			, SPLIT_DELIMITER, _userName, SPLIT_DELIMITER, _password1);
		int ret = sendMessage(request);
		if (ret == SOCKET_ERROR)
			printf("Error %d\n", WSAGetLastError());
		char response[BUFF_SIZE];
		ret = receiveMessage(response);
		if (ret > 0) {
			printf("Reponse from server: %s \n", response);
		}
		char * typeReq = strtok(response, SPLIT_DELIMITER);
		if (strcmp(typeReq, "200") == 0) {
			printf("Register success!\n");
		}
		else if (strcmp(typeReq, "201") == 0) {
			printf("User already exists!\n");
		}
	}
	else {
		printf("Password did not match: Please try again... \n");
		showRegisterScene();
	}

}