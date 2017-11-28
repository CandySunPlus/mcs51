#include <mcs51/stc12.h>

void main() {
    /**
     * For 12MHz 晶振
     * 硬件时钟 1/1000000
     * 10ms = 10000 个硬件时钟
     *
     * 定时周期 50ms 为 50000 个硬件时钟
     */

    unsigned int i = 0;
    unsigned int offset = 0;

    TMOD = 0x01;
    TR0 = 1;


    TH0 = 0x3C;
    TL0 = 0xB0;

    P1 = 0xFE;

    while(1) {
        if (TF0 == 1) {
            TF0 = 0;
            TH0 = 0x3C;
            TL0 = 0xB0;
            i++;

            // 20 次循环达到 1s
            if (i >= 20) {
                i = 0;
                P1 = ~(0x01 << offset);
                // 循环流动
                offset = offset == 7 ? offset = 0 : offset + 1;
            }
        }
    }

}
