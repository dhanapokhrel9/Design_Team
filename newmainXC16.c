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


void ms_delay(int N) // miliseocnd dlay 
{
    TMR2 = 0; //clear timer,  use timer 2 since timer 1 is used by us delay 
    T2CON = 0x8030; // pre scale of 256
    while (TMR2 < 62.5 * N) // 0.001/(1/16,000,000)*256)=62.5
    {
    }
}

#define RTS _RF13 // Output, For potential hardware handshaking.
#define CTS _RF12 // Input, For potential hardware handshaking.

char ReadLCD(int addr) {
    // As for dummy read, see 13.4.2, the first read has previous value in PMDIN1
    int dummy;
    while (PMMODEbits.BUSY); // wait for PMP to be available
    PMADDR = addr; // select the command address
    dummy = PMDIN1; // initiate a read cycle, dummy
    while (PMMODEbits.BUSY); // wait for PMP to be available
    return ( PMDIN1); // read the status register
} // ReadLCD
// In the following, addr = 0 -> access Control, addr = 1 -> access Data
#define BusyLCD() ReadLCD( 0) & 0x80 // D<7> = Busy Flag
#define AddrLCD() ReadLCD( 0) & 0x7F // Not actually used here
#define getLCD() ReadLCD( 1) // Not actually used here.

void WriteLCD(int addr, char c) {
    while (BusyLCD());
    while (PMMODEbits.BUSY); // wait for PMP to be available
    PMADDR = addr;
    PMDIN1 = c;
} // WriteLCD
// In the following, addr = 0 -> access Control, addr = 1 -> access Data
#define putLCD( d) WriteLCD( 1, (d))
#define CmdLCD( c) WriteLCD( 0, (c))
#define HomeLCD() WriteLCD( 0, 2) // See HD44780 instruction set in
#define ClrLCD() WriteLCD( 0, 1) // Table 9.1 of text book


void InitLCD(void) {
    // PMP is in Master Mode 1, simply by writing to PMDIN1 the PMP takes care
    // of the 3 control signals so as to write to the LCD.
    PMADDR = 0; // PMA0 physically connected to RS, 0 select Control register
    PMDIN1 = 0b00111000; // 8-bit, 2 lines, 5X7. See Table 9.1 of text Function set
    ms_delay(1); // 1ms > 40us
    PMDIN1 = 0b00001100; // ON, cursor off, blink off
    ms_delay(1); // 1ms > 40us
    PMDIN1 = 0b00000001; // clear display
    ms_delay(2); // 2ms > 1.64ms
    PMDIN1 = 0b00000110; // increment cursor, no shift
    ms_delay(2); // 2ms > 1.64ms
} // InitLCD

void InitPMP(void) {
    // PMP initialization. See my notes in Sec 13 PMP of Fam. Ref. Manual
    PMCON = 0x8303; // Following Fig. 13-34. Text says 0x83BF (it works) *
    PMMODE = 0x03FF; // Master Mode 1. 8-bit data, long waits.
    PMAEN = 0x0001; // PMA0 enabled
}


void putsLCD(char *s) {
    while (*s) putLCD(*s++); // See paragraph starting at bottom, pg. 87 text
} //putsLCD

void SetCursorAtLine(int i) {
    int k;
    if (i == 1)
        CmdLCD(0x80); // Set DDRAM (i.e., LCD) address to upper left (0x80 | 0x00)
    else if (i == 2)
        CmdLCD(0xC0); // Set DDRAM (i.e., LCD) address to lower left (0x80 | 0x40)
    else {
        TRISA = 0x00; // Set PORTA<7:0> for output.
        for (k = 1; k < 20; k++) // Flash all 7 LED's @ 5Hz. for 4 seconds.
        {
            PORTA = 0xFF;
            ms_delay(100); // 100 ms for ON then OFF yields 5Hz
            PORTA = 0x00;
            ms_delay(100);
        }
    }
}

void InitU2(void) {
    U2BRG = 415; // PIC24FJ128GA010 data sheet, 17.1 for calculation, Fcy = 16MHz.
    U2MODE = 0x8008; // obtained from data sheet, pg 148. Enable UART2, BRGH = 1,
    // Idle state = 1, 8 data, No parity, 1 Stop bit
    U2STA = 0x0400; // See data sheet, pg. 150, Transmit Enable
    // Following lines pertain Hardware handshaking
     TRISFbits.TRISF13 = 1; // enable RTS , output
    RTS = 1; // default status , not ready to send
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

    IncData = getU2();

    ms_delay(32); // At least 30ms for LCD Internal Initialization
    InitPMP(); // Initialize the Parallel Master Port
    InitLCD(); // Initialize the LCD
  //  putsLCD("Hello World"); // Put message on first line of LCD
    // main loop, empty for now
   
    ms_delay(1000); // 2 seconds on 
    ms_delay(1000);
    
    while (1) {
        
           // sets cursor at line 1
            if (IncData){
                 SetCursorAtLine(1); 
                putsLCD(IncData);
                 PORTBbits.RB0 = 1;
            }
            else if (!IncData)
                PORTBbits.RB0 = 0;

        
        CmdLCD(0x0C);
        ms_delay(1000);
        ms_delay(1000);
        CmdLCD(0x08);
        ms_delay(1000);
        
        
    }

    return 0;
}
