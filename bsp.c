/* Board Support Package */
#include "TM4C123GH6PM.h"
#include "bsp.h"
#include "4dig7seg_driver.h"

extern const uint8_t numbers[];

void io_init_hcsr04() {
    // TIMER0 init
    SYSCTL->RCGCTIMER = 1u;  // clock enable to TIMER0
    TIMER0->CTL |= 0;  // TAEN disable timer A, B during config
    TIMER0->CFG |= 0x04u;  // 16-bit
    // TAMR capture mode, TACMR edge-time mode, //TACDIR count up
    TIMER0->TAMR |= 0x3u | P2 | P4;
    TIMER0->TBMR &= ~P2;  // TBAMS PWM
    TIMER0->TBMR |= 0x2 | P3;  // TBMR periodic mode, TBCMR to 'edge-count'
    TIMER0->ICR |= P2;  // CAECINT clear event capture flag
    TIMER0->CTL |= P2;  // TAEVENT both edge event detection
    TIMER0->TAV = 0;  // ensure timers 0
    TIMER0->TBV = 0;
    TIMER0->IMR |= (0x3u << 2);  // CAEIM, CBEIM capture mode interrupt enable
    uint32_t period = 2 * SYS_CLOCK_HZ / 10;
    TIMER0->TAILR = period;
    TIMER0->TBILR = period;  // 0.2s
    TIMER0->TBMATCHR = period - (SYS_CLOCK_HZ / 100000);  // 10us
    TIMER0->TBPMR = (period - (SYS_CLOCK_HZ / 100000)) >> 16;
    TIMER0->TAPR = (2 * SYS_CLOCK_HZ / 10) >> 16;  // pr match regs 23:16
    TIMER0->TBPR = (2 * SYS_CLOCK_HZ / 10) >> 16;

    // SONIC DIST SENSOR INIT
    GPIOB_AHB->DEN |= P7 | P6;
    GPIOB_AHB->DIR |= P7;  // PB7 output
    GPIOB_AHB->AFSEL |= P7 | P6;  // alternate func
    // GPIOPCTL use T0CCP0/1 (timer 0 periph)
    GPIOB_AHB->PCTL &= ~(0xFFu << 24u);  // clear for sure
    GPIOB_AHB->PCTL |= (0x77 << 24u);  // tie PB6/7 to timers

    // Timer 0A interrupt- vec#: 35, interrupt#: 19, vec offset: 0x0000.008C
    NVIC->ISER[0] |= (1u << 19);
}

/* potential TODO's:
 * fix flickering: probably because ISR is too long
 * improve accuracy; it's 20% over at dist of 150mm
 * deal with noisy value, taking moving average
 * use button to switch to inches and cm display
 */


void start_timers() {
    TIMER0->CTL |= P0 | (1u << 8);
}

void Timer0A_IRQHandler(void) {
    static uint32_t pulse_start = 0;
    TIMER0->ICR |= (1u << 2);  // interrupt status clear
    if (GPIOB_AHB->DATA & P6) {  // if rising edge
        pulse_start = TIMER0->TAR;
        return;
    }
    // else neg edge
    uint32_t ticks = TIMER0->TAR - pulse_start;   // duration of pulse

    // calculate and set global dist variable
    // op order designed to not use floats and be accurate
    uint32_t digit = (ticks * MACH1) / (SYSCLOCK_PER_MS * 2);
    uint8_t digits[NUM_DIGITS];
    for (int i = 0; i < NUM_DIGITS; i++) {
        digits[i] = digit % 10;
        digit = digit / 10;
    }

    // don't display leading 0's
    uint8_t to_display[NUM_DIGITS] = {0};
    to_display[0] = numbers[digits[0]];  // always display least sig digit
    if (digits[NUM_DIGITS-1]) {
        to_display[NUM_DIGITS-1] = numbers[digits[NUM_DIGITS-1]];
    }
    for (int i = NUM_DIGITS - 2; i > 0; i--) {
        if (to_display[i+1] || digits[i]) {
            to_display[i] = numbers[digits[i]];
        } else {
            to_display[i] = 0;
        }
    }
    set_display(to_display);
}

__attribute__((naked)) void assert_failed (char const *file, int line) {
    /* TBD: damage control */
    NVIC_SystemReset(); /* reset the system */
}
