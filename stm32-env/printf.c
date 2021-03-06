/* printf.c: Output format for a controller environment.
 *
 * This implements a limited subset of printf(), with only the
 * format features typically needed in an embedded environment.
 *
 * It's compatible with the AVR, with the format string read from
 * program space / flash, and all parameters from data space / RAM.
 * See 'S' for an optional exception for constant strings.
 *
 * Before "fixing"/extending this implementation, please consider
 * the target use on very small machine.
 *
 * This implementation is only printf().  There is no option for
 * sprintf() or fprintf().  It queues the characters directly to the
 * output device with serial_putch(), with the only additional space
 * used a small buffer for building %d/%x output.
 * 
 * Originally written 2010-2011 by Donald Becker and William Carlson
 * for the QAR EV motor controller project.  Released under GPLv2.1
 * Contact the authors for use under other terms.
 */

#include <stdarg.h>
#if defined(STM32)
#define PGM_P __const void *
#define pgm_read_byte(addr) (*(const char *)(addr))
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;

#elif defined(SDCC_pic16)
#include <stdint.h>
#define inline
#define __restrict
#define PGM_P const char *
#define pgm_read_byte(addr) (*(const char *)(addr))

#elif defined(__AVR)
#include <avr/pgmspace.h>

#else
#warning "Processor type undefined."
#define PGM_P const char *
#define pgm_read_byte(addr) (*(const char *)(addr))
#endif

/* Send a character to the output device.
 * Put any output-redirect hook here.
 */
unsigned char uart_putchar(char c);
static void inline serial_putch(char c)
{
	while(uart_putchar(c) != 0)	/* Returns -1 if full queue.  We busy-wait. */
		;
}

/* The only buffer space we reserve.
 * It could be moved to the caller's stack. */
static char buf[12];

/* Sleazy versions of number to string conversions.  These convert VAL
 * to a string in the local buffer with the specified number of
 * digits. Leading zeros are included, overflow is truncated and the
 * string is not terminated.
 */
void u32_to_uart(uint32_t val, int digits)
{
	char *str = buf + sizeof(buf) - 1;
	*str-- = 0;
	do {
		*str-- = (unsigned char)(val % 10) + '0';
		val = val / 10;
	} while (--digits > 0 || val > 0);
	do {
		serial_putch(*++str);
	} while (str < buf + sizeof(buf) - 1);
	return;
}

/* Same thing in hex. */
void uint_to_hex_uart(unsigned val, unsigned char digits)
{
	unsigned char nibble;
	char *str = buf + sizeof(buf) - 1;
	*str-- = 0;
	while (digits-- > 0) {
		nibble = val & 0x000f;
		if (nibble >= 10) nibble = (nibble - 10) + 'A';
		else nibble = nibble + '0';
		*str-- = nibble;
		val = val >> 4;
	}
	++str;
	do {
		serial_putch(*str);
	} while (*++str);
}

int serprintf(const PGM_P __restrict format, ...);

int serprintf(const PGM_P format, ...)
{
	va_list args;
	uint8_t c, j = 0;
	va_start(args, format);

	while ((c = pgm_read_byte(format++))) {
		if (j) {
			switch(c) {
#if defined(PGM_S_FORMAT)
			/* Format a string from PGM space on a Harvard architecture.
			 * Note: Archaic use of 'S' was for wide chars. */
			case 'S': {
				uint8_t *str = va_arg(args, uint8_t *);
				do {
					j = pgm_read_byte(str++);
					if (j == 0) break; /* Clears precision  */
					serial_putch(j);
				} while (1);
				break;
			}
#endif
			case 's': {
				uint8_t *str = va_arg(args, uint8_t *);
				do {
					j = *str++;
					if (j == 0) break; /* Clears precision  */
					serial_putch(j);
				} while (1);
				break;
			}
			case 'l':  j = sizeof(long);  continue;
			case 'u':
				u32_to_uart(j == sizeof(long) ? va_arg(args, long) :
							va_arg(args, int), j);
				break;
			case 'd': {
				int32_t val = (j == sizeof(long) ? va_arg(args, long) :
							   va_arg(args, int));
				if (val < 0) {
					serial_putch('-');
					val = -val;
				}
				u32_to_uart(val, j);
				break;
			}
			case 'c':
				serial_putch(va_arg(args, int));
				break;
			case 'x':
			case 'X': {
				if (j == 1) {		/* Do "%#x" rather than "%x" */
					serial_putch('0');
					serial_putch(c);		/* Match case 0x or 0X */
				}
				uint_to_hex_uart(j == sizeof(long) ? va_arg(args, long) :
								 va_arg(args, int), j);
				break;
			}
			default:
				serial_putch(c);
				break;
			}
			j = 0;
		} else {
			if (c == '%') {
				char d = pgm_read_byte(format);
				if ('0' <= d && d <= '9') {
					format++;
					j = d - '0';
				} else
					j = 1;
			} else {
				/* Automatic CR+LF expansion saves space. */
				if (c == '\n')
					serial_putch('\r');
				serial_putch(c);
			}
		}
	}
	va_end(args);
	return 0;					/* Incorrect, but this is a sleazy version. */
}

/*
 * Local variables:
 *  compile-command: "make printf.o"
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
