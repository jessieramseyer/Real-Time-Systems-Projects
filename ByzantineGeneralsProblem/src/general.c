
#include <cmsis_os2.h>
#include "general.h"

// add any #includes here
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"

// add any #defines here
#define QUEUE_SIZE 20
#define MAX_GENERALS 7

// add global variables here
 
osSemaphoreId_t broadcastSem;
osSemaphoreId_t turnstile1, turnstile2;
osMutexId_t mutex;
int count;
uint8_t N_Generals;
uint8_t m;
uint8_t Reporter;

osMessageQueueId_t general_queues[MAX_GENERALS][3];

bool loyalty[MAX_GENERALS];


/** Record parameters and set up any OS and other resources
  * needed by your general() and broadcast() functions.
  * nGeneral: number of generals
  * loyal: array representing loyalty of corresponding generals
  * reporter: general that will generate output
  * return true if setup successful and n > 3*m, false otherwise
  */
bool setup(uint8_t nGeneral, bool loyal[], uint8_t reporter) {
    
	// recording parameters into globals
	m = 0;
	count =0;
	// may need more than one semaphore for synchronization
	turnstile1 = osSemaphoreNew(1,0,NULL);
	turnstile2 = osSemaphoreNew(1,1,NULL);
	broadcastSem = osSemaphoreNew(1,0,NULL);
	mutex = osMutexNew(NULL);
	
	N_Generals = nGeneral;
	Reporter = reporter;
	
	for(int i = 0; i < nGeneral; i ++) {
		
		for(uint8_t j = 0; j < 3; j ++){
			general_queues[i][j] = osMessageQueueNew(10, 7, NULL);
		}
		loyalty[i] = loyal[i];
		
		if(loyal[i] == false) {
			m++;
		}
	}
	
	if(!c_assert(nGeneral >3*m)) return false;
	
	return true;
}
void reusableBarrier(int order){
	
	osMutexAcquire(mutex, osWaitForever);
	count ++;
	if (count == N_Generals-1)
	{
		osSemaphoreAcquire(turnstile2, osWaitForever);
		osSemaphoreRelease(turnstile1);
	}
	osMutexRelease(mutex);
	osSemaphoreAcquire(turnstile1, osWaitForever);
	osSemaphoreRelease(turnstile1);
	
	osMutexAcquire(mutex, osWaitForever);
	count --;
	if (count == 0)
	{
		osSemaphoreAcquire(turnstile1, osWaitForever);
		osSemaphoreRelease(turnstile2);
	}
	osMutexRelease(mutex);
	osSemaphoreAcquire(turnstile2, osWaitForever);
	osSemaphoreRelease(turnstile2);
}

void OM(uint8_t order, char* command, uint8_t id){
	
	char newcommand[7];
	
	if(order >0){
		
		if(loyalty[id] == false){
			char traitorCommand = id % 2 == 0 ? 'R' : 'A';
			*strstr(command, "R") = traitorCommand;
			*strstr(command, "A") = traitorCommand;
			sprintf(newcommand, "%u:%s", id, command);
		}
		else
			sprintf(newcommand, "%u:%s", id, command);
		
		for(int i  = 0; i < N_Generals; i++)
		{
			char n[1];
			sprintf(n, "%d", i);
			const char* nn = n;
			const char* commandc = newcommand;
			
			if(strstr(commandc, nn)== 0)
			{
				 osMessageQueuePut(general_queues[i][order], newcommand, 0, 0);
			}			
		}
		
		reusableBarrier(order);
		
		char message[7];
		
		while(osMessageQueueGet(general_queues[id][order], message, 0, 0) == osOK){
			OM(order-1, message, id);
		}

	}
	else{
		if(id == Reporter){
			// print message
			printf("%s\n", command);
			}
		}
}


/** Delete any OS resources created by setup() and free any memory
  * dynamically allocated by setup().
  */
void cleanup(void) {
	
	// delete semaphore and message queues
	
	osSemaphoreDelete(broadcastSem);
	osSemaphoreDelete(turnstile1);
	osSemaphoreDelete(turnstile2);
	osMutexDelete(mutex);
	for(uint8_t i =0; i < N_Generals; i ++)
	{
		for(uint8_t j = 0; j < 3; j++){ 
		osMessageQueueDelete(general_queues[i][j]);
		}
	}
}


/** This function performs the initial broadcast to n-1 generals.
  * It should wait for the generals to finish before returning.
  * Note that the general sending the command does not participate
  * in the OM algorithm.
  * command: either 'A' or 'R'
  * sender: general sending the command to other n-1 generals
  */
void broadcast(char command, uint8_t commander) {
	
	char commanderCommand[3];
	for(int i = 0; i < N_Generals; i++){
		if(loyalty[commander]== false)
		{
			command = i %2 == 0 ? 'R' : 'A';
		}
		if ( i != commander)
		{
			sprintf(commanderCommand, "%u:%c", commander, command);
			osMessageQueuePut(general_queues[i][m], commanderCommand, 0,0);
		}	
	}
	
	osSemaphoreAcquire(broadcastSem, osWaitForever);
}


/** Generals are created before each test and deleted after each
  * test.  The function should wait for a value from broadcast()
  * and then use the OM algorithm to solve the Byzantine General's
  * Problem.  The general designated as reporter in setup()
  * should output the messages received in OM(0).
  * idPtr: pointer to general's id number which is in [0,n-1]
  */
void general(void *idPtr) {
    uint8_t id = *(uint8_t *)idPtr;
	
	char command[7];
	osMessageQueueGet(general_queues[id][m], command, NULL, osWaitForever);
	OM(m, command, id);
	reusableBarrier(m);
	osSemaphoreRelease(broadcastSem);
	
}
