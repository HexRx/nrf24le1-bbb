#ifndef __WIRING_H__
#define __WIRING_H__

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/types.h>

/* nrf24LE1 required signals */
#define WIRING_NRF_PROG_PIN	(1*32 + 17) // GPIO1_17 P9_23
#define WIRING_NRF_RESET_PIN	(0*32 + 15) // GPIO0_15 P9_24

/* Macros for sleep happiness */
#define udelay(us)		usleep(us)
#define mdelay(ms)		usleep(ms*1000)

/* Wiring functions for bootstraping SPI */
bool wiring_init(const char *device);
void wiring_destroy(void);

/* Full-duplex read and write function */
uint8_t wiring_write_then_read(uint8_t* out, 
	                           uint16_t out_len, 
	                   		   uint8_t* in, 
	                   		   uint16_t in_len);

/* Function for setting gpio values */
void wiring_set_gpio_value(uint8_t pin, uint8_t state);

#endif
