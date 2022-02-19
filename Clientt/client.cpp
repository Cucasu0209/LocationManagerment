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

SOCKET client;
bool isLogin = false;
char UserName[BUFF_SIZE];
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
void showLocationDetail(int, bool);




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
	printf("================    LOCATION LIST      ================\n");
	printf("0. Back.\n");
	char request[BUFF_SIZE];
	if (isShareType) {
		snprintf(request, sizeof(request), "LISTPL%s%c%s%s"
			, SPLIT_DELIMITER, '0', SPLIT_DELIMITER,secretKey);
	}
	else {
		snprintf(request, sizeof(request), "LISTPL%s%d%s%s"
			, SPLIT_DELIMITER, categoryid, SPLIT_DELIMITER, secretKey);
	}

	int ret = sendMessage(request);
	if (ret == SOCKET_ERROR)
		printf("Error %d\n", WSAGetLastError());
	char response[BUFF_SIZE];
	ret = receiveMessage(response);
	char * typeReq = strtok(response, SPLIT_DELIMITER);
	if (strcmp(typeReq, "700") == 0) {
		if (isShareType) {
			printf("PlaceId\t\tUSERSHARE\t\t\tNAME\n");
			char * place_id = strtok(NULL, SPLIT_DELIMITER);
			while (place_id != NULL) {
				char * place_name = strtok(NULL, SPLIT_DELIMITER);
				char * user_share = strtok(NULL, SPLIT_DELIMITER);
				printf("%-7s\t\t%-20s\t\t%-40s\n", place_id, user_share, place_name);
				//bypass category=0
				strtok(NULL, SPLIT_DELIMITER);
				place_id = strtok(NULL, SPLIT_DELIMITER);
			}
		}
		else {
			printf("PlaceId\t\tUSERSHARE\t\tCATEGORY_NAME\t\t\tPLACE_NAME\n");
			char * place_id = strtok(NULL, SPLIT_DELIMITER);
			while (place_id != NULL) {
				char * place_name = strtok(NULL, SPLIT_DELIMITER);
				char * category_name = strtok(NULL, SPLIT_DELIMITER);
				char * user_share = strtok(NULL, SPLIT_DELIMITER);
				printf("%-7s\t\t%-20s\t%-30s\t%-40s\n", place_id, user_share, category_name, place_name);
				place_id = strtok(NULL, SPLIT_DELIMITER);
			}
		}
		
		printf("==================================================\n");
		printf("You select one location ID: ");
		while (true) {
			int in = getInputOption();
			if (in < 0) {
				printf("\n-LOG: Wrong. Select Again: ");
				continue;
			}
			else if (in == 0) {
				return;
			}
			else {
				showLocationDetail(in, isShareType);
				return;
			}
		}

	}
	else if (strcmp(typeReq, "701") == 0) {
		printf("No location found!\n");
	}
}
void showNotifyLocationScene(){
	showAllLocationByType(0, true);
}

