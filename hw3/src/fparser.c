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
		if (!strncmp(command,"BEGIN",strlen("BEGIN"))){//got begin
			printf("got BEGIN\n");
			break;
		}
	} while (1);
	while (strncmp(command,"END",strlen("END"))){//loop until end
		do{
			fgets(command,N,stdin);//we assume the input is correct
			if (!strncmp(command,"BARRIER",strlen("BARRIER"))){
				printf("reached barrier\n");
				break;
			}
			if (!strncmp(command,"END",strlen("END"))) {
				printf("reached END\n");
				break;
			}
			//list.insert(command);
			//TODO: write a simple list to contain the commands.
			printf("handling command:\n %s\n",command);
		} while (1);
		//handle burst
		//TODO: handle burst
		printf("handling burst\n");
		if (strncmp(command,"END",strlen("END"))) {
			printf("reached END\n");
			break;
		}
	}
	printf("end actions\n");
	//system("pause");
	return 0;
}

