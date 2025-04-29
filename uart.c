
#include "uart.h"

/* Global buffer instances for UART1 */
volatile UART_RxBuffer uart1_rx;
volatile UART_TxBuffer uart1_tx;

/* Initialize UART1 peripheral */
void UART1_Init(uint32_t baudrate) {
    // Baud rate calculation 
    U1BRG = (uint16_t)((FCY / (16UL * baudrate)) - 1);
    
    // UART control registers setup
    U1MODEbits.UARTEN = 1;   // Enable UART module
    U1STAbits.UTXEN = 1;     // Enable transmitter
    
    // Initialize circular buffers
    UART1_RxBuffer_Init(&uart1_rx);
    UART1_TxBuffer_Init(&uart1_tx);
    
    // Interrupt configuration
    INTCON2bits.GIE = 1;      // Global interrupt enable
    IFS0bits.U1TXIF = 0;      // Clear transmit interrupt flag
    IFS0bits.U1RXIF = 0;      // Clear receive interrupt flag
    U1STAbits.URXISEL = 0b00; // Interrupt on every received character
    U1STAbits.UTXISEL0 = 1;   // Interrupt when transmit buffer is empty
    U1STAbits.UTXISEL1 = 0;
    IEC0bits.U1RXIE = 1;      // Enable receive interrupt
    IEC0bits.U1TXIE = 0;
}

/* UART1 Receive Handler ----------------------------------------------------*/

/* Function to initialize the receive buffer*/
void UART1_RxBuffer_Init(volatile UART_RxBuffer *buf) {
    buf->head = 0;
    buf->tail = 0;
    buf->overflow = false;
}

/* Function to write to the receive buffer */
bool UART1_RxBuffer_Write(volatile UART_RxBuffer *buf, uint8_t data) {
    uint16_t next_head = (buf->head + 1) % UART_RX_BUF_SIZE;
    
    // Always overwrite oldest data if full
    if (next_head == buf->tail) {
        buf->tail = (buf->tail + 1) % UART_RX_BUF_SIZE;  // Move tail forward
        buf->overflow = true;  // Set overflow flag
    }
    
    buf->buffer[buf->head] = data;
    buf->head = next_head;
    return true;
}

/* Function to read from the receive buffer */
bool UART1_RxBuffer_Read(volatile UART_RxBuffer *buf, uint8_t *data) {
    if (buf->head == buf->tail) {
        return false;  // Buffer empty
    }
    
    *data = buf->buffer[buf->tail];
    buf->tail = (buf->tail + 1) % UART_RX_BUF_SIZE;
    return true;
}

/* Function to check if the receive buffer is empty */
bool UART1_RxBuffer_IsEmpty(volatile UART_RxBuffer *buf) {
    return (buf->head == buf->tail);
}

/* Function to check if the receive buffer is full */
bool UART1_RxBuffer_IsFull(volatile UART_RxBuffer *buf) {
    return ((buf->head + 1) % UART_RX_BUF_SIZE == buf->tail);
}

// UART receive interrupt function
void __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void) {
    // Check for hardware error
    if (U1STAbits.OERR) {
        U1STAbits.OERR = 0;  // Clear overrun error to allow new data
    }
    
    while (U1STAbits.URXDA) {  // While data available
        UART1_RxBuffer_Write(&uart1_rx, U1RXREG);
    }
    
    // Clear interrupt flag
    IFS0bits.U1RXIF = 0;
}

/* UART1 Transmit Handler ---------------------------------------------------*/

/* Function to initialize the transmit buffer */
void UART1_TxBuffer_Init(volatile UART_TxBuffer *buf) {
    buf->head = 0;
    buf->tail = 0;
    buf->overflow = false;
}

/* Function to write to the transmit buffer */
bool UART1_TxBuffer_Write(volatile UART_TxBuffer *buf, uint8_t data) {
    uint16_t next_head = (buf->head + 1) % UART_TX_BUF_SIZE;
    
    // Overwrite oldest data if full
    if (next_head == buf->tail) {
        buf->tail = (buf->tail + 1) % UART_TX_BUF_SIZE;  // Move tail forward
        buf->overflow = true;  // Set overflow flag
    }
    
    buf->buffer[buf->head] = data;
    buf->head = next_head;
    return true;
}

/* Function to read from the transmit buffer */
bool UART1_TxBuffer_Read(volatile UART_TxBuffer *buf, uint8_t *data) {
    if (buf->head == buf->tail) {
        return false;  // Buffer empty
    }
    
    *data = buf->buffer[buf->tail];
    buf->tail = (buf->tail + 1) % UART_TX_BUF_SIZE;
    return true;
}

/* Function to check if the transmit buffer is empty */
bool UART1_TxBuffer_IsEmpty(volatile UART_TxBuffer *buf) {
    return (buf->head == buf->tail);
}

/* Function to check if the transmit buffer is full */
bool UART1_TxBuffer_IsFull(volatile UART_TxBuffer *buf) {
    return ((buf->head + 1) % UART_TX_BUF_SIZE == buf->tail);
}

/* UART transmit interrupt function */
void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {
    IFS0bits.U1TXIF = 0;  // Clear the interrupt flag first

    uint8_t data;
    // Fill the UART hardware FIFO as much as possible
    while (!UART1_TxBuffer_IsEmpty(&uart1_tx)) {
        if (U1STAbits.UTXBF) {
            break;  // UART hardware FIFO is full
        }
        if (UART1_TxBuffer_Read(&uart1_tx, &data)) {
            U1TXREG = data;  // Send one byte
        }
    }

    if (UART1_TxBuffer_IsEmpty(&uart1_tx)) {
        IEC0bits.U1TXIE = 0;  // Disable UART TX interrupt if buffer empty
    }
}
