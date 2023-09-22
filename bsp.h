#ifndef __BSP_H__
#define __BSP_H__

/* Board Support Package for the EK-TM4C123GXL board */

/* system clock setting [Hz] */
#define SYS_CLOCK_HZ 16000000U

/* on-board pins, in order from A-H */
#define P0           (1U)
#define P1           (1U << 1)
#define P2           (1U << 2)
#define P3           (1U << 3)
#define P4           (1U << 4)
#define P5           (1U << 5)
#define P6           (1U << 6)
#define P7           (1U << 7)
#define EMASK         (0x03u)
#define AMASK         (0xfCu)
#define ENABLE_PORTA  (1u)
#define ENABLE_PORTE  (1u << 4)
#define ENABLE_PORTB  (1u << 1)
#define NUM_DIGITS    (4)
#define SEG0 (0x3Fu)
#define SEG1 (0x06u)
#define SEG2 (0x5Bu)
#define SEG3 (0x4Fu)
#define SEG4 (0x66u)
#define SEG5 (0x6Du)
#define SEG6 (0x7Du)
#define SEG7 (0x07u)
#define SEG8 (0x7Fu)
#define SEG9 (0x6Fu)
#define PCTL_T0 (7u)
#define MACH1           (343u)
#define SYSCLOCK_PER_MS (16000u)

void io_init();
void render_digit(uint16_t place, uint32_t number);

#endif // __BSP_H__
