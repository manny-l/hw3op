/*
 * fparser.c
 *
 *  Created on: May 15, 2012
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100

int main(){
	char command[N];
	do{
		fgets(command,N,stdin);
		if (strncmp(command,"BEGIN",strlen("BEGIN"))){//got begin
			break;
		}
	} while (1);
	do{
		do{
			fgets(command,N,stdin);
			//list.insert(command);
			//TODO: write a simple list to contain the commands.
			printf("%s\n",command);
		}while (strncmp(command,"BARRIER",strlen("BARRIER")));
		//handle burst
		printf("handling burst\n");
		fgets(command,N,stdin);
	} while (strncmp(command,"END",strlen("END")));//loop until end
	//system("pause");
	return 0;
}
