/* 
 * File:   config.h
 * Author: Rubin 
 *
 * Created on April 7, 2025, 6:52 PM
 */

#ifndef CONFIG_H
#define	CONFIG_H

#include "xc.h"          
#include "stdint.h"      
#include "stdbool.h"     
#include "stdio.h"     
#include "string.h"     
#include "stddef.h"  

#ifdef __cplusplus
extern "C" { 
#endif

/* Clock Configuration */
#define FCY (72000000UL)  // Instruction cycle frequency (72 MHz)
    
// Timing intervals
#define TIMER1_PERIOD_MS      10    // Main system tick interval
#define LED_BLINK_INTERVAL_MS 500   // LED toggle interval 
#define DATA_READ_INTERVAL    400   // Data reading interval
#define YAW_SEND_INTERVAL_MS  200   // Yaw data transmission interval
    
// Derived counts
#define DATA_READ_TICKS (DATA_READ_INTERVAL/TIMER1_PERIOD_MS)       // SPI datat read tick rate
#define LED_BLINK_TICKS (LED_BLINK_INTERVAL_MS / TIMER1_PERIOD_MS)  // LED blink tick rate
#define YAW_SEND_TICKS  (YAW_SEND_INTERVAL_MS / TIMER1_PERIOD_MS)   // YAW send tick rate

/* Hardware Pin Mapping */
// LEDs
#define LED1 LATAbits.LATA0   // LED 1 definition
#define LED2 LATGbits.LATG9   // LED 2 definition
// Buttons
#define BUTTON1 PORTEbits.RE8 // Button 1 definition
#define BUTTON2 PORTEbits.RE9 // Button 2 definition

/* System Initialization */
void config_init();  // Initialize clock, ports, peripherals

#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */

