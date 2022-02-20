#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include "winsock2.h"
#include "ws2tcpip.h"
#include "string"
#include <windows.h>
#include "InputHelper.h"
#include "client-constants.h"

#define BUFF_SIZE  2048
#define SPLIT_DELIMITER "\##"
#define ENDING_DELIMITER "\r\n"
#pragma comment (lib, "Ws2_32.lib")
using namespace std;

int sendMessage(char* msg);
int receiveMessage(char* buff);
void showFirstScene();
void showHomeScene();
int getNotifyStatus();
void showLogoutScene();
void showRegisterScene();
void showLoginScene();
void showLocationDetail(int, bool);
void showNotifyLocationScene();
int showCategorySelector();
void showAddNewLocationScene();
void showUpdateLocationDetail(int, bool);
void showDeleteLocation(int);
void showShareLocation(int);
void showLocationsByCategoryScene();

SOCKET client;
bool isExitApp = false;
bool isLogin = false;
char UserName[BUFF_SIZE];
char secretKey[BUFF_SIZE];


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
	while (!isExitApp) {
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


/*
* @function sendMessage : send message to server
* @param msg : a pointer to message string
*
* @return :  bytes send sucess
*
*/
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

/*
* @function receiveMessage : receive message from server
* @param msg : a pointer string to receive message 
*
* @return :  bytes receive sucess
*
*/
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


//show log out screen
void showLogoutScene() {
	printf("Are you sure to Logout?\n");
	if (yesNoQuestion()) {
		printf("Log out Success.\n");
		isLogin = false;
		strcpy(secretKey, "");
		strcpy(UserName, "");
	}
}
//show log in screen
void showLoginScene() {
	char _userName[BUFF_SIZE], _password[BUFF_SIZE];
	printf("\t\tUsername: ");
	gets_s(_userName, BUFF_SIZE);
	printf("\t\tPassword: ");
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
	if (strcmp(typeReq,  LOGIN_OK) == 0) {
		char * token = strtok(NULL, SPLIT_DELIMITER);
		isLogin = true;
		strcpy(secretKey, token);
	}
	else if (strcmp(typeReq, LOGIN_NOK) == 0) {
		isLogin = false;
		printf("\t\tLogin failed, account or password not correct! \n");
	}
}

//show register screen
void showRegisterScene() {
	char _userName[BUFF_SIZE], _password1[BUFF_SIZE], _password2[BUFF_SIZE];
	printf("\n\t\tUsername: ");
	gets_s(_userName, BUFF_SIZE);
	printf("\t\tPassword: ");
	gets_s(_password1, BUFF_SIZE);
	printf("\t\tPassword Again: ");
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
		if (strcmp(typeReq, REGIS_OK) == 0) {
			printf("\t\tRegister success!\n");
		}
		else if (strcmp(typeReq, REGIS_NOK) == 0) {
			printf("\t\tUser already exists!\n");
		}
	}
	else {
		printf("\t\tPassword did not match: Please try again... \n");
		showRegisterScene();
	}

}


