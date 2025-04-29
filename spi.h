/* 
 * File:   SPI.h
 * Author: Rubin
 *
 * Created on April 4, 2025, 2:34 PM
 */

#ifndef SPI_H
#define	SPI_H

#include "uart.h"
#include "math.h"

#ifdef	__cplusplus
extern "C" {
#endif

// Magnetometer Chip Select (CS) Pin
#define MAG_CS LATDbits.LATD6
    
// Moving average buffer size
#define MAG_AVG_WINDOW 5 
    
// Magnetometer Register Addresses
#define MAG_POWER_CTRL 0x4B  // Power mode control
#define MAG_CTRL_REG2  0x4C  // Configuration register
#define MAG_CHIP_ID    0x40  // Device ID
#define MAG_DATA_X_LSB 0x42  // X-axis data LSB
 
/* Data Structures */
// Raw magnetometer data (X, Y, Z axes)
typedef struct {
    float x;  
    float y;
    float z;
} MagData;

// Buffer for storing magnetometer data for moving average.
typedef struct {
    float x[MAG_AVG_WINDOW];  
    float y[MAG_AVG_WINDOW];
    float z[MAG_AVG_WINDOW];
    uint8_t idx;
} MagAvgBuffer;

/* SPI Functions */
void spi_init(void);                // Initialize SPI
uint16_t spi_write(uint16_t data);  // Write 16-bit data

/* Magnetometer Functions */
void mag_sleep(void);              // Enter sleep mode
void mag_active(void);             // Wake up magnetometer
uint8_t read_chip_id(void);        // Read device ID
MagData read_mag_all(void);        // Read X, Y, Z data
void update_mag_avg(MagAvgBuffer *buf, MagData new_data);  // Update moving average
MagData get_avg_mag(const MagAvgBuffer *buf);  // Get averaged data
float compute_yaw_angle(const MagData *avg);   // Calculate yaw (degrees)

/* Communication Functions */
void send_mag_data(const MagData *data);    // Send Magnetometer data via UART
void send_yaw_data(float yaw_angle);        // Send yaw angle via UART

#ifdef	__cplusplus
}
#endif

#endif	/* SPI_H */