int showCategorySelector() {
	printf("================    CATEGORY LIST      ================\n");
	printf("0. BACK\n");
	char request[BUFF_SIZE];
	snprintf(request, sizeof(request), "LISTCA%s%s"
		, SPLIT_DELIMITER, secretKey);
	int ret = sendMessage(request);
	if (ret == SOCKET_ERROR)
		printf("Error %d\n", WSAGetLastError());
	char response[BUFF_SIZE];
	ret = receiveMessage(response);
	char * typeReq = strtok(response, SPLIT_DELIMITER);
	if (strcmp(typeReq, "800") == 0) {
		printf("\n");
		printf("CATEGORY_ID\t\tNAME\n");
		char * category_id = strtok(NULL, SPLIT_DELIMITER);
		while (category_id != NULL) {
			char * category_name = strtok(NULL, SPLIT_DELIMITER);
			printf("%.11s\t\t%.40s\n", category_id, category_name);
			category_id = strtok(NULL, SPLIT_DELIMITER);
		}
		printf("==================================================\n");
		printf("You Select one category ID: ");
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
	char _locationName[BUFF_SIZE]="";
	printf("Location Name: ");
	gets_s(_locationName, BUFF_SIZE);
	int category_choice = showCategorySelector();
	if (category_choice <= 0 || strcmp(_locationName, "") == 0) {
		printf("Location not empty and categoryId must valid");
		return;
	}
	//build query
	char request[BUFF_SIZE];
	snprintf(request, sizeof(request), "SAVEPL%s%s%s%d%s%s"
		, SPLIT_DELIMITER, _locationName, SPLIT_DELIMITER, category_choice, SPLIT_DELIMITER, secretKey);
	int ret = sendMessage(request);
	if (ret == SOCKET_ERROR)
		printf("Error %d\n", WSAGetLastError());
	char response[BUFF_SIZE];
	ret = receiveMessage(response);
	char * typeReq = strtok(response, SPLIT_DELIMITER);
	if (strcmp(typeReq, "400") == 0) {
		printf("Create success!\n");
	}
	else if (strcmp(typeReq, "401") == 0) {
		printf("Create fail\n");
	}
	

}

void showAddNewCategoryScence() {
	char _catetoryName[BUFF_SIZE] = "";
	printf("Category Name: ");
	gets_s(_catetoryName, BUFF_SIZE);
	if (strcmp(_catetoryName, "") == 0) {
		printf("Category Name not empty");
		return;
	}
	//build query
	char request[BUFF_SIZE];
	snprintf(request, sizeof(request), "CREATECA%s%s%s%s"
		, SPLIT_DELIMITER, _catetoryName, SPLIT_DELIMITER, secretKey);
	int ret = sendMessage(request);
	if (ret == SOCKET_ERROR)
		printf("Error %d\n", WSAGetLastError());
	char response[BUFF_SIZE];
	ret = receiveMessage(response);
	char * typeReq = strtok(response, SPLIT_DELIMITER);
	if (strcmp(typeReq, "900") == 0) {
		printf("Create success!\n");
	}
	else if (strcmp(typeReq, "901") == 0) {
		printf("Create fail\n");
	}
}

void showUpdateLocationDetail(int locationID, bool isShareType) {
	char _location_name[BUFF_SIZE] = "";
	if (!isShareType) {
		printf("Location Name(press 0 to no change): ");
		gets_s(_location_name, BUFF_SIZE);
	}
	//show all category
	printf("CategoryID(press 0 to no change)");
	int category_choice = showCategorySelector();
	if (category_choice <= 0 && strcmp(_location_name, "") == 0) {
			printf("You don't change anything!");
			return;	
	}
	else if (category_choice <= 0 && strcmp(_location_name, "0") == 0) {
		printf("You don't change anything!");
		return;
	}
	else {
		if (strcmp(_location_name, "0") == 0 || strcmp(_location_name, "") == 0) {
			strcpy(_location_name, " ");
		}
		if (category_choice <= 0) {
			category_choice = 0;
		}
		//build query
		char request[BUFF_SIZE];
		snprintf(request, sizeof(request), "UPDATEPL%s%d%s%s%s%d%s%s"
			, SPLIT_DELIMITER, locationID, SPLIT_DELIMITER, _location_name, SPLIT_DELIMITER, category_choice, SPLIT_DELIMITER, secretKey);
		int ret = sendMessage(request);
		if (ret == SOCKET_ERROR)
			printf("Error %d\n", WSAGetLastError());
		char response[BUFF_SIZE];
		ret = receiveMessage(response);
		char * typeReq = strtok(response, SPLIT_DELIMITER);
		if (strcmp(typeReq, "500") == 0) {
			printf("Update success!\n");
		}
		else if (strcmp(typeReq, "501") == 0) {
			printf("You don't have permission to change name of share location!\n");
		}
		else if (strcmp(typeReq, "502") == 0) {
			printf("Update fail!\n");
		}
		return;
	}
}

void showDeleteLocation(int locationId) {
	char request[BUFF_SIZE];
	snprintf(request, sizeof(request), "DELETEPL%s%d%s%s"
		, SPLIT_DELIMITER, locationId, SPLIT_DELIMITER, secretKey);
	int ret = sendMessage(request);
	if (ret == SOCKET_ERROR)
		printf("Error %d\n", WSAGetLastError());
	char response[BUFF_SIZE];
	ret = receiveMessage(response);
	char * typeReq = strtok(response, SPLIT_DELIMITER);
	if (strcmp(typeReq, "600") == 0) {
		printf("Delete success!\n");
	}
	else if (strcmp(typeReq, "601") == 0) {
		printf("Delete fail\n");
	}
}

void showShareLocation(int locationId) {
	char _user_be_shared[BUFF_SIZE] = "";
	printf("Username: ");
	gets_s(_user_be_shared, BUFF_SIZE);
	
	char request[BUFF_SIZE];
	snprintf(request, sizeof(request), "SHAREPL%s%d%s%s%s%s"
		, SPLIT_DELIMITER, locationId, SPLIT_DELIMITER, _user_be_shared, SPLIT_DELIMITER, secretKey);
	int ret = sendMessage(request);
	if (ret == SOCKET_ERROR)
	printf("Error %d\n", WSAGetLastError());
	char response[BUFF_SIZE];
	ret = receiveMessage(response);
	char * typeReq = strtok(response, SPLIT_DELIMITER);
	if (strcmp(typeReq, "300") == 0) {
		printf("Share success!\n");
	}
	else if (strcmp(typeReq, "301") == 0) {
		printf("User already be shared this location\n");
	}
}

void showLocationDetail(int locationID, bool isShareType) {
	printf("\n");
	printf("Options with locationID=%d..........\n", locationID);
	printf("1.Share		2.Update      3.Delete      4.Cancel\n");
	printf("Pick your choice: ");
	while (true) {
		int in = getInputOption();
		switch (in)
		{
		case 1:
			//share
			showShareLocation(locationID);
			return;
		case 2:
			showUpdateLocationDetail(locationID, isShareType);
			return;
		case 3:
			if (isShareType) {
				printf("You don't have permission to delete share location!\n");
				return;
			}
			else {
				printf("Do you want to delete?\n");
				if (yesNoQuestion()) {
					//handle delete placeID
					showDeleteLocation(locationID);
					return;
				}
			}
			return;
		case 4:
			return;
		default:
			printf("-LOG: Wrong. Select Again: ");
			break;
		}
	}
	
}


void showLocationsScene() {
	printf("\n");
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

int getNotifyStatus() {
	char request[BUFF_SIZE];
	snprintf(request, sizeof(request), "STATUS%s%s", SPLIT_DELIMITER, secretKey);
	int ret = sendMessage(request);
	if (ret == SOCKET_ERROR)
		printf("Error %d\n", WSAGetLastError());
	char response[BUFF_SIZE];
	ret = receiveMessage(response);
	char * typeReq = strtok(response, SPLIT_DELIMITER);
	if (strcmp(typeReq, "1000") == 0) {
		char * count_share_unmanage_location = strtok(NULL, SPLIT_DELIMITER);
		return atoi(count_share_unmanage_location);
	}
	else if (strcmp(typeReq, "1001") == 0) {
		return -1;
	}
}
void showHomeScene() {
	printf("\n");
	printf("================       Hello %s        ================\n", UserName);
	int count_share_unmanage_location = getNotifyStatus();
	if (count_share_unmanage_location >= 0) {
		printf("1. You has %d share locations don't have category!\n", count_share_unmanage_location);
	}
	
	printf("2. See all Location by Category\n");
	printf("3. Add new Location\n");
	printf("4. Add Custom Category\n");
	printf("5. Log out\n");
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
			showLocationsScene();
			return;
		case 3:
			showAddNewLocationScene();
			return;
		case 4:
			showAddNewCategoryScence();
			return;
		case 5:
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
	/*isLogin = true;
	strcpy(secretKey, "tokengenerate");
	strcpy(UserName, "thangtv");*/
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
		strcpy(UserName, "");
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