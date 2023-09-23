#include "4dig7seg_driver.h"
#include "TM4C123GH6PM.h"

uint8_t curr_display[] = {0, 0, 0, 0};
const uint8_t numbers[] = {SEG0, SEG1, SEG2, SEG3, SEG4, SEG5, SEG6, SEG7,
                            SEG8, SEG9 };

void io_init_7seg_4dig() {
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
}

void render_digit(uint16_t place) {
    // clear all segments first so it doesn't render out of position
    GPIOE_AHB->DATA_Bits[EMASK] = 0;
    GPIOA_AHB->DATA_Bits[AMASK] = 0;

    // clear placeth bit to make it cathode
    uint16_t mask = (1u << place);
    mask = (~mask) & (P0 | P1 | P2 | P3);
    GPIOB_AHB->DATA_Bits[(P0 | P1 | P2 | P3)] = mask;

    GPIOE_AHB->DATA_Bits[EMASK] = curr_display[place];
    GPIOA_AHB->DATA_Bits[AMASK] = curr_display[place];
}

/*
 * set what will be displayed on 4-digit, 7-segment display
 * in render_digit()
 * lower index is less significant digit
 */
void set_display(uint8_t segment_digit_codes[]) {
    for (int i = 0; i < NUM_DIGITS; i++) {
        curr_display[i] = segment_digit_codes[i];
    }
}
