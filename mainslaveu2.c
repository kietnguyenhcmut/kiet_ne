// CONFIG
#pragma config FOSC = XT
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config BOREN = OFF
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#include <xc.h>
#define _XTAL_FREQ 8000000

void UART_init(void) {
    BRGH = 1; SPBRG = 51; SYNC = 0; SPEN = 1;
    TRISC6 = 1; TXEN = 1;
}
void UART_write_c(char d) {
    while (!TRMT);
    TXREG = d;
}
void I2C_Write(unsigned char d) {
    BF = 0;
    SSPBUF = d;
    SSPCONbits.CKP = 1;
    while (SSPSTATbits.BF);
}

short z;
unsigned char readData;

void __interrupt() I2C_Slave_ISR() {
    if (SSPIF) {
        SSPCONbits.CKP = 0;
        // X? lý tràn/bong bóng
        if (SSPCONbits.SSPOV || SSPCONbits.WCOL) {
            z = SSPBUF;
            SSPCONbits.SSPOV = 0;
            SSPCONbits.WCOL = 0;
            SSPCONbits.CKP = 1;
        }
        // Master ghi data xu?ng (D_nA=0, R_nW=0)
        if (!SSPSTATbits.D_nA && !SSPSTATbits.R_nW) {
            z = SSPBUF;          // ??c ?? clear BF
            while (!BF);
            readData = SSPBUF;
            UART_write_c(readData);
            SSPCONbits.CKP = 1;
        }
        // Không còn branch g?i ng??c
        SSPIF = 0;
    }
}

void I2C_Slave_Init(short address) {
    SSPSTAT = 0x80;
    SSPADD = address;
    SSPCON = 0x36;
    SSPCON2 = 0x01;
    TRISC3 = TRISC4 = 1;
    GIE = PEIE = 1;
    SSPIF = 0;
    SSPIE = 1;
}

void main() {
    UART_init();
    TRISB = 0xFF;   // RB là input
    I2C_Slave_Init(0x10);
    while (1) { /* ch? IRQ */ }
}
