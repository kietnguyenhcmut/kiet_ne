// CONFIG1
#pragma config FOSC = HS        // HS oscillator
#pragma config WDTE = OFF       // WDT disabled
#pragma config PWRTE = OFF      // PWRT disabled
#pragma config CP = OFF         // Code Protection off
#pragma config CPD = OFF        // Data EE protection off
#pragma config BOREN = OFF      // BOR disabled
#pragma config WRT = OFF        // Write protection off

#include <xc.h>
#define _XTAL_FREQ 8000000

// ??a ch? 8-bit trên bus (7-bit<<1 | R/W)
#define SLA2_W 0x10     // Slave U2 write  (0x08<<1)
#define SLA2_R 0x11     // Slave U2 read   (0x08<<1 | 1)
#define SLA3_W 0x12     // Slave U3 write  (0x09<<1)
#define SLA3_R 0x13     // Slave U3 read   (0x09<<1 | 1)

void I2C_Master_Init(const unsigned long c) {
    SSPCON = 0b00101000;
    SSPCON2 = 0;
    SSPADD = (_XTAL_FREQ/(4*c)) - 1;
    SSPSTAT = 0;
    TRISC3 = TRISC4 = 1;
}
void UART_init(void) {
    BRGH = 1; SPBRG = 51; SYNC = 0; SPEN = 1;
    TRISC6 = 1; TXEN = 1;
}
void UART_write_c(char d) {
    while(!TRMT);
    TXREG = d;
}
void I2C_Master_Wait() {
    while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));
}
void I2C_Master_Start() {
    I2C_Master_Wait(); SEN = 1;
}
void I2C_Master_Stop() {
    I2C_Master_Wait(); PEN = 1;
}
void I2C_Master_Write(unsigned d) {
    I2C_Master_Wait(); SSPBUF = d;
}
unsigned short I2C_Master_Read(unsigned short ack) {
    unsigned short val;
    I2C_Master_Wait();
    RCEN = 1;
    I2C_Master_Wait();
    val = SSPBUF;
    I2C_Master_Wait();
    ACKDT = (ack)?0:1;
    ACKEN = 1;
    return val;
}
// S? d?ng chung hàm ghi, truy?n address ??ng
void I2C_Write(int address, int data) {
    I2C_Master_Start();
    I2C_Master_Write(address);
    I2C_Master_Write(data);
    I2C_Master_Stop();
    __delay_ms(50);
}

unsigned char readData;
void main() {
    UART_init();
    I2C_Master_Init(100000);

    TRISB0 = 1;  // RB0 input
    TRISB1 = 1;  // RB1 input

    while(1) {
        // Nút RB0: g?i "12.56 " và "3.78 " ??n Slave U2
        if (!PORTBbits.RB0) {
            while (!PORTBbits.RB0);
            // "12.56 "
            I2C_Write(SLA2_W, '1');
            I2C_Write(SLA2_W, '2');
            I2C_Write(SLA2_W, '.');
            I2C_Write(SLA2_W, '5');
            I2C_Write(SLA2_W, '6');
            I2C_Write(SLA2_W, ' ');
            // "3.78 "
            I2C_Write(SLA2_W, '3');
            I2C_Write(SLA2_W, '.');
            I2C_Write(SLA2_W, '7');
            I2C_Write(SLA2_W, '8');
            I2C_Write(SLA2_W, ' ');
        }

        // Nút RB1: ??c 10 ký t? t? Slave U3, hi?n th? lên terminal Master
        if (!PORTBbits.RB1) {
            while (!PORTBbits.RB1);
            for (int j = 0; j < 10; j++) {
                I2C_Master_Start();
                I2C_Master_Write(SLA3_R);
                readData = I2C_Master_Read(j < 9);  // ACK m?i byte tr? byte cu?i
                I2C_Master_Stop();
                UART_write_c(readData);
            }
        }
    }
}
