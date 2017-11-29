#include <stc12.h>

/**
 * For 12MHz 晶振
 * 硬件时钟 1/1000000
 * 10ms = 10000 个硬件时钟
 *
 * 定时周期 1ms 为 1000 个硬件时钟
 */

unsigned int cnt = 0;
unsigned char __code chars[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
unsigned char buffer[4] = { 0x00, 0x00, 0x00, 0x00 };

void main()
{
    unsigned long sec = 0;
    TMOD = 0x01;
    TR0 = 1;
    ET0 = 1; // T0 中断开关
    EA = 1; // 中断使然总开关

    TH0 = 0xFC;
    TL0 = 0x18;

    P2_4 = 0;
    P2_5 = 0;
    P2_6 = 0;
    P2_7 = 0;

    P1 = 0xFE;

    while (1)
    {
        if (cnt >= 1000) {
            cnt = 0;
            sec++;
            P1 = ~(0x01 << (sec % 8));
            buffer[0] = chars[sec / 1000 % 10];
            buffer[1] = chars[sec / 100 % 10];
            buffer[2] = chars[sec / 10 % 10];
            buffer[3] = chars[sec % 10];
        }
    }
}

void interruptTimer0() __interrupt (1) {
    static unsigned char i = 0;

    TH0 = 0xFC;
    TL0 = 0x18;
    cnt++;

    P2_4 = 0;
    P2_5 = 0;
    P2_6 = 0;
    P2_7 = 0;
    P0 = buffer[i++];
    switch (i - 1) {
        case 0: P2_4 = 1; break;
        case 1: P2_5 = 1; break;
        case 2: P2_6 = 1; break;
        case 3: P2_7 = 1; i = 0; break;
    }
}
