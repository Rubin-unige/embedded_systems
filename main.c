/*
 * File:   main.c
 * Author: Rubin
 *
 * Created on April 4, 2025, 2:35 PM
 */

#include "spi.h"
#include "parser.h"

/* Global counters for periodic tasks */
static uint8_t led_timer_count = 0;    // Counter for LED blinking
static uint8_t mag_read_count = 0;     // Counter for mag data reading rate
static uint8_t yaw_rate_count = 0;     // Counter for yaw data transmission
static uint8_t mag_rate_count = 0;     // Counter for mag data transmission
volatile uint8_t mag_rate = 5;         // Default magnetometer rate (5Hz)

/* Magnetometer data buffer */
static MagAvgBuffer mag_buffer = {
    .x = {0}, 
    .y = {0},  
    .z = {0},  
    .idx = 0   
};

/* Function to simulation 7 ms execution time*/
void algorithm() {
    tmr_wait_ms(TIMER2, 7);
}

// Helper Function
void UART1_SendString(const char *str);

int main(void) {
    
    // Initialize all required configurations
    config_init();  // GPIO, UART, SPI, Timers, Magnetometer
    
    // parser initialization
    parser_state pstate = {
        .state = STATE_DOLLAR,
        .index_type = 0,
        .index_payload = 0
    };
    
    // Set up 10ms periodic timer
    tmr_setup_period(TIMER1, TIMER1_PERIOD_MS);
    
    while(1) {
        algorithm();
        
        /* Code to handle the assignment */
       
        /* Blink LED2 with 1Hz frequency */
        led_timer_count++;
        if (led_timer_count >= LED_BLINK_TICKS) {    // 50*10 ms = 500 ms
            LED2^= 1;                   // Toggle LED2
            led_timer_count = 0;        // Reset the counter
        }
        
        /* Process Receive UART messages */
        uint8_t byte;
        
        // Disable RX interrupt temporarily
        IEC0bits.U1RXIE = 0;
        // Critical section: Read and process
        while (UART1_RxBuffer_Read(&uart1_rx, &byte)) {
            if (parse_byte(&pstate, byte) == NEW_MESSAGE) {
                if (strcmp(pstate.msg_type, "RATE") == 0) {
                    int rate = extract_integer(pstate.msg_payload);
                    
                    // Validate rate (0,1,2,4,5,10 Hz)
                    const int valid_rates[] = {0,1,2,4,5,10};
                    bool valid = false;
                    for (int i = 0; i < 6; i++) {
                        if (rate == valid_rates[i]) {
                            valid = true;
                            break;
                        }
                    }
                    
                    if (valid) {
                        mag_rate = rate;
                    } else {
                        UART1_SendString("$ERR,1*");
                    }
                }
            }
        }
        // Re-enable RX interrupt
        IEC0bits.U1RXIE = 1;
        
        /* Read Magnetometer Data at 25Hz (every 40ms) */
        mag_read_count++;
        if (mag_read_count >= DATA_READ_TICKS) {  // 40ms elapsed (4*10ms)
            mag_read_count = 0;
            MagData raw_data = read_mag_all();      // Read raw data
            update_mag_avg(&mag_buffer, raw_data);  // Update moving average
        }
        
        /* Send Magnetometer Data at configured rate */
        if (mag_rate > 0) {  // Skip if rate is 0 (disabled)
            mag_rate_count++;
            uint16_t mag_ticks = 100 / mag_rate;  // Convert Hz to ticks
            if (mag_rate_count >= mag_ticks) {
                mag_rate_count = 0;
                MagData avg = get_avg_mag(&mag_buffer);  // Get averaged data
                send_mag_data(&avg);                     // Transmit via UART
            }
        }
        
        /* Send YAW angles at 5Hz (every 200ms) */
        yaw_rate_count++;
        if (yaw_rate_count >= YAW_SEND_TICKS) {  // 200ms elapsed (20*10ms)
            yaw_rate_count = 0;
            MagData avg = get_avg_mag(&mag_buffer);
            float yaw = compute_yaw_angle(&avg);  // Calculate yaw angle
            send_yaw_data(yaw);                   // Transmit via UART
        }
        
        uint8_t ret = tmr_wait_period(TIMER1);
        if (ret > 0) LED1 ^= 1;  // Toggle LED1 if deadline missed (debug)
    }
    return 0;
}

/* Helper function to send strings for error */
void UART1_SendString(const char *str) {
    IEC0bits.U1TXIE = 0;  // Disable TX interrupt
    while (*str) {
        while (UART1_TxBuffer_IsFull(&uart1_tx));  // Wait for buffer space
        UART1_TxBuffer_Write(&uart1_tx, *str++);   // Send character
    }
    IEC0bits.U1TXIE = 1;  // Re-enable interrupt
    IFS0bits.U1TXIF = 1;  // Trigger transmission
}