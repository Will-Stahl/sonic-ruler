#include "TM4C123GH6PM.h"
#include "bsp.h"
#include "4dig7seg_driver.h"

int main() {


#if (__ARM_FP != 0) /* if VFP available... */
    /* make sure that the FPU is enabled by seting CP10 & CP11 Full Access */
    SCB->CPACR |= ((3UL << 20U)|(3UL << 22U));
#endif

    io_init_7seg_4dig();
    io_init_hcsr04();
    __ISB(); /* Instruction Synchronization Barrier */
    __DSB(); /* Data Memory Barrier */

    __enable_irq();
    uint16_t digit_place = 0;
    start_timers();
    while (1) {
        render_digit(digit_place);
        digit_place = (digit_place + 1) % NUM_DIGITS;
        int delay = 10;  // not fixing the flickering
        while (delay--);
    }
    //return 0;
}
