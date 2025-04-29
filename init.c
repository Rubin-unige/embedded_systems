

#include "SPI.h"

/* Initial Configurations definitions */
void config_init(){
    
    /* Disable analog functionality of all the pins */
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    
    /* GPIO Direction Configuration */
    // LED
    TRISAbits.TRISA0 = 0;  // Set RA0 as output
    TRISGbits.TRISG9 = 0;  // Set RG9 as output
    
    // BUTTON
    TRISEbits.TRISE8 = 1;  // Set RE8 as input
    TRISEbits.TRISE9 = 1;  // Set RE9 as input
    
    // UART Pins
    TRISDbits.TRISD0 = 0;  // Set RD0 (RP64) as output
    TRISDbits.TRISD11 = 1; // Set RD11 (RP75) as input
    
    //SPI Pins
    TRISAbits.TRISA1 = 1;    // MISO (RA1 = RP17)  
    TRISFbits.TRISF12 = 0;   // SCK (RF12 = RP108)  
    TRISFbits.TRISF13 = 0;   // MOSI (RF13 = RP109)
    
    /* Peripheral Pin Remapping */
    // UART1 Remapping
    RPOR0bits.RP64R = 0b000001;  // Map U1TX to RD0 (RP64)
    RPINR18bits.U1RXR = 0x4B;    // Map U1RX to RD11 (RP75)
    
    // Button Interrupt Remapping
    RPINR0bits.INT1R = 0x58;    // Map RE8 to INT1
    RPINR1bits.INT2R = 0x59;    // Map RE9 to INT2  
    
    // SPI Remapping
    RPINR20bits.SDI1R = 0b00010001; // MISO = RP17  
    RPOR12bits.RP109R = 0b000101;   // MOSI = RF13  
    RPOR11bits.RP108R = 0b000110;   // SCK = RF12  
    
    /* Peripheral Initialization */
    UART1_Init(BAUDRATE);   // Initialize UART1 at 115200 bps
    spi_init();             // Initialize SPI peripheral
    mag_sleep();            // Sleep MAG
    mag_active();           // Wake up MAG 
    
}
