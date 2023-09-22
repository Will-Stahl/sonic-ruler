/* Board Support Package */
#include "TM4C123GH6PM.h"
#include "bsp.h"

uint32_t counter = 0;
char gen_pulse_step = 1;
const uint16_t numbers[] = {SEG0, SEG1, SEG2, SEG3, SEG4, SEG5, SEG6, SEG7,
                            SEG8, SEG9 };

void io_init() {
    // enable Run mode for GPIO A, B, E
    SYSCTL->RCGCGPIO  |= ENABLE_PORTA | ENABLE_PORTE | ENABLE_PORTB;
    // enable AHB for GPIO A, B, E
    SYSCTL->GPIOHBCTL |= ENABLE_PORTA | ENABLE_PORTE | ENABLE_PORTB;

    /* make sure the Run Mode and AHB-enable take effects
     * before accessing the peripherals
     */
    uint32_t delay = 10;
    while (delay--);

    // pins 2-7 as output on portA
    GPIOA_AHB->DIR |= P2 | P3 | P4 | P5 | P6 | P7;
    GPIOA_AHB->DEN |= P2 | P3 | P4 | P5 | P6 | P7;

    // pins 0-1 as output on portE
    GPIOE_AHB->DIR |= P0 | P1;
    GPIOE_AHB->DEN |= P0 | P1;

    // pins 0-3 as output on portB
    GPIOB_AHB->DIR |= P0 | P1 | P2 | P3;
    GPIOB_AHB->DEN |= P0 | P1 | P2 | P3;

    // initialize digit output to high so it does not behave as cathode
    GPIOB_AHB->DATA_Bits[(P0 | P1 | P2 | P3)] = P0 | P1 | P2 | P3;

    // TIMER0 init
    SYSCTL->RCGCTIMER = 1u;  // clock enable to TIMER0
    TIMER0->CTL |= 0;  // TAEN disable timer during config
    TIMER0->CFG |= 0x04u;  // 16-bit
    // TAMR capture mode, TACMR edge-time mode, TACDIR count up
    TIMER0->TAMR |= 0x3u | (1 << 2) | (1 << 4);
    TIMER0->ICR |= (1u << 2);  // CAECINT clear event capture flag
    TIMER0->CTL |= (1u << 2);  // TAEVENT neg edge event detection
    TIMER0->TAV = 0;  // ensure timer and prescaler 0
//    TIMER0->TAPR = 0;  // ensure prescaler 0
    TIMER0->IMR |= (1u << 2);  // CAEIM, CBEIM capture mode interrupt enable

    // SONIC DIST SENSOR INIT
    GPIOB_AHB->DEN |= P6 | P7;
    GPIOB_AHB->DIR |= P7;  // PB7 output
    GPIOB_AHB->AFSEL |= P6;
    // GPIOPCTL use T0CCP0 (timer 0 periph)
    GPIOB_AHB->PCTL &= ~(0xFu << 24u);  // clear for sure
    GPIOB_AHB->PCTL |= (PCTL_T0 << 24u);  // tie PB6 to timer

    // Timer 0A interrupt- vec#: 35, interrupt#: 19, vec offset: 0x0000.008C
    NVIC->ISER[0] |= (1u << 19);

}

/* potential TODO's:
 * use timer B in PWM to generate exact 10 us pulse
 * use rising-edge capture on timer A to get exact pulse
 *   (operate on axact same period as B to avoid overflow problems)
 * set display values ONCE on interrupt in reusable way
 * improve accuracy; it's 20% over at dist of 150mm, probably because we don't wait for 8 sonic pulses
 * deal with noisy display
 * use button to switch to in display
 * re-pin with a wide timer to avoid weirdness of resetting with overflow
 */

void render_digit(uint16_t place, uint32_t number) {
    uint32_t digit = number;
    uint32_t digits[4];
    for (int i = 0; i < NUM_DIGITS; i++) {
        digits[i] = digit % 10;
        digit = digit / 10;
    }

    // don't display leading 0's
    // always display 0 as a single 0
    uint8_t to_display[] = {1, 0, 0, 0};
    to_display[3] = (digits[3] && 1);
    for (int i = NUM_DIGITS - 2; i > 0; i--) {
        to_display[i] = to_display[i+1] || digits[i];
    }
    // clear all segments first so it doesn't render out of position
    GPIOE_AHB->DATA_Bits[EMASK] = 0;
    GPIOA_AHB->DATA_Bits[AMASK] = 0;

    // clear placeth bit to make it cathode (only if to_display[place])
    uint16_t mask = ((1u & to_display[place]) << place);
    mask = (~mask) & (P0 | P1 | P2 | P3);
    GPIOB_AHB->DATA_Bits[(P0 | P1 | P2 | P3)] = mask;

    GPIOE_AHB->DATA_Bits[EMASK] = numbers[digits[place]];
    GPIOA_AHB->DATA_Bits[AMASK] = numbers[digits[place]];
}

void SysTick_Handler(void) {
    GPIOB_AHB->DATA_Bits[P7] = 0;  // always turn off PB7
    TIMER0->CTL |= 1u;  // start timer0
    if (gen_pulse_step) {
        TIMER0->CTL &= ~(1u);  // stop timer if this step
        TIMER0->TAILR = 0xFFFF;  // reset timer interval to max
        TIMER0->TAPR = 0x0;
        TIMER0->TAV = 0;  // precise 0 reset now that prescaler dealt with
        // generate 10 us pulse width
        GPIOB_AHB->DATA_Bits[P7] |= P7;
        SysTick->CTRL &= ~(1u);  // disable systick
        SysTick->LOAD = SYS_CLOCK_HZ / 100000;  // 10 usec
        SysTick->VAL  = 0u;  // clear
        SysTick->CTRL |= 1u;  // enable systick
        gen_pulse_step = 0;
        return;
    }
    gen_pulse_step = 1;
    SysTick->CTRL &= ~(1u);  // disable systick if this step
}

void Timer0A_IRQHandler(void) {
    TIMER0->CTL &= ~(1u);  // disable timer0
    TIMER0->ICR |= (1u << 2);  // interrupt status clear
    uint32_t ticks = TIMER0->TAR;   // time of event

    /* workaround for not being able to set bits 23:16 of timer:
     * set the interval to occur right after the current timer
     * so when it restarts, it overflows to 0
     */
    TIMER0->TAILR = TIMER0->TAV + 1;
    TIMER0->TAPR = TIMER0->TAV >> 16;

    // calculate and set global dist variable
    // op order designed to not use floats and be accurate
    counter = (ticks * MACH1) / (SYSCLOCK_PER_MS * 2);

    // restart systick
    SysTick->LOAD = SYS_CLOCK_HZ/5U - 1U;
    SysTick->VAL  = 0U;  // clear
    gen_pulse_step = 1;
    SysTick->CTRL |= 1u;
}

__attribute__((naked)) void assert_failed (char const *file, int line) {
    /* TBD: damage control */
    NVIC_SystemReset(); /* reset the system */
}
