#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <unistd.h>

#include "gpio.h"

static volatile GPIO_t *gpio[GPIO_NUM_BANKS];

static uint32_t GPIO_ADDRS[GPIO_NUM_BANKS] = { 0x44E07000, 0x4804C000, 0x481AC000, 0x481AE000 };

// offsets of CLKCTRL registers relative to CM_OFFSET.
static uint32_t GPIO_CLKCTRL[GPIO_NUM_BANKS] = {
        CM_WKUP_GPIO0_CLKCTRL,
        CM_PER_GPIO1_CLKCTRL,
        CM_PER_GPIO2_CLKCTRL,
        CM_PER_GPIO3_CLKCTRL
};


static void verify_gpio_map()
{
	assert(offsetof(GPIO_t, REVISION) == 0x00);
	assert(offsetof(GPIO_t, SYSCONFIG) == 0x10);
	assert(offsetof(GPIO_t, EOI) == 0x20);
	assert(offsetof(GPIO_t, SYSSTATUS) == 0x114);
	assert(offsetof(GPIO_t, CTRL) == 0x130);
	assert(offsetof(GPIO_t, CLEARDATAOUT) == 0x190);
}

static int gpio_mmap(int fd)
{
	for (int i = 0; i < GPIO_NUM_BANKS; ++i) {

		gpio[i] = mmap(NULL, GPIO_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_ADDRS[i]);

		if (gpio[i] == MAP_FAILED)
			return errno;
	}

	return 0;
}

void gpio_exit()
{
	for (int i = 0; i < GPIO_NUM_BANKS; ++i)
		munmap((void *)gpio[i], GPIO_MEM_SIZE);
}

static int gpio_clock_enable(int fd)
{
	uint8_t *cm = mmap(NULL, CM_PER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CM_OFFSET);

	if (cm == MAP_FAILED)
		return errno;

	for (int i = 0; i < GPIO_NUM_BANKS; ++i) {

		// enable clocks to GPIO and spin while not fully functional
		volatile uint32_t *clkctrl= (uint32_t*)(cm+GPIO_CLKCTRL[i]);
		*clkctrl &= ~GPIO_CLKCTRL_MODULEMODE_BM;
		*clkctrl |= GPIO_CLKCTRL_MODULEMODE_ENABLE;

		while ( ((*clkctrl) & GPIO_CLKCTRL_IDLEST_BM) != GPIO_CLKCTRL_IDLEST_FUNCTIONAL ) ;

		// ungate clock (page 4892)
		gpio[i]->CTRL &= (~0x01) | (~0x06);
	}

	munmap(cm, CM_PER_SIZE);
	return 0;
}

int gpio_init(void)
{
	verify_gpio_map();

	int fd = open("/dev/mem", O_RDWR);
	int status = 0;
	int ret;

	if (fd == -1) {
		fprintf(stderr, "failed to open /dev/mem: %s\n", strerror(errno));
		return errno;
	}

	ret = gpio_mmap(fd);
	if (ret) {
		fprintf(stderr, "failed to mmap gpio banks: %s\n", strerror(errno));
		status = errno;
		goto ret_close;
	}

	ret = gpio_clock_enable(fd);
	if (ret) {
		fprintf(stderr, "failed to enable gpio clocks: %s\n", strerror(errno));
		status = errno;
		goto ret_close;
	}

ret_close:
	close(fd);
	return status;
}

void gpio_oe(unsigned char bank, unsigned char bit)
{
	gpio[bank]->OE &= ~(1 << (bit));
}

void gpio_set(unsigned char bank, unsigned char bit, bool state)
{
	if (state) {
		gpio[bank]->SETDATAOUT |= 1 << bit;
	} else {
		gpio[bank]->CLEARDATAOUT = 1 << bit;
	}
}


