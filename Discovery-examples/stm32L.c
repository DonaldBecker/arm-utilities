/* stm32L.c: STM32L-Discovery example program. */
/*
	Written 2010-2015 by Donald Becker, William Carlson and H. William Becker

	This is a demo program for the STM32L-Discovery development board.
	This board is based on the STM32L152RBT6 and RCT6
	This board has the following peripherals:
	LED3 Green LED on PB7, high to light (option of TIM4_CH2)
	LED4 Blue LED on PB6, high to light (option of TIM4_CH1)
	USER button on PA0, normally low, debounced high (also used for IDD wakeup)
	Touch slider
	* PA6,PA7 Group 2
	* PC4,PC5 Group 9
	* PB0,PB1 Group 3
	IDD measurement
	* PA4 analog input
	* PC13 IDD_CNT_ENB
	LCD display (Note: JTAG must be disabled)
	* PA1-PA3,PA8-PA10,PA15,PB3-5,PB8-PB15,PC0-PC3, PC6-PC11
	32KHz crystal PC14-PC15
	SWD PA13/PA14
	Boot1 
	Unassigned PB2
	PA5,PA11,PA12,PC12,PD2
*/
static const char versionA[] =
"$Id: stm32L.c $\n";
#include <stm32L1xx.h>
#include "stm32-env.h"

/* Definitions for this Discovery board. */
#define LED_BLUE	(1<<6)
#define LED_GREEN	(1<<7)
#define LED_TURNON GPIOB_BSRR = LED_BLUE | LED_GREEN;
#define TOGGLE_LED GPIOB_ODR = PORTB ^ LED_GREEN;

uint32_t clock_1msec = 0;

/* Standard SysTick timer config. */
/* Does NOT get an ISR() wrapper. */
void SysTick_Handler(void)
{
	clock_1msec++;
}

/* Configure the I/O port functions and directions.
 * This changes with the hardware platform and specific MCU part.
 * We need to set up the PWM timer, clock timer, ADC, LED pins,
 */

/* Enable all of the functions that the board supports.
 * The additional power is unimportant during development.
 */
unsigned int _RCC_APB1ENR = APB1ENR_LCDEN | APB1ENR_TIM4EN;
unsigned int _RCC_APB2ENR = APB2ENR_ADC1EN;
unsigned int _RCC_AHBENR = AHBENR_GPIOA | AHBENR_GPIOB | AHBENR_GPIOC
	| AHBENR_GPIOD;

/* Configure the clocks, devices, and pins. */
void setup_io_ports(void)
{
	/* There is no high speed crystal fitted by default.
	 * Use the 16MHz high speed internal oscillator.
	 * Set the PLL to x3 to get the 48MHz USB and SDIO clocks.
	 * Use the /2 PLLDIV to get a 24MHz SYSCLK (max 32MHz).
	 * ADC:12MHz APB:24MHz, all clock dividers /1. */
	RCC_CSR = 0x00010100;		/* Turn on LSE 32KHz crystal, use for LCD. */
	RCC_CR = 0x00000001;		/* Enable internal clock, /8 LCD prescale */
	RCC_CFGR = 0x00400003;		/* PLL to 3x, APB1,2,AHB to 24MHz */
	RCC_CR = 0x01000001;		/* Switch to PLL */

	/* Enable devices. */
	AHBENR = _RCC_AHBENR;
	/* Pin enable and mapping. */
	/* PortB pins 6 and 7 are GPIO outputs for LEDs. */
	GPIOB_MODER  = 0x00005000;
	GPIOB_OTYPER = 0x00000000;	/* All outputs push-pull. */
	GPIOB_OSPEEDR= 0x00000000;	/* All outputs slow, electrically quiet. */
	GPIOB_PUPDR  = 0x00000000;	/* No pull-up or -down. */
	GPIOB_BSRR   = LED_BLUE | LED_GREEN;	/* Turn LEDs on. */

	return;
}

int main(void)
{
	/* We may re-start from a watchdog reset, so we prioritize setting the
	 * outputs to a safe state and enabling status reporting.
	 */
	setup_io_ports();			/* Includes clock tree config. */

	while(1) {
		asm("wfe");
	}

	return 0;					/* No exit occurs. */
}

/*
 * Local variables:
 *  compile-command: "make stm32L.bin"
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
