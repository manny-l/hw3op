/*
 * fparser.c
 *
 *  Created on: May 15, 2012
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>

int main(){
	char command[100];
	fgets(command,100,stdin);
	printf("%s",command);
	system("pause");
	return 0;
}
