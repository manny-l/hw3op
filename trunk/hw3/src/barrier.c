#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>

#include "doublyLinkedList.h"

#define INIT_ARR_SIZE 25
#define MAX_COMMAND_SIZE 25

struct barrierData{
	int threadNum;
	int threadsInBarrier;
	sem_t enterBarrier;
	sem_t leaveBarrier;
};

typedef struct threadData_t
{
	char currOrder[MAX_COMMAND_SIZE];
	int	key;
	char val;
	pthread_t threadID;
}* threadDataPnt;

struct barrierData bData;
pthread_cond_t conterCond;
pthread_mutex_t writeToOutputLock;
pthread_mutexattr_t attr1;
pthread_mutex_t counterLock;
pthread_mutexattr_t attr2;
DoublyLinkedList list;

//forward functions declarations
int initBarrierStruct(int threadNum);
int getOrderBatch();
void runOrders(threadDataPnt commandList[],int threadNum);
void newThreads(void* data);
void barrier();
void freeCurrList(threadDataPnt commandList[],int threadsNum);
int initLocks();
void freeStuffDestroy();

int main()
{
	int res = 0;
	char currOrder[MAX_COMMAND_SIZE];

	scanf ("%s",currOrder);

	if (strcmp(currOrder,"BEGIN") != 0)
	{
		return (-1);
	}
	else
	{
		printf("BEGIN\n");
	}

	if (initLocks() == -1)
	{
		return -1;
	}

	Initialize();

	while (res == 0)
	{
		res = getOrderBatch();
	}

	freeStuffDestroy();

	return 0;
}

void freeStuffDestroy()
{
	Destroy();

	sem_destroy(&(bData.enterBarrier));
	sem_destroy(&(bData.leaveBarrier));
	pthread_mutex_destroy(&writeToOutputLock);
	pthread_mutex_destroy(&counterLock);
	pthread_cond_destroy(&conterCond);
}

int initBarrierStruct(int threadNum)
{
	bData.threadNum = threadNum;
	bData.threadsInBarrier = 0;
	int enterBarrierVal;
	int leaveBarrierVal;
	sem_getvalue(&(bData.enterBarrier),&enterBarrierVal);
	sem_getvalue(&(bData.leaveBarrier),&leaveBarrierVal);
	while(enterBarrierVal > 1)
	{
		sem_post(&(bData.enterBarrier));
		sem_getvalue(&(bData.enterBarrier),&enterBarrierVal);
	}
	while(leaveBarrierVal > 1)
	{
		sem_post(&(bData.leaveBarrier));
		sem_getvalue(&(bData.leaveBarrier),&leaveBarrierVal);
	}
	return 0;
}

int initLocks()
{
	if (sem_init(&(bData.enterBarrier),0,1)!=0)
	{
		exit(0);
	}

	if (sem_init(&(bData.leaveBarrier),0,0)!=0)
	{
		exit(0);
	}

	if (pthread_cond_init(&conterCond,NULL)!=0)
	{
		exit(0);
	}

	if (pthread_mutexattr_init(&attr1)!=0)
	{
		exit(0);
	}

	pthread_mutexattr_settype(&attr1,PTHREAD_MUTEX_ERRORCHECK_NP);

	if (pthread_mutex_init(&writeToOutputLock,&attr1)!=0)
	{
		exit(0);
	}

	if (pthread_mutexattr_init(&attr2)!=0)
	{
		exit(0);
	}

	pthread_mutexattr_settype(&attr2,PTHREAD_MUTEX_ERRORCHECK_NP);

	if (pthread_mutex_init(&counterLock,&attr2)!=0)
	{
		exit(0);
	}

	return 0;
}

