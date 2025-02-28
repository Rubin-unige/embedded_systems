/*
 * File:   main_blinky.c
 * Author: Rubin Khadka Chhetri
 *
 * Created on February 22, 2025, 12:48 PM
 */

#include "xc.h"
#include "stdint.h"
#include "stdio.h"

#define FCY 4000000UL // Define System Clock
#include <libpic30.h>

uint8_t buttonState = 0; // Check status of button

int main(void) {
    // Disable analog functionality of all the pins
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    // Configure RA0 as Output (LED)
    TRISAbits.TRISA0 = 0; 
    LATAbits.LATA0 = 0; 
    
    // Configure RE8 as Input (Button)
    TRISEbits.TRISE8 = 1;  
    
    while(1){
        if (PORTEbits.RE8 == 0){ // If button is pressed
            __delay_ms(20); // Debounce delay
            if (PORTEbits.RE8 == 0) { // Confirm stable press
                if (!buttonState) { 
                    buttonState = 1; // Set button pressed state
                    LATAbits.LATA0 ^= 1; // Toggle LED
                }
            }
        } else {
            buttonState = 0; // Reset state when button is released
        }
        __delay_ms(20); // Short delay for stability
    }
    
    return 0;
}
