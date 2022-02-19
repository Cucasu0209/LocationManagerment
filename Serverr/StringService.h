#pragma once
#include <iostream>
#define SECRET_LEN 30

/*
* @function generateString : generate random string from a string
*
* @param input : pointer to a string
*
*
* @return :  a string contains input+generate_string
*
*/
char * generateString(char * input) {
	char keyGen[SECRET_LEN];
	char charArr[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/,.-+=~`<>:";
	for (int index = 0; index < SECRET_LEN; index++)
	{
		keyGen[index] = charArr[rand() % (sizeof charArr - 1)];
	}
	char * tmp = (char *)malloc(sizeof(char) * 100);
	strcpy(tmp, input);
	return strcat(tmp, keyGen);
}