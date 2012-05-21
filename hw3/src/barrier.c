#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>//TODO: remove assert at end of debug
#include <pthread.h>
#include <semaphore.h>
//#include "b5-9.h"
#include "doublyLinkedList.h"

#define MAX_COMMAND_SIZE 25
#define INIT_ARR_SIZE 25

//barrier struct dfsdfgdfg
struct barrierData{
	int threadNum;
	int threadsInBarrier;
	sem_t enterBarrier;
	sem_t leaveBarrier;
};

//struct for parameters for threads
typedef struct threadData_t{
	char currOrder[MAX_COMMAND_SIZE];
	int	key;
	char val;
	pthread_t threadID;
}*threadDataPnt;

//Global parameters
struct barrierData bData;
pthread_cond_t conterCond;
pthread_mutex_t writeToOutputLock;
pthread_mutexattr_t attr1;
pthread_mutex_t counterLock;
pthread_mutexattr_t attr2;
//tree currTree;
DoublyLinkedList list;

//functions declarations
int initBarrierStruct(int threadNum);
int getOrderBatch();
void runOrders(threadDataPnt commandList[],int threadNum);
void newThreads(void* data);
void barrier();
void freeCurrList(threadDataPnt commandList[],int threadsNum);
int initLocks();
void freeAndDestroy();

/*----------------------------------------------------------------
The Main Thread:
Description: call getOrderBatch in a loop
			 until the word END is read
-----------------------------------------------------------------*/
int main(){

	int res = 0;
	char currOrder[MAX_COMMAND_SIZE];

	//start reading data from stdin
	scanf ("%s",currOrder);

	//checking if first line is: BEGIN
	if (strcmp(currOrder,"BEGIN") != 0){
		printf("Error, first command is not BEGIN!\n");
		return (-1);
	}
	else{
		printf("BEGIN\n");
	}

	//locks initializing
	if (initLocks() == -1){
		printf("Error, lock initializing failed!\n");
		return -1;
	}

	//tree initializing
	/*
	if (-1 ==  initTree(&currTree)){
		printf("Error, tree initializing failed!\n");
		return (-1);
	}
	*/
	Initialize();

	//executing batches of commands until END is read
	while (res == 0){
		res = getOrderBatch();
	}

	//free DBLL & detroy locks
	freeAndDestroy();

	return res;
}

void freeAndDestroy(){
	//free the tree
	//freeTree(currTree);
	Destroy();

	//destroy locks for barrier
	sem_destroy(&(bData.enterBarrier));
	sem_destroy(&(bData.leaveBarrier));
	pthread_mutex_destroy(&writeToOutputLock);
	pthread_mutex_destroy(&counterLock);
	pthread_cond_destroy(&conterCond);

}

//initialize the barrier struct
int initBarrierStruct(int threadNum){
	bData.threadNum = threadNum;
	bData.threadsInBarrier = 0;
	int enterBarrierVal;
	int leaveBarrierVal;
	sem_getvalue(&(bData.enterBarrier),&enterBarrierVal);
	sem_getvalue(&(bData.leaveBarrier),&leaveBarrierVal);
	while(enterBarrierVal > 1){
		sem_post(&(bData.enterBarrier));
		sem_getvalue(&(bData.enterBarrier),&enterBarrierVal);
	}
	while(leaveBarrierVal > 1){
		sem_post(&(bData.leaveBarrier));
		sem_getvalue(&(bData.leaveBarrier),&leaveBarrierVal);
	}
	return 0;
}

//initialize all the locks
int initLocks(){
	if (sem_init(&(bData.enterBarrier),0,1)!=0){
		printf("Error in sem_init\n");
		exit(0);
	}
	if (sem_init(&(bData.leaveBarrier),0,0)!=0){
		printf("Error in sem_init\n");
		exit(0);
	}

	if (pthread_cond_init(&conterCond,NULL)!=0){
		printf("Error in pthread_cond_init\n");
		exit(0);
	}

	if (pthread_mutexattr_init(&attr1)!=0){
		printf("Error in pthread_mutexattr_init\n");
		exit(0);
	}

	pthread_mutexattr_settype(&attr1,PTHREAD_MUTEX_ERRORCHECK_NP);

	if (pthread_mutex_init(&writeToOutputLock,&attr1)!=0){
		printf("Error in pthread_mutex_init\n");
		exit(0);
	}

	if (pthread_mutexattr_init(&attr2)!=0){
		printf("Error in pthread_mutexattr_init\n");
		exit(0);
	}

	pthread_mutexattr_settype(&attr2,PTHREAD_MUTEX_ERRORCHECK_NP);

	if (pthread_mutex_init(&counterLock,&attr2)!=0){
		printf("Error in pthread_mutex_init\n");
		exit(0);
	}

	return 0;
}

