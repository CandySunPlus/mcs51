#include <stc12.h>

unsigned int cnt = 0;
unsigned char __code chars[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
unsigned char buffer[4] = {0x00, 0x00, 0x00, 0x00};
unsigned char T0RH, T0RL;

void configTimer0(unsigned int ms);

void main()
{
    unsigned long sec = 0;
    EA = 1;  // 中断使然总开关
    configTimer0(1); // 1ms 中断

    P2 &= 0x0F; // 低4位不变， 高4位清零
    P1 = 0xFE;

    while (1)
    {
        if (cnt >= 1000)
        {
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

/**
 * For 11.0592MHz 晶振
 * 硬件时钟 12/11059200
 * 
 * 12x/11059200 = 0.001
 * 12x = 11059.2
 * x = 11059.2 / 12 
 * x = 921
 */
void configTimer0(unsigned int ms)
{
    unsigned long tmp;
    tmp = 11059200 / 12; // 定时器计数频率
    tmp = (tmp * ms) / 1000; // 计算所需数值
    tmp = 65536 - tmp; // 计算定时器重载值
    tmp = tmp + 18; // 补偿误差

    TMOD &= 0xF0;
    TMOD |= 0x01;

    T0RH = (unsigned char)(tmp >> 8); // 高8位 
    T0RL = (unsigned char)tmp; // 低8位

    TH0 = T0RH;
    TL0 = T0RL;
    ET0 = 1;
    TR0 = 1;
    
}

void interruptTimer0() __interrupt(1)
{
    static unsigned char i = 0;

    TH0 = T0RH;
    TL0 = T0RL;
    cnt++;

    P2 &= 0x0F;
    P0 = buffer[i];
    P2 |= 0x10 << i; // 高位位移
    i = ++i & 0x03; // i = 0 ... 3
}