/*
* @function showAllLocationByType : show all location by type
* @param categoryid :  ID of category
* @parm isShareType: check share type (true/false)
*
* @return : void
*/
void showAllLocationByType(int categoryid, bool isShareType = false) {
	printf("\n\t\t**********   LOCATION LIST     ********** \n");
	printf("\t\t0. Back\n");
	char request[BUFF_SIZE];
	if (isShareType) {
		snprintf(request, sizeof(request), "LISTPL%s%c%s%s"
			, SPLIT_DELIMITER, '0', SPLIT_DELIMITER, secretKey);
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
	if (strcmp(typeReq, LISTPL_OK) == 0) {
		if (isShareType) {
			printf("\t\t===================================================================\n");
			printf("\t\tPlaceId\t\tUSERSHARE\t\t\tNAME\n");
			printf("\t\t===================================================================\n");
			char * place_id = strtok(NULL, SPLIT_DELIMITER);
			while (place_id != NULL) {
				char * place_name = strtok(NULL, SPLIT_DELIMITER);
				char * user_share = strtok(NULL, SPLIT_DELIMITER);
				printf("\t\t%-7s\t\t%-20s\t\t%-40s\n", place_id, user_share, place_name);
				//bypass category=0
				strtok(NULL, SPLIT_DELIMITER);
				place_id = strtok(NULL, SPLIT_DELIMITER);
			}
		}
		else {
			printf("\t\t=======================================================================\n");
			printf("\t\tPlaceId\t\tUSERSHARE\t\tCATEGORY_NAME\t\t\tPLACE_NAME\n");
			printf("\t\t=======================================================================\n");
			char * place_id = strtok(NULL, SPLIT_DELIMITER);
			while (place_id != NULL) {
				char * place_name = strtok(NULL, SPLIT_DELIMITER);
				char * category_name = strtok(NULL, SPLIT_DELIMITER);
				char * user_share = strtok(NULL, SPLIT_DELIMITER);
				printf("\t\t%-7s\t\t%-20s\t%-30s\t%-40s\n", place_id, user_share, category_name, place_name);
				place_id = strtok(NULL, SPLIT_DELIMITER);
			}
		}

		printf("\t\t==================================================\n");
		printf("\t\tYou select one location ID: ");
		while (true) {
			int in = getInputOption();
			if (in < 0) {
				printf("\tWrong. Select Again: ");
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
	else if (strcmp(typeReq, LISTPL_NOK) == 0) {
		printf("\t\tNo location found!\n");
	}
}

//screen to show all share location
void showNotifyLocationScene() {
	showAllLocationByType(0, true);
}

/*
* @function showCategorySelector : show all category to choice
*
* @return : category_id base on choice
*/
int showCategorySelector() {
	printf("\n\t\t**********    CATEGORY LIST     **********\n");
	printf("\t\t0. BACK\n");
	char request[BUFF_SIZE];
	snprintf(request, sizeof(request), "LISTCA%s%s"
		, SPLIT_DELIMITER, secretKey);
	int ret = sendMessage(request);
	if (ret == SOCKET_ERROR)
		printf("Error %d\n", WSAGetLastError());
	char response[BUFF_SIZE];
	ret = receiveMessage(response);
	char * typeReq = strtok(response, SPLIT_DELIMITER);
	if (strcmp(typeReq, LISTCA_OK) == 0) {
		printf("\t\t=============================================================\n");
		printf("\t\tCATEGORY_ID\t\tNAME\n");
		printf("\t\t=============================================================\n");
		char * category_id = strtok(NULL, SPLIT_DELIMITER);
		while (category_id != NULL) {
			char * category_name = strtok(NULL, SPLIT_DELIMITER);
			printf("\t\t%.15s\t\t%.40s\n", category_id, category_name);
			category_id = strtok(NULL, SPLIT_DELIMITER);
		}
		printf("\t\t==================================\n");
		printf("\t\tYou Select one category ID: ");
		while (true) {
			int in = getInputOption();
			if (in < 0) {
				printf("\tWrong. Select Again: ");
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
	else if (strcmp(typeReq, LISTCA_NOK) == 0) {
		printf("\t\tNo Category found!\n");
	}
	return -1;

}

//show screen to add new location
void showAddNewLocationScene() {
	char _locationName[BUFF_SIZE] = "";
	printf("\n\t\tLocation Name: ");
	gets_s(_locationName, BUFF_SIZE);
	int category_choice = showCategorySelector();
	if (category_choice <= 0 || strcmp(_locationName, "") == 0) {
		printf("\t\tLocation not empty and categoryId must valid");
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
	if (strcmp(typeReq, ADDPL_OK) == 0) {
		printf("\t\tCreate success!\n");
	}
	else if (strcmp(typeReq, ADDPL_NOK) == 0) {
		printf("\t\tCreate fail\n");
	}


}
//show screen to add new category
void showAddNewCategoryScence() {
	char _catetoryName[BUFF_SIZE] = "";
	printf("\t\tCategory Name: ");
	gets_s(_catetoryName, BUFF_SIZE);
	if (strcmp(_catetoryName, "") == 0 || strcmp(_catetoryName, "0") == 0) {
		printf("\t\tCategory Name not empty");
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
	if (strcmp(typeReq, CREATECA_OK) == 0) {
		printf("\t\tCreate success!\n");
	}
	else if (strcmp(typeReq, CREATECA_NOK) == 0) {
		printf("\t\tCreate fail\n");
	}
}
/*
* @function showUpdateLocationDetail : handle update a location 
* @param locationID :  ID of location
* @parm isShareType: check share type (true/false)
*
*/
void showUpdateLocationDetail(int locationID, bool isShareType) {
	char _location_name[BUFF_SIZE] = "";
	if (!isShareType) {
		printf("\t\tLocation Name(press 0 to no change): ");
		gets_s(_location_name, BUFF_SIZE);
	}
	//show all category
	//printf("\t\tChoose Category ID( press 0 to no change)\n");
	int category_choice = showCategorySelector();
	if (category_choice <= 0 && strcmp(_location_name, "") == 0) {
		printf("\t\tYou don't change anything!");
		return;
	}
	else if (category_choice <= 0 && strcmp(_location_name, "0") == 0) {
		printf("\t\tYou don't change anything!");
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
		if (strcmp(typeReq, UPDATEPL_OK) == 0) {
			printf("\t\tUpdate success!\n");
		}
		else if (strcmp(typeReq, UPDATEPL_NON_PERSSION) == 0) {
			printf("\t\tYou don't have permission to change name of share location!\n");
		}
		else if (strcmp(typeReq, UPDATEPL_NOK) == 0) {
			printf("\t\tUpdate fail!\n");
		}
		return;
	}
}
/*
* @function showDeleteLocation : handle delete a location
* @param locationID :  ID of location
*
*/
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
	if (strcmp(typeReq, DELETEPL_OK) == 0) {
		printf("\t\tDelete success!\n");
	}
	else if (strcmp(typeReq, DELETEPL_NOK) == 0) {
		printf("\t\tDelete fail\n");
	}
}
/*
* @function showDeleteLocation : handle share a location
* @param locationID :  ID of location
*
*/
void showShareLocation(int locationId) {
	char _user_be_shared[BUFF_SIZE] = "";
	printf("\t\tUsername: ");
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
	if (strcmp(typeReq, SHAREPL_OK) == 0) {
		printf("\t\tShare success!\n");
	}
	else if (strcmp(typeReq, SHAREPL_NOK) == 0) {
		printf("\t\tUser already be shared this location\n");
	}
}

/*
* @function showLocationDetail : handle options with a  location
* @param locationID :  ID of location
* @parm isShareType: check share type (true/false)
*
*/
void showLocationDetail(int locationID, bool isShareType) {
	printf("\t\tOptions with location_ID=%d: \n", locationID);
	printf("\t\t1.Share		2.Update      3.Delete      4.Cancel\n");
	printf("\t\tPick your choice: ");
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
				printf("\t\tYou don't have permission to delete share location!\n");
				return;
			}
			else {
				printf("\t\tDo you want to delete?\n");
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
			printf("\tWrong. Select Again: ");
			break;
		}
	}

}

//screen: choice category to show all locations
void showLocationsByCategoryScene() {
	printf("\n");
	int input = showCategorySelector();
	if (input == -1) {
		return;
	}
	else {
		showAllLocationByType(input);
	}
}
//show screen when not login
void showFirstScene() {
	printf("\n");
	printf("================              MENU              =========\n");
	printf("1. Log in\n");
	printf("2. Sign up\n");
	printf("3. Exit app\n");
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
		case 3:
			isExitApp = true;
			return;
		default:
			printf("\tWrong. Select Again: ");
			break;
		}
	}

}
/*
* @function getNotifyStatus : get number of unmanaged share locations
* 
*  @return : int
*
*/
int getNotifyStatus() {
	char request[BUFF_SIZE];
	snprintf(request, sizeof(request), "STATUS%s%s", SPLIT_DELIMITER, secretKey);
	int ret = sendMessage(request);
	if (ret == SOCKET_ERROR)
		printf("Error %d\n", WSAGetLastError());
	char response[BUFF_SIZE];
	ret = receiveMessage(response);
	char * typeReq = strtok(response, SPLIT_DELIMITER);
	if (strcmp(typeReq, STATUS_OK) == 0) {
		char * count_share_unmanage_location = strtok(NULL, SPLIT_DELIMITER);
		return atoi(count_share_unmanage_location);
	}
	else if (strcmp(typeReq, STATUS_NOK) == 0) {
		return -1;
	}
}
//show home screen when login
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
	printf("6. Refresh\n");
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
			showLocationsByCategoryScene();
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
		case 6:
			return;
		default:
			printf("\tWrong. Select Again: ");
			break;
		}
	}
}
