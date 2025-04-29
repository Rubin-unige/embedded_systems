/* 
 * File:   UART.h
 * Author: Rubin
 *
 * Created on April 4, 2025, 2:34 PM
 */

#ifndef UART_H
#define UART_H

#include "timer.h"

#ifdef __cplusplus
extern "C" { 
#endif

// Is buffer enough
#define BAUDRATE            115200  // Default UART baud rate
#define UART_RX_BUF_SIZE    32      // Receive Circular buffer size
#define UART_TX_BUF_SIZE    128      // Transmit Circular buffer size
    
// RX Buffer Structure
typedef struct {
    volatile uint8_t buffer[UART_RX_BUF_SIZE];  // Fixed-size storage
    volatile uint16_t head;                     // Write position 
    volatile uint16_t tail;                     // Read position
    volatile bool overflow;                     // Overflow flag
} UART_RxBuffer;

// TX Buffer Structure
typedef struct {
    volatile uint8_t buffer[UART_TX_BUF_SIZE];  // Fixed-size storage
    volatile uint16_t head;                     // Write position
    volatile uint16_t tail;                     // Read position
    volatile bool overflow;                     // Overflow flag
} UART_TxBuffer;

// Global Buffer Instances
extern volatile UART_RxBuffer uart1_rx;
extern volatile UART_TxBuffer uart1_tx;

// Initialization
void UART1_Init(uint32_t baudrate);

// Buffer Operations
void UART1_RxBuffer_Init(volatile UART_RxBuffer *buf);
void UART1_TxBuffer_Init(volatile UART_TxBuffer *buf);
bool UART1_RxBuffer_Write(volatile UART_RxBuffer *buf, uint8_t data);
bool UART1_RxBuffer_Read(volatile UART_RxBuffer *buf, uint8_t *data);
bool UART1_TxBuffer_Write(volatile UART_TxBuffer *buf, uint8_t data);
bool UART1_TxBuffer_Read(volatile UART_TxBuffer *buf, uint8_t *data);

// Status Checks
bool UART1_RxBuffer_IsEmpty(volatile UART_RxBuffer *buf);
bool UART1_RxBuffer_IsFull(volatile UART_RxBuffer *buf);
bool UART1_TxBuffer_IsEmpty(volatile UART_TxBuffer *buf);
bool UART1_TxBuffer_IsFull(volatile UART_TxBuffer *buf);

#ifdef __cplusplus
}
#endif

#endif /* UART_H */