
#include "wiring.h"

#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <linux/spi/spidev.h>

#include "gpio.h"

static int fd = -1;

bool
wiring_init(const char *device)
{
	int speed = 4000000;
	uint8_t mode = SPI_MODE_0;

	fd = open(device, O_RDWR);
	if (fd < 0) {
		printf("Failed to open the %s\n", device);
		return 0;
	}

	if (ioctl(fd, SPI_IOC_WR_MODE, &mode)<0)   {
		perror("can't set spi mode");
		goto exit;
	}

	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)<0) {
		perror("can't set max speed hz");
		goto exit;
	}

	if (gpio_init() != 0) {
		goto exit;
	}

	gpio_oe(WIRING_NRF_PROG_PIN / 32, WIRING_NRF_PROG_PIN % 32);
	gpio_oe(WIRING_NRF_RESET_PIN / 32, WIRING_NRF_RESET_PIN % 32);

	return 1;

exit:
	close(fd);
	return 0;
}

uint8_t
wiring_write_then_read(uint8_t* out, uint16_t out_len, 
	               uint8_t* in, uint16_t in_len)
{
	struct spi_ioc_transfer xfer[2];

	memset(xfer, 0, sizeof(xfer));

	xfer[0].tx_buf = (unsigned long)out;
	xfer[0].len = out_len;
	xfer[0].bits_per_word = 8;

	xfer[1].rx_buf = (unsigned long)in;
	xfer[1].len = in_len;
	xfer[1].bits_per_word = 8;

	int status = ioctl(fd, SPI_IOC_MESSAGE(in_len > 0 ? 2 : 1), &xfer[0]);
	if (status < 0) {
		perror("SPI_IOC_MESSAGE");
		return 0;
	}

	return in_len + out_len;
}

void
wiring_set_gpio_value(uint8_t pin, uint8_t state)
{
	gpio_set(pin / 32, pin % 32, state);
}

void
wiring_destroy(void)
{
	close(fd);
	gpio_exit();
}
