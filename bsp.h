#ifndef __BSP_H__
#define __BSP_H__

/* Board Support Package for the EK-TM4C123GXL board */

/* system clock setting [Hz] */
#define SYS_CLOCK_HZ 16000000U

#define P0           (1U)
#define P1           (1U << 1)
#define P2           (1U << 2)
#define P3           (1U << 3)
#define P4           (1U << 4)
#define P5           (1U << 5)
#define P6           (1U << 6)
#define P7           (1U << 7)
#define PCTL_T0 (7u)
#define MACH1           (343u)
#define SYSCLOCK_PER_MS (16000u)

void io_init_hcsr04();
void start_timers();

#endif // __BSP_H__
