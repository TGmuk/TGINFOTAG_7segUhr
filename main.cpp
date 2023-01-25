/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "platform/mbed_thread.h"
#include <sstream>
#include <string>


// Blinking rate in milliseconds
#define BLINKING_RATE_MS                                                    1

#define Set_data_command 0b00100010
#define Display_ON 0b11110001
#define read_buttons 0b01000010

#define Dig0 0b00000011

int seg7[10]={0b11111100,0b01100000,0b11011010,
0b11110010,0b01100110,0b10110110,
0b10111110,0b11100000,0b11111110,
0b11110110};
int Dig[8]={ 0b00000011, 0b01000011, 0b00100011, 0b01100011, 0b00010011, 0b01010011, 0b00110011, 0b01110011};
int LED[8]={ 0b10000011, 0b11000011, 0b10100011, 0b11100011, 0b10010011, 0b11010011, 0b10110011, 0b11110011};


DigitalOut STB(PB_12);
PortOut tastenanzeige(PortC,0xFF);

SPI anzeige(PB_15,PB_14,PB_13);


void anzeigen(int Nr, int Ziffer)
{
            STB=0;
            anzeige.write(Set_data_command);
            STB=1;  
            
            STB=0;
            anzeige.write(Dig[Nr]);
            anzeige.write(seg7[Ziffer]);
            STB=1;  
}

void ledschalten(int Nr, bool ein)
{
            STB=0;
            anzeige.write(Set_data_command);
            STB=1;  
            STB=0;
            anzeige.write(LED[Nr]);
            if (ein==true) anzeige.write(0x80);
            else anzeige.write(0);
            STB=1;   
}



int counter = 0;
int ledCounter = 0b10000000;
int millis, seconds, minutes, hours;

string strCounter;


void tim6isr(){


    millis++;
    if(millis > 999){
        seconds++;
        millis = 0;

    }
    if(seconds >= 60){
        minutes++;
        seconds = 0;

    }
    if(minutes >= 60){
        hours++;
        minutes = 0;

    }
    if(hours >= 10){
        hours = 0;

    }



    TIM6 -> SR = 0;
    HAL_NVIC_ClearPendingIRQ(TIM6_IRQn);
}

void tim7isr(){
    ledCounter = ledCounter >> 1;

    if(ledCounter <= 0){
        ledCounter = 0b10000000;
    }

    


    TIM7 -> SR = 0;
    HAL_NVIC_ClearPendingIRQ(TIM7_IRQn);
}




int main()
{
    int b0, b1, b2, b3;
    DigitalInOut MOSI(PB_15);
    MOSI.mode(OpenDrain);
    STB=1;
    DigitalOut led(LED1);
    anzeige.format(8,3);
    anzeige.frequency(100000);
    
    STB=0;
    anzeige.write(Display_ON);
    STB=1;

    RCC -> APB1ENR |=0b10000;
    TIM6 -> PSC = 31;
    TIM6 -> ARR = 1000;
    TIM6 -> CNT = 0;
    TIM6 -> SR = 0;

    TIM6 -> DIER = 1;
    NVIC_SetVector(TIM6_IRQn, (uint32_t) &tim6isr);
    HAL_NVIC_EnableIRQ(TIM6_IRQn);

    TIM6 -> CR1 = 1;


    RCC -> APB1ENR |=0b100000;
    TIM7 -> PSC = 31999;
    TIM7 -> ARR = 50;
    TIM7 -> CNT = 0;
    TIM7 -> SR = 0;

    TIM7 -> DIER = 1;
    NVIC_SetVector(TIM7_IRQn, (uint32_t) &tim7isr);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);

    TIM7 -> CR1 = 1;

    
    while (true) {
        
        STB=0;
        
        anzeigen(0, hours);
        anzeigen(1, minutes / 10);
        anzeigen(2, minutes % 10);
        anzeigen(3, seconds / 10);
        anzeigen(4, seconds % 10);
        anzeigen(5, millis / 100 );
        anzeigen(6, millis / 10);
        anzeigen(7, millis % 10);

        for(int i=0; i<8;i++){
            
            ledschalten(i, (ledCounter >> i) & 1);
        }

        

        
        
        STB=1; 

    }
}