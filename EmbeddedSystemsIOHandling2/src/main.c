#include <cmsis_os2.h>
#include <LPC17xx.h>
#include <stdio.h>
#include <stdlib.h>

__NO_RETURN void Poll_Joystick(void *arg){
	
	// configure LED bits as output	
	// Port 2 bits 2,3,4,5,6
	LPC_GPIO2->FIODIR = 0x7C;
	
	while(1){
		
		uint32_t onBits = 0;
		//BUTTON PUSHED = 5th LED
		if ((LPC_GPIO1->FIOPIN & (1u << 20)) == 0){
			onBits |= (1u << 2);
        }
        //NORTH = 1
        if((LPC_GPIO1->FIOPIN & (1u << 23)) == 0){ 
			onBits |= (1u << 6);
        }
        //SOUTH = 3 
        else if((LPC_GPIO1->FIOPIN & (1u << 25)) == 0){
			onBits |= (1u << 6) | (1u << 5);
        }
        //EAST 2 
        else if((LPC_GPIO1->FIOPIN & (1u << 24)) == 0){
			onBits |= (1u << 5);
        }
        //WEST = 4
        else if((LPC_GPIO1->FIOPIN & (1u << 26)) == 0){
			onBits |= (1u << 4);
        }
        
		 // Clear Port 2 bits 2,3,4,5,6
		LPC_GPIO2->FIOCLR = 0x7C;
		LPC_GPIO2->FIOSET = onBits;
		
		// puts the calling thread to the back of its ready queue
		osThreadYield();   
    }
}

__NO_RETURN void Read_ADC(void *arg){
	
	// ADC Configuration
	// Power up the device
	LPC_SC->PCONP |= 1u << 12; 	// sets bit 12

	// Clock rate set to Cclk /8 = 12.5MHz in system configuration
	
	// Select pin function
	LPC_PINCON->PINSEL1 |= 1u << 18; // set bit 18
	
	// ADC Control Registers
	// Select AD0.2
	LPC_ADC->ADCR = 1u << 2;
	// Leave APB clock at 0 so that APB clock is 12.5/1 = 12.5MHz
	
	// Select ADC converter to be operational
	LPC_ADC->ADCR |= 1u << 21;
	
	while (1){	
		// Start ADC
		LPC_ADC->ADCR |= 1u << 24;
		
		//check if A/D conversion is done
		if((LPC_ADC->ADGDR & (1u << 31)) == (1u << 31)){
			//apply conversion
			float result = ((LPC_ADC->ADGDR & 0xFFF0) >> 4u) * 3.3/((1<<12)-1);
			printf("Voltage is %.2f\n", result);
			// puts the calling thread to the back of its ready queue
			osThreadYield();	
		}
	}	
}

__NO_RETURN void Pushbutton_Toggle(void * arg){

	// Port 1 bits 28,29,31
	LPC_GPIO1->FIODIR = 0xB0000000;
	
	int prevPress = 0; // variable to determine if the last time the button was checked it was pushed down
	
	while(1){
		// Check if button is pushed down (P2.10)
		if ((LPC_GPIO2->FIOPIN & (1u << 10)) == 0){
			//LPC_GPIO1->FIOSET = (1u << 28);
			prevPress = 1;
		}
		else{
			if(prevPress ==1){	// if prevPress is 1 that means that the LED needs to change from off to on or on to off
				if (LPC_GPIO1->FIOSET == (1u << 28)){
					LPC_GPIO1->FIOCLR = (1u << 28);
				}
				else{
					LPC_GPIO1->FIOSET = (1u << 28);
				}
			}
			prevPress = 0;
		}
		// puts the calling thread to the back of its ready queue
		osThreadYield();
	}	
}

__NO_RETURN void app_main(void * argument){

	// create other threads
	osThreadNew(Poll_Joystick, NULL, NULL);
	osThreadNew(Read_ADC, NULL, NULL);
	osThreadNew(Pushbutton_Toggle, NULL, NULL);
	
	for (;;)
	{}
}

int main(void){

	// System Initialization
	
	SystemCoreClockUpdate(); 	// update system core clock
	
	// ..... //
	
	osKernelInitialize(); 				// Initialize CMSIS-RTOS
	osThreadNew(app_main, NULL, NULL);	// Create application main thread
	osKernelStart();					// Start thread execution
	for(;;){}
}

