
#include "SPI.h"

/* Initialize SPI module in master mode with 4.5MHz clock */
void spi_init() {
    // Configure SPI control registers
    SPI1CON1bits.MSTEN = 1;   // Master mode
    SPI1CON1bits.MODE16 = 0;  // 8-bit mode
    SPI1CON1bits.CKP = 1;     // Clock idle high
    SPI1CON1bits.CKE = 0;     // Data changes on idle->active edge
    
    // Set clock prescalers for 4.5MHz
    SPI1CON1bits.PPRE = 1;    // Primary prescaler 4:1
    SPI1CON1bits.SPRE = 4;    // Secondary prescaler 4:1

    SPI1STATbits.SPIEN = 1;   // Enable SPI module
    
    // Configure magnetometer chip select pin
    TRISDbits.TRISD6 = 0;     // Set CS pin as output
    MAG_CS = 1;               // Deselect magnetometer initially
}

/* Write 16-bit data to SPI and return received data */
uint16_t spi_write(uint16_t data) {
    // Wait for transmit buffer to be empty
    while (SPI1STATbits.SPITBF);
    
    SPI1BUF = data;           // Send data
    
    // Wait for receive buffer to have data
    while (!SPI1STATbits.SPIRBF);
    
    return SPI1BUF;           // Return received data
}

/* Put magnetometer into low-power sleep mode */
void mag_sleep(void) {
    MAG_CS = 0;                          // Select magnetometer
    spi_write(MAG_POWER_CTRL);           // Address power control register
    spi_write(0x01);                     // Set sleep mode bit
    MAG_CS = 1;                         // Deselect magnetometer
    tmr_wait_ms(TIMER1, 3);             // Wait 3ms for command to complete
}

/* Wake magnetometer and set to active measurement mode */
void mag_active(void) {
    MAG_CS = 0;                          // Select magnetometer
    spi_write(MAG_CTRL_REG2);            // Address control register 2
    spi_write((0b110 << 3) | 0b00);      // Set 25Hz data rate
    MAG_CS = 1;                         // Deselect magnetometer
    tmr_wait_ms(TIMER1, 3);             // Wait 3ms for command to complete
}

/* Read magnetometer's chip identification register */
uint8_t read_chip_id(void) {
    uint8_t id;
    
    MAG_CS = 0;                          // Select magnetometer
    spi_write(MAG_CHIP_ID | 0x80);       // Read command (MSB=1)
    spi_write(0x00);                     // Dummy write
    id = spi_write(0x00);                // Read actual ID
    MAG_CS = 1;                         // Deselect magnetometer
    
    return id;                           // Return chip ID
}

/* Read raw magnetometer data for all axes */
MagData read_mag_all(void) {
    MagData data;
    uint8_t lsb_x, msb_x, lsb_y, msb_y, lsb_z, msb_z;

    MAG_CS = 0;                          // Select magnetometer
    spi_write(MAG_DATA_X_LSB | 0x80);    // Start read from X_LSB register
    
    // Read all 6 bytes (LSB/MSB pairs for X, Y, Z)
    lsb_x = spi_write(0x00);
    msb_x = spi_write(0x00);
    lsb_y = spi_write(0x00);
    msb_y = spi_write(0x00);
    lsb_z = spi_write(0x00);
    msb_z = spi_write(0x00);
    MAG_CS = 1;                         // Deselect magnetometer

    // Convert raw 13-bit signed values to float
    int16_t raw_x = ((int16_t)(msb_x << 8) | (lsb_x & 0xF8));
    int16_t raw_y = ((int16_t)(msb_y << 8) | (lsb_y & 0xF8));
    int16_t raw_z = ((int16_t)(msb_z << 8) | (lsb_z & 0xF8));
    
    // Scale values (divide by 8 to get proper units)
    data.x = (float)raw_x / 8.0f;
    data.y = (float)raw_y / 8.0f;
    data.z = (float)raw_z / 8.0f;

    return data;
}

/* Update moving average buffer with new magnetometer data */
void update_mag_avg(MagAvgBuffer *buf, MagData new_data) {
    // Store new data in circular buffer
    buf->x[buf->idx] = new_data.x;
    buf->y[buf->idx] = new_data.y;
    buf->z[buf->idx] = new_data.z;
    
    // Update index with wrap-around
    buf->idx = (buf->idx + 1) % MAG_AVG_WINDOW;
}

/* Calculate averaged magnetometer data from buffer */
MagData get_avg_mag(const MagAvgBuffer *buf) {
    MagData avg = {0.0f, 0.0f, 0.0f};
    
    // Sum all values in buffer
    for (uint8_t i = 0; i < MAG_AVG_WINDOW; i++) {
        avg.x += buf->x[i];
        avg.y += buf->y[i];
        avg.z += buf->z[i];
    }
    
    // Divide by window size to get average
    avg.x /= MAG_AVG_WINDOW;
    avg.y /= MAG_AVG_WINDOW;
    avg.z /= MAG_AVG_WINDOW;
    
    return avg;
}

/* Compute yaw angle (in degrees) from magnetometer data */
float compute_yaw_angle(const MagData *avg) {
    // Calculate angle using atan2 and convert to degrees
    return atan2f(avg->y, avg->x) * (180.0f / M_PI);
}

/* Send magnetometer data via UART in $MAG,X,Y,Z* format */
void send_mag_data(const MagData *data) {
    char buffer[64];
    // Format data string
    int len = snprintf(buffer, sizeof(buffer), "$MAG,%.2f,%.2f,%.2f*",
                      (double)data->x, (double)data->y, (double)data->z);
    
    if (len <= 0 || len >= sizeof(buffer)) return;
    
    // Critical section for UART transmission
    IEC0bits.U1TXIE = 0; // Disable UART TX interrupt
    for (int i = 0; i < len; i++) {
        UART1_TxBuffer_Write(&uart1_tx, buffer[i]);
    }
    IEC0bits.U1TXIE = 1; // Re-enable UART TX interrupt
    IFS0bits.U1TXIF = 1; // Trigger transmit interrupt
}

/* Send yaw angle via UART in $YAW,ANGLE* format */
void send_yaw_data(float yaw_angle) {
    char buffer[32];
    // Format yaw string
    int len = snprintf(buffer, sizeof(buffer), "$YAW,%.2f*\n", (double)yaw_angle);
    
    if (len <= 0 || len >= sizeof(buffer)) return;
    
    // Critical section for UART transmission
    IEC0bits.U1TXIE = 0; // Disable UART TX interrupt
    for (int i = 0; i < len; i++) {
        UART1_TxBuffer_Write(&uart1_tx, buffer[i]);
    }
    IEC0bits.U1TXIE = 1; // Re-enable UART TX interrupt
    IFS0bits.U1TXIF = 1; // Trigger transmit interrupt
}