int getOrderBatch()
{
	int arrSize = INIT_ARR_SIZE;
	threadDataPnt* commandList;
	char command[MAX_COMMAND_SIZE];
	threadDataPnt currData;
	int threadsNum = 0;
	int i;

	scanf ("%s",command);
	if (!strcmp(command,"BARRIER"))
	{
		printf("BARRIER\n");
		return 0;
	}

	if (!strcmp(command,"END"))
	{
		printf("END\n");
		return 1;
	}

	commandList = (threadDataPnt*)malloc(arrSize*sizeof(threadDataPnt));
	if(commandList == NULL)
	{
		exit(0);
	}

	while(strcmp(command,"BARRIER") && strcmp(command,"END"))
	{
		currData = (threadDataPnt)malloc(sizeof(struct threadData_t));
		if (currData == NULL)
		{
			freeCurrList(commandList,threadsNum);
			exit(0);
		}

		strcpy(currData->currOrder,command);

		if ((strcmp(command,"INSERT_HEAD")==0) || (strcmp(command,"INSERT_TAIL")==0))
		{
			scanf ("%d %c",&(currData->key),&(currData->val));
		}
		else
		{
			scanf ("%d",&(currData->key));
		}


		if (threadsNum == arrSize)
		{
			arrSize = arrSize*2;
			commandList = (threadDataPnt*)realloc
						(commandList,arrSize*sizeof(threadDataPnt));
			if (commandList == NULL)
			{
				freeCurrList(commandList,threadsNum);
				return -1;
			}
		}


		commandList[threadsNum] = currData;

		threadsNum++;

		scanf ("%s",command);
	}

	if (threadsNum > 0)
	{
		if (initBarrierStruct(threadsNum) == -1)
		{
			freeCurrList(commandList,threadsNum);
			return -1;
		}

		runOrders(commandList,threadsNum);

		for (i = 0; i < threadsNum; i++)
		{
			if(commandList[i]->threadID != -1)
			{
				pthread_join(commandList[i]->threadID,NULL);
			}
		}

		freeCurrList(commandList,threadsNum);
	}
	else
	{
		free(commandList);
	}

	if (!strcmp(command,"BARRIER"))
	{
		printf("BARRIER\n");
		return 0;
	}

	if (!strcmp(command,"END"))
	{
		printf("END\n");
		return 1;
	}

	return -1;
}

void freeCurrList(threadDataPnt commandList[],int threadsNum){

	int i;
	threadDataPnt currData;

	for (i = 0; i < threadsNum; i++)
	{
		currData = commandList[i];
		free(currData);
		commandList[i] = 0;
	}

	free(commandList);
}


void runOrders(threadDataPnt commandList[],int threadNum)
{
	pthread_t threadIndex;
	int i;

	for (i = 0; i < threadNum; i++)
	{

		if (pthread_create(&(commandList[i]->threadID),NULL,newThreads,commandList[i]))
		{
			printf ("Error in thread creation\n");
			commandList[i]->threadID = -1;
			pthread_mutex_lock(&counterLock);
				bData.threadNum--;
			pthread_mutex_unlock(&counterLock);
		}
	}

	return;
};

void barrier()
{
	sem_wait(&bData.enterBarrier);

	bData.threadsInBarrier++;

	if(bData.threadsInBarrier < bData.threadNum)
	{
		sem_post(&bData.enterBarrier);
	}
	else
	{
		sem_post(&(bData.leaveBarrier));
	}

	sem_wait(&bData.leaveBarrier);

	bData.threadsInBarrier--;

	if(bData.threadsInBarrier > 0)
	{
		sem_post(&bData.leaveBarrier);
	}
	else
	{
		sem_post(&(bData.enterBarrier));
	}
	return;
}


void newThreads(void* data)
{
	bool res;

	int z;

	barrier();

	if (data != NULL){

		threadDataPnt currData = (threadDataPnt)data;

		if (strcmp(currData->currOrder,"INSERT_HEAD") == 0)
		{
			res= InsertHead(currData->key,currData->val);
		}
		else if (strcmp(currData->currOrder,"INSERT_TAIL") == 0)
		{
			res= InsertTail(currData->key,currData->val);
		}
		else if (strcmp(currData->currOrder,"DELETE") == 0)
		{
			res= Delete(currData->key);
		}
		else
		{
			assert((strcmp(currData->currOrder,"SEARCH") == 0));
			res = Search(currData->key,&(currData->val));
		}

		//write result to output
		pthread_mutex_lock(&writeToOutputLock);
			if (strcmp(currData->currOrder,"INSERT_HEAD") == 0)
			{
				if (res==true){
					printf("%s %d %c->%s\n",currData->currOrder,currData->key,currData->val,"TRUE");
				} else {
					printf("%s %d %c->%s\n",currData->currOrder,currData->key,currData->val,"FALSE");
				}
			}
			else if (strcmp(currData->currOrder,"INSERT_TAIL") == 0){
				if (res==true){
					printf("%s %d %c->%s\n",currData->currOrder,currData->key,currData->val,"TRUE");
				} else {
					printf("%s %d %c->%s\n",currData->currOrder,currData->key,currData->val,"FALSE");
				}
			} else if (strcmp(currData->currOrder,"DELETE") == 0){
				if (res==true){
					printf("%s %d->%s\n",currData->currOrder,currData->key,"TRUE");
				} else {
					printf("%s %d->%s\n",currData->currOrder,currData->key,"FALSE");
				}
			} else { assert((strcmp(currData->currOrder,"SEARCH") == 0));
				if (res==true){
					printf("%s %d->%c\n",currData->currOrder,currData->key,currData->val);
				} else {
					printf("%s %d->%s\n",currData->currOrder,currData->key,"FALSE");
				}
			}

		pthread_mutex_unlock(&writeToOutputLock);
	}

	return;
};



