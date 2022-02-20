#pragma once
#include <iostream>
#define BUFF_SIZE 2048

/*
* @function convertCharArrayToInt : convert from string to int
* @param a : a pointer to a number_string
*
* @return :  exact value type:int 
*---------    -1 if contain non number character
*			  
*/
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

/*
* @function getInputOption : get input number
*
* @return :   number input type:int
*---------     -1 if contain non number character
*
*/
int getInputOption() {
	char key[BUFF_SIZE];
	gets_s(key, BUFF_SIZE);
	return convertCharArrayToInt(key);
}


/*
* @function yesNoQuestion : get input yes or no
*
* @return :   true or false base on input
*
*/
bool yesNoQuestion() {
	printf("\t\t\t1. Yes         2. No\n");
	printf("\t\t\tYou select: ");
	while (true) {
		int sel = getInputOption();
		if (sel == 1)return true;
		if (sel == 2)return false;
		printf("\t\t\tSelect again: ");
	}
	return true;

}


/*
* @function isExistInArray : check number exsit in array number
* @param arr : array number
* @param size_arr : length array
* @param num : number check

* @return :   true if exsit, else return false
*
*/
bool isExistInArray(int arr[], int size_arr, int num) {
	for (int i = 0; i < size_arr; i++) {
		if (arr[i] == num) {
			return true;
		}
	}
	return false;
}