/*----------------------------------------------------------------
getOrderBatch function:
Input: NONE (uses global data)
Output: 0 - if the last command in the batch was Barrier
		1 - if the last command in the batch was End
	   -1 - for errors
Description: - reading the commands of a single section
			 - saving the commands in commandList
			 - initializing the BarrierStruct
			 - calling the function runOrders
-----------------------------------------------------------------*/
int getOrderBatch(){

	//local parameters
	int arrSize = INIT_ARR_SIZE;
	threadDataPnt* commandList;
	char command[MAX_COMMAND_SIZE];
	threadDataPnt currData;
	int threadsNum = 0;
	int i;

	//reading the first command
	scanf ("%s",command);
	if (!strcmp(command,"BARRIER")){
		printf("BARRIER\n");
		return 0;
	}
	if (!strcmp(command,"END")){
		printf("END\n");
		return 1;
	}

	//allocating a list for the data of the commands
	commandList = (threadDataPnt*)malloc(arrSize*sizeof(threadDataPnt));
	if(commandList == NULL){
		printf("Error in memory allocation\n");
		exit(0);
	}

	//reading data while the section is not finished
	while(strcmp(command,"BARRIER") && strcmp(command,"END")){

		//allocated data struct for command data
		currData = (threadDataPnt)malloc(sizeof(struct threadData_t));
		if (currData == NULL){
			printf("Error in memory allocation\n");
			freeCurrList(commandList,threadsNum);
			exit(0);
		}

		//insert command data for thread
		strcpy(currData->currOrder,command);

		if ((strcmp(command,"INSERT_HEAD")==0) || (strcmp(command,"INSERT_HEAD")==0)){
			scanf ("%d %c",&(currData->key),&(currData->val));
		} else { // COMMAND == DELETE or FIND
			scanf ("%d",&(currData->key));
		}

		//Enlarging data array if it is too small
		if (threadsNum == arrSize){
			arrSize = arrSize*2;
			commandList = (threadDataPnt*)realloc
						(commandList,arrSize*sizeof(threadDataPnt));
			if (commandList == NULL){
				freeCurrList(commandList,threadsNum);
				printf("Error in memory allocation\n");
				return -1;
			}
		}


		//inserting data to commandList as array
		commandList[threadsNum] = currData;

		//counting the num of commands
		threadsNum++;

		//reading the next command
		scanf ("%s",command);
	} //end of while loop, reached BARRIER or END

	if (threadsNum > 0){

		//initialize the Barrier Struct
		if (initBarrierStruct(threadsNum) == -1){
			freeCurrList(commandList,threadsNum);
			return -1;
		}

		//run the commands
		runOrders(commandList,threadsNum);

		//wait for all the threads to end
		for (i = 0; i < threadsNum; i++){
			if(commandList[i]->threadID != -1){
				pthread_join(commandList[i]->threadID,NULL);
			}
		}

		//free list
		freeCurrList(commandList,threadsNum);
	}
	else{
		free(commandList);
	}

	//RETURN VALUES
	if (!strcmp(command,"BARRIER")){
		printf("BARRIER\n");
		return 0;
	}

	if (!strcmp(command,"END")){
		printf("END\n");
		return 1;
	}
	return -1;
}

/*----------------------------------------------------------------
freeCurrList function:
Input: commandList = list of commands
	   threadNum = number of commands in the list
Output: none
Description: frees the command list
-----------------------------------------------------------------*/
void freeCurrList(threadDataPnt commandList[],int threadsNum){

	int i;
	threadDataPnt currData;
	//free the commands in the command list
	for (i = 0; i < threadsNum; i++){
		currData = commandList[i];
		free(currData);
		commandList[i] = 0;
	}

	//free the command list
	free(commandList);
}


/*----------------------------------------------------------------
runOrders function:
Input: commandList = list of commands
	   threadNum = number of commands in the list
Output: none
Description: creates a thread for every command in the list and sends
		it to the newThreads function with the relevant command data
-----------------------------------------------------------------*/
void runOrders(threadDataPnt commandList[],int threadNum){

	pthread_t threadIndex;
	int i;

	for (i = 0; i < threadNum; i++){

		//allocate thread
		if (pthread_create(&(commandList[i]->threadID),NULL,newThreads,commandList[i])){
			printf ("Error in thread creation\n");
			commandList[i]->threadID = -1;
			//if thread creation failed update number of running threads
			pthread_mutex_lock(&counterLock);
				bData.threadNum--;
			pthread_mutex_unlock(&counterLock);
		}
	}

	return;
};

