/*
 * File:   newmainXC16.c
 * Author: dadab
 *
 * Created on November 3, 2021, 10:02 AM
 */


#include "config.h"
#include "xc.h"
#include<stdio.h>
#include<stdlib.h>

void us_delay(int time) // micro seocnd delay
{
    TMR1=0;
    T1CON=0x8010; // pre scale of 8
    while (TMR1<2*time) // 0.000001/(1/16,000,000)*8)=2
    {
    }
}

void ms_delay(int N)    // miliseocnd dlay 
{
    TMR2=0;         //clear timer,  use timer 2 sice timer 1 is used by us delay 
    T2CON=0x8030; // pre scale of 256
    while(TMR2<62.5*N)  // 0.001/(1/16,000,000)*256)=62.5
    {
    }
}

int main(void) {
    
    TRISB = 0;
    
    while(1){
        PORTBbits.RB0 = 1;
        ms_delay(100);
        PORTBbits.RB0 = 0;
        ms_delay(100);
      
    }
    
    
    
    return 0;
}

