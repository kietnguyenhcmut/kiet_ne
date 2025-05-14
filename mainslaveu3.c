// CONFIG
#pragma config FOSC = XT    // XT oscillator
#pragma config WDTE = OFF   // WDT disabled
#pragma config PWRTE = OFF  // PWRT disabled
#pragma config BOREN = OFF  // BOR disabled
#pragma config LVP = OFF    // Low-voltage programming disabled
#pragma config CPD = OFF    // Data EEPROM code protection off
#pragma config WRT = OFF    // Flash write protection off
#pragma config CP = OFF     // Flash code protection off

#include <xc.h>
#define _XTAL_FREQ 8000000

void I2C_Write(unsigned char d) {
    BF = 0;
    SSPBUF = d;
    SSPCONbits.CKP = 1;
    while (SSPSTATbits.BF);
}

short z;
unsigned char sendCount = 0;

void __interrupt() I2C_Slave_ISR() {
    if (SSPIF) {
        // Kéo SCL xu?ng tr??c khi x? lý
        SSPCONbits.CKP = 0;

        // X? lý tràn ho?c va ch?m
        if (SSPCONbits.SSPOV || SSPCONbits.WCOL) {
            z = SSPBUF;               // ??c ?? clear BF
            SSPCONbits.SSPOV = 0;
            SSPCONbits.WCOL = 0;
            SSPCONbits.CKP = 1;       // th? SCL
        }

        // Master ??c data (D_nA=0, R_nW=1)
        if (!SSPSTATbits.D_nA && SSPSTATbits.R_nW) {
            z = SSPBUF;               // clear BF
            sendCount++;

            // G?i "5.34 "
            if (sendCount == 1) I2C_Write('5');
            if (sendCount == 2) I2C_Write('.');
            if (sendCount == 3) I2C_Write('3');
            if (sendCount == 4) I2C_Write('4');
            if (sendCount == 5) I2C_Write(' ');

            // G?i "6.23 "
            if (sendCount == 6) I2C_Write('6');
            if (sendCount == 7) I2C_Write('.');
            if (sendCount == 8) I2C_Write('2');
            if (sendCount == 9) I2C_Write('3');
            if (sendCount == 10) {
                I2C_Write(' ');
                sendCount = 0;      // reset counter
            }
        }

        // *** Thêm dòng này ?? luôn th? SCL sau m?i transaction ***
        SSPCONbits.CKP = 1;

        // Clear interrupt flag
        SSPIF = 0;
    }
}

void I2C_Slave_Init(short address) {
    SSPSTAT = 0x80;
    SSPADD  = address;
    SSPCON  = 0x36;
    SSPCON2 = 0x01;
    TRISC3 = TRISC4 = 1;  // SDA, SCL inputs
    GIE   = PEIE = 1;
    SSPIF = 0;
    SSPIE = 1;
}

void main() {
    // Không kh?i UART (không g?n terminal)
    TRISB = 0xFF;        // PORTB làm input
    I2C_Slave_Init(0x12);
    while (1) {
        // ch? IRQ ?? g?i data
    }
}
