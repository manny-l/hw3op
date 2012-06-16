/*
 ============================================================================
 Name        : hw4test2.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
	puts("!!!Hellocvxcv World!!!"); /* prints !!!Hello World!!! */

	int device_1=open("/dev/rng0",O_RDWR);

	printf("device_1 is %d\n",device_1);

	int buff = 5;

	int i, writeRslt;
	for (i=0;i<1;i++)
	{
		writeRslt = write(device_1,&buff,sizeof(buff));
		printf("write %d: %d\n",i,writeRslt);
	}

	close(device_1);

	return EXIT_SUCCESS;
}
