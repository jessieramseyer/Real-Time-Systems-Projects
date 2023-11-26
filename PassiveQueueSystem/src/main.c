#include <cmsis_os2.h>
#include <LPC17xx.h>
#include <stdio.h>
#include <stdlib.h>
#include "random.h"
#include "math.h"

#define MAX_MSGS 10
#define NUM_QUEUES 2
#define SERVICE_RATE 10
#define ARRIVAL_RATE 9

typedef struct{
	
	osMessageQueueId_t qid;
	int messages_received;
	int messages_sent;
	int overflows;
	float server_sleep_time;
	
} queue_t;

__NO_RETURN void clientThread(void * args) {
	
	// osThreadNew takes a void pointer as its argument, but we are trying to pass an array of pointers to queues so we need to assign it to that
	// converting void* to queue_t pointer which is a struct
	queue_t* queueStruct = (queue_t*)args;
	
	int msg = 1;
	
	while(1){
		
		//generate exponentially distributed random number with arrival rate lambda
		float random_s = (next_event()/ARRIVAL_RATE) /pow(2,16);
		
		//convert random number to number of osticks
		int nticks = random_s * osKernelGetTickFreq();
		
		//call osDelay() with the converted number of osTicks
		osDelay(nticks);
		
		//send message to the queue
		// will return osErrorResource if not enough space
		if (osMessageQueuePut(queueStruct->qid, (void*) &msg, 0,0) != osErrorResource)
		{
			queueStruct->messages_sent ++;
		}
		else queueStruct->overflows++;
		
		// puts the calling thread to the back of its ready queue
		osThreadYield(); 
	}	
}

__NO_RETURN void serverThread(void * args){
	
	// osThreadNew takes a void pointer as its argument, but we are trying to pass an array of pointers to queues so we need to assign it to that
	// converting void* to queu_t pointer which is a struct
	queue_t *queueStruct = (queue_t*)args;
	
	int msg;
	
	while(1){
		//generate exponentially distributed random number with service rate
		float random_s = (next_event()/SERVICE_RATE) / pow(2,16);
		queueStruct->server_sleep_time += random_s;
		
		// convert random number to osTicks
		int nticks = random_s * osKernelGetTickFreq();
		
		//call osDelay() with the converted number of osTicks
		osDelay(nticks);
		
		//receive a message from the queue
		osMessageQueueGet(queueStruct->qid, (void*) &msg, NULL, osWaitForever);
		queueStruct->messages_received++;
		
		// puts the calling thread to the back of its ready queue
		osThreadYield(); 
	}
}

void client(queue_t queues[]){
	
	for (int i =0; i <NUM_QUEUES; i ++){
		osThreadNew(clientThread, &queues[i], NULL);	// create client threads
	}
}

void server(queue_t queues[]){

	for (int i =0; i <NUM_QUEUES; i ++){
		osThreadNew(serverThread, &queues[i], NULL);	// create server threads
	}
}

__NO_RETURN void monitorThread(void * args){
	
	// osThreadNew takes a void pointer as its argument, but we are trying to pass an array of pointers to queues so we need to assign it to that
	// converting void* to queue_t pointer which is an array of structs
	queue_t *queueStructs = (queue_t*)args;
	
	// elapsed time in seconds
	int elapsed_time =0;
	
	float rho = (float)ARRIVAL_RATE/SERVICE_RATE;
	float true_loss = pow(rho, 10)*(1-rho)/(1-pow(rho, 11));

	while(1){
		
		// every 20 seconds print the table heading
		if(elapsed_time % 20 == 0)
			printf("Qid, Time, Sent, Recv, Over, Wait,   P_blk,    Arrv,    Serv,    Epblk,   Earrv,   Eserv\n"); 
		
		//print message for each queue
		for (int i  = 0; i < NUM_QUEUES; i++){
			
			//queue id
			printf(" Q%d,", i);
			
			// need elapsed time in seconds
			printf("%5d,", elapsed_time);
			
			//total number of messages sent
			printf("%5d,", queueStructs[i].messages_sent);
			//total number of messages received
			printf("%5d,", queueStructs[i].messages_received);
			
			//total number of overflows
			printf("%5d,", queueStructs[i].overflows);
			
			//current number of messages in the queue (Wait)
			printf("%5d,", osMessageQueueGetCount(queueStructs[i].qid));
			
			//message loss ratio (Pblk)
			float message_loss = (float)queueStructs[i].overflows/queueStructs[i].messages_sent;
			printf("%8.4f,", message_loss);
			
			//average message arrival rate
			float avg_arrival = (float)queueStructs[i].messages_sent/elapsed_time;
			printf("%8.4f,", avg_arrival);
			
			//average service rate
			float avg_service = (float)queueStructs[i].messages_received/queueStructs[i].server_sleep_time;
			printf("%8.4f,", avg_service);
			
			//calculating message loss error
			float loss_error = (float)(message_loss-true_loss)/true_loss;
			printf("%9.4f,", loss_error);
			
			//calculating arrival rate error
			float arrival_error = (float)(avg_arrival-ARRIVAL_RATE)/ARRIVAL_RATE;
			printf("%8.4f,", arrival_error);
			
			// calculation service rate error
			float service_error = (float)(avg_service-SERVICE_RATE)/SERVICE_RATE;
			printf("%8.4f,", service_error);
			
			printf("\n");
			
			// puts the calling thread to the back of its ready queue
			osThreadYield();
		}
		//add 1 second delay
		osDelay(osKernelGetTickFreq());
		elapsed_time++;
	}
}

int main(void){
	// System Initialization
	
	SystemCoreClockUpdate(); 	// update system core clock
	
	// ..... //
	
	osKernelInitialize(); 				// Initialize CMSIS-RTOS
	
	// Create N queues
	queue_t queues[NUM_QUEUES];
	for( int i = 0; i < NUM_QUEUES; i ++){
		queues[i].qid = osMessageQueueNew(10, sizeof(int), NULL);
	}
	
	client(queues);
	server(queues);
	osThreadNew(monitorThread, queues, NULL); // create monitor output thread
	
	osKernelStart();					// Start thread execution
	for(;;){}

}

