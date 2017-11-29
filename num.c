#include <mcs51/stc12.h>

void main()
{
    /**
     * For 12MHz 晶振
     * 硬件时钟 1/1000000
     * 10ms = 10000 个硬件时钟
     *
     * 定时周期 1ms 为 1000 个硬件时钟
     */

    unsigned int i = 0;
    unsigned long sec = 0;
    unsigned char __code chars[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71};

    TMOD = 0x01;
    TR0 = 1;

    TH0 = 0xFC;
    TL0 = 0x18;

    P2_4 = 0;
    P2_5 = 0;
    P2_6 = 0;
    P2_7 = 0;

    P0 = chars[0];
    P1 = 0xFE;

    while (1)
    {
        if (TF0 == 1)
        {
            TF0 = 0;
            TH0 = 0xFC;
            TL0 = 0x18;
            i++;

            // 1000 次循环达到 1s
            if (i >= 1000)
            {
                i = 0;
                sec++;
                P1 = ~(0x01 << (sec % 8));
            }

            P2_4 = 0;
            P2_5 = 0;
            P2_6 = 0;
            P2_7 = 0;
            switch (i % 4)
            {
            case 0:
                P0 = chars[(sec / 1000) % 10];
                P2_4 = 1;
                break;
            case 1:
                P0 = chars[(sec / 100) % 10];
                P2_5 = 1;
                break;
            case 2:
                P0 = chars[(sec / 10) % 10];
                P2_6 = 1;
                break;
            case 3:
                P0 = chars[sec % 10];
                P2_7 = 1;
                break;
            }
        }
    }
}
