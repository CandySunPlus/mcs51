#include <stc12.h>

#define KEY_1 P3_4
#define KEY_2 P3_5
#define KEY_3 P3_6
#define KEY_4 P3_7

unsigned int cnt = 0;
unsigned char T0RH, T0RL;

unsigned char keyState[4] = {1, 1, 1, 1};
unsigned char buffer[4] = {0x3F, 0x3F, 0x3F, 0x3F};
unsigned char __code KEY_CODE[] = { 0x01, 0x02, 0x03, 0x04 };
unsigned char __code CHARS[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};

void configTimer0(unsigned int ms);
void keyAction(unsigned char action);
void keyScan();
void keyDriver();
void ledScan();
void showNum(unsigned int num);

void main()
{
    EA = 1;  // 中断使然总开关
    configTimer0(1); // 1ms 中断
    P1 = 0xFE;
    P2 &= 0x0F; // 低4位不变，高4位清零

    while (1)
    {
        keyDriver();
    }
}

/**
 * For 11.0592MHz 晶振
 * 硬件时钟 12/11059200
 */
void configTimer0(unsigned int ms)
{
    unsigned long tmp;
    tmp = 11059200 / 12; // 定时器计数频率
    tmp = (tmp * ms) / 1000; // 计算所需数值
    tmp = 65536 - tmp; // 计算定时器重载值
    tmp = tmp + 18; // 补偿误差

    TMOD &= 0xF0;    TMOD |= 0x01;

    T0RH = (unsigned char)(tmp >> 8); // 高8位 
    T0RL = (unsigned char)tmp; // 低8位

    TH0 = T0RH;
    TL0 = T0RL;
    ET0 = 1;
    TR0 = 1;
    
}

void keyDriver() {
    unsigned char i;
    static unsigned char backKeyState[4] = {1, 1, 1, 1};
    for (i = 0; i < 4; i++) {
        if (keyState[i] != backKeyState[i]) {
            if (backKeyState[i] == 0) {
                // 前次是 0，这次是弹起
                keyAction(KEY_CODE[i]);
            }
            backKeyState[i] = keyState[i];
        }
    }
}

void keyAction(unsigned char action) {
    static unsigned char i = 0;
    static unsigned int j = 0;
    if (action == KEY_CODE[0] || action == KEY_CODE[1]) {
        i = action == KEY_CODE[0] ? i + 1 : i - 1;
        // 溢出后为 0xFF
        // 0xFF & 0x07 => 0x07
        // 0x08 & 0x07 => 0x00
        i = i & 0x07;
        P1 = ~(0x01 << i);
    } else if (action == KEY_CODE[2] || action == KEY_CODE[3]) {
        j = action == KEY_CODE[2] ? j + 1 : j - 1;
        // 最大到 9999
        j = j > 9999 ? 0 : j;
        showNum(j);
    }
}

void showNum(unsigned int num) {
    buffer[0] = CHARS[num / 1000 % 10];
    buffer[1] = CHARS[num / 100 % 10];
    buffer[2] = CHARS[num / 10 % 10];
    buffer[3] = CHARS[num % 10];
}

void ledScan() {
    static unsigned char i = 0;

    P2 &= 0x0F;
    P0 = buffer[i];
    P2 |= (0x10 << i); // 高位位移，开启开关
    i = ++i & 0x03; // 0x00 ... 0x03，到4归0
}

void keyScan() {
    unsigned char i;
    static unsigned char buf[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    buf[0] = (buf[0] << 1) | KEY_1;
    buf[1] = (buf[1] << 1) | KEY_2;
    buf[2] = (buf[2] << 1) | KEY_3;
    buf[3] = (buf[3] << 1) | KEY_4;

    for (i = 0; i < 4; i++) {
        if ((buf[i] & 0x0F) == 0x00) {
            // 低4位都为0
            keyState[i] = 0;
        } else if ((buf[i] & 0x0F) == 0x0F) {
            // 低4位都为1
            keyState[i] = 1;
        }
    }
}

void interruptTimer0() __interrupt(1)
{
    TH0 = T0RH;
    TL0 = T0RL;
    keyScan();
    ledScan();
}
