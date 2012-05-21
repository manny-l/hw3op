#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "b5-9.h"

#define MAX_ORDER_SIZE 10
#define INIT_ARR_SIZE 25

//barrier struct
struct barrierData{
	int threadNum;
	int threadsInBarrier;
	sem_t enterBarrier;
	sem_t leaveBarrier;
};

//struct for parameters for threads
typedef struct threadData_t{
	char currOrder[MAX_ORDER_SIZE];
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
tree currTree;

//functions declarations
int initBarrierStruct(int threadNum);
int getOrderBatch();
void runOrders(threadDataPnt orderList[],int threadNum);
void* newThreads(void* data);
void barrier();
void freeCurrList(threadDataPnt orderList[],int threadsNum);
int initLocks();
void freeAndDestroy();

/*----------------------------------------------------------------
The Main Thread:
Description: call getOrderBatch in a loop
			 until the word END is read
-----------------------------------------------------------------*/
int main(){

	int i,res = 0;
	char currOrder[MAX_ORDER_SIZE];

	//start reading data from stdin
	scanf ("%s",currOrder);

	//checking if first line is: BEGIN
	if (strcmp(currOrder,"BEGIN") != 0){
		printf("Error, first order is not BEGIN!\n");
		return (-1);
	}
	else{
		printf("BEGIN\n");
	}

	//locks initializing
	if (-1 == initLocks()){
		printf("Error, lock initializing failed!\n");
		return -1;
	}

	//tree initializing
	if (-1 ==  initTree(&currTree)){
		printf("Error, tree initializing failed!\n");
		return (-1);
	}

	//executing batches of orders until END is read
	while (0 == res){
		res = getOrderBatch();
	}

	//free tree & detroy locks
	freeAndDestroy();

	return res;
}

void freeAndDestroy(){
	//free the tree
	freeTree(currTree);

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
Output: 0 - if the last order in the batch was Barrier
		1 - if the last order in the batch was End
	   -1 - for errors
Description: - reading the orders of a single section
			 - saving the orders in orderList
			 - initializing the BarrierStruct
			 - calling the function runOrders
-----------------------------------------------------------------*/
int getOrderBatch(){

	//local parameters
	int arrSize = INIT_ARR_SIZE;
	threadDataPnt* orderList;
	char order[MAX_ORDER_SIZE];
	threadDataPnt currData;
	int threadsNum = 0;
	int ind;

	//reading the first order
	scanf ("%s",order);
	if (!strcmp(order,"BARRIER")){
		printf("BARRIER\n");
		return 0;
	}
	if (!strcmp(order,"END")){
		printf("END\n");
		return 1;
	}

	//allocating a list for the data of the orders
	orderList = (threadDataPnt*)malloc(arrSize*sizeof(threadDataPnt));
	if(orderList == NULL){
		printf("Error in memory allocation\n");
		exit(0);
	}

	//reading data while the section is not finished
	while(strcmp(order,"BARRIER") && strcmp(order,"END")){

		//allocated data struct for order data
		currData = (threadDataPnt)malloc(sizeof(struct threadData_t));
		if (currData == NULL){
			printf("Error in memory allocation\n");
			freeCurrList(orderList,threadsNum);
			exit(0);
		}

		//insert order data for thread
		strcpy(currData->currOrder,order);

		if (strcmp(order,"INSERT")==0){
			scanf ("%d %c",&(currData->key),&(currData->val));
		}
		else{ // ORDER=FIND
			scanf ("%d",&(currData->key));
		}

		//Enlarging data array if it is too small
		if (threadsNum == arrSize){
			arrSize = arrSize*2;
			orderList = (threadDataPnt*)realloc
						(orderList,arrSize*sizeof(threadDataPnt));
			if (orderList == NULL){
				freeCurrList(orderList,threadsNum);
				printf("Error in memory allocation\n");
				return -1;
			}
		}


		//inserting data to orderList as array
		orderList[threadsNum] = currData;

		//counting the num of orders
		threadsNum++;

		//reading the next order
		scanf ("%s",order);
	}

	if (threadsNum > 0){

		//initialize the Barrier Struct
		if (-1 == initBarrierStruct(threadsNum)){
			freeCurrList(orderList,threadsNum);
			return -1;
		}

		//run the orders
		runOrders(orderList,threadsNum);

		//wait for all the threads to end
		for (ind = 0; ind < threadsNum; ind++){
			if(orderList[ind]->threadID != -1){
				pthread_join(orderList[ind]->threadID,NULL);
			}
		}

		//free list
		freeCurrList(orderList,threadsNum);
	}
	else{
		free(orderList);
	}

	//RETURN VALUES
	if (!strcmp(order,"BARRIER")){
		printf("BARRIER\n");
		return 0;
	}

	if (!strcmp(order,"END")){
		printf("END\n");
		return 1;
	}
	return -1;
}

/*----------------------------------------------------------------
freeCurrList function:
Input: orderList = list of orders
	   threadNum = number of orders in the list
Output: none
Description: frees the order list
-----------------------------------------------------------------*/
void freeCurrList(threadDataPnt orderList[],int threadsNum){

	int ind;
	threadDataPnt currData;
	//free the orders in the order list
	for (ind = 0; ind < threadsNum; ind++){
		currData = orderList[ind];
		free(currData);
		orderList[ind] = 0;
	}

	//free the the order list
	free(orderList);
}


/*----------------------------------------------------------------
runOrders function:
Input: orderList = list of orders
	   threadNum = number of orders in the list
Output: none
Description: creates a thread for every order in the list and sends
		it to the newThreads function with the relevant order data
-----------------------------------------------------------------*/
void runOrders(threadDataPnt orderList[],int threadNum){

	pthread_t threadIndex;
	int ind;

	for (ind = 0; ind < threadNum; ind++){

		//allocate thread
		if (pthread_create(&(orderList[ind]->threadID),NULL,newThreads,orderList[ind])){
			printf ("Error in thread creation\n");
			orderList[ind]->threadID = -1;
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
Input: data struct containing: order,key & val
Output: none
Description: waits in the barrier for the other threads.
	And then performs the order and prints the result to stdout
-----------------------------------------------------------------*/
void* newThreads(void* data){

	int res;
	//waiting for all the other threads
	barrier();

	if (data != NULL){

		//converting data struct
		threadDataPnt currData = (threadDataPnt)data;

		//find/insert in tree
		if (strcmp(currData->currOrder,"FIND") == 0){
			res = findElm(currTree,currData->key,&(currData->val));
		}
		else{
			res = addElm(currTree,currData->key,currData->val);
		}

		if (-2==res){
			printf("Fatal error in find or add functions\n");
			exit(0);
		}

		//write result to output
		pthread_mutex_lock(&writeToOutputLock);
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
		pthread_mutex_unlock(&writeToOutputLock);
	}

	return;
};



