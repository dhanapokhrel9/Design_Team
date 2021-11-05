/*
 * File:   newmainXC16.c
 * Author: Dhanapati
 *
 * Created on October 27, 2021, 2:53 PM
 */


#include "config.h"
#include "xc.h"
#include<stdio.h>
#include<stdlib.h>

char IncData;


#define RTS _RF13 // Output, For potential hardware handshaking.
#define CTS _RF12 // Input, For potential hardware handshaking.

void us_delay(int time) // micro seocnd delay
{
    TMR1 = 0;
    T1CON = 0x8010; // pre scale of 8
    while (TMR1 < 2 * time) // 0.000001/(1/16,000,000)*8)=2
    {
    }
}

void ms_delay(int N) // miliseocnd dlay 
{
    TMR2 = 0; //clear timer,  use timer 2 since timer 1 is used by us delay 
    T2CON = 0x8030; // pre scale of 256
    while (TMR2 < 62.5 * N) // 0.001/(1/16,000,000)*256)=62.5
    {
    }
}

void InitU2(void) {
    U2BRG = 415; // PIC24FJ128GA010 data sheet, 17.1 for calculation, Fcy = 16MHz.
    U2MODE = 0x8008; // obtained from data sheet, pg 148. Enable UART2, BRGH = 1,
    // Idle state = 1, 8 data, No parity, 1 Stop bit
    U2STA = 0x0400; // See data sheet, pg. 150, Transmit Enable
    // Following lines pertain Hardware handshaking
    // TRISFbits.TRISF13 = 1; // enable RTS , output
    //RTS = 1; // default status , not ready to send
}

char putU2(char c) {
    // while (CTS); //wait for !CTS (active low)
    while (U2STAbits.UTXBF); // Wait if transmit buffer full.
    U2TXREG = c; // Write value to transmit FIFO
    return c;
}

char getU2(void) {
    // RTS = 0; // telling the other side !RTS
    while (!U2STAbits . URXDA); // wait
    // RTS = 1; // telling the other side RTS
    return U2RXREG; // from receiving buffer
} //getU2

int main(void) {

    TRISB = 0X00; // output
    InitU2(); // Initialize UART module at 9600 bps
    ms_delay(100); // Wait for UART module to stabilize
    putU2(" Bluetooth starting");

    IncData = U2RXREG;

    while (1) {
        while (!getU2()) {
            break;
        }
        if (getU2()) {
            if (IncData == 'A')
                PORTBbits.RB0 = 1;
            if (IncData == 'a')
                PORTBbits.RB0 = 0;

        }

    }

    return 0;
}
