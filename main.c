#include "TM4C123GH6PM.h"
#include "bsp.h"

extern uint32_t counter;

int main() {


#if (__ARM_FP != 0) /* if VFP available... */
    /* make sure that the FPU is enabled by seting CP10 & CP11 Full Access */
    SCB->CPACR |= ((3UL << 20U)|(3UL << 22U));
#endif

    io_init();
    __ISB(); /* Instruction Synchronization Barrier */
    __DSB(); /* Data Memory Barrier */


    SysTick->LOAD = SYS_CLOCK_HZ/5U - 1U;
    SysTick->VAL  = 0U;  // clear
    SysTick->CTRL = (1U << 2U) | (1U << 1U) | 1U;

    __enable_irq();
    uint16_t digit_place = 0;
    while (1) {
        render_digit(digit_place, counter);
        digit_place = (digit_place + 1) % NUM_DIGITS;
    }
    //return 0;
}
