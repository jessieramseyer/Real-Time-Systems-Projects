#include <stdlib.h>
#include <stdio.h>
#include <LPC17xx.h>

#define Part1

#ifdef Part1
int main(){
	
	// turn LED P1.28 on when button down and off when 
	
	// configure LED bits as output
	// Port 1 bits 28,29,31
	LPC_GPIO1->FIODIR = 0xB0000000; // equivalent to LPC_GPIO1->FIODIR = (1u << 28) | (1u << 29) | (1u << 31);
	
	// Port 2 bits 2,3,4,5,6
	LPC_GPIO2->FIODIR = 0x7C;
	
	
	while(1){
	// Check if button is pushed down (P2.10)
		if ((LPC_GPIO2->FIOPIN & (1u << 10)) == 0){
			LPC_GPIO1->FIOSET = (1u << 28);
		}
		else{
			LPC_GPIO1->FIOCLR = (1u << 28);
		}
	}	
}
#endif

#ifdef Part2
int main(){
	
	while(1){
        //NORTH
        if((LPC_GPIO1->FIOPIN & (1u << 23)) == 0){ 
            printf("North and ");
        }
        //SOUTH
        else if((LPC_GPIO1->FIOPIN & (1u << 25)) == 0){
            printf("South and ");
        }
        //EAST
        else if((LPC_GPIO1->FIOPIN & (1u << 24)) == 0){
            printf("East and ");
        }
        //WEST
        else if((LPC_GPIO1->FIOPIN & (1u << 26)) == 0){
            printf("West and ");
        }
		else{
			printf("Center and ");
		}
        //BUTTON PUSHED
        if ((LPC_GPIO1->FIOPIN & (1u << 20)) == 0){
            
            printf("joystick is pressed \n");
        }
        //BUTTON NOT PUSHED
        else{
            printf("joystick is not pressed \n");
        }
    }
}
#endif

#ifdef Part3
int main(){
	// configure LED bits as output
    // Port 1 bits 28,29,31
    LPC_GPIO1->FIODIR = 0xB0000000; // equivalent to LPC_GPIO1->FIODIR = (1u << 28) | (1u << 29) | (1u << 31);
    
    // Port 2 bits 2,3,4,5,6
    LPC_GPIO2->FIODIR = 0x7C;
        
    while(1){
        int a[8] = {0,0,0,0,0,0,0,0};
		int n=0;    
        printf("Enter the number to convert: ");    
        scanf("%d",&n);
            
		//clear LED bits
		LPC_GPIO1->FIOCLR = 0xB0000000; // Port 1 bits 28,29,31
		LPC_GPIO2->FIOCLR = 0x7C; // Port 2 bits 2,3,4,5,6
            
        for(int i=0;n>0;i++)    
        {    
            a[i]=n%2;    
            n=n/2; 
        }
                
		for(int i=7;i>=0;i--)    
		{    
			printf("%d",a[i]);    
		} 
		printf("\n");
                
        if(a[7])
        {
            LPC_GPIO1->FIOSET = (1u << 28);
        }
        if(a[6])
        {
            LPC_GPIO1->FIOSET = (1u << 29);
        }
        if(a[5])
        {
            LPC_GPIO1->FIOSET = (1u << 31);
        }
        if(a[4])
        {
            LPC_GPIO2->FIOSET = (1u << 2);
        }
        if(a[3])
        {
            LPC_GPIO2->FIOSET = (1u << 3);
        }
        if(a[2])
        {
            LPC_GPIO2->FIOSET = (1u << 4);
        }
        if(a[1])
        {
            LPC_GPIO2->FIOSET = (1u << 5);
        }
        if(a[0])
        {
            LPC_GPIO2->FIOSET = (1u << 6);
        }
    }	
}
#endif

#ifdef Part4
int main(){
	// ADC Configuration
	// Power up the device
	LPC_SC->PCONP |= 1u << 12; 	// sets bit 12
	
	// Select the peripheral clock
	// Set clock rate to Cclk /8 = 12.5MHz
	// LPC_SC->PCLKSEL0 |= (1u << 24) | (1u << 25); 
	
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
			
			//should 3.5 display 3.3 V?
			
			printf("Voltage is %.2f\n", result);
		}			
	}	
}
#endif

