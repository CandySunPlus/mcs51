#include <stc12.h>

#define IR P3_2

__bit irflag = 0;
unsigned char resetTimer[4] = { 0x00, 0x00, 0x00, 0x00 };
unsigned char ledBuff[4] = { 0x3F, 0x3F, 0x3F, 0x3F };
unsigned char ircode[4];
unsigned char __code CHARS[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};

// 71ms
void configTimer(unsigned char, unsigned int, unsigned char*);
unsigned int getHighTime();
unsigned int getLowTime();
void LedScan();
void initIR();

void main() {
    EA = 1;
    P2 &= 0x0F; // 低4位不变，高4位清零
    initIR();
    configTimer(0, 1, resetTimer);
    ET0 = 1;
    TR0 = 1;
    PT0 = 1; // 抢占中断
    while(1) {
        if (irflag) {
            irflag = 0;
            // 用户码
            ledBuff[0] = CHARS[ircode[0] >> 4];
            ledBuff[1] = CHARS[ircode[0] & 0x0F];
            // 按键码
            ledBuff[2] = CHARS[ircode[2] >> 4];
            ledBuff[3] = CHARS[ircode[2] & 0x0F];
        }
    }
}

void initIR() {
    configTimer(1, 71, resetTimer);
    IT0 = 1; // INT0 负边沿触发中断
    EX0 = 1; // INT0 使能中断
}

unsigned int getHighTime() {
    // 高电平时间计数器
    TH1 = 0;
    TL1 = 0;
    TR1 = 1;
    while (IR) {
        if (TH1 >= 0x40) {
            break;
        }
    }
    TR1 = 0;
    return (TH1 << 8) | TL1;
}

unsigned int getLowTime() {
    // 低电平时间计数器
    TH1 = 0;
    TL1 = 0;
    TR1 = 1;
    while (!IR) {
        if (TH1 >= 0x40) {
            break;
        }
    }
    TR1 = 0;
    return (TH1 << 8) | TL1;
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
void configTimer(unsigned char b, unsigned int ms, unsigned char *reset) {
    unsigned long tmp;
    tmp = 11059200 / 12; // 定时器计数频率
    tmp = (tmp * ms) / 1000; // 计算所需数值
    tmp = 65536 - tmp; // 计算定时器重载值
    tmp = tmp + 18; // 补偿误差

    if (b == 0) {
        TMOD &= 0xF0;
        TMOD |= 0x01;
        TH0 = reset[0] = (unsigned char)(tmp >> 8); // 高8位
        TL0 = reset[1] = (unsigned char)tmp; // 低8位
        ET0 = 0;
        TR0 = 0;
    } else if (b == 1) {
        TMOD &= 0x0F;
        TMOD |= 0x10;
        TH1 = reset[2] = (unsigned char)(tmp >> 8); // 高8位
        TL1 = reset[3] = (unsigned char)tmp; // 低8位
        ET1 = 0;
        TR1 = 0;
    }

}

void LedScan() {
    static unsigned char i = 0;

    P2 &= 0x0F;
    P0 = ledBuff[i];
    P2 |= (0x10 << i); // 高位位移，开启开关
    i = ++i & 0x03; // 0x00 ... 0x03，到4归0
}

void EXINT0_ISR() __interrupt(0) {
    unsigned int time;
    unsigned char i, j;
    unsigned char byt = 0x00;

    time = getLowTime();

    // 1ms 921 硬件周期
    // 8.5 ~ 9.5ms 低电平
    if (time < 7833 || time > 8755) {
        IE0 = 0;
        return;
    }

    time = getHighTime();
    // 4.0 ~ 5.0ms
    if (time < 3686 || time > 4608) {
        IE0 = 0;
        return;
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 8; j++) {
            time = getLowTime();

            // 340 ~ 780us
            if (time < 313 || time > 718) {
                IE0 = 0;
                return;
            }

            time = getHighTime();

            if (time > 313 && time < 718) {
                byt >>= 1;
            } else if (time > 1345 && time < 1751) {
                byt >>= 1;
                byt |= 0x80;
            } else {
                IE0 = 0;
                return;
            }
        }
        ircode[i] = byt;
    }

    irflag = 1;
    IE0 = 0;
}

void TIMER0() __interrupt (1) {
    TH0 = resetTimer[0];
    TL0 = resetTimer[1];
    LedScan();
}