/*----------------------------------------------------------------
barrier function:
Input: global struct bData
Output: none
Description: lets threadNum of threads enter the barrier and
	lets them leave only when all threadNum threads has enterd
-----------------------------------------------------------------*/
void barrier(){

	//wait for entrance lock
	sem_wait(&bData.enterBarrier);

	//update number of threads
	bData.threadsInBarrier++;

	//opening one of the barrier locks
	if(bData.threadsInBarrier < bData.threadNum){
		sem_post(&bData.enterBarrier);
	}
	else {
		sem_post(&(bData.leaveBarrier));
	}

	//wait for exit lock
	sem_wait(&bData.leaveBarrier);

	//update number of threads
	bData.threadsInBarrier--;

	//opening one of the barrier locks
	if(bData.threadsInBarrier > 0){
		sem_post(&bData.leaveBarrier);
	}
	else {
		sem_post(&(bData.enterBarrier));
	}
	return;
}

/*----------------------------------------------------------------
newThreads function:
Input: data struct cotaining: command,ket & val
Output: none
Description: waits in the barrier for the other threads.
	And then performs the command and prints the result to stdout
-----------------------------------------------------------------*/
void newThreads(void* data){

	int res;
	//waiting for all the other threads
	barrier();

	if (data != NULL){

		//converting data struct
		threadDataPnt currData = (threadDataPnt)data;

		//find/insert in tree
		if (strcmp(currData->currOrder,"INSERT_HEAD") == 0){
			//res = findElm(currTree,currData->key,&(currData->val));
			//TODO: insert to head of DBLL command here
			res= InsertHead(currData->key,currData->val);
		} else if (strcmp(currData->currOrder,"INSERT_TAIL") == 0){
			//res = addElm(currTree,currData->key,currData->val);
			//TODO: insert to tail of DBLL command here
			res= InsertTail(currData->key,currData->val);
		} else if (strcmp(currData->currOrder,"DELETE") == 0){
			//TODO: delete element from DBLL command here
			res= Delete(currData->key);
		} else {
			assert((strcmp(currData->currOrder,"SEARCH") == 0));
			//TODO: search element within DBLL
			res = Search(currData->key,currData->val);
		}

		if (-2==res){
			printf("Fatal error in find or add functions\n");
			exit(0);
		}

		//write result to output
		pthread_mutex_lock(&writeToOutputLock);
			if (strcmp(currData->currOrder,"INSERT_HEAD") == 0){//TODO: change res references
				if (!res){
					printf("%s %d %c -> %s\n",currData->currOrder,currData->key,currData->val,"TRUE");
				} else {
					printf("%s %d %c -> %s\n",currData->currOrder,currData->key,currData->val,"FALSE");
				}
			} else if (strcmp(currData->currOrder,"INSERT_TAIL") == 0){
				if (!res){
					printf("%s %d %c -> %s\n",currData->currOrder,currData->key,currData->val,"TRUE");
				} else {
					printf("%s %d %c -> %s\n",currData->currOrder,currData->key,currData->val,"FALSE");
				}
			} else if (strcmp(currData->currOrder,"DELETE") == 0){
				if (!res){
					printf("%s %d -> %s\n",currData->currOrder,currData->key,"TRUE");
				} else {
					printf("%s %d -> %s\n",currData->currOrder,currData->key,"FALSE");
				}
			} else { assert((strcmp(currData->currOrder,"SEARCH") == 0));
				if (!res){
					printf("%s %d -> %c\n",currData->currOrder,currData->key,currData->val);
				} else {
					printf("%s %d -> %s\n",currData->currOrder,currData->key,"FALSE");
				}
			}
			/*
			if (strcmp(currData->currOrder,"FIND") == 0){
				if (!res){
					printf("%s %d -> %c\n",currData->currOrder,currData->key,currData->val);
				}
				else{
					printf("%s %d -> %s\n",currData->currOrder,currData->key,"FALSE");
				}
			}
			else{ //for INSERT
				if (!res){
					printf("%s %d %c -> %s\n",currData->currOrder,currData->key,currData->val,"TRUE");
				}
				else{
					printf("%s %d %c -> %s\n",currData->currOrder,currData->key,currData->val,"FALSE");
				}
			}
			*/
		pthread_mutex_unlock(&writeToOutputLock);
	}

	return;
